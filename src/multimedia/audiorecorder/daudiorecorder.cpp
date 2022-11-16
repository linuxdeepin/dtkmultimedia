// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daudiorecorder_p.h"

DMULTIMEDIA_USE_NAMESPACE

DAudioRecorder::DAudioRecorder(QObject *parent)
    : QAudioRecorder(parent), d_ptr(new DAudioRecorderPrivate(this))
{
    Q_D(DAudioRecorder);
    d->encoderInterface = new DAudioEncoderInterface;
}

DAudioRecorder::~DAudioRecorder()
{
    Q_D(DAudioRecorder);
    if (d->encoderInterface)
        delete d->encoderInterface;
}

QString DAudioRecorder::codec() const
{
    Q_D(const DAudioRecorder);
    return d->encoderInterface->codec();
}

void DAudioRecorder::setCodec(const QString &codec)
{
    Q_D(DAudioRecorder);
    d->encoderInterface->setCodec(codec);
}

int DAudioRecorder::bitRate() const
{
    Q_D(const DAudioRecorder);
    return d->encoderInterface->bitRate();
}

void DAudioRecorder::setBitRate(int bitrate)
{
    Q_D(DAudioRecorder);
    d->encoderInterface->setBitRate(bitrate);
}

int DAudioRecorder::channelCount() const
{
    Q_D(const DAudioRecorder);
    return d->encoderInterface->channelCount();
}

void DAudioRecorder::setChannelCount(int channels)
{
    Q_D(DAudioRecorder);
    d->encoderInterface->setChannelCount(channels);
}

int DAudioRecorder::sampleRate() const
{
    Q_D(const DAudioRecorder);
    return d->encoderInterface->sampleRate();
}

void DAudioRecorder::setSampleRate(int rate)
{
    Q_D(DAudioRecorder);
    d->encoderInterface->setSampleRate(rate);
}

QString DAudioRecorder::audioInput() const
{
    Q_D(const DAudioRecorder);
    return d->encoderInterface->audioInput();
}

void DAudioRecorder::setAudioInput(const QString &name)
{
    Q_D(DAudioRecorder);
    d->encoderInterface->setAudioInput(name);
}

QUrl DAudioRecorder::outputLocation() const
{
    Q_D(const DAudioRecorder);
    return d->encoderInterface->outputLocation();
}

bool DAudioRecorder::setOutputLocation(const QUrl &location)
{
    Q_D(DAudioRecorder);
    return d->encoderInterface->setOutputLocation(location);
}

QMediaRecorder::State DAudioRecorder::state() const
{
    Q_D(const DAudioRecorder);
    return d->encoderInterface->state();
}

void DAudioRecorder::record()
{
    Q_D(DAudioRecorder);
    d->encoderInterface->startEncode();

    //QAudioRecorder::record();
}

void DAudioRecorder::pause()
{
    Q_D(DAudioRecorder);
    d->encoderInterface->pauseEncode();

    //QAudioRecorder::pause();
}

void DAudioRecorder::stop()
{
    Q_D(DAudioRecorder);
    d->encoderInterface->stopEncode();

    //QAudioRecorder::stop();
}
