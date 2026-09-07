// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DtkOcrWrapper.h"

#include <DOcr>

#include <QtDebug>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

bool DtkOcrWrapper::initialize()
{
    if (m_loaded)
        return true;
    m_loaded = m_ocr.loadPlugin(QStringLiteral("PPOCR_V5"));
    if (!m_loaded)
        qWarning() << "DtkOcrWrapper: load PPOCR_V5 failed, installed:" << m_ocr.installedPluginNames();
    return m_loaded;
}

bool DtkOcrWrapper::recognize(const QImage &image, QList<OcrTextBox> &boxes, QString &error)
{
    boxes.clear();
    if (!m_loaded && !initialize()) {
        error = QStringLiteral("OCR 插件未就绪");
        return false;
    }
    if (image.isNull()) {
        error = QStringLiteral("输入图片无效");
        return false;
    }

    m_ocr.setImage(image);
    if (!m_ocr.analyze()) {
        error = QStringLiteral("OCR analyze 失败");
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

D_TABLERECOGNIZER_END_NAMESPACE
