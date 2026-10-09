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

    // 最近一次 detect() 的失败分类（与 error 出参的详细诊断文本配合使用）。
    // 用于把「内部处理失败」与「图片里确实没有表格」区分开。
    enum class Failure {
        None = 0,
        InvalidImage,      // 传入图片无效
        ConversionFailed,  // QImage -> cv::Mat 转换失败（内部处理失败）
        NoTableLines,      // 检测到的表格线不足（图片里没有表格）
        NoGrid,            // 线不足以构成网格（图片里没有表格）
    };
    Failure lastFailure() const;

private:
    Failure m_failure = Failure::None;
};

D_TABLERECOGNIZER_END_NAMESPACE

#endif // IMG2TABLEFALLBACK_H
