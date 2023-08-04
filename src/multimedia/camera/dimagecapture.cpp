// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dimagecapture_p.h"
#include "dcamera.h"
#include "datamanager.h"

DMULTIMEDIA_USE_NAMESPACE

#if BUILD_Qt6
DImageCapture::DImageCapture(QObject *parent)
    : QImageCapture(parent), d_ptr(new DImageCapturePrivate(this))
#else
DImageCapture::DImageCapture(QMediaObject *parent)
    : QCameraImageCapture(parent), d_ptr(new DImageCapturePrivate(this))
#endif
{
    Q_D(DImageCapture);
    d->camera = qobject_cast<DCamera *>(parent);
#if BUILD_Qt6
    if (d->camera->isFfmpegEnv()) {
        connect(d->camera, &QCamera::activeChanged, [=]() {
            emit QImageCapture::readyForCaptureChanged(true);
        });
    }
#else
    if (d->camera->isFfmpegEnv()) {
        connect(d->camera, &QCamera::stateChanged, [=](QCamera::State state) {
            bool ready = (state == QCamera::ActiveState);
            emit QCameraImageCapture::readyForCaptureChanged(ready);
        });
    }
#endif
}

DImageCapture::~DImageCapture()
{
}

bool DImageCapture::isAvailable() const
{
    Q_D(const DImageCapture);
    if (d->camera->isFfmpegEnv()) {
        return DataManager::instance()->getdevStatus() == CAM_CANUSE;
    }

#if BUILD_Qt6
    return QImageCapture::isAvailable();
#else
    return QCameraImageCapture::isAvailable();
#endif
}

DMediaCaptureSession *DImageCapture::captureSession() const
{
    Q_D(const DImageCapture);
    return d->camera->captureSession();
}

#if BUILD_Qt6
QImageCapture::Error DImageCapture::error() const
{
    return QImageCapture::error();
}

QString DImageCapture::errorString() const
{
    return QImageCapture::errorString();
}

bool DImageCapture::isReadyForCapture() const
{
    return QImageCapture::isReadyForCapture();
}
#else
QCameraImageCapture::Error DImageCapture::error() const
{
    return QCameraImageCapture::error();
}

QString DImageCapture::errorString() const
{
    return QCameraImageCapture::errorString();
}

bool DImageCapture::isReadyForCapture() const
{
    return QCameraImageCapture::isReadyForCapture();
}
#endif
DImageCapture::FileFormat DImageCapture::fileFormat() const
{
    Q_D(const DImageCapture);
    return d->fileFormat;
}

void DImageCapture::setFileFormat(DImageCapture::FileFormat format)
{
    Q_D(DImageCapture);
    if (d->fileFormat != format) {
        d->fileFormat = format;
        emit fileFormatChanged();
    }
}

QList<DImageCapture::FileFormat> DImageCapture::supportedFormats()
{
    return QList<DImageCapture::FileFormat>();
}

QString DImageCapture::fileFormatName(DImageCapture::FileFormat f)
{
    const char *name = nullptr;
    switch (f) {
    case UnspecifiedFormat:
        name = "Unspecified image format";
        break;
    case JPEG:
        name = "JPEG";
        break;
    case PNG:
        name = "PNG";
        break;
    case WebP:
        name = "WebP";
        break;
    case Tiff:
        name = "Tiff";
        break;
    }
    return QString::fromUtf8(name);
}

QString DImageCapture::fileFormatDescription(DImageCapture::FileFormat f)
{
    const char *name = nullptr;
    switch (f) {
    case UnspecifiedFormat:
        name = "Unspecified image format";
        break;
    case JPEG:
        name = "JPEG";
        break;
    case PNG:
        name = "PNG";
        break;
    case WebP:
        name = "WebP";
        break;
    case Tiff:
        name = "Tiff";
        break;
    }
    return QString::fromUtf8(name);
}

QSize DImageCapture::resolution() const
{
    Q_D(const DImageCapture);
    return d->resolution;
}

void DImageCapture::setResolution(const QSize &size)
{
    Q_D(DImageCapture);
    if (d->resolution != size) {
        d->resolution = size;
        emit resolutionChanged();
    }
}

DImageCapture::Quality DImageCapture::quality() const
{
    Q_D(const DImageCapture);
    return d->quality;
}

void DImageCapture::setQuality(DImageCapture::Quality quality)
{
    Q_D(DImageCapture);
    if (d->quality != quality) {
        d->quality = quality;
        emit qualityChanged();
    }
}

DMediaMetaData DImageCapture::metaData() const
{
    Q_D(const DImageCapture);
    return d->metaData;
}

void DImageCapture::setMetaData(const DMediaMetaData &metaData)
{
    Q_D(DImageCapture);
    if (d->metaData != metaData) {
        d->metaData = metaData;
        emit metaDataChanged();
    }
}

void DImageCapture::addMetaData(const DMediaMetaData &metaData)
{
    Q_D(DImageCapture);
    for (DMediaMetaData::Key key : metaData.keys()) {
        d->metaData[key] = metaData.value(key);
    }
    if (!metaData.isEmpty()) {
        emit metaDataChanged();
    }
}

int DImageCapture::captureToFile(const QString &location)
{
    Q_D(DImageCapture);
    if (d->camera->isFfmpegEnv()) {
        d->camera->takeOne(location);
        return 0;
    } else {
#if BUILD_Qt6
        return QImageCapture::captureToFile(location);
#else
        return -1;
#endif
    }

}

int DImageCapture::capture(const QString &location)
{
    Q_D(DImageCapture);
    if (d->camera->isFfmpegEnv()) {
        d->camera->takeOne(location);
        return 0;
    } else {
#if BUILD_Qt6
        return QImageCapture::captureToFile(location);
#else
        return QCameraImageCapture::capture(location);
#endif
    }
}
