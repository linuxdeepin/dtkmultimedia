#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
#
# 统计报告生成器 —— 从 bench/results/ 下的 JSON 数据生成 Markdown 报告
# 计算 min / max / mean / median / stddev / P95，Median 和 Mean 双口径
# 格式与 x86_64 报告 V-566-性能基准报告.md 一致
#
# 用法:
#   ./generate-report.py                              # 读取 bench/results/，输出到 bench/results/report.md
#   ./generate-report.py --results-dir <dir>          # 指定结果目录
#   ./generate-report.py --output <file>              # 指定输出文件
#   ./generate-report.py --base-sha <sha>             # 指定 Base SHA
#   ./generate-report.py --patch-version <ver>        # 指定 Patch 版本

import argparse
import glob
import json
import math
import os
import platform
import re
import sys
from datetime import datetime

# ---- 统计计算 ----

def percentile(sorted_values, p):
    """计算百分位数（nearest rank 方法）。"""
    if not sorted_values:
        return 0.0
    n = len(sorted_values)
    idx = math.ceil(p / 100.0 * n) - 1
    idx = max(0, min(idx, n - 1))
    return sorted_values[idx]

def compute_cross_stats(values):
    """对一组值计算跨轮次统计量。"""
    if not values:
        return {"min": 0, "max": 0, "mean": 0, "median": 0, "stddev": 0, "p95": 0}
    s = sorted(values)
    n = len(s)
    mean = sum(s) / n
    if n % 2 == 0:
        median = (s[n // 2 - 1] + s[n // 2]) / 2.0
    else:
        median = s[n // 2]
    variance = sum((v - mean) ** 2 for v in s) / n
    stddev = math.sqrt(variance)
    return {
        "min": s[0],
        "max": s[-1],
        "mean": mean,
        "median": median,
        "stddev": stddev,
        "p95": percentile(s, 95),
    }

def fmt_ms(v):
    """格式化毫秒数值，保持两位小数。"""
    if v == int(v):
        return f"{int(v):,}"
    return f"{v:,.2f}"

# ---- 读取结果 ----

def load_results(results_dir):
    """加载所有 run-*.json 文件，返回按轮次排序的列表。"""
    pattern = os.path.join(results_dir, "run-*.json")
    files = sorted(glob.glob(pattern))
    if not files:
        print(f"错误: 未找到结果文件 ({pattern})", file=sys.stderr)
        sys.exit(1)

    runs = []
    for f in files:
        with open(f) as fh:
            data = json.load(fh)
        summary = data.get("summary", {})
        run_info = {
            "file": os.path.basename(f),
            "imageCount": data.get("imageCount", 0),
            "successCount": data.get("successCount", 0),
            "threads": data.get("threads", 0),
            "structureMedian": summary.get("structureMs", {}).get("median", 0),
            "structureMean": summary.get("structureMs", {}).get("mean", 0),
            "ocrMedian": summary.get("ocrMs", {}).get("median", 0),
            "ocrMean": summary.get("ocrMs", {}).get("mean", 0),
            "totalMedian": summary.get("totalMs", {}).get("median", 0),
            "totalMean": summary.get("totalMs", {}).get("mean", 0),
        }
        runs.append(run_info)
    return runs

def load_meta(results_dir):
    """加载 meta.json 元数据。"""
    meta_path = os.path.join(results_dir, "meta.json")
    if os.path.exists(meta_path):
        with open(meta_path) as f:
            return json.load(f)
    return {}

# ---- 报告生成 ----

def generate_report(runs, meta, base_sha="", patch_version=""):
    """生成 Markdown 报告。"""
    arch = meta.get("architecture", platform.machine())
    timestamp = meta.get("timestampIso", datetime.now().isoformat())
    threads = meta.get("threads", runs[0].get("threads", "?") if runs else "?")
    rounds = len(runs)
    sample_count = meta.get("sampleCount", runs[0].get("imageCount", "?") if runs else "?")
    warmup = meta.get("warmup", 3)

    total_success = sum(r["successCount"] for r in runs)
    total_attempts = sum(r["imageCount"] for r in runs)
    success_rate = f"{total_success} / {total_attempts}"
    if total_attempts > 0 and total_success == total_attempts:
        success_rate += f"（100%）"

    # 提取 per-run 统计序列
    metrics = ["structureMs", "ocrMs", "totalMs"]
    calibers = {
        "Median": {"structureMs": [r["structureMedian"] for r in runs],
                    "ocrMs": [r["ocrMedian"] for r in runs],
                    "totalMs": [r["totalMedian"] for r in runs]},
        "Mean":   {"structureMs": [r["structureMean"] for r in runs],
                    "ocrMs": [r["ocrMean"] for r in runs],
                    "totalMs": [r["totalMean"] for r in runs]},
    }

    lines = []
    lines.append("# libdtk6tablerecognizer 表格识别性能基准报告")
    lines.append("")
    lines.append("## 一、验证环境")
    lines.append("")
    lines.append("| 项目 | 值 |")
    lines.append("| --- | --- |")
    lines.append(f"| 架构 | {arch} |")
    lines.append(f"| Base SHA | `{base_sha}` |")
    if patch_version:
        lines.append(f"| Patch 版本 | {patch_version} |")
    lines.append(f"| 样本数 | {sample_count} 张/轮 |")
    lines.append(f"| 轮数 | {rounds} 轮 |")
    lines.append(f"| Warmup 轮数 | {warmup} |")
    lines.append(f"| ORT 线程数 | {threads} |")
    lines.append(f"| 识别成功率 | {success_rate} |")
    lines.append(f"| 测试时间 | {timestamp} |")
    lines.append("")
    lines.append("## 二、指标说明")
    lines.append("")
    lines.append("本次性能基准测试衡量表格识别流程各阶段的耗时（单位：毫秒，ms），共三个指标：")
    lines.append("")
    lines.append("- **structureMs（结构检测耗时）**：表格结构检测模型的推理耗时，即从图片输入到输出表格 HTML 结构标签的耗时。反映结构识别阶段的性能。")
    lines.append("- **ocrMs（OCR 识别耗时）**：表格单元格内文字识别的耗时，即对检测到的单元格区域进行 OCR 文字提取的耗时。反映文字识别阶段的性能。")
    lines.append("- **totalMs（端到端总耗时）**：从图片输入到完整输出表格 HTML（含结构标签与单元格文本）的端到端总耗时。反映整体流程性能。")
    lines.append("")
    lines.append("每个指标在每轮中以两种口径统计：")
    lines.append("")
    lines.append("- **Median（中位数）**：该轮样本耗时的中位数。对极端值不敏感，反映典型样本的性能水平。")
    lines.append("- **Mean（均值）**：该轮样本耗时的算术平均值。受极端值影响，反映整体平均性能水平。")
    lines.append("")
    lines.append(f"跨轮次统计基于 {rounds} 轮的 per-run Median 和 per-run Mean 序列，分别计算 min、max、mean、median、stddev 和 P95，用于衡量多轮运行的稳定性和分布特征。")
    lines.append("")

    # 跨轮次统计表
    metric_labels = {"structureMs": "structureMs", "ocrMs": "ocrMs", "totalMs": "totalMs"}
    for caliber_name, caliber_data in calibers.items():
        lines.append(f"### 基于 per-run {caliber_name} 的统计")
        lines.append("")
        lines.append("| 指标 | Min | Max | Mean | Median | StdDev | P95 |")
        lines.append("| --- | ---: | ---: | ---: | ---: | ---: | ---: |")
        for m in metrics:
            stats = compute_cross_stats(caliber_data[m])
            lines.append(f"| {metric_labels[m]} | {fmt_ms(stats['min'])} | {fmt_ms(stats['max'])} | {fmt_ms(stats['mean'])} | {fmt_ms(stats['median'])} | {fmt_ms(stats['stddev'])} | {fmt_ms(stats['p95'])} |")
        lines.append("")

    # 逐轮明细表
    lines.append("## 四、逐轮明细表")
    lines.append("")
    lines.append("| 轮次 | 成功数 | structureMedian (ms) | ocrMedian (ms) | totalMedian (ms) | structureMean (ms) | ocrMean (ms) | totalMean (ms) |")
    lines.append("| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |")
    for i, r in enumerate(runs):
        lines.append(f"| {i + 1} | {r['successCount']} | {fmt_ms(r['structureMedian'])} | {fmt_ms(r['ocrMedian'])} | {fmt_ms(r['totalMedian'])} | {fmt_ms(r['structureMean'])} | {fmt_ms(r['ocrMean'])} | {fmt_ms(r['totalMean'])} |")
    lines.append("")

    # 验证结论
    lines.append("## 五、验证结论")
    lines.append("")
    median_stats = {m: compute_cross_stats(calibers["Median"][m]) for m in metrics}
    mean_stats = {m: compute_cross_stats(calibers["Mean"][m]) for m in metrics}
    lines.append(f"{rounds} 轮 × {sample_count} 张样本的性能基准测试完成。")
    lines.append("")
    lines.append("**典型性能水平**（基于 per-run Median 的中位数）：")
    lines.append("")
    lines.append(f"- 结构检测（structureMs）中位数 **{fmt_ms(median_stats['structureMs']['median'])} ms**，均值 {fmt_ms(median_stats['structureMs']['mean'])} ms")
    lines.append(f"- OCR 识别（ocrMs）中位数 **{fmt_ms(median_stats['ocrMs']['median'])} ms**，均值 {fmt_ms(median_stats['ocrMs']['mean'])} ms")
    lines.append(f"- 端到端总耗时（totalMs）中位数 **{fmt_ms(median_stats['totalMs']['median'])} ms**，均值 {fmt_ms(median_stats['totalMs']['mean'])} ms")
    lines.append("")

    # 耗时分布特征
    total_median_median = median_stats["totalMs"]["median"]
    struct_median_median = median_stats["structureMs"]["median"]
    ocr_median_median = median_stats["ocrMs"]["median"]
    if total_median_median > 0:
        ocr_pct = ocr_median_median / total_median_median * 100
        struct_pct = struct_median_median / total_median_median * 100
        lines.append("**耗时分布特征**：")
        lines.append("")
        lines.append(f"- OCR 识别是主要耗时环节，占总耗时的约 {ocr_pct:.0f}%，结构检测约 {struct_pct:.0f}%。")
        total_stddev = median_stats["totalMs"]["stddev"]
        total_mean_val = median_stats["totalMs"]["mean"]
        cv = total_stddev / total_mean_val * 100 if total_mean_val > 0 else 0
        lines.append(f"- 跨轮次稳定性：totalMs 的 per-run Median StdDev 为 {fmt_ms(total_stddev)} ms（变异系数约 {cv:.1f}%）。")
        mean_median_diff = mean_stats["totalMs"]["median"] - median_stats["totalMs"]["median"]
        if mean_median_diff > 0:
            lines.append(f"- Mean 口径高于 Median 口径（totalMs Mean {fmt_ms(mean_stats['totalMs']['median'])} ms vs Median {fmt_ms(median_stats['totalMs']['median'])} ms），说明存在少量耗时较高的样本拉高了均值。")
        lines.append(f"- P95 与 Max 对比：totalMs P95 {fmt_ms(median_stats['totalMs']['p95'])} ms vs Max {fmt_ms(median_stats['totalMs']['max'])} ms。")
    lines.append("")

    return "\n".join(lines)

# ---- 主入口 ----

def main():
    parser = argparse.ArgumentParser(description="生成表格识别性能基准报告")
    parser.add_argument("--results-dir", default=None, help="结果目录（默认 bench/results/）")
    parser.add_argument("--output", "-o", default=None, help="输出文件（默认 stdout）")
    parser.add_argument("--base-sha", default="", help="Base SHA")
    parser.add_argument("--patch-version", default="", help="Patch 版本")
    args = parser.parse_args()

    # 确定结果目录
    if args.results_dir:
        results_dir = args.results_dir
    else:
        script_dir = os.path.dirname(os.path.abspath(__file__))
        results_dir = os.path.join(script_dir, "results")

    if not os.path.isdir(results_dir):
        print(f"错误: 结果目录不存在: {results_dir}", file=sys.stderr)
        sys.exit(1)

    runs = load_results(results_dir)
    meta = load_meta(results_dir)
    report = generate_report(runs, meta, base_sha=args.base_sha, patch_version=args.patch_version)

    if args.output:
        with open(args.output, "w", encoding="utf-8") as f:
            f.write(report)
        print(f"报告已生成: {args.output}", file=sys.stderr)
    else:
        print(report)

if __name__ == "__main__":
    main()
