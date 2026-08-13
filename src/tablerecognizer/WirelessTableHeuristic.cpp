// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "WirelessTableHeuristic.h"

#include <QDebug>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

QList<QList<int>> WirelessTableHeuristic::clusterRows(const QList<OcrTextBox> &boxes, qreal yTolerance)
{
    QList<QList<int>> rows;
    if (boxes.isEmpty())
        return rows;

    // 索引按中心 y 升序排列（从上到下）。
    QList<int> order;
    order.reserve(boxes.size());
    for (int i = 0; i < boxes.size(); ++i)
        order.append(i);
    std::sort(order.begin(), order.end(), [&boxes](int a, int b) {
        return boxes.at(a).bbox.center().y() < boxes.at(b).bbox.center().y();
    });

    // 以每行首个框（锚点）的中心 y 为基准分簇：与锚点差距 <= yTolerance 归入同行。
    // 锚点基准避免运行均值漂移导致的过度分裂。行高被限制在 ~2*yTolerance 内。
    for (int idx : order) {
        const qreal cy = boxes.at(idx).bbox.center().y();
        if (!rows.isEmpty() && !rows.last().isEmpty()) {
            const qreal anchorY = boxes.at(rows.last().first()).bbox.center().y();
            if (std::abs(cy - anchorY) <= yTolerance) {
                rows.last().append(idx);
                continue;
            }
        }
        rows.append(QList<int>{idx});
    }

    // 行内按 x 升序（从左到右），便于下游稳定与可读。
    for (QList<int> &row : rows)
        std::sort(row.begin(), row.end(), [&boxes](int a, int b) {
            return boxes.at(a).bbox.center().x() < boxes.at(b).bbox.center().x();
        });
    return rows;
}

QList<QPair<qreal, qreal>> WirelessTableHeuristic::splitColumns(const QList<OcrTextBox> &boxes,
                                                                  qreal gapTolerance)
{
    QList<QPair<qreal, qreal>> cols;
    if (boxes.isEmpty())
        return cols;

    // 收集每个文本框的水平区间 [left, right]，按 left 升序。
    struct Interval
    {
        qreal left;
        qreal right;
    };
    QList<Interval> ivs;
    ivs.reserve(boxes.size());
    for (const OcrTextBox &b : boxes)
        ivs.append({b.bbox.left(), b.bbox.right()});
    std::sort(ivs.begin(), ivs.end(), [](const Interval &a, const Interval &b) {
        return a.left < b.left;
    });

    // 按 x 重叠/邻近聚类：下一个区间左端 <= 当前列右端 + gapTolerance 视为同列，
    // 否则分列。列范围取簇内所有区间的并 [minLeft, maxRight]。
    qreal curLeft = ivs.first().left;
    qreal curRight = ivs.first().right;
    for (int i = 1; i < ivs.size(); ++i) {
        const Interval &iv = ivs.at(i);
        if (iv.left <= curRight + gapTolerance) {
            curRight = std::max(curRight, iv.right);
        } else {
            cols.append({curLeft, curRight});
            curLeft = iv.left;
            curRight = iv.right;
        }
    }
    cols.append({curLeft, curRight});
    return cols;
}

