// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "CellTextMapper.h"

#include <algorithm>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

double CellTextMapper::iou(const QRectF &a, const QRectF &b)
{
    const QRectF inter = a.intersected(b);
    if (inter.isEmpty())
        return 0.0;
    const double interArea = inter.width() * inter.height();
    const double unionArea = a.width() * a.height() + b.width() * b.height() - interArea;
    if (unionArea <= 0.0)
        return 0.0;
    return interArea / unionArea;
}

void CellTextMapper::map(QList<DetectedCell> &cells, const QList<OcrTextBox> &ocrBoxes)
{
    // 清空已有文本，避免重复识别时残留。
    for (DetectedCell &cell : cells)
        cell.text.clear();

    // 每个 cell 收集命中的 OCR 框索引。
    QList<QList<int>> hits;
    hits.resize(cells.size());

    for (int i = 0; i < ocrBoxes.size(); ++i) {
        const OcrTextBox &box = ocrBoxes.at(i);
        const QPointF center = box.bbox.center();

        int bestCell = -1;
        // 1) 中心点落入单元格 bbox 即归入。
        for (int c = 0; c < cells.size(); ++c) {
            if (cells.at(c).bbox.contains(center)) {
                bestCell = c;
                break;
            }
        }

        // 2) IoU 兜底：处理跨格/边界框，按最大重叠归入。
        if (bestCell < 0) {
            double bestIou = 0.0;
            for (int c = 0; c < cells.size(); ++c) {
                const double score = iou(cells.at(c).bbox, box.bbox);
                if (score > bestIou) {
                    bestIou = score;
                    bestCell = c;
                }
            }
            // 重叠度过低则丢弃该框，避免误归入。
            if (bestIou < 0.05)
                bestCell = -1;
        }

        if (bestCell >= 0)
            hits[bestCell].append(i);
    }

    // 同一单元格多个 OCR 框：按从左到右、从上到下拼接。
    for (int c = 0; c < cells.size(); ++c) {
        QList<int> &idxs = hits[c];
        if (idxs.isEmpty())
            continue;
        std::sort(idxs.begin(), idxs.end(), [&ocrBoxes](int lhs, int rhs) {
            const QRectF &l = ocrBoxes.at(lhs).bbox;
            const QRectF &r = ocrBoxes.at(rhs).bbox;
            // 行差判定：纵向落差大于较高框高度的一半视为不同行。
            const qreal rowGap = std::max(l.height(), r.height()) * 0.5;
            if (qAbs(l.top() - r.top()) > rowGap)
                return l.top() < r.top();
            return l.left() < r.left();
        });
        QStringList parts;
        parts.reserve(idxs.size());
        for (int idx : idxs)
            parts << ocrBoxes.at(idx).text;
        cells[c].text = parts.join(QString());
    }
}

D_TABLERECOGNIZER_END_NAMESPACE
