#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: LGPL-3.0-or-later
#
# Python 参考对齐脚本：用 onnxruntime 加载 SLANet_plus.onnx 复现与 C++ 相同的
# 结构检测流程（预处理 488 + ImageNet mean/std + argmax + token 解码），并将
# token id 序列与 cell 集合与 C++ 输出逐张比对，用于验证 C++ 移植无回退。
#
# 依赖：numpy, onnxruntime, Pillow（均非当前环境预装；需 pip 安装，见 README）。
# 运行：python3 slanet_ref_align.py <image_dir> [--model path/to/SLANet_plus.onnx]
#
# 50-token 词表与 C++ TableStructureDetector::vocab() 完全一致。

import argparse
import os
import sys
import contextlib
import numpy as np

# 与 C++ TableStructureDetector 一致的 50 token 词表。
VOCAB = [
    "<sos>", "<thead>", "</thead>", "<tbody>", "</tbody>", "<tr>", "</tr>", "<td>",
    ">", "</td>",
    ' colspan="2"',  ' colspan="3"',  ' colspan="4"',  ' colspan="5"',
    ' colspan="6"',  ' colspan="7"',  ' colspan="8"',  ' colspan="9"',
    ' colspan="10"', ' colspan="11"', ' colspan="12"', ' colspan="13"',
    ' colspan="14"', ' colspan="15"', ' colspan="16"', ' colspan="17"',
    ' colspan="18"', ' colspan="19"', ' colspan="20"',
    ' rowspan="2"',  ' rowspan="3"',  ' rowspan="4"',  ' rowspan="5"',
    ' rowspan="6"',  ' rowspan="7"',  ' rowspan="8"',  ' rowspan="9"',
    ' rowspan="10"', ' rowspan="11"', ' rowspan="12"', ' rowspan="13"',
    ' rowspan="14"', ' rowspan="15"', ' rowspan="16"', ' rowspan="17"',
    ' rowspan="18"', ' rowspan="19"', ' rowspan="20"',
    "<td></td>", "<eos>",
]
EOS_ID = 49
INPUT_SIZE = 488
MEAN = np.array([0.485, 0.456, 0.406], dtype=np.float32)
STD = np.array([0.229, 0.224, 0.225], dtype=np.float32)


@contextlib.contextmanager
def suppress_stderr():
    """抑制 ORT 的 shape 告警（不影响结果）。"""
    devnull = open(os.devnull, "w")
    old = sys.stderr
    sys.stderr = devnull
    try:
        yield
    finally:
        sys.stderr = old
        devnull.close()


def preprocess(img_path):
    """QImage 等价预处理：resize 到 488，RGB888，归一化 NCHW[1,3,488,488]。"""
    from PIL import Image
    img = Image.open(img_path).convert("RGB").resize(
        (INPUT_SIZE, INPUT_SIZE), Image.BILINEAR)
    arr = np.asarray(img, dtype=np.float32) / 255.0
    arr = (arr - MEAN) / STD
    arr = np.transpose(arr, (2, 0, 1))  # HWC -> CHW
    return np.expand_dims(arr, 0).astype(np.float32)


def detect(session, img_path):
    """运行 SLANet_plus 推理并解码结构 token + bbox，与 C++ 逻辑对齐。"""
    input_tensor = preprocess(img_path)
    in_name = session.get_inputs()[0].name
    with suppress_stderr():
        outputs = session.run(None, {in_name: input_tensor})

    # 与 C++ 相同：按输出最后一维大小判断哪个是结构(50)、哪个是 bbox(8)。
    struct_idx, bbox_idx = 0, 1
    if len(outputs) >= 2 and outputs[0].shape[-1] < outputs[1].shape[-1]:
        struct_idx, bbox_idx = 1, 0
    struct_out = outputs[struct_idx]  # shape [1, T, 50]
    bbox_out = outputs[bbox_idx]      # shape [1, T, 8]

    V = struct_out.shape[-1]
    struct2d = struct_out.reshape(-1, V)
    T = struct2d.shape[0]
    ids = [int(np.argmax(struct2d[t])) for t in range(T)]

    # <eos> 截断（与 C++ 一致）
    if EOS_ID in ids:
        ids = ids[:ids.index(EOS_ID) + 1]
    return ids, bbox_out, T


