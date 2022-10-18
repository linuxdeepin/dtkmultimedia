// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dplatformmediaplayer.h"
#include "qmediaplayer.h"

QT_BEGIN_NAMESPACE


/*!
    \class QPlatformMediaPlayer
    \internal

    \brief The QPlatformMediaPlayer class provides access to the media playing
    functionality.

    This control provides a means to set the \l {setMedia()}{media} to play,
    \l {play()}{start}, \l {pause()} {pause} and \l {stop()}{stop} playback,
    \l {setPosition()}{seek}, and control the \l {setVolume()}{volume}.
    It also provides feedback on the \l {duration()}{duration} of the media,
    the current \l {position()}{position}, and \l {bufferProgress()}{buffering}
    progress.

    The functionality provided by this control is exposed to application
    code through the QMediaPlayer class.

    \sa QMediaPlayer
*/

DPlatformMediaPlayer::~DPlatformMediaPlayer()
{
}

/*! \fn QPlatformMediaPlayer::QPlatformMediaPlayer(QMediaPlayer *parent)

    Constructs a new media player control with the given \a parent.
*/

/*!
    \fn QPlatformMediaPlayer::state() const

    Returns the state of a player control.
*/

/*!
    \fn QPlatformMediaPlayer::stateChanged(QMediaPlayer::State newState)

    Signals that the state of a player control has changed to \a newState.

    \sa state()
*/

void DPlatformMediaPlayer::stateChanged(QMediaPlayer::State newState)
{
    if (newState == m_state)
        return;
    m_state = newState;
//    player->d_func()->setState(newState);
    player->stateChanged(newState);
}

/*!
    \fn QPlatformMediaPlayer::mediaStatus() const

    Returns the status of the current media.
*/


/*!
    \fn QPlatformMediaPlayer::mediaStatusChanged(QMediaPlayer::MediaStatus status)

    Signals that the \a status of the current media has changed.

    \sa mediaStatus()
*/
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

QT_END_NAMESPACE
