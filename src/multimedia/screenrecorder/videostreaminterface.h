// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VIDEOSTREAMINTERFACE_H
#define VIDEOSTREAMINTERFACE_H

#include <DScreenRecorder>

#include <QObject>
#include <QPoint>

#include <thread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libavdevice/avdevice.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

DMULTIMEDIA_BEGIN_NAMESPACE

DMULTIMEDIA_USE_NAMESPACE
class VideoStreamInterface : public QObject
{
    Q_OBJECT
public:
    explicit VideoStreamInterface(QObject *parent = nullptr);

    DScreenRecorder::VCodecID codec() const;
    void setCodec(const DScreenRecorder::VCodecID &codec);

    DScreenRecorder::PixFormatID pixfmt() const;
    void setPixfmt(const DScreenRecorder::PixFormatID pixfmt);

    int bitRate() const;
    void setBitRate(int bitrate);

    int frameRate() const;
    void setFrameRate(int rate);

    QPoint topLeft() const;
    void setTopLeft(const int x, const int y);

    QUrl outputLocation() const;
    bool setOutputLocation(const QUrl &location);

    void setResolution(const int width, const int height);
    QSize resolution() const;

    void setStreamAcceptFunc(VideoStreamCallback function, void *obj);

public:
    virtual void record() = 0;
    virtual void stop() = 0;

protected:
    int bitrate = { 1800000 };
    int framerate { 24 };
    QPoint topLeftP { 0, 0 };
    QSize resolutionSize { 1280, 1024 };
    QUrl outFilePath { "outVideoFile.mp4" };
    AVCodecID codecId { AV_CODEC_ID_MPEG4 };
    AVPixelFormat pixFormat { AV_PIX_FMT_ARGB };

    std::thread *videoRecorderThread { nullptr };
    std::atomic_bool isRecording { false };
    std::atomic_bool isPause { false };

    bool isLoadFunction { false };
    bool isOpenInputVideoCtx { false };
    bool isOpenOutputVideoCtx { false };

    VideoStreamCallback sendDataFunc = { nullptr };
    void *sendDataObj = { nullptr };
};

DMULTIMEDIA_END_NAMESPACE

#endif   // VIDEOSTREAMINTERFACE_H
