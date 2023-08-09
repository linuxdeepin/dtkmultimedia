// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "videostreaminterface.h"
#include <QImage>

DMULTIMEDIA_USE_NAMESPACE
VideoStreamInterface::VideoStreamInterface(QObject *parent)
    : QObject(parent)
{
}

VideoStreamInterface::~VideoStreamInterface()
{
}

DScreenRecorder::VCodecID VideoStreamInterface::codec() const
{
    return DScreenRecorder::VCodecID::CODEC_ID_MPEG4;
}

void VideoStreamInterface::setCodec(const DScreenRecorder::VCodecID &codec)
{
    switch (codec) {
    case DScreenRecorder::CODEC_ID_NO: {
        codecId = AV_CODEC_ID_NONE;
        break;
    }
    case DScreenRecorder::CODEC_ID_H264: {
        codecId = AV_CODEC_ID_H264;
        break;
    }
    case DScreenRecorder::CODEC_ID_MPEG4: {
        codecId = AV_CODEC_ID_MPEG4;
        break;
    }
    }
}

DScreenRecorder::PixFormatID VideoStreamInterface::pixfmt() const
{
    return DScreenRecorder::PixFormatID::PIX_FMT_ARGB;
}

void VideoStreamInterface::setPixfmt(const DScreenRecorder::PixFormatID pixfmt)
{
    switch (pixfmt) {
    case DScreenRecorder::PIX_FMT_YUV420P: {
        pixFormat = AV_PIX_FMT_YUV420P;
        break;
    }
    case DScreenRecorder::PIX_FMT_YUYV422: {
        pixFormat = AV_PIX_FMT_YUYV422;
        break;
    }
    case DScreenRecorder::PIX_FMT_BGR24: {
        pixFormat = AV_PIX_FMT_BGR24;
        break;
    }
    case DScreenRecorder::PIX_FMT_RGB24: {
        pixFormat = AV_PIX_FMT_RGB24;
        break;
    }
    case DScreenRecorder::PIX_FMT_ARGB: {
        pixFormat = AV_PIX_FMT_ARGB;
        break;
    }
    case DScreenRecorder::PIX_FMT_RGBA: {
        pixFormat = AV_PIX_FMT_RGBA;
        break;
    }
    }
}

int VideoStreamInterface::bitRate() const
{
    return bitrate;
}

void VideoStreamInterface::setBitRate(int bitrate)
{
    this->bitrate = bitrate;
}

int VideoStreamInterface::frameRate() const
{
    return framerate;
}

void VideoStreamInterface::setFrameRate(int rate)
{
    this->framerate = rate;
}

QPoint VideoStreamInterface::topLeft() const
{
    return topLeftP;
}

void VideoStreamInterface::setTopLeft(const int x, const int y)
{
    topLeftP.setX(x);
    topLeftP.setY(y);
}

QUrl VideoStreamInterface::outputLocation() const
{
    return outFilePath;
}

bool VideoStreamInterface::setOutputLocation(const QUrl &location)
{
    outFilePath = location;
}

void VideoStreamInterface::setResolution(const int width, const int height)
{
    resolutionSize.setWidth(width);
    resolutionSize.setHeight(height);
}

QSize VideoStreamInterface::resolution() const
{
    return resolutionSize;
}

void VideoStreamInterface::setStreamAcceptFunc(VideoStreamCallback function, void *obj)
{
    sendDataFunc = function;
    sendDataObj = obj;
}

#if BUILD_Qt6
State VideoStreamInterface::state() const
{
    return stateValue;
}
#else
QMediaRecorder::State VideoStreamInterface::state() const
{
    return stateValue;
}
#endif

void VideoStreamInterface::propertyRevise()
{
    if (outFilePath.isValid() && codecId != AV_CODEC_ID_NONE) {
        recStyle = Encoding;
    } else if (sendDataFunc != nullptr) {
        recStyle = CallbackReceive;
    } else {
        recStyle = SignalReceive;
    }
}
