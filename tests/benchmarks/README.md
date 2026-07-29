# Phase 0 TEDS 基准评测 harness

本目录提供 Phase 0 表格结构检测的客观基准评测工具，完全本地离线运行。

## 组件

- `teds_benchmark.cpp` — TEDS 基准 harness 程序。内置一组确定性合成样本
  （有线/无线/含 colspan 的表格图），用 SLANet_plus.onnx 检测结构，将检测结果
  渲染为结构 HTML，与 ground-truth HTML 计算归一化树编辑距离（TEDS），
  记录逐图 TEDS 与单图耗时，并输出 JSON 报告。支持可选的真实截图样本目录
  `samples/real/`（每张图配同名 `.html` 作为结构 GT）。

- `samples/real/` — 真实截图样本放置目录（可选）。有线图以 `wired_` 前缀，
  无线图以 `wireless_` 前缀命名，每张图配同名 `.html` 描述结构 ground-truth。

## 构建

在项目根目录配置并构建（需 onnxruntime，见 `src/tablerecognizer/CMakeLists.txt` 分架构查找）：

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
          -DONNXRUNTIME_ROOT_DIR=/path/to/ortroot
    cmake --build build --target teds_benchmark

## 运行

需在仓库根目录运行（模型与报告路径相对仓库根）：

    LD_LIBRARY_PATH=/path/to/ort/lib:./build/src/tablerecognizer \
      ./build/tests/benchmarks/teds_benchmark

输出逐图结果与汇总到 `tests/benchmarks/teds_baseline_report.json`。

## Python 参考对齐

`python_ref/slanet_ref_align.py` 用 onnxruntime 复现与 C++ 相同的结构检测流程，
用于验证 C++ 移植无回退。依赖：`onnxruntime numpy Pillow`。

    pip install onnxruntime numpy Pillow
    python3 tests/benchmarks/python_ref/slanet_ref_align.py <image_dir>

## TEDS 实现说明

本 harness 采用标签序列编辑距离作为 TEDS 的近似实现（PubTabNet 使用精确 APTED
树编辑距离）。标签流视为树前序遍历，以 Levenshtein 距离归一化。该近似作为
Phase 0 阶段的客观相对指标足够，后续阶段可替换为精确 APTED。

## Phase 0 基线记录（x86_64，开发架构）

本阶段仅记录基线、不判定阈值（阈值待 PM 基准跑出后钉死）。合成样本基线：

| 分组     | 样本数 | TEDS 均值 | 单图耗时 min/avg/max (ms) |
|----------|--------|-----------|---------------------------|
| 有线     | 4      | ~0.99     | 67 / 69 / 73              |
| 无线     | 3      | ~0.87     | 60 / 61 / 64              |
| 全部     | 7      | ~0.94     | 60 / 66 / 73              |

单图耗时显著优于 v1 PoC 基线 0.648s/图。
