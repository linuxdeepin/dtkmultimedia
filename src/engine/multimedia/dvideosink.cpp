// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dvideosink.h"

//#include "qvideoframeformat.h"
#include "qvideoframe.h"
#include "qmediaplayer.h"
//#include "qmediacapturesession.h"

#include <qvariant.h>
#include <qpainter.h>
#include <qmatrix4x4.h>
#include <QDebug>
//#include <private/qplatformmediaintegration_p.h>
//#include <private/qplatformvideosink_p.h>

QT_BEGIN_NAMESPACE

class DVideoSinkPrivate {
public:
    DVideoSinkPrivate(DVideoSink *q)
        : q_ptr(q)
    {
//        videoSink = QPlatformMediaIntegration::instance()->createVideoSink(q);
    }
    ~DVideoSinkPrivate()
    {
        delete videoSink;
    }
    void unregisterSource()
    {
        if (!source)
            return;
        auto *old = source;
        source = nullptr;
//        if (auto *player = qobject_cast<QMediaPlayer *>(old))
//            player->setVideoSink(nullptr);
//        else if (auto *capture = qobject_cast<QMediaCaptureSession *>(old))
//            capture->setVideoSink(nullptr);
    }

    DVideoSink *q_ptr = nullptr;
    QPlatformVideoSink *videoSink = nullptr;
    QObject *source = nullptr;
    QRhi *rhi = nullptr;
};

/*!
    \class QVideoSink

    \brief The QVideoSink class represents a generic sink for video data.
    \inmodule QtMultimedia
    \ingroup multimedia
    \ingroup multimedia_video

    The QVideoSink class can be used to retrieve video data on a frame by frame
    basis from Qt Multimedia.

    QVideoSink can operate in two modes. In the first mode, it can render the video
    stream to a native window of the underlying windowing system. In the other mode,
    it will provide individual video frames to the application developer through the
    videoFrameChanged() signal.

    The video frame can then be used to read out the data of those frames and handle them
    further. When using QPainter, the QVideoFrame can be drawing using the paint() method
    in QVideoSink.

    QVideoFrame objects can consume a significant amount of memory or system resources and
    should thus not be held for longer than required by the application.

    \sa QMediaPlayer, QMediaCaptureSession

*/

/*!
    Constructs a new QVideoSink object with \a parent.
 */
DVideoSink::DVideoSink(QObject *parent)
    : QObject(parent),
    d(new DVideoSinkPrivate(this))
{
    qRegisterMetaType<QVideoFrame>();
}

/*!
    Destroys the object.
 */
DVideoSink::~DVideoSink()
{
    d->unregisterSource();
    delete d;
}

/*!
    \internal
    Returns the QRhi instance being used to create texture data in the video frames.
 */
QRhi *DVideoSink::rhi() const
{
    return d->rhi;
}

/*!
    \internal
    Sets the QRhi instance being used to create texture data in the video frames
    to \a rhi.
 */
void DVideoSink::setRhi(QRhi *rhi)
{
    if (d->rhi == rhi)
        return;
    d->rhi = rhi;
//    d->videoSink->setRhi(rhi);
}

/*!
    \internal
*/
QPlatformVideoSink *DVideoSink::platformVideoSink() const
{
    return d->videoSink;
}

/*!
    Returns the current video frame.
 */
QVideoFrame DVideoSink::videoFrame() const
{
//    return d->videoSink->currentVideoFrame();
}

/*!
    Sets the current video \a frame.
*/
void DVideoSink::setVideoFrame(const QVideoFrame &frame)
{
//    d->videoSink->setVideoFrame(frame);
}

/*!
    Returns the current subtitle text.
*/
QString DVideoSink::subtitleText() const
{
//    return d->videoSink->subtitleText();
}

/*!
    Sets the current \a subtitle text.
*/
void DVideoSink::setSubtitleText(const QString &subtitle)
{
//    d->videoSink->setSubtitleText(subtitle);
}

/*!
    Returns the size of the video currently being played back. If no video is
    being played, this method returns an invalid size.
 */
QSize DVideoSink::videoSize() const
{
//    return d->videoSink ? d->videoSink->nativeSize() : QSize{};
    return  QSize();
}

void DVideoSink::setSource(QObject *source)
{
    if (d->source == source)
        return;
    if (source)
        d->unregisterSource();
    d->source = source;
}

QT_END_NAMESPACE

//#include "moc_qvideosink.cpp"


