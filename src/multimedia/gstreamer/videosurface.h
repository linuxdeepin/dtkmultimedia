// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VIDEOSURFACE_H
#define VIDEOSURFACE_H

#ifdef BUILD_Qt6
#include <QObject>
#include <QVideoFrame>
#include <QVideoSink>
#else
#include <QAbstractVideoSurface>
#endif
#include <dtkmultimedia.h>

DMULTIMEDIA_BEGIN_NAMESPACE

#ifdef BUILD_Qt6
class VideoSurface : public QObject
#else
class VideoSurface : public QAbstractVideoSurface
#endif
{
        Q_OBJECT

public:
explicit VideoSurface(QObject *parent = Q_NULLPTR);
        ~VideoSurface();
#ifdef BUILD_Qt6
    QList<QVideoFrameFormat::PixelFormat> pixelFormat() const;
    QVideoSink *videoSink() const { return m_sink; }
#else
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;
#endif
        bool present(const QVideoFrame &frame);

signals:
        void frameAvailable(QVideoFrame &frame);

#ifdef BUILD_Qt6
private:
    QVideoSink *m_sink = nullptr;
#endif
};
DMULTIMEDIA_END_NAMESPACE
#endif // VIDEOSURFACE_H
