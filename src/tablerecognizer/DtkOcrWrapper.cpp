// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DtkOcrWrapper.h"

#include "TableErrorUtils.h"

#include <DOcr>

#include <QtDebug>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

bool DtkOcrWrapper::initialize()
{
    if (m_loaded)
        return true;
    m_loaded = m_ocr.loadPlugin(QStringLiteral("PPOCR_V5"));
    if (!m_loaded) {
        qWarning() << "DtkOcrWrapper: load PPOCR_V5 failed, installed:" << m_ocr.installedPluginNames();
        return false;
    }

    // 与 deepin-ocr 的默认使用方式保持一致：PPOCR_V5 的识别阶段支持 OpenMP
    // 并行，但插件默认 maxThreadsUsed=1；表格图片文字框多，loong64 CPU 路径下
    // 单线程容易超过表格识别默认超时。
    m_ocr.setUseMaxThreadsCount(2);
    return true;
}

bool DtkOcrWrapper::recognize(const QImage &image, QList<OcrTextBox> &boxes, QString &error)
{
    boxes.clear();
    m_failure = Failure::None;
    if (!m_loaded && !initialize()) {
        m_failure = Failure::PluginUnavailable;
        const QStringList installed = m_ocr.installedPluginNames();
        error = TableErrorUtils::detail(
            QStringLiteral("ocr"), QStringLiteral("engine unavailable"),
            {{QStringLiteral("plugin"), QStringLiteral("PPOCR_V5")},
             {QStringLiteral("installed"),
              installed.isEmpty() ? QStringLiteral("none") : installed.join(QLatin1Char(','))}});
        return false;
    }
    if (image.isNull()) {
        m_failure = Failure::InvalidImage;
        error = TableErrorUtils::detail(QStringLiteral("ocr"), QStringLiteral("invalid input image"),
                                        {{QStringLiteral("image"), QStringLiteral("null")}});
        return false;
    }

    m_ocr.setImage(image);
    if (!m_ocr.analyze()) {
        // PPOCR_V5 的 analyze() 用 false 同时表示「执行失败」和「一个文本框都没有」
        // （实现见 src/ocr/ppocr/ppocrv5.cpp 的 `return !allTextBoxes.empty();`）。
        // 这里按「引擎可用 + 文本框数为 0」判定为无文字，使上层能区分
        // 「图里没有文字」和「OCR 挂了」——否则两者都会落成同一个错误码。
        const int boxCount = m_ocr.textBoxes().size();
        const QList<TableErrorUtils::Field> fields = {
            {QStringLiteral("engine"), QStringLiteral("PPOCR_V5")},
            {QStringLiteral("boxes"), QString::number(boxCount)},
            {QStringLiteral("image"), QStringLiteral("%1x%2").arg(image.width()).arg(image.height())}};
        if (boxCount == 0) {
            m_failure = Failure::NoTextDetected;
            error = TableErrorUtils::detail(QStringLiteral("ocr"), QStringLiteral("no text detected"),
                                            fields);
            return false;
        }
        m_failure = Failure::EngineFailed;
        error = TableErrorUtils::detail(QStringLiteral("ocr"), QStringLiteral("analyze failed"),
                                        fields);
        return false;
    }

    const QList<Dtk::Ocr::TextBox> tbs = m_ocr.textBoxes();
    boxes.reserve(tbs.size());
    for (int i = 0; i < tbs.size(); ++i) {
        OcrTextBox box;
        const QList<QPointF> &pts = tbs.at(i).points;
        if (!pts.isEmpty()) {
            qreal minX = pts.first().x(), minY = pts.first().y();
            qreal maxX = minX, maxY = minY;
            for (const QPointF &p : pts) {
                minX = std::min(minX, p.x());
                minY = std::min(minY, p.y());
                maxX = std::max(maxX, p.x());
                maxY = std::max(maxY, p.y());
            }
            box.bbox = QRectF(minX, minY, maxX - minX, maxY - minY);
        }
        box.text = m_ocr.resultFromBox(i);
        boxes.append(box);
    }
    return true;
}

bool DtkOcrWrapper::available() const
{
    return m_loaded;
}

DtkOcrWrapper::Failure DtkOcrWrapper::lastFailure() const
{
    return m_failure;
}

D_TABLERECOGNIZER_END_NAMESPACE
