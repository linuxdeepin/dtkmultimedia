// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "videostreaminterface.h"

DMULTIMEDIA_USE_NAMESPACE
VideoStreamInterface::VideoStreamInterface(QObject *parent)
    : QObject(parent)
{
}

DScreenRecorder::VCodecID VideoStreamInterface::codec() const
{
    return DScreenRecorder::VCodecID::CODEC_ID_MPEG4;
}

void VideoStreamInterface::setCodec(const DScreenRecorder::VCodecID &codec)
{
    codecId = AV_CODEC_ID_MPEG4;
}

DScreenRecorder::PixFormatID VideoStreamInterface::pixfmt() const
{
    return DScreenRecorder::PixFormatID::PIX_FMT_ARGB;
}

void VideoStreamInterface::setPixfmt(const DScreenRecorder::PixFormatID pixfmt)
{
    pixFormat = AV_PIX_FMT_BGR24;
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
