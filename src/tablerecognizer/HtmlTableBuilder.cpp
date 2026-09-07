// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "HtmlTableBuilder.h"

#include <algorithm>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

static QString htmlEscape(const QString &s)
{
    QString out;
    out.reserve(int(s.size() * 1.1));
    for (const QChar &ch : s) {
        switch (ch.unicode()) {
        case '&':
            out += QStringLiteral("&amp;");
            break;
        case '<':
            out += QStringLiteral("&lt;");
            break;
        case '>':
            out += QStringLiteral("&gt;");
            break;
        case '"':
            out += QStringLiteral("&quot;");
            break;
        case '\'':
            out += QStringLiteral("&#39;");
            break;
        default:
            out += ch;
            break;
        }
    }
    return out;
}

QString HtmlTableBuilder::build(const QList<DTableCell> &cells)
{
    if (cells.isEmpty())
        return QStringLiteral("<table></table>");

    int maxRow = 0;
    int maxCol = 0;
    for (const DTableCell &c : cells) {
        maxRow = std::max(maxRow, c.row + std::max(1, c.rowSpan) - 1);
        maxCol = std::max(maxCol, c.col + std::max(1, c.colSpan) - 1);
    }

    const int rows = maxRow + 1;
    const int cols = maxCol + 1;
    // 占位：-1 表示待填，-2 表示被跨格覆盖，>=0 为单元格索引。
    QVector<QVector<int>> grid(rows, QVector<int>(cols, -1));

    QList<DTableCell> sorted = cells;
    std::sort(sorted.begin(), sorted.end(), [](const DTableCell &a, const DTableCell &b) {
        if (a.row != b.row)
            return a.row < b.row;
        return a.col < b.col;
    });

    for (int i = 0; i < sorted.size(); ++i) {
        const DTableCell &c = sorted.at(i);
        const int rs = std::max(1, c.rowSpan);
        const int cs = std::max(1, c.colSpan);
        int r = c.row;
        int col = c.col;
        if (r < 0)
            r = 0;
        if (col < 0)
            col = 0;
        if (r >= rows)
            r = rows - 1;
        if (col >= cols)
            col = cols - 1;
        for (int rr = r; rr < std::min(r + rs, rows); ++rr) {
            for (int cc = col; cc < std::min(col + cs, cols); ++cc) {
                grid[rr][cc] = -2;
            }
        }
        grid[r][col] = i;
    }

    QString html;
    html.reserve(256);
    html += QStringLiteral("<table>");
    for (int r = 0; r < rows; ++r) {
        // H2 v2：跳过「整行无任何有文本映射的单元格」的行。
        // v1 仅检查「全 -1」（无单元格），但 SLANet 的伪空首行含占位 cell（grid 值 ≥0）
        // 但该 cell 无 OCR 文本映射——v1 不过滤，v2 放宽为检查是否有文本。
        // 有内容行中的空单元格（占位列）仍保留为 <td></td>，不误伤。
        bool rowHasText = false;
        for (int c = 0; c < cols; ++c) {
            const int v = grid[r][c];
            if (v >= 0 && !sorted.at(v).text.isEmpty()) {
                rowHasText = true;
                break;
            }
        }
        if (!rowHasText)
            continue;

        html += QStringLiteral("<tr>");
        for (int c = 0; c < cols; ++c) {
            const int v = grid[r][c];
            if (v == -2)
                continue;
            if (v >= 0) {
                const DTableCell &cell = sorted.at(v);
                QString td = QStringLiteral("<td");
                if (cell.rowSpan > 1)
                    td += QStringLiteral(" rowspan=\"%1\"").arg(cell.rowSpan);
                if (cell.colSpan > 1)
                    td += QStringLiteral(" colspan=\"%1\"").arg(cell.colSpan);
                td += QStringLiteral(">");
                td += htmlEscape(cell.text);
                td += QStringLiteral("</td>");
                html += td;
            } else {
                html += QStringLiteral("<td></td>");
            }
        }
        html += QStringLiteral("</tr>");
    }
    html += QStringLiteral("</table>");
    return html;
}

D_TABLERECOGNIZER_END_NAMESPACE
