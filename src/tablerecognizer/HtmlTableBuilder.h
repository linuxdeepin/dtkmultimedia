// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HTMLTABLEBUILDER_H
#define HTMLTABLEBUILDER_H

#include "dtablerecognizer_global.h"
#include "dtablecell.h"

#include <QList>
#include <QString>
#include <QVector>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

// 结构化单元格 -> HTML 表格。按 row/col 排序，rowSpan/colSpan 写入属性，文本做 HTML 转义。
class HtmlTableBuilder
{
public:
    static QString build(const QList<DTableCell> &cells);
};

D_TABLERECOGNIZER_END_NAMESPACE

#endif // HTMLTABLEBUILDER_H
