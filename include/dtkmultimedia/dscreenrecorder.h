// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSCREENRECORDER_H
#define DSCREENRECORDER_H

#include <QUrl>
#include <QMediaRecorder>
#include <dtkmultimedia.h>
DMULTIMEDIA_BEGIN_NAMESPACE

typedef void (*VideoStreamCallback)(void *obj, int dataLength, unsigned char *data);
class DScreenRecorderPrivate;

class Q_MULTIMEDIA_EXPORT DScreenRecorder : public QMediaRecorder
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DScreenRecorder)

#ifdef BUILD_Qt6
typedef RecorderState State ;
#endif

public:
    enum VCodecID {
        CODEC_ID_NO,
        CODEC_ID_H264,
        CODEC_ID_MPEG4,
    };

    enum PixFormatID {
        PIX_FMT_YUV420P,
        PIX_FMT_YUYV422,
        PIX_FMT_RGB24,
        PIX_FMT_BGR24,
        PIX_FMT_ARGB,
        PIX_FMT_RGBA,
    };

public:
    DScreenRecorder(QObject *parent = nullptr);
    ~DScreenRecorder();

    VCodecID codec() const;
    void setCodec(const VCodecID &codec);

    PixFormatID pixfmt() const;
    void setPixfmt(const PixFormatID pixfmt);

    int bitRate() const;
    void setBitRate(int bitrate);

    int frameRate() const;
    void setFrameRate(int rate);

    void setResolution(const int width, const int height);
    QSize resolution() const;

    QUrl outputLocation() const;
    bool setOutputLocation(const QUrl &location);

    void setStreamAcceptFunc(VideoStreamCallback function, void *obj);

    QPoint topLeft() const;
    void setTopLeft(const int x, const int y);

#if BUILD_Qt6
    State state() const;
#else
    State state() const;
#endif

Q_SIGNALS:
    void screenStreamData(QImage);
public Q_SLOTS:
    void record();
    void stop();

protected:
    QScopedPointer<DScreenRecorderPrivate> d_ptr;
};

DMULTIMEDIA_END_NAMESPACE

#endif   // DSCREENRECORDER_H