bool WirelessTableHeuristic::build(const QSize &imageSize, const QList<OcrTextBox> &ocrBoxes,
                                    QList<DetectedCell> &cells, QString &error)
{
    cells.clear();
    Q_UNUSED(imageSize)
    if (ocrBoxes.isEmpty()) {
        error = QStringLiteral("无线启发式：无 OCR 文本框可用");
        return false;
    }

    // 自适应容差：以文本框中位宽/高为基准（中位数对离群框鲁棒），退化时回退小常数。
    QList<qreal> widths, heights;
    widths.reserve(ocrBoxes.size());
    heights.reserve(ocrBoxes.size());
    for (const OcrTextBox &b : ocrBoxes) {
        widths.append(b.bbox.width());
        heights.append(b.bbox.height());
    }
    std::sort(widths.begin(), widths.end());
    std::sort(heights.begin(), heights.end());
    const qreal medW = widths.at(widths.size() / 2);
    const qreal medH = heights.at(heights.size() / 2);
    const qreal yTol = std::max(qreal(2.0), qreal(0.5) * medH);
    const qreal gapTol = std::max(qreal(2.0), qreal(0.3) * medW);

    const QList<QList<int>> rows = clusterRows(ocrBoxes, yTol);
    const QList<QPair<qreal, qreal>> cols = splitColumns(ocrBoxes, gapTol);
    if (rows.isEmpty() || cols.isEmpty()) {
        error = QStringLiteral("无线启发式：行列聚类为空");
        return false;
    }

    // 行 y 带：每行所含框的最小 top / 最大 bottom。
    QList<QPair<qreal, qreal>> rowBands;
    rowBands.reserve(rows.size());
    for (const QList<int> &row : rows) {
        qreal yMin = ocrBoxes.at(row.first()).bbox.top();
        qreal yMax = ocrBoxes.at(row.first()).bbox.bottom();
        for (int idx : row) {
            yMin = std::min(yMin, ocrBoxes.at(idx).bbox.top());
            yMax = std::max(yMax, ocrBoxes.at(idx).bbox.bottom());
        }
        rowBands.append({yMin, yMax});
    }

    // 按 (row, col) 网格构建单元格：仅当该格含至少一个文本框中心时产出（稀疏网格），
    // 空位交由 HtmlTableBuilder 自动补 <td></td>，保持行列结构完整。
    for (int r = 0; r < rows.size(); ++r) {
        const qreal rowYMin = rowBands.at(r).first;
        const qreal rowYMax = rowBands.at(r).second;
        for (int c = 0; c < cols.size(); ++c) {
            const qreal colXMin = cols.at(c).first;
            const qreal colXMax = cols.at(c).second;
            bool hasBox = false;
            for (int idx : rows.at(r)) {
                const QPointF ctr = ocrBoxes.at(idx).bbox.center();
                if (ctr.x() >= colXMin && ctr.x() <= colXMax && ctr.y() >= rowYMin && ctr.y() <= rowYMax) {
                    hasBox = true;
                    break;
                }
            }
            if (!hasBox)
                continue;
            DetectedCell cell;
            cell.row = r;
            cell.col = c;
            cell.rowSpan = 1;
            cell.colSpan = 1;
            // 单元格 bbox = 列 x 区间 × 行 y 区间（规整矩形，便于后续文本映射/合并判定）。
            cell.bbox = QRectF(colXMin, rowYMin, colXMax - colXMin, rowYMax - rowYMin);
            cells.append(cell);
        }
    }

    if (cells.isEmpty()) {
        error = QStringLiteral("无线启发式：未构建出单元格");
        return false;
    }
    return true;
}

float WirelessTableHeuristic::lineDensity(const QImage &image)
{
    if (image.isNull())
        return 0.0f;

    QImage rgb = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(rgb.height(), rgb.width(), CV_8UC3, const_cast<uchar *>(rgb.constBits()),
                static_cast<size_t>(rgb.bytesPerLine()));
    cv::Mat bgr;
    cv::cvtColor(mat, bgr, cv::COLOR_RGB2BGR);
    cv::Mat gray;
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
    const int w = bgr.cols;
    const int h = bgr.rows;
    const size_t total = static_cast<size_t>(w) * static_cast<size_t>(h);
    if (total == 0)
        return 0.0f;
    // H3 v2：自适应局部阈值（adaptiveThreshold）替代全局 Otsu。
    // 全局 Otsu 把浅色细线当背景 → 有线表线密度≈0，与无线表重叠。
    // adaptiveThreshold（Gaussian，blockSize≈15）自适应局部阈值能捕获浅色细线。
    // blockSize 必须为奇数且 ≥3。
    const int blockSize = std::max(3, ((std::min(w, h) / 15) | 1));
    cv::Mat binary;
    cv::adaptiveThreshold(gray, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                           cv::THRESH_BINARY_INV, blockSize, 5);

    // 水平长线：morph open 用 cols/20 长度水平核，仅保留长水平线，排除短文本笔画。
    cv::Mat horiz;
    const cv::Mat hKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(std::max(1, w / 20), 1));
    cv::morphologyEx(binary, horiz, cv::MORPH_OPEN, hKernel);
    // 垂直长线。
    cv::Mat vert;
    const cv::Mat vKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, std::max(1, h / 20)));
    cv::morphologyEx(binary, vert, cv::MORPH_OPEN, vKernel);

    const size_t hPx = static_cast<size_t>(cv::countNonZero(horiz));
    const size_t vPx = static_cast<size_t>(cv::countNonZero(vert));
    // 归一化到 0~1：水平/垂直线像素各占总像素的比例均值（满网格线表≈1，无线表≈0）。
    return static_cast<float>(static_cast<double>(hPx + vPx) / (2.0 * static_cast<double>(total)));
}

bool WirelessTableHeuristic::available() const
{
    return true;
}

D_TABLERECOGNIZER_END_NAMESPACE
