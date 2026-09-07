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

// 将同一单元格内多个 OCR 文本框按阅读顺序拼接为一段连贯文本。
// 通用问题：原实现 parts.join(QString()) 无分隔直接拼接，相邻文本框末尾/开头
// 若本身无空格（CJK 常见），长文本会被粘连切断，可读性差。这里按通用规则插入分隔：
//  - 默认在相邻文本框之间补一个空格；
//  - 中日韩（CJK）字符相邻时不补空格（CJK 无词间空格）；
//  - 前一段已以空白结尾、或后一段以空白开头时不重复补，避免出现双空格。
// 该逻辑对任何表格、任何语言的 OCR 输出通用，不针对特定样张。
static bool isCjk(QChar ch)
{
    const ushort u = ch.unicode();
    // CJK 统一汉字 + 兼容表意 + CJK 标点（全角）常用区间。
    return (u >= 0x3400 && u <= 0x9FFF)      // CJK 统一/扩展A
        || (u >= 0xF900 && u <= 0xFAFF)      // CJK 兼容表意
        || (u >= 0x3000 && u <= 0x303F)      // CJK 标点/符号
        || (u >= 0xFF00 && u <= 0xFFEF);     // 全角形式
}

static QString joinOcrParts(const QStringList &parts)
{
    if (parts.isEmpty())
        return QString();
    if (parts.size() == 1)
        return parts.first();

    QString out = parts.first();
    for (int i = 1; i < parts.size(); ++i) {
        const QString &next = parts.at(i);
        if (next.isEmpty())
            continue;

        const QChar lastChar = out.isEmpty() ? QChar() : out.at(out.size() - 1);
        const QChar firstChar = next.at(0);

        // 前段已以空白结尾、或后段以空白开头：不再补空格，避免双空格。
        const bool prevEndsSpace = !out.isEmpty() && lastChar.isSpace();
        const bool nextStartsSpace = firstChar.isSpace();
        // CJK 相邻不补空格（CJK 无词间空格）。
        const bool bothCjk = !out.isEmpty() && isCjk(lastChar) && isCjk(firstChar);

        if (!prevEndsSpace && !nextStartsSpace && !bothCjk)
            out += QLatin1Char(' ');
        out += next;
    }
    return out;
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
        cells[c].text = joinOcrParts(parts);
    }
}

D_TABLERECOGNIZER_END_NAMESPACE
