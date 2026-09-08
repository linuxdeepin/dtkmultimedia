#!/bin/bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
#
# 样本数据准备脚本
# 用法:
#   ./prepare-samples.sh                    # 解包 samples.tar.gz（若存在）
#   ./prepare-samples.sh --pack <图片目录>  # 在 x86 机器上打包样本
#   ./prepare-samples.sh --synthetic        # 生成 50 张合成表格图片（仅测试流程）
#   ./prepare-samples.sh --check            # 检查样本状态

set -euo pipefail

BENCH_DIR="$(cd "$(dirname "$0")" && pwd)"
SAMPLES_DIR="$BENCH_DIR/samples"
SAMPLES_TAR="$BENCH_DIR/samples.tar.gz"
DEFAULT_SAMPLE_COUNT=50

# 颜色
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
NC='\033[0m'
ok()   { echo -e "${GREEN}[OK]${NC}   $*"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $*"; }
fail() { echo -e "${RED}[FAIL]${NC} $*"; }

count_images() {
    find "$1" -maxdepth 1 \( -name '*.png' -o -name '*.jpg' -o -name '*.jpeg' -o -name '*.bmp' -o -name '*.tiff' -o -name '*.webp' \) 2>/dev/null | wc -l
}

# ---- --check 模式 ----
if [ "${1:-}" = "--check" ]; then
    if [ -d "$SAMPLES_DIR" ] && [ "$(count_images "$SAMPLES_DIR")" -gt 0 ]; then
        ok "样本就绪: $(count_images "$SAMPLES_DIR") 张图片 in $SAMPLES_DIR"
        exit 0
    elif [ -f "$SAMPLES_TAR" ]; then
        ok "压缩包就绪: $SAMPLES_TAR"
        exit 0
    else
        warn "样本未准备"
        exit 1
    fi
fi

# ---- --pack 模式（在 x86 机器上打包） ----
if [ "${1:-}" = "--pack" ]; then
    SRC_DIR="${2:-}"
    if [ -z "$SRC_DIR" ] || [ ! -d "$SRC_DIR" ]; then
        fail "请指定图片目录: $0 --pack <图片目录>"
        exit 1
    fi
    COUNT="$(count_images "$SRC_DIR")"
    if [ "$COUNT" -eq 0 ]; then
        fail "目录中未找到图片: $SRC_DIR"
        exit 1
    fi
    echo "打包 $COUNT 张图片 from $SRC_DIR ..."
    tar -czf "$SAMPLES_TAR" -C "$SRC_DIR" .
    SIZE="$(du -h "$SAMPLES_TAR" | cut -f1)"
    ok "已打包: $SAMPLES_TAR ($SIZE, $COUNT 张图片)"
    echo ""
    if [ "$(echo "$SAMPLES_TAR" | stat --format=%s 2>/dev/null || stat -f%z "$SAMPLES_TAR" 2>/dev/null)" -gt 52428800 ]; then
        echo "压缩包超过 50MB，建议上传到 Issue 附件后删除本地副本。"
    fi
    exit 0
fi

# ---- --synthetic 模式（生成合成图片） ----
if [ "${1:-}" = "--synthetic" ]; then
    mkdir -p "$SAMPLES_DIR"
    echo "生成 $DEFAULT_SAMPLE_COUNT 张合成表格图片（仅用于测试流程，非真实数据）..."
    python3 - "$SAMPLES_DIR" "$DEFAULT_SAMPLE_COUNT" <<'PYEOF'
import os, sys, struct, zlib, random

outdir = sys.argv[1]
count = int(sys.argv[2])

def make_png(path, width, height, pixels):
    """生成最小化 PNG 文件（仅依赖标准库）。"""
    def chunk(ctype, data):
        c = ctype + data
        return struct.pack('>I', len(data)) + c + struct.pack('>I', zlib.crc32(c) & 0xffffffff)
    # PNG signature
    sig = b'\x89PNG\r\n\x1a\n'
    # IHDR
    ihdr = struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0)  # 8-bit RGB
    # IDAT - raw pixel data with filter byte per row
    raw = b''
    for y in range(height):
        raw += b'\x00'  # filter: None
        raw += pixels[y]
    idat = zlib.compress(raw)
    # IEND
    with open(path, 'wb') as f:
        f.write(sig)
        f.write(chunk(b'IHDR', ihdr))
        f.write(chunk(b'IDAT', idat))
        f.write(chunk(b'IEND', b''))

random.seed(42)
for i in range(count):
    w, h = random.randint(400, 800), random.randint(300, 600)
    # 生成简单的表格线条图片（白底 + 黑线网格）
    pixels = []
    for y in range(h):
        row = bytearray()
        for x in range(w):
            # 表格边框和网格线
            is_border = (x < 3 or x >= w - 3 or y < 3 or y >= h - 3)
            is_grid = False
            if not is_border:
                # 水平线
                for gy in range(40, h - 20, random.randint(40, 80)):
                    if abs(y - gy) < 2:
                        is_grid = True
                        break
                # 垂直线
                if not is_grid:
                    for gx in range(60, w - 20, random.randint(60, 120)):
                        if abs(x - gx) < 2:
                            is_grid = True
                            break
            if is_border or is_grid:
                row.extend([20, 20, 20])  # 深灰线条
            else:
                row.extend([255, 255, 255])  # 白色背景
        pixels.append(bytes(row))
    fname = f"synthetic_{i+1:03d}.png"
    make_png(os.path.join(outdir, fname), w, h, pixels)

print(f"已生成 {count} 张合成图片到 {outdir}")
PYEOF
    ok "合成样本已生成: $(count_images "$SAMPLES_DIR") 张图片 in $SAMPLES_DIR"
    warn "合成样本仅用于测试流程，非真实表格数据，性能数据不可与 x86 基准对比"
    exit 0
fi

# ---- 默认模式：解包 samples.tar.gz ----
if [ -d "$SAMPLES_DIR" ] && [ "$(count_images "$SAMPLES_DIR")" -gt 0 ]; then
    ok "样本已就绪: $(count_images "$SAMPLES_DIR") 张图片"
    exit 0
fi

if [ ! -f "$SAMPLES_TAR" ]; then
    fail "未找到 $SAMPLES_TAR"
    echo ""
    echo "请通过以下方式准备样本："
    echo "  1. 从 x86 机器复制 samples.tar.gz 到 $BENCH_DIR/"
    echo "  2. 在 x86 机器上打包: $0 --pack <图片目录>"
    echo "  3. 生成合成样本: $0 --synthetic"
    exit 1
fi

echo "解包 $SAMPLES_TAR ..."
mkdir -p "$SAMPLES_DIR"
tar -xzf "$SAMPLES_TAR" -C "$SAMPLES_DIR"
ok "已解包: $(count_images "$SAMPLES_DIR") 张图片 to $SAMPLES_DIR"
