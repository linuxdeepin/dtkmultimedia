// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "videosurface.h"
DMULTIMEDIA_USE_NAMESPACE
VideoSurface::VideoSurface(QObject *parent)
    : QAbstractVideoSurface(parent)
{
}

VideoSurface::~VideoSurface()
{
}

QList<QVideoFrame::PixelFormat> VideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    QList<QVideoFrame::PixelFormat> listPixelFormats;

    listPixelFormats << QVideoFrame::Format_RGB32;

    return listPixelFormats;
}

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
