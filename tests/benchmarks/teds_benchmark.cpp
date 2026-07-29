// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Phase 0 TEDS 基准评测 harness：本地离线评测 C++ 表格结构检测精度（TEDS）与单图耗时。
// 输入：一组表格图片 + 对应 ground-truth 结构 HTML；输出：逐图 TEDS、min/avg/max 耗时、
// 有线/无线分组均值。本阶段仅记录，不判定阈值。

#include "OrtInferenceEngine.h"
#include "TableStructureDetector.h"

#include <QImage>
#include <QPainter>
#include <QElapsedTimer>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QList>
#include <QDebug>

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdio>

using namespace Dtk::TableRecognizer;

// ---------------------------------------------------------------------------
// TEDS：Tree-Edit-Distance based Similarity for table structure.
//
// 将结构 HTML（仅标签，不含文字内容）解析为标签序列，计算与 ground-truth 之间的
// 归一化树编辑距离，TEDS = 1 - norm_tree_edit_distance。
//
// 本实现采用简化的"标签序列编辑距离"近似：将 HTML 标签流视为树的前序遍历，
// 用 Levenshtein 编辑距离衡量序列差异，再除以两序列长度和做归一化。这与 PubTabNet
// 使用的 APTED 精确树编辑距离不完全等价，但作为 Phase 0 结构对齐的客观相对指标足够，
// 且完全本地、无外部依赖。后续阶段可替换为精确 APTED。
// ---------------------------------------------------------------------------

namespace {

// 解析 HTML 标签流为标签序列（仅保留结构标签，忽略属性与文本）。
std::vector<std::string> tokenizeStructure(const QString &html)
{
    std::vector<std::string> tags;
    int i = 0;
    const int n = html.size();
    while (i < n) {
        if (html[i] == QLatin1Char('<')) {
            int end = html.indexOf(QLatin1Char('>'), i);
            if (end < 0)
                break;
            QString tag = html.mid(i + 1, end - i - 1).trimmed();
            // 归一化：<td ...> -> td，去掉属性与斜杠。
            tag = tag.split(QLatin1Char(' ')).first();
            tag = tag.replace(QLatin1Char('/'), QString());
            if (!tag.isEmpty())
                tags.push_back(tag.toStdString());
            i = end + 1;
        } else {
            ++i;
        }
    }
    return tags;
}

// 经典 Levenshtein 编辑距离（插入/删除/替换各代价 1）。
int editDistance(const std::vector<std::string> &a, const std::vector<std::string> &b)
{
    const int m = a.size();
    const int n = b.size();
    std::vector<int> prev(n + 1), cur(n + 1);
    for (int j = 0; j <= n; ++j)
        prev[j] = j;
    for (int i = 1; i <= m; ++i) {
        cur[0] = i;
        for (int j = 1; j <= n; ++j) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            cur[j] = std::min({cur[j - 1] + 1, prev[j] + 1, prev[j - 1] + cost});
        }
        std::swap(prev, cur);
    }
    return prev[n];
}

// TEDS = 1 - editDist / (lenA + lenB)。
double computeTeds(const QString &predHtml, const QString &gtHtml)
{
    const auto a = tokenizeStructure(predHtml);
    const auto b = tokenizeStructure(gtHtml);
    const int dist = editDistance(a, b);
    const int total = a.size() + b.size();
    if (total == 0)
        return 1.0;
    return 1.0 - double(dist) / double(total);
}

// 将 DetectedCell 列表渲染为结构 HTML（仅标签，无文字，用于 TEDS）。
QString cellsToStructureHtml(const QList<DetectedCell> &detectedCells)
{
    if (detectedCells.isEmpty())
        return QString();

    // 按 row 主序、col 次序排序。
    QList<DetectedCell> sorted = detectedCells;
    std::sort(sorted.begin(), sorted.end(), [](const DetectedCell &x, const DetectedCell &y) {
        if (x.row != y.row)
            return x.row < y.row;
        return x.col < y.col;
    });

    QString html;
    int curRow = -1;
    html += QLatin1String("<html><body><table>");
    for (const DetectedCell &c : sorted) {
        if (c.row != curRow) {
            if (curRow >= 0)
                html += QLatin1String("</tr>");
            html += QLatin1String("<tr>");
            curRow = c.row;
        }
        QString td = QLatin1String("<td");
        if (c.colSpan > 1)
            td += QStringLiteral(" colspan=\"%1\"").arg(c.colSpan);
        if (c.rowSpan > 1)
            td += QStringLiteral(" rowspan=\"%1\"").arg(c.rowSpan);
        td += QLatin1String("></td>");
        html += td;
    }
    if (curRow >= 0)
        html += QLatin1String("</tr>");
    html += QLatin1String("</table></body></html>");
    return html;
}

} // namespace

// ---------------------------------------------------------------------------
// 确定性合成样本生成：用 QPainter 绘制有线/无线表格图。
// ---------------------------------------------------------------------------
struct SampleImage
{
    QImage image;
    QString gtHtml;     // 仅结构标签
    bool wired;
    QString name;
};

