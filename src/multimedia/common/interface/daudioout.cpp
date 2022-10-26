// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daudioout.h"



DMULTIMEDIA_USE_NAMESPACE



DAudioOutput::DAudioOutput(QObject *parent)
    :QMediaObject(parent, NULL)
{

}

DAudioOutput::~DAudioOutput()
{

}

void DAudioOutput::setDevice(const DAudioDevice &device)
{
    m_audioDevice = device;
}

void DAudioOutput::setVolume(float volume)
{
    m_fVolume = volume;
    volumeChanged(volume);
}

void DAudioOutput::setMuted(bool muted)
{
    m_bMuted = muted;
    mutedChanged(muted);
}


DAudioDevice DAudioOutput::device() const
{
    return m_audioDevice;
}

float DAudioOutput::volume() const
{
    return m_fVolume;
}

bool DAudioOutput::isMuted() const
{
    return m_bMuted;
}
