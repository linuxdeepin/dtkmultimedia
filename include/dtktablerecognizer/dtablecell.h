// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTABLECELL_H
#define DTABLECELL_H

#include "dtablerecognizer_global.h"

#include <QRectF>
#include <QString>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

struct DTableCell
{
    int row = 0;       // 单元格逻辑行位置（0-based）
    int col = 0;       // 单元格逻辑列位置（0-based）
    int rowSpan = 1;   // 跨行数
    int colSpan = 1;   // 跨列数
    QRectF bbox;       // 单元格在原图中的物理包围盒
    QString text;      // 单元格内识别文字（OCR 填充）
};

D_TABLERECOGNIZER_END_NAMESPACE

#endif // DTABLECELL_H
