# 多架构一键基准测试套件

在 x86_64 / aarch64 / loongarch64 机器上独立运行表格识别性能基准测试，无需智能体环境。

## 文件说明

| 文件 | 功能 |
| --- | --- |
| `setup-env.sh` | 环境检查：检测架构、运行时/编译依赖、bench 二进制、样本数据 |
| `prepare-samples.sh` | 样本准备：解包 `samples.tar.gz`、打包样本、生成合成样本 |
| `run-bench.sh` | 一键执行：自动检测物理核数，50 样本 × 10 轮 + warmup 3，输出原始 JSON |
| `generate-report.py` | 报告生成：从 JSON 计算 min/max/mean/median/stddev/P95，输出 Markdown 报告 |
| `README.md` | 本说明文件 |

## 完整使用流程

### 1. Clone 代码

```bash
git clone -b phase0/v634-ort-table-structure https://github.com/dengzhongyuan365-dev/dtkmultimedia.git
cd dtkmultimedia/bench
```

### 2. 环境检查

```bash
./setup-env.sh
```

按提示安装缺失依赖。全部 `[OK]` 后继续。

#### 依赖安装（三架构通用）

```bash
# UOS / Debian 系
sudo apt install cmake g++ make qt6-base-dev \
    libonnxruntime-dev libdtk6ocr-dev libxlsxwriter-dev libopencv-dev
```

> **注意**：若 UOS 软件源缺少 `libonnxruntime`，从 [ONNX Runtime Releases](https://github.com/microsoft/onnxruntime/releases) 下载对应架构的包安装。

### 3. 编译

```bash
cd ..
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc) tablerecognizerbench
cd ../bench
```

### 4. 准备样本数据

**方式一：从 x86 机器打包传输（推荐，确保结果可比）**

在 x86 机器上：
```bash
cd dtkmultimedia/bench
./prepare-samples.sh --pack /path/to/samples   # 打包为 samples.tar.gz
```

将 `samples.tar.gz` 传到目标机器的 `bench/` 目录。

在目标机器上：
```bash
./prepare-samples.sh     # 自动解包到 samples/
```

**方式二：直接放置图片**

将 50 张表格图片放到 `bench/samples/` 目录（支持 png/jpg/jpeg/bmp/tiff/webp）。

**方式三：生成合成样本（仅测试流程，非真实数据）**

```bash
./prepare-samples.sh --synthetic
```

> 合成样本仅用于验证测试流程是否通畅，性能数据不可与 x86 基准对比。

### 5. 运行基准测试

```bash
./run-bench.sh
```

默认参数：10 轮 × 50 样本，warmup 3，线程数取物理核数。

自定义参数：
```bash
./run-bench.sh --rounds 5 --threads 8 --warmup 5
```

结果输出到 `bench/results/`：
- `run-01.json` ~ `run-10.json` — 每轮原始 JSON
- `meta.json` — 架构、时间戳、参数等元数据

### 6. 生成报告

```bash
python3 generate-report.py --output results/report.md
```

报告包含：
- 验证环境（架构、SHA、样本数、轮数等）
- 指标说明（structureMs / ocrMs / totalMs）
- 跨轮次统计汇总表（基于 per-run Median 和 per-run Mean 双口径）
- 逐轮明细表
- 验证结论

可指定 Base SHA 和 Patch 版本：
```bash
python3 generate-report.py --base-sha 59c3aee --patch-version v1 --output results/report.md
```

## 常见问题排查

### Q: `tablerecognizerBench` 未编译

```bash
cd /path/to/dtkmultimedia
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc) tablerecognizerbench
```

bench 脚本默认查找 `../build/examples/tablerecognizerBench/tablerecognizerBench`。

### Q: 模型文件缺失

表格识别依赖 ONNX 模型 `SLANet_plus.onnx`，位于 `src/tablerecognizer/models/`。编译安装库后会自动部署。如运行时报模型加载错误，检查：

```bash
# 查找模型文件
find /usr -name "SLANet_plus.onnx" 2>/dev/null
# 或从源码复制
cp src/tablerecognizer/models/SLANet_plus.onnx /usr/lib/  # 或对应库搜索路径
```

### Q: libonnxruntime 找不到

```bash
# 检查是否安装
ldconfig -p | grep onnxruntime

# 若缺失，从 GitHub 下载对应架构包
# aarch64: onnxruntime-linux-aarch64-*.tgz
# loongarch64: 需从 UOS 软件源或社区构建获取
```

### Q: 样本图片无法加载

确认图片格式为 png/jpg/jpeg/bmp/tiff/webp，且文件未损坏：
```bash
file samples/*.png | head -5
```

### Q: 线程数不正确

`run-bench.sh` 默认使用 `nproc` 检测物理核数。如需手动指定：
```bash
./run-bench.sh --threads 4
```

### Q: 报告中架构信息不正确

架构从 `meta.json` 读取（由 `run-bench.sh` 自动写入）。如需手动指定，编辑 `results/meta.json` 中的 `architecture` 字段。

## 测试参数基准

| 参数 | 值 |
| --- | --- |
| 样本数 | 50 张/轮 |
| 轮数 | 10 轮 |
| Warmup | 3 次 |
| 线程数 | 物理核数（`nproc`） |
| 输出格式 | JSON |

## 样本数据打包方式

- **打包**：`./prepare-samples.sh --pack <图片目录>` → 生成 `samples.tar.gz`
- **解包**：`./prepare-samples.sh` → 解包到 `samples/`
- **检查**：`./prepare-samples.sh --check`
- **合成**：`./prepare-samples.sh --synthetic` → 生成 50 张合成表格图片

如 `samples.tar.gz` 超过 50MB，建议上传到 Issue 附件，在此注明下载方式。
