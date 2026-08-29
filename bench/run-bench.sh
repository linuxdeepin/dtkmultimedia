#!/bin/bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
#
# 一键执行脚本 —— 自动检测物理核数，运行 50 样本 × 10 轮（warmup 3）
# 调用 tablerecognizerBench，输出原始 JSON 到 bench/results/
#
# 用法:
#   ./run-bench.sh                     # 使用默认参数（10 轮, warmup 3）
#   ./run-bench.sh --rounds 5          # 自定义轮数
#   ./run-bench.sh --threads 8         # 自定义线程数
#   ./run-bench.sh --warmup 5          # 自定义预热次数
#   ./run-bench.sh --samples-dir /path # 自定义样本目录

set -euo pipefail

BENCH_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR="$(cd "$BENCH_DIR/.." && pwd)"
BENCH_BIN="$REPO_DIR/build/examples/tablerecognizerBench/tablerecognizerBench"
SAMPLES_DIR="$BENCH_DIR/samples"
RESULTS_DIR="$BENCH_DIR/results"

# 默认参数
ROUNDS=10
WARMUP=3
THREADS=""
SAMPLES_DIR_CUSTOM=""

# 颜色
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
NC='\033[0m'
ok()   { echo -e "${GREEN}[OK]${NC}   $*"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $*"; }
fail() { echo -e "${RED}[FAIL]${NC} $*"; }

# ---- 解析参数 ----
while [ $# -gt 0 ]; do
    case "$1" in
        --rounds)    ROUNDS="$2"; shift 2 ;;
        --warmup)    WARMUP="$2"; shift 2 ;;
        --threads)   THREADS="$2"; shift 2 ;;
        --samples-dir) SAMPLES_DIR_CUSTOM="$2"; shift 2 ;;
        --help|-h)
            echo "用法: $0 [选项]"
            echo "  --rounds N         运行轮数（默认 10）"
            echo "  --warmup N         预热次数（默认 3）"
            echo "  --threads N        ORT 线程数（默认物理核数）"
            echo "  --samples-dir PATH 样本图片目录（默认 bench/samples/）"
            exit 0 ;;
        *) fail "未知参数: $1"; exit 1 ;;
    esac
done

if [ -n "$SAMPLES_DIR_CUSTOM" ]; then
    SAMPLES_DIR="$SAMPLES_DIR_CUSTOM"
fi

# ---- 前置检查 ----
echo "=========================================="
echo "  表格识别基准测试 - 一键执行"
echo "=========================================="
echo ""

# 架构
ARCH="$(uname -m)"
echo "架构: $ARCH"

# 物理核数
if [ -n "$THREADS" ]; then
    CORES="$THREADS"
else
    if command -v nproc &>/dev/null; then
        CORES="$(nproc)"
    else
        CORES="$(grep -c ^processor /proc/cpuinfo 2>/dev/null || echo 1)"
    fi
fi
echo "线程数: $CORES"
echo "轮数: $ROUNDS"
echo "预热: $WARMUP"
echo ""

# 检查 bench 二进制
if [ ! -x "$BENCH_BIN" ]; then
    fail "tablerecognizerBench 未编译: $BENCH_BIN"
    echo "请先编译:"
    echo "  cd $REPO_DIR"
    echo "  mkdir -p build && cd build"
    echo "  cmake .. -DCMAKE_BUILD_TYPE=Release"
    echo "  make -j$CORES tablerecognizerbench"
    exit 1
fi
ok "bench 二进制: $BENCH_BIN"

# 准备样本
if [ ! -d "$SAMPLES_DIR" ] || [ "$(find "$SAMPLES_DIR" -maxdepth 1 \( -name '*.png' -o -name '*.jpg' -o -name '*.jpeg' -o -name '*.bmp' -o -name '*.tiff' -o -name '*.webp' \) 2>/dev/null | wc -l)" -eq 0 ]; then
    warn "样本目录为空，尝试自动准备..."
    bash "$BENCH_DIR/prepare-samples.sh" || {
        fail "样本准备失败，请手动准备样本后重试"
        exit 1
    }
fi

SAMPLE_COUNT="$(find "$SAMPLES_DIR" -maxdepth 1 \( -name '*.png' -o -name '*.jpg' -o -name '*.jpeg' -o -name '*.bmp' -o -name '*.tiff' -o -name '*.webp' \) | wc -l)"
ok "样本目录: $SAMPLES_DIR ($SAMPLE_COUNT 张图片)"
echo ""

# ---- 准备结果目录 ----
mkdir -p "$RESULTS_DIR"
TIMESTAMP="$(date +%Y%m%d_%H%M%S)"
RUN_PREFIX="$RESULTS_DIR/run"

# 清理旧结果
rm -f "$RUN_PREFIX"-*.json "$RESULTS_DIR/meta.json" 2>/dev/null || true

# 写入元数据
cat > "$RESULTS_DIR/meta.json" <<EOF
{
  "architecture": "$ARCH",
  "timestamp": "$TIMESTAMP",
  "timestampIso": "$(date -Iseconds)",
  "threads": $CORES,
  "rounds": $ROUNDS,
  "warmup": $WARMUP,
  "sampleCount": $SAMPLE_COUNT,
  "benchBinary": "$BENCH_BIN",
  "samplesDir": "$SAMPLES_DIR"
}
EOF
ok "元数据已写入: $RESULTS_DIR/meta.json"

# ---- 执行基准测试 ----
echo ""
echo "开始基准测试: $ROUNDS 轮 × $SAMPLE_COUNT 样本 (warmup=$WARMUP, threads=$CORES)"
echo "------------------------------------------"

TOTAL_SUCCESS=0
for round in $(seq 1 "$ROUNDS"); do
    OUTPUT_FILE="$RUN_PREFIX-$(printf '%02d' "$round").json"
    echo ""
    echo "[轮次 $round/$ROUNDS] 运行中..."

    "$BENCH_BIN" \
        --threads "$CORES" \
        --warmup "$WARMUP" \
        --format json \
        --output "$OUTPUT_FILE" \
        "$SAMPLES_DIR" 2>&1 | tail -5

    if [ ! -f "$OUTPUT_FILE" ]; then
        fail "轮次 $round 未生成输出文件"
        exit 1
    fi

    # 提取成功数
    SUCCESS_COUNT="$(python3 -c "
import json, sys
with open('$OUTPUT_FILE') as f:
    d = json.load(f)
print(d.get('successCount', 0))
" 2>/dev/null || echo 0)"

    ok "轮次 $round 完成: $SUCCESS_COUNT/$SAMPLE_COUNT 成功 -> $OUTPUT_FILE"
done

echo ""
echo "------------------------------------------"
ok "全部 $ROUNDS 轮基准测试完成"
echo ""
echo "原始 JSON 结果: $RESULTS_DIR/run-*.json"
echo "元数据: $RESULTS_DIR/meta.json"
echo ""
echo "下一步: 生成报告"
echo "  python3 $BENCH_DIR/generate-report.py"
echo "  或: python3 $BENCH_DIR/generate-report.py --results-dir $RESULTS_DIR --output $RESULTS_DIR/report.md"
echo "=========================================="
