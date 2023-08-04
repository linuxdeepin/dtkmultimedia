// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daudioout_p.h"

DMULTIMEDIA_USE_NAMESPACE

#if BUILD_Qt6
DAudioOutput::DAudioOutput(QObject *parent)
    : QMediaCaptureSession(parent), d_ptr(new DAudioOutputPrivate(this))
#else
DAudioOutput::DAudioOutput(QObject *parent)
    : QMediaObject(parent, NULL), d_ptr(new DAudioOutputPrivate(this))
#endif
{
}

DAudioOutput::~DAudioOutput()
{
}

void DAudioOutput::setDevice(const DAudioDevice &device)
{
    Q_D(DAudioOutput);
    d->audioDevice = device;
}

void DAudioOutput::setVolume(float volume)
{
    Q_D(DAudioOutput);
    d->fVolume = volume;
    volumeChanged(volume);
}

void DAudioOutput::setMuted(bool muted)
{
    Q_D(DAudioOutput);
    d->bMuted = muted;
    mutedChanged(muted);
}

DAudioDevice DAudioOutput::device() const
{
    Q_D(const DAudioOutput);
    return d->audioDevice;
}

float DAudioOutput::volume() const
{
    Q_D(const DAudioOutput);
    return d->fVolume;
}

bool DAudioOutput::isMuted() const
{
    Q_D(const DAudioOutput);
    return d->bMuted;
}
