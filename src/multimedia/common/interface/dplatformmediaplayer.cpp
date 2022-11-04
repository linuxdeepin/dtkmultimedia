// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dplatformmediaplayer_p.h"
#include "qmediaplayer.h"

DMULTIMEDIA_USE_NAMESPACE

DPlatformMediaPlayer::~DPlatformMediaPlayer()
{
}

DPlatformMediaPlayer::DPlatformMediaPlayer(QMediaPlayer *parent)
    : QObject(parent), d_ptr(new DPlatformMediaPlayerPrivate(this))
{
    Q_D(DPlatformMediaPlayer);
    d->m_player = parent;
}

void DPlatformMediaPlayer::stateChanged(QMediaPlayer::State newState)
{
    Q_D(DPlatformMediaPlayer);
    if(newState == d->m_state) return;
    d->m_state = newState;
    d->m_player->stateChanged(newState);
}

void DPlatformMediaPlayer::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    Q_D(DPlatformMediaPlayer);
    if(d->m_status == status) return;
    d->m_status = status;
}


QMediaPlayer::State DPlatformMediaPlayer::state() const
{
    Q_D(const DPlatformMediaPlayer);
    return d->m_state;
}

QMediaPlayer::MediaStatus DPlatformMediaPlayer::mediaStatus() const
{
    Q_D(const DPlatformMediaPlayer);
    return d->m_status;
}

bool DPlatformMediaPlayer::isAudioAvailable() const
{
    Q_D(const DPlatformMediaPlayer);
    return d->m_audioAvailable;
}

bool DPlatformMediaPlayer::isVideoAvailable() const
{
    Q_D(const DPlatformMediaPlayer);
    return d->m_videoAvailable;
}

bool DPlatformMediaPlayer::isSeekable() const
{
    Q_D(const DPlatformMediaPlayer);
    return d->m_seekable;
}

bool DPlatformMediaPlayer::streamPlaybackSupported() const
{
    return false;
}

void DPlatformMediaPlayer::setAudioOutput(QPlatformAudioOutput *)
{
}

DMediaMetaData DPlatformMediaPlayer::metaData() const
{
    return {};
}

int DPlatformMediaPlayer::trackCount(TrackType)
{
    return 0;
}

DMediaMetaData DPlatformMediaPlayer::trackMetaData(TrackType, int)
{
    return DMediaMetaData();
}

int DPlatformMediaPlayer::activeTrack(TrackType)
{
    return -1;
}

void DPlatformMediaPlayer::setActiveTrack(TrackType, int)
{
}

void DPlatformMediaPlayer::durationChanged(qint64 duration)
{
    Q_D(DPlatformMediaPlayer);
    d->m_player->durationChanged(duration);
}

void DPlatformMediaPlayer::positionChanged(qint64 position)
{
    Q_D(DPlatformMediaPlayer);
    d->m_player->positionChanged(position);
}

void DPlatformMediaPlayer::audioAvailableChanged(bool audioAvailable)
{
    Q_D(DPlatformMediaPlayer);
    if(d->m_audioAvailable == audioAvailable) return;
    d->m_audioAvailable = audioAvailable;
}

void DPlatformMediaPlayer::videoAvailableChanged(bool videoAvailable)
{
    Q_D(DPlatformMediaPlayer);
    if(d->m_videoAvailable == videoAvailable) return;
    d->m_videoAvailable = videoAvailable;
}

void DPlatformMediaPlayer::seekableChanged(bool seekable)
{
    Q_D(DPlatformMediaPlayer);
    if(d->m_seekable == seekable) return;
    d->m_seekable = seekable;
    d->m_player->seekableChanged(seekable);
}

void DPlatformMediaPlayer::volumeChanged(int volume)
{
    Q_D(DPlatformMediaPlayer);
    if(d->m_volume == volume) return;
    d->m_volume = volume;
    d->m_player->volumeChanged(volume);
}

void DPlatformMediaPlayer::mutedChanged(bool muted)
{
    Q_D(DPlatformMediaPlayer);
    if(d->m_muted == muted) return;
    d->m_muted = muted;
    d->m_player->mutedChanged(muted);
}

void DPlatformMediaPlayer::playbackRateChanged(qreal rate)
{
    Q_D(DPlatformMediaPlayer);
    d->m_player->playbackRateChanged(rate);
}

void DPlatformMediaPlayer::bufferProgressChanged(float progress)
{
}

void DPlatformMediaPlayer::metaDataChanged()
{
    Q_D(DPlatformMediaPlayer);
    d->m_player->metaDataChanged();
}

void DPlatformMediaPlayer::tracksChanged()
{
}

void DPlatformMediaPlayer::activeTracksChanged()
{
}


void DPlatformMediaPlayer::error(int error, const QString &errorString)
{
}

void DPlatformMediaPlayer::resetCurrentLoop()
{
    Q_D(DPlatformMediaPlayer);
    d->m_currentLoop = 0;
}

bool DPlatformMediaPlayer::doLoop()
{
    Q_D(DPlatformMediaPlayer);
    return isSeekable() && (d->m_loops < 0 || ++d->m_currentLoop < d->m_loops);
}

int DPlatformMediaPlayer::loops()
{
    Q_D(DPlatformMediaPlayer);
    return d->m_loops;
}

void DPlatformMediaPlayer::setLoops(int loops)
{
    Q_D(DPlatformMediaPlayer);
    if(d->m_loops == loops) return;
    d->m_loops = loops;
}
