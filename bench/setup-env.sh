#!/bin/bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
#
# 环境检查脚本 —— 检测架构、运行时/编译依赖、bench 二进制
# 支持 x86_64 / aarch64 / loongarch64 三架构
# 用法: ./setup-env.sh

set -euo pipefail

BENCH_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR="$(cd "$BENCH_DIR/.." && pwd)"
BENCH_BIN="$REPO_DIR/build/examples/tablerecognizerBench/tablerecognizerBench"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'

ok()   { echo -e "${GREEN}[OK]${NC}   $*"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $*"; }
fail() { echo -e "${RED}[FAIL]${NC} $*"; }

echo "=========================================="
echo "  表格识别基准测试 - 环境检查"
echo "=========================================="
echo ""

# ---- 1. 架构检测 ----
echo "--- 1. CPU 架构 ---"
ARCH="$(uname -m)"
case "$ARCH" in
    x86_64)         echo "架构: x86_64 (Intel/AMD)" ;;
    aarch64)        echo "架构: aarch64 (ARM 64-bit)" ;;
    loongarch64)    echo "架构: loongarch64 (LoongArch 64-bit)" ;;
    *)              warn "未知架构: $ARCH（脚本未验证，可尝试继续）" ;;
esac

# 物理核数
if command -v nproc &>/dev/null; then
    CORES="$(nproc)"
else
    CORES="$(grep -c ^processor /proc/cpuinfo 2>/dev/null || echo 1)"
fi
echo "物理核数: $CORES"
echo ""

# ---- 2. 运行时依赖检查 ----
echo "--- 2. 运行时依赖 ---"
check_lib() {
    local lib="$1"
    local desc="$2"
    if ldconfig -p 2>/dev/null | grep -q "$lib"; then
        ok "$desc 已安装"
        return 0
    else
        fail "$desc 未找到 ($lib)"
        return 1
    fi
}

RT_MISSING=0
check_lib "libonnxruntime"  "ONNX Runtime (libonnxruntime)"   || RT_MISSING=1
check_lib "libdtk6ocr"      "DTK6 OCR (libdtk6ocr)"           || RT_MISSING=1
check_lib "libxlsxwriter"   "xlsxwriter (libxlsxwriter)"      || RT_MISSING=1
check_lib "libopencv"       "OpenCV (libopencv)"              || RT_MISSING=1

if [ "$RT_MISSING" -eq 1 ]; then
    echo ""
    echo "缺失运行时依赖，请安装："
    echo "  # UOS / Debian 系："
    echo "  sudo apt install libonnxruntime-dev libdtk6ocr-dev libxlsxwriter-dev libopencv-dev"
    echo ""
    echo "  # 若 UOS 软件源中缺少 libonnxruntime，可从以下途径获取："
    echo "  # - ONNX Runtime Releases: https://github.com/microsoft/onnxruntime/releases"
    echo "  # - 下载对应架构的 .deb 包后: sudo dpkg -i onnxruntime-*.deb"
fi
echo ""

# ---- 3. 编译依赖检查 ----
echo "--- 3. 编译依赖 ---"
BUILD_MISSING=0
check_cmd() {
    local cmd="$1"
    local desc="$2"
    if command -v "$cmd" &>/dev/null; then
        ok "$desc 已安装 ($($(command -v "$cmd") --version 2>&1 | head -1))"
        return 0
    else
        fail "$desc 未找到 ($cmd)"
        return 1
    fi
}

check_cmd cmake "CMake"         || BUILD_MISSING=1
check_cmd g++   "GCC/g++"       || BUILD_MISSING=1
check_cmd make  "Make"          || BUILD_MISSING=1

# Qt6 检查
if pkg-config --exists Qt6Core 2>/dev/null; then
    ok "Qt6 已安装 ($(pkg-config --modversion Qt6Core))"
else
    fail "Qt6 未找到"
    BUILD_MISSING=1
fi

if [ "$BUILD_MISSING" -eq 1 ]; then
    echo ""
    echo "缺失编译依赖，请安装："
    echo "  # UOS / Debian 系："
    echo "  sudo apt install cmake g++ make qt6-base-dev"
fi
echo ""

# ---- 4. bench 二进制检查 ----
echo "--- 4. tablerecognizerBench 二进制 ---"
if [ -x "$BENCH_BIN" ]; then
    ok "已编译: $BENCH_BIN"
else
    warn "tablerecognizerBench 未编译"
    echo ""
    echo "请执行以下步骤编译："
    echo "  cd $REPO_DIR"
    echo "  mkdir -p build && cd build"
    echo "  cmake .. -DCMAKE_BUILD_TYPE=Release"
    echo "  make -j$CORES tablerecognizerbench"
    echo ""
    echo "编译完成后重新运行: $BENCH_DIR/setup-env.sh"
fi
echo ""

# ---- 5. 样本数据检查 ----
echo "--- 5. 样本数据 ---"
SAMPLES_DIR="$BENCH_DIR/samples"
SAMPLES_TAR="$BENCH_DIR/samples.tar.gz"

if [ -d "$SAMPLES_DIR" ] && [ "$(find "$SAMPLES_DIR" -maxdepth 1 \( -name '*.png' -o -name '*.jpg' -o -name '*.jpeg' \) | wc -l)" -gt 0 ]; then
    SAMPLE_COUNT="$(find "$SAMPLES_DIR" -maxdepth 1 \( -name '*.png' -o -name '*.jpg' -o -name '*.jpeg' \) | wc -l)"
    ok "样本目录存在: $SAMPLES_DIR ($SAMPLE_COUNT 张图片)"
elif [ -f "$SAMPLES_TAR" ]; then
    ok "样本压缩包存在: $SAMPLES_TAR"
    echo "  运行 run-bench.sh 时将自动解包"
else
    warn "样本数据未找到"
    echo ""
    echo "请通过以下方式之一准备样本："
    echo "  1. 从 x86 机器复制: 将 bench/samples.tar.gz 传到本机 bench/ 目录"
    echo "  2. 手动放置: 将图片放到 $SAMPLES_DIR/"
    echo "  3. 生成合成样本(仅测试流程用): bash $BENCH_DIR/prepare-samples.sh --synthetic"
    echo ""
    echo "在 x86 机器上打包样本: bash $BENCH_DIR/prepare-samples.sh --pack <图片目录>"
fi
echo ""

# ---- 汇总 ----
echo "=========================================="
if [ "$RT_MISSING" -eq 0 ] && [ "$BUILD_MISSING" -eq 0 ] && [ -x "$BENCH_BIN" ]; then
    ok "环境检查通过，可以运行基准测试"
    echo "  下一步: $BENCH_DIR/run-bench.sh"
else
    warn "环境检查未完全通过，请按上述提示修复后重试"
fi
echo "=========================================="
