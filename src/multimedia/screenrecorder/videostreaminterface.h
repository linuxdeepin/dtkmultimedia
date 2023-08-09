// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VIDEOSTREAMINTERFACE_H
#define VIDEOSTREAMINTERFACE_H

#include <DScreenRecorder>
#include <QPoint>
#include <thread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>
#include <libavdevice/avdevice.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

DMULTIMEDIA_BEGIN_NAMESPACE

DMULTIMEDIA_USE_NAMESPACE

#ifdef BUILD_Qt6
typedef QMediaRecorder::RecorderState State ;
#endif
class VideoStreamInterface : public QObject
{
    Q_OBJECT
public:
    enum StreamReceiveStyle {
        SignalReceive,
        CallbackReceive,
        Encoding,
    };

public:
    explicit VideoStreamInterface(QObject *parent = nullptr);
    ~VideoStreamInterface();

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

#if BUILD_Qt6
    //    enum State
    //    {
    //        StoppedState,
    //        RecordingState,
    //        PausedState
    //    };
    State state() const;
#else
    QMediaRecorder::State state() const;
#endif

Q_SIGNALS:
    void screenStreamData(QImage);

public:
    virtual void record() = 0;
    virtual void stop() = 0;

protected:
    void propertyRevise();

protected:
    int bitrate = { 1800000 };
    int framerate { 12 };
    QPoint topLeftP { 0, 0 };
    QSize resolutionSize { 800, 600 };
    QUrl outFilePath { "" };
    AVCodecID codecId { AV_CODEC_ID_NONE };
    AVPixelFormat pixFormat { AV_PIX_FMT_RGBA };

#if BUILD_Qt6
    QMediaRecorder::RecorderState stateValue { QMediaRecorder::StoppedState };
#else
    QMediaRecorder::State stateValue { QMediaRecorder::StoppedState };
#endif
    std::thread *videoRecorderThread { nullptr };
    std::atomic_bool isRecording { false };
    std::atomic_bool isPause { false };

    bool isLoadFunction { false };
    bool isOpenInputVideoCtx { false };
    bool isOpenOutputVideoCtx { false };

    VideoStreamCallback sendDataFunc = { nullptr };
    void *sendDataObj = { nullptr };

    StreamReceiveStyle recStyle = { SignalReceive };
};

DMULTIMEDIA_END_NAMESPACE

#endif   // VIDEOSTREAMINTERFACE_H
