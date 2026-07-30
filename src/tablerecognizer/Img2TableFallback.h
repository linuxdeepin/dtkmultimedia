// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef IMG2TABLEFALLBACK_H
#define IMG2TABLEFALLBACK_H

#include "dtablerecognizer_global.h"
#include "tabletypes.h"

#include <QImage>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

// OpenCV 有线表格降级检测：形态学/霍夫检测表格线 -> 单元格。
class Img2TableFallback
{
public:
    bool detect(const QImage &image, QList<DetectedCell> &cells, QString &error);
    bool available() const;
};

D_TABLERECOGNIZER_END_NAMESPACE

#endif // IMG2TABLEFALLBACK_H
