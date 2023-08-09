// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daudioinput_p.h"
#ifdef BUILD_Qt6
#include <QMediaDevices>
#else
#include <QAudioDeviceInfo>
#include <QAudioRecorder>
#endif
#include <QtGlobal>
#include <utility>

DMULTIMEDIA_USE_NAMESPACE

#ifdef BUILD_Qt6
DAudioInput::DAudioInput(QObject *parent)
    : QAudioInput(parent)
{

}

DAudioInput::DAudioInput(const QString &device, QObject *parent)
    : QAudioInput(parent), d_ptr(new DAudioInputPrivate(this))
#else
DAudioInput::DAudioInput(const QString &device, QObject *parent)
    : QObject(parent), d_ptr(new DAudioInputPrivate(this))
#endif
{
    setDevice(device);
}

DAudioInput::~DAudioInput()
{
}

float DAudioInput::volume() const
{
    Q_D(const DAudioInput);
    return d->volume;
}

void DAudioInput::setVolume(float volume)
{
    Q_D(DAudioInput);
    volume = qBound(0.f, volume, 1.f);
    if (d->volume == volume)
        return;
    d->volume = volume;
    emit volumeChanged(volume);
}

bool DAudioInput::isMuted() const
{
    Q_D(const DAudioInput);
    return d->muted;
}

void DAudioInput::setMuted(bool muted)
{
    Q_D(DAudioInput);
    if (d->muted == muted)
        return;
    d->muted = muted;
    emit mutedChanged(muted);
}

QString DAudioInput::device() const
{
    Q_D(const DAudioInput);
    return d->device;
}

void DAudioInput::setDevice(const QString &device)
{
    Q_D(DAudioInput);
    auto dev = device;
#ifndef BUILD_Qt6
    if (dev.isNull())
        dev = d->recorder.defaultAudioInput();
#endif
    if (d->device == dev)
        return;
    d->device = dev;
    emit deviceChanged();
}

#include "moc_daudioinput.cpp"
