// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dplatformmediaplayer.h"
#include "qmediaplayer.h"

DMULTIMEDIA_USE_NAMESPACE

DPlatformMediaPlayer::~DPlatformMediaPlayer()
{
}


void DPlatformMediaPlayer::stateChanged(QMediaPlayer::State newState)
{
    if (newState == m_state)
        return;
    m_state = newState;
//    player->d_func()->setState(newState);
    m_player->stateChanged(newState);
}

void DPlatformMediaPlayer::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (m_status == status)
        return;
    m_status = status;
//    player->d_func()->setStatus(status);
}

void DPlatformMediaPlayer::error(int error, const QString &errorString)
{
//    player->d_func()->setError(error, errorString);
}

