// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIMAGECAPTURE_P_H
#define DIMAGECAPTURE_P_H
#include <dimagecapture.h>

DMULTIMEDIA_BEGIN_NAMESPACE
class DImageCapturePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DImageCapture)
public:
    explicit DImageCapturePrivate(DImageCapture *parent = nullptr)
        : QObject(parent), q_ptr(parent)
    {
    }

private:
    DImageCapture *q_ptr;

private:
    DCamera *camera;
    DMediaCaptureSession *mediaCapSession = nullptr;
    DImageCapture::FileFormat fileFormat = DImageCapture::JPEG;
    QSize resolution;
    DImageCapture::Quality quality = DImageCapture::NormalQuality;
    DMediaMetaData metaData;
};

DMULTIMEDIA_END_NAMESPACE

#endif   // DIMAGECAPTURE_P_H
