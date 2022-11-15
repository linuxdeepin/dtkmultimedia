// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daudioencoderinterface_p.h"

#include <QDir>
#include <QDebug>
#include <QLibrary>
#include <QLibraryInfo>

DMULTIMEDIA_USE_NAMESPACE

DAudioEncoderInterface::DAudioEncoderInterface(QObject *parent)
    : QObject(parent), d_ptr(new DAudioEncoderInterfacePrivate(this))
{
}

DAudioEncoderInterface::~DAudioEncoderInterface()
{
    // TODO 关闭输入输出音频流
}

int DAudioEncoderInterface::bitRate() const
{
    Q_D(const DAudioEncoderInterface);
    return d->bitRate;
}

void DAudioEncoderInterface::setBitRate(int bitrate)
{
    Q_D(DAudioEncoderInterface);
    d->bitRate = bitrate;
}

int DAudioEncoderInterface::channelCount() const
{
    Q_D(const DAudioEncoderInterface);
    return d->channels;
}

void DAudioEncoderInterface::setChannelCount(int channels)
{
    Q_D(DAudioEncoderInterface);
    d->channels = channels;
}

int DAudioEncoderInterface::sampleRate() const
{
    Q_D(const DAudioEncoderInterface);
    return d->sampleRate;
}

void DAudioEncoderInterface::setSampleRate(int rate)
{
    Q_D(DAudioEncoderInterface);
    d->sampleRate = rate;
}

QString DAudioEncoderInterface::audioInput() const
{
    Q_D(const DAudioEncoderInterface);
    return d->deviceName;
}

void DAudioEncoderInterface::setAudioInput(const QString &name)
{
    Q_D(DAudioEncoderInterface);
    d->deviceName = name;
}

QUrl DAudioEncoderInterface::outputLocation() const
{
    Q_D(const DAudioEncoderInterface);
    return d->outFilePath;
}

bool DAudioEncoderInterface::setOutputLocation(const QUrl &location)
{
    Q_D(DAudioEncoderInterface);
    d->outFilePath = location;
    return true;
}

QString DAudioEncoderInterface::codec() const
{
    Q_D(const DAudioEncoderInterface);
    return d->codec;
}

void DAudioEncoderInterface::setCodec(const QString &codec)
{
    Q_D(DAudioEncoderInterface);
    d->codec = codec;
}

bool DAudioEncoderInterface::openInputAudioCtx()
{
    // TODO 打开音频输入流信息，及编码信息

    return true;
}

bool DAudioEncoderInterface::openOutputAudioCtx()
{
    // TODO 打开音频输出流信息
    return true;
}

void DAudioEncoderInterface::startEncode()
{
    // TODO 开始录音
}

void DAudioEncoderInterface::stopEncode()
{
    // TODO 停止录音
}

void DAudioEncoderInterface::pauseEncode()
{
    // TODO 暂停录音
}

void DAudioEncoderInterface::encodeWork()
{
    // TODO 录音
}

bool DAudioEncoderInterface::loadFunction()
{
    // TODO 加载FFmpeg相关接口

    return true;
}
