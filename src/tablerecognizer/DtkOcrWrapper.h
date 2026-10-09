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

    // 最近一次 recognize() 的失败分类（与 error 出参的详细诊断文本配合使用）。
    enum class Failure {
        None = 0,
        PluginUnavailable,  // PPOCR_V5 未安装 / 加载失败
        InvalidImage,       // 传入图片无效
        NoTextDetected,     // 引擎可用，但一个文本框都没有（含被取消）
        EngineFailed,       // 其它执行失败
    };
    Failure lastFailure() const;

private:
    Dtk::Ocr::DOcr m_ocr;
    bool m_loaded = false;
    Failure m_failure = Failure::None;
};

D_TABLERECOGNIZER_END_NAMESPACE

#endif // DTKOCRWRAPPER_H
