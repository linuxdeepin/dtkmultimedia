// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "videosurface.h"
DMULTIMEDIA_USE_NAMESPACE
#ifdef BUILD_Qt6
VideoSurface::VideoSurface(QObject *parent)
    : QObject(parent)
#else
VideoSurface::VideoSurface(QObject *parent)
    : QAbstractVideoSurface(parent)
#endif
{
}

VideoSurface::~VideoSurface()
{
}

#ifdef BUILD_Qt6
QList<QVideoFrameFormat::PixelFormat> VideoSurface::pixelFormat() const
{
    QList<QVideoFrameFormat::PixelFormat> listPixelFormats;

    listPixelFormats << QVideoFrameFormat::Format_ARGB8888;

    return listPixelFormats;
}
#else
QList<QVideoFrame::PixelFormat> VideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    QList<QVideoFrame::PixelFormat> listPixelFormats;

    listPixelFormats << QVideoFrame::Format_RGB32;

    return listPixelFormats;
}
#endif

bool  VideoSurface::present(const QVideoFrame &frame)
{
    if (frame.isValid())
    {
        QVideoFrame cloneFrame(frame);
        emit frameAvailable(cloneFrame);

        return true;
    }

    return false;
}
