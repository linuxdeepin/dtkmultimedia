// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dscreenrecorder_p.h"
#include "videostreamffmpeg.h"
#include <QImage>

DMULTIMEDIA_USE_NAMESPACE

#if BUILD_Qt6
DScreenRecorder::DScreenRecorder(QObject *parent)
    : QMediaRecorder(parent), d_ptr(new DScreenRecorderPrivate(this))
#else
DScreenRecorder::DScreenRecorder(QObject *parent)
    : QMediaRecorder(nullptr, parent), d_ptr(new DScreenRecorderPrivate(this))
#endif
{
    Q_D(DScreenRecorder);
    d->videoInterface = new VideoStreamFfmpeg(this);
    connect(d->videoInterface, &VideoStreamInterface::screenStreamData, this, &DScreenRecorder::screenStreamData);
}

DScreenRecorder::~DScreenRecorder()
{
    Q_D(DScreenRecorder);
    if (d->videoInterface)
        delete d->videoInterface;
}

DScreenRecorder::VCodecID DScreenRecorder::codec() const
{
    Q_D(const DScreenRecorder);
    return d->videoInterface->codec();
}

void DScreenRecorder::setCodec(const VCodecID &codec)
{
    Q_D(DScreenRecorder);
    d->videoInterface->setCodec(codec);
}

DScreenRecorder::PixFormatID DScreenRecorder::pixfmt() const
{
    Q_D(const DScreenRecorder);
    return d->videoInterface->pixfmt();
}

void DScreenRecorder::setPixfmt(const DScreenRecorder::PixFormatID pixfmt)
{
    Q_D(DScreenRecorder);
    d->videoInterface->setPixfmt(pixfmt);
}

int DScreenRecorder::bitRate() const
{
    Q_D(const DScreenRecorder);
    return d->videoInterface->bitRate();
}

void DScreenRecorder::setBitRate(int bitrate)
{
    Q_D(DScreenRecorder);
    d->videoInterface->setBitRate(bitrate);
}

int DScreenRecorder::frameRate() const
{
    Q_D(const DScreenRecorder);
    return d->videoInterface->frameRate();
}

void DScreenRecorder::setFrameRate(int rate)
{
    Q_D(DScreenRecorder);
    d->videoInterface->setFrameRate(rate);
}

void DScreenRecorder::setResolution(const int width, const int height)
{
    Q_D(DScreenRecorder);
    d->videoInterface->setResolution(width, height);
}

QSize DScreenRecorder::resolution() const
{
    Q_D(const DScreenRecorder);
    return d->videoInterface->resolution();
}

QUrl DScreenRecorder::outputLocation() const
{
    Q_D(const DScreenRecorder);
    return d->videoInterface->outputLocation();
}

bool DScreenRecorder::setOutputLocation(const QUrl &location)
{
    Q_D(DScreenRecorder);
    return d->videoInterface->setOutputLocation(location);
}

void DScreenRecorder::setStreamAcceptFunc(VideoStreamCallback function, void *obj)
{
    Q_D(DScreenRecorder);
    return d->videoInterface->setStreamAcceptFunc(function, obj);
}

QPoint DScreenRecorder::topLeft() const
{
    Q_D(const DScreenRecorder);
    return d->videoInterface->topLeft();
}

void DScreenRecorder::setTopLeft(const int x, const int y)
{
    Q_D(DScreenRecorder);
    d->videoInterface->setTopLeft(x, y);
}

#if BUILD_Qt6
DScreenRecorder::State DScreenRecorder::state() const
{
    Q_D(const DScreenRecorder);
    return d->videoInterface->state();
}
#else
QMediaRecorder::State DScreenRecorder::state() const
{
    Q_D(const DScreenRecorder);
    return d->videoInterface->state();
}
#endif
void DScreenRecorder::record()
{
    Q_D(DScreenRecorder);
    d->videoInterface->record();
}

void DScreenRecorder::stop()
{
    Q_D(DScreenRecorder);
    d->videoInterface->stop();
}
