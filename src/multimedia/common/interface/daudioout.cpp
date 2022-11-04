// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daudioout_p.h"



DMULTIMEDIA_USE_NAMESPACE



DAudioOutput::DAudioOutput(QObject *parent)
    : QMediaObject(parent, NULL), d_ptr(new DAudioOutputPrivate(this))
{
}

DAudioOutput::~DAudioOutput()
{
}

void DAudioOutput::setDevice(const DAudioDevice &device)
{
    Q_D(DAudioOutput);
    d->m_audioDevice = device;
}

void DAudioOutput::setVolume(float volume)
{
    Q_D(DAudioOutput);
    d->m_fVolume = volume;
    volumeChanged(volume);
}

void DAudioOutput::setMuted(bool muted)
{
    Q_D(DAudioOutput);
    d->m_bMuted = muted;
    mutedChanged(muted);
}


DAudioDevice DAudioOutput::device() const
{
    Q_D(const DAudioOutput);
    return d->m_audioDevice;
}

float DAudioOutput::volume() const
{
    Q_D(const DAudioOutput);
    return d->m_fVolume;
}

bool DAudioOutput::isMuted() const
{
    Q_D(const DAudioOutput);
    return d->m_bMuted;
}
