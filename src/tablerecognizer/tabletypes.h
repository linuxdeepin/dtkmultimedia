// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
// 内部共享数据类型：表格结构检测单元与 OCR 文本框。

#ifndef TABLERECOGNIZER_TABLETYPES_H
#define TABLERECOGNIZER_TABLETYPES_H

#include "dtablerecognizer_global.h"

#include <QList>
#include <QRectF>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

// 表格结构检测输出单元（内部）
struct DetectedCell
{
    int row = 0;
    int col = 0;
    int rowSpan = 1;
    int colSpan = 1;
    QRectF bbox;   // 在原图坐标系下的物理包围盒
    QString text;   // OCR 填充文字（映射阶段填入）
};

// OCR 文本框（内部）
struct OcrTextBox
{
    QRectF bbox;
    QString text;
};

D_TABLERECOGNIZER_END_NAMESPACE

#endif // TABLERECOGNIZER_TABLETYPES_H