// 生成有线表格：带边框线的 rows×cols 网格。
SampleImage makeWiredSample(int rows, int cols, int cellW, int cellH, const QString &name)
{
    const int w = cols * cellW;
    const int h = rows * cellH;
    QImage img(w, h, QImage::Format_RGB888);
    img.fill(Qt::white);
    QPainter p(&img);
    p.setPen(QPen(Qt::black, 2));
    for (int r = 0; r <= rows; ++r)
        p.drawLine(0, r * cellH, w, r * cellH);
    for (int c = 0; c <= cols; ++c)
        p.drawLine(c * cellW, 0, c * cellW, h);
    p.end();

    QString html = QLatin1String("<html><body><table>");
    for (int r = 0; r < rows; ++r) {
        html += QLatin1String("<tr>");
        for (int c = 0; c < cols; ++c)
            html += QLatin1String("<td></td>");
        html += QLatin1String("</tr>");
    }
    html += QLatin1String("</table></body></html>");
    return {img, html, true, name};
}

// 生成无线表格：仅靠等间距空白形成视觉对齐（无边框线）。
// 用浅灰分隔以模拟文字对齐形成的"虚拟列"。
SampleImage makeWirelessSample(int rows, int cols, int cellW, int cellH, const QString &name)
{
    const int w = cols * cellW;
    const int h = rows * cellH;
    QImage img(w, h, QImage::Format_RGB888);
    img.fill(Qt::white);
    QPainter p(&img);
    // 模拟单元格内文字块（无外边框）。
    QFont font(QStringLiteral("Sans"), 10);
    p.setFont(font);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const int x0 = c * cellW + 6;
            const int y0 = r * cellH + 6;
            const int cw = cellW - 12;
            const int ch = cellH - 12;
            p.fillRect(QRect(x0, y0, cw, ch), Qt::lightGray);
        }
    }
    p.end();

    QString html = QLatin1String("<html><body><table>");
    for (int r = 0; r < rows; ++r) {
        html += QLatin1String("<tr>");
        for (int c = 0; c < cols; ++c)
            html += QLatin1String("<td></td>");
        html += QLatin1String("</tr>");
    }
    html += QLatin1String("</table></body></html>");
    return {img, html, false, name};
}

