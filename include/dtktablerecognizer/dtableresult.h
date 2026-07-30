// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTABLERESULT_H
#define DTABLERESULT_H

#include "dtablecell.h"

#include <QList>
#include <QString>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

struct DTableResult
{
    bool success = false;          // 识别是否成功
    QString html;                  // HTML 表格文本
    QList<DTableCell> cells;       // 结构化单元格列表
    QString source;                // 识别来源："SLANet_plus" 或 "img2table"
    QString errorMessage;          // 失败时的错误说明
};

D_TABLERECOGNIZER_END_NAMESPACE

#endif // DTABLERESULT_H