def decode_structure(struct_ids, bboxes, image_size, bbox_stride=8,
                     bbox_per_time_step=True):
    """与 C++ TableStructureDetector::decodeStructure 等价的解码逻辑。"""
    cells = []
    row, col = 0, 0
    span_rows = {}
    cell_index = 0
    in_td = False
    pending_colspan = 1
    pending_rowspan = 1

    def advance_row():
        nonlocal row, col
        for c in list(span_rows.keys()):
            left = span_rows[c] - 1
            if left <= 0:
                del span_rows[c]
            else:
                span_rows[c] = left
        row += 1
        col = 0

    def emit_cell(rowspan, colspan, time_step):
        nonlocal col, cell_index
        while span_rows.get(col, 0) > 0:
            col += 1
        start_col = col
        bbox_idx_val = time_step if bbox_per_time_step else cell_index
        cell = {"row": row, "col": start_col, "rowspan": rowspan,
                "colspan": colspan, "bbox": [0.0, 0.0, 0.0, 0.0]}
        bi = bbox_idx_val * bbox_stride
        if bi + bbox_stride <= len(bboxes):
            if bbox_stride >= 8:
                xs = [bboxes[bi], bboxes[bi + 2], bboxes[bi + 4], bboxes[bi + 6]]
                ys = [bboxes[bi + 1], bboxes[bi + 3], bboxes[bi + 5], bboxes[bi + 7]]
                cell["bbox"] = [min(xs), min(ys), max(xs), max(ys)]
            else:
                cell["bbox"] = [bboxes[bi], bboxes[bi + 1],
                                bboxes[bi + 2], bboxes[bi + 3]]
        cells.append(cell)
        for c in range(start_col, start_col + colspan):
            if rowspan > 1:
                span_rows[c] = rowspan
            else:
                span_rows.pop(c, None)
        col += colspan
        cell_index += 1

    for i, sid in enumerate(struct_ids):
        if sid < 0 or sid >= len(VOCAB):
            continue
        tok = VOCAB[sid]
        if in_td:
            if tok == ">" or tok == "</td>":
                emit_cell(pending_rowspan, pending_colspan, i)
                pending_colspan = 1
                pending_rowspan = 1
                in_td = False
            elif 10 <= sid <= 28:
                pending_colspan = sid - 8
            elif 29 <= sid <= 47:
                pending_rowspan = sid - 27
            continue
        if tok == "<tr>":
            pass
        elif tok == "</tr>":
            advance_row()
        elif tok == "<td":
            in_td = True
            pending_colspan = 1
            pending_rowspan = 1
        elif tok == "<td></td>":
            emit_cell(1, 1, i)
        # 其它 token（<sos>/<eos>/<thead>/</thead>/<tbody>/</tbody>/</td>）忽略

    if in_td:  # 容错
        emit_cell(pending_rowspan, pending_colspan, len(struct_ids) - 1)

    # 缩放 bbox 到原图坐标
    for c in cells:
        c["bbox"][0] *= image_size[0]
        c["bbox"][1] *= image_size[1]
        c["bbox"][2] *= image_size[0]
        c["bbox"][3] *= image_size[1]
    return cells


def main():
    ap = argparse.ArgumentParser(description="SLANet_plus Python reference alignment")
    ap.add_argument("image_dir", nargs="?", help="目录含待对齐图片")
    ap.add_argument("--model", default="src/tablerecognizer/models/SLANet_plus.onnx")
    args = ap.parse_args()

    try:
        import onnxruntime as ort
    except ImportError:
        print("ERROR: onnxruntime not installed. pip install onnxruntime numpy Pillow",
              file=sys.stderr)
        sys.exit(2)

    sess = ort.InferenceSession(args.model)
    if not args.image_dir:
        print("no image_dir given; nothing to do", file=sys.stderr)
        sys.exit(0)

    for name in sorted(os.listdir(args.image_dir)):
        if not name.lower().endswith((".png", ".jpg", ".jpeg")):
            continue
        path = os.path.join(args.image_dir, name)
        from PIL import Image
        w, h = Image.open(path).size
        ids, bbox, T = detect(sess, path)
        cells = decode_structure(ids, bbox.flatten(), (w, h))
        print(f"{name}: T={T} token_ids={ids}")
        print(f"  -> {len(cells)} cells:")
        for c in cells:
            print(f"    r={c['row']} c={c['col']} rs={c['rowspan']} "
                  f"cs={c['colspan']} bbox=({c['bbox'][0]:.1f},{c['bbox'][1]:.1f},"
                  f"{c['bbox'][2]:.1f},{c['bbox'][3]:.1f})")


if __name__ == "__main__":
    main()
