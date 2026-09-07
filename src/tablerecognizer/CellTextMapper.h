// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CELLTEXTMAPPER_H
#define CELLTEXTMAPPER_H

#include "dtablerecognizer_global.h"
#include "tabletypes.h"

D_TABLERECOGNIZER_BEGIN_NAMESPACE

// 将 OCR 文本框映射到表格结构单元：中心点归入 + IoU 兜底 + 从左到右/从上到下拼接。
class CellTextMapper
{
public:
    void map(QList<DetectedCell> &cells, const QList<OcrTextBox> &ocrBoxes);

private:
    static double iou(const QRectF &a, const QRectF &b);
};

D_TABLERECOGNIZER_END_NAMESPACE

#endif // CELLTEXTMAPPER_H