// 生成含 colspan 的有线表格样本。
SampleImage makeWiredColspanSample(int rows, int cols, int cellW, int cellH, const QString &name)
{
    const int w = cols * cellW;
    const int h = rows * cellH;
    QImage img(w, h, QImage::Format_RGB888);
    img.fill(Qt::white);
    QPainter p(&img);
    p.setPen(QPen(Qt::black, 2));
    for (int r = 0; r <= rows; ++r)
        p.drawLine(0, r * cellH, w, r * cellH);
    for (int c = 0; c <= cols; ++c)
        p.drawLine(c * cellW, 0, c * cellW, h);
    p.end();

    // GT: 第一行第一格跨2列。
    QString html = QLatin1String("<html><body><table>");
    html += QLatin1String("<tr><td colspan=\"2\"></td><td></td></tr>");
    for (int r = 1; r < rows; ++r) {
        html += QLatin1String("<tr>");
        for (int c = 0; c < cols; ++c)
            html += QLatin1String("<td></td>");
        html += QLatin1String("</tr>");
    }
    html += QLatin1String("</table></body></html>");
    return {img, html, true, name};
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    QString modelPath = QStringLiteral("src/tablerecognizer/models/SLANet_plus.onnx");
    QString samplesDir = QStringLiteral("tests/benchmarks/samples");
    QString reportPath = QStringLiteral("tests/benchmarks/teds_baseline_report.json");

    OrtInferenceEngine engine;
    if (!engine.loadModel(modelPath)) {
        qWarning() << "loadModel failed:" << engine.lastError();
        return 1;
    }
    TableStructureDetector detector(&engine);

    QList<SampleImage> samples;
    // 有线样本
    samples << makeWiredSample(2, 2, 100, 50, QStringLiteral("wired_2x2"));
    samples << makeWiredSample(3, 3, 100, 50, QStringLiteral("wired_3x3"));
    samples << makeWiredSample(4, 4, 100, 50, QStringLiteral("wired_4x4"));
    samples << makeWiredColspanSample(3, 3, 100, 50, QStringLiteral("wired_colspan"));
    // 无线样本
    samples << makeWirelessSample(2, 2, 100, 50, QStringLiteral("wireless_2x2"));
    samples << makeWirelessSample(3, 3, 100, 50, QStringLiteral("wireless_3x3"));
    samples << makeWirelessSample(4, 4, 100, 50, QStringLiteral("wireless_4x4"));

    // 如有真实截图样本目录则一并纳入。
    QDir realDir(samplesDir + QStringLiteral("/real"));
    if (realDir.exists()) {
        const QStringList imgs = realDir.entryList({QStringLiteral("*.png"), QStringLiteral("*.jpg")}, QDir::Files);
        for (const QString &imgName : imgs) {
            QImage img(realDir.filePath(imgName));
            if (img.isNull())
                continue;
            // 真实样本的 GT HTML 存于同名 .html 文件。
            QFile gtFile(realDir.filePath(imgName.section(QLatin1Char('.'), 0, 0) + QStringLiteral(".html")));
            QString gtHtml;
            if (gtFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                // 仅当文件开头存在 UTF-8 BOM 时跳过首行，避免误丢 HTML 内容。
                const QByteArray head = gtFile.peek(3);
                if (head.size() >= 3
                        && static_cast<unsigned char>(head[0]) == 0xEF
                        && static_cast<unsigned char>(head[1]) == 0xBB
                        && static_cast<unsigned char>(head[2]) == 0xBF) {
                    gtFile.readLine(); // consume BOM line
                }
                gtHtml = QString::fromUtf8(gtFile.readAll());
                gtFile.close();
            }
            const bool wired = imgName.startsWith(QLatin1String("wired"), Qt::CaseInsensitive);
            samples.append({img, gtHtml, wired, QStringLiteral("real/") + imgName});
        }
    }

    struct Stat
    {
        double sum = 0;
        int count = 0;
        double minT = 1e18, maxT = 0;
        double sumT = 0;
    };
    Stat wiredStat, wirelessStat, allStat;

    QJsonArray results;
    for (const SampleImage &s : samples) {
        QList<DetectedCell> detectedCells;
        QString err;
        QElapsedTimer t;
        t.start();
        bool ok = detector.detect(s.image, detectedCells, err);
        const double ms = double(t.nsecsElapsed()) / 1e6;

        QString predHtml;
        double teds = 0.0;
        if (ok)
            predHtml = cellsToStructureHtml(detectedCells);
        if (ok && !s.gtHtml.isEmpty())
            teds = computeTeds(predHtml, s.gtHtml);

        Stat &st = s.wired ? wiredStat : wirelessStat;
        st.sum += teds;
        st.count++;
        st.sumT += ms;
        st.minT = std::min(st.minT, ms);
        st.maxT = std::max(st.maxT, ms);
        allStat.sum += teds;
        allStat.count++;
        allStat.sumT += ms;
        allStat.minT = std::min(allStat.minT, ms);
        allStat.maxT = std::max(allStat.maxT, ms);

        QJsonObject o;
        o[QStringLiteral("name")] = s.name;
        o[QStringLiteral("wired")] = s.wired;
        o[QStringLiteral("ok")] = ok;
        o[QStringLiteral("cells")] = detectedCells.size();
        o[QStringLiteral("time_ms")] = ms;
        o[QStringLiteral("teds")] = teds;
        results.append(o);

        std::fprintf(stdout,
                     "%-22s wired=%-5s ok=%-5s cells=%-3lld time_ms=%8.3f teds=%.4f\n",
                     s.name.toUtf8().constData(),
                     s.wired ? "true" : "false",
                     ok ? "true" : "false",
                     (long long)detectedCells.size(), ms, teds);
    }

    auto summary = [](const Stat &st) -> QJsonObject {
        QJsonObject o;
        o[QStringLiteral("count")] = st.count;
        if (st.count > 0) {
            o[QStringLiteral("teds_mean")] = st.sum / st.count;
            o[QStringLiteral("time_ms_min")] = st.minT;
            o[QStringLiteral("time_ms_avg")] = st.sumT / st.count;
            o[QStringLiteral("time_ms_max")] = st.maxT;
        }
        return o;
    };

    QJsonObject report;
    report[QStringLiteral("model")] = modelPath;
    report[QStringLiteral("samples")] = results;
    report[QStringLiteral("wired")] = summary(wiredStat);
    report[QStringLiteral("wireless")] = summary(wirelessStat);
    report[QStringLiteral("all")] = summary(allStat);

    QFile rf(reportPath);
    if (rf.open(QIODevice::WriteOnly | QIODevice::Text)) {
        rf.write(QJsonDocument(report).toJson(QJsonDocument::Indented));
        rf.close();
        std::fprintf(stdout, "\nReport written to %s\n", reportPath.toUtf8().constData());
    }
    std::fprintf(stdout, "\n=== Summary ===\n");
    if (wiredStat.count) {
        std::fprintf(stdout, "Wired  : TEDS mean=%.4f  time min/avg/max=%.3f/%.3f/%.3f ms (n=%d)\n",
                     wiredStat.sum / wiredStat.count, wiredStat.minT,
                     wiredStat.sumT / wiredStat.count, wiredStat.maxT, wiredStat.count);
    }
    if (wirelessStat.count) {
        std::fprintf(stdout, "Wireless: TEDS mean=%.4f  time min/avg/max=%.3f/%.3f/%.3f ms (n=%d)\n",
                     wirelessStat.sum / wirelessStat.count, wirelessStat.minT,
                     wirelessStat.sumT / wirelessStat.count, wirelessStat.maxT, wirelessStat.count);
    }
    if (allStat.count) {
        std::fprintf(stdout, "All    : TEDS mean=%.4f  time min/avg/max=%.3f/%.3f/%.3f ms (n=%d)\n",
                     allStat.sum / allStat.count, allStat.minT,
                     allStat.sumT / allStat.count, allStat.maxT, allStat.count);
    }
    return 0;
}
