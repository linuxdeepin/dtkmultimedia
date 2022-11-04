// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dplayerbackend_p.h"

DMULTIMEDIA_USE_NAMESPACE
DPlayerBackend::DebugLevel DPlayerBackendPrivate::debugLevel = DPlayerBackend::DebugLevel::Info;
DPlayerBackend::DPlayerBackend(QObject *parent)
    : QObject(parent), d_ptr(new DPlayerBackendPrivate(this))
{
}

DPlayerBackend::~DPlayerBackend()
{
}

void DPlayerBackend::setPlayFile(const QUrl &url)
{
    Q_D(DPlayerBackend);
    d->file = url;
}
void DPlayerBackend::setDVDDevice(const QString &path)
{
    Q_D(DPlayerBackend);
    d->dvdDevice = path;
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
    Q_D(DPlayerBackend);
    return d->state == PlayState::Paused;
}

DPlayerBackend::PlayState DPlayerBackend::state() const
{
    Q_D(const DPlayerBackend);
    return d->state;
}

void DPlayerBackend::setState(PlayState state)
{
    Q_D(DPlayerBackend);
    d->state = state;
}

void DPlayerBackend::setDebugLevel(DebugLevel lvl)
{
    DPlayerBackendPrivate::debugLevel = lvl;
}

DPlayerBackend::DebugLevel DPlayerBackend::debugLevel()
{
    return DPlayerBackendPrivate::debugLevel;
}

QUrl DPlayerBackend::urlFile() const
{
    Q_D(const DPlayerBackend);
    return d->file;
}

void DPlayerBackend::setWinID(const qint64 &winID)
{
    Q_D(DPlayerBackend);
    d->winId = winID;
}

qint64 DPlayerBackend::winID() const
{
    Q_D(const DPlayerBackend);
    return d->winId;
}
