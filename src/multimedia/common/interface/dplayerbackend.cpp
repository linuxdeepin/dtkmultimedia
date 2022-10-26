// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dplayerbackend.h"

DMULTIMEDIA_USE_NAMESPACE
DPlayerBackend::DebugLevel DPlayerBackend::m_debugLevel = DPlayerBackend::DebugLevel::Info;
void DPlayerBackend::setPlayFile(const QUrl &url)
{
    m_file = url;
}
void DPlayerBackend::setDVDDevice(const QString &path)
{
    m_dvdDevice = path;
}

qint64 DPlayerBackend::duration() const
{
    return 0;
}

qint64 DPlayerBackend::elapsed() const
{
    return 0;
}

bool DPlayerBackend::paused()
{
    return m_state == PlayState::Paused;
}

DPlayerBackend::PlayState DPlayerBackend::state() const
{
    return m_state;
}

void DPlayerBackend::setDebugLevel(DebugLevel lvl)
{
    m_debugLevel = lvl;
}

void DPlayerBackend::setWinID(const qint64 &winID) 
{
    m_winId = winID;
}

