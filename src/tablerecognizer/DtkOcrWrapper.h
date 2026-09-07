// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTKOCRWRAPPER_H
#define DTKOCRWRAPPER_H

#include "dtablerecognizer_global.h"
#include "tabletypes.h"

#include <DOcr>
#include <QImage>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

// 封装 Dtk::Ocr::DOcr 调用，显式加载 PPOCR_V5。
class DtkOcrWrapper
{
public:
    // 加载 PPOCR_V5 插件；返回是否就绪。
    bool initialize();
    bool recognize(const QImage &image, QList<OcrTextBox> &boxes, QString &error);
    bool available() const;

private:
    Dtk::Ocr::DOcr m_ocr;
    bool m_loaded = false;
};

D_TABLERECOGNIZER_END_NAMESPACE

#endif // DTKOCRWRAPPER_H
