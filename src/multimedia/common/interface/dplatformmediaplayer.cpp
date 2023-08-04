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
    d->player = parent;
}

#if BUILD_Qt6
void DPlatformMediaPlayer::playbackStateChanged(QMediaPlayer::PlaybackState newState)
{
    Q_D(DPlatformMediaPlayer);
    if (newState == d->playbackState) return;
    d->playbackState = newState;
    d->player->playbackStateChanged(newState);
}
#else
void DPlatformMediaPlayer::stateChanged(QMediaPlayer::State newState)
{
    Q_D(DPlatformMediaPlayer);
    if (newState == d->state) return;
    d->state = newState;
    d->player->stateChanged(newState);
}
#endif

void DPlatformMediaPlayer::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    Q_D(DPlatformMediaPlayer);
    if (d->status == status) return;
    d->status = status;
}

#if BUILD_Qt6
QMediaPlayer::PlaybackState DPlatformMediaPlayer::playbackState() const
{
    Q_D(const DPlatformMediaPlayer);
    return d->playbackState;
}
#else
QMediaPlayer::State DPlatformMediaPlayer::state() const
{
    Q_D(const DPlatformMediaPlayer);
    return d->state;
}
#endif

QMediaPlayer::MediaStatus DPlatformMediaPlayer::mediaStatus() const
{
    Q_D(const DPlatformMediaPlayer);
    return d->status;
}

bool DPlatformMediaPlayer::isAudioAvailable() const
{
    Q_D(const DPlatformMediaPlayer);
    return d->audioAvailable;
}

bool DPlatformMediaPlayer::isVideoAvailable() const
{
    Q_D(const DPlatformMediaPlayer);
    return d->videoAvailable;
}

bool DPlatformMediaPlayer::isSeekable() const
{
    Q_D(const DPlatformMediaPlayer);
    return d->seekable;
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
    d->player->durationChanged(duration);
}

void DPlatformMediaPlayer::positionChanged(qint64 position)
{
    Q_D(DPlatformMediaPlayer);
    d->player->positionChanged(position);
}

void DPlatformMediaPlayer::audioAvailableChanged(bool audioAvailable)
{
    Q_D(DPlatformMediaPlayer);
    if (d->audioAvailable == audioAvailable) return;
    d->audioAvailable = audioAvailable;
}

void DPlatformMediaPlayer::videoAvailableChanged(bool videoAvailable)
{
    Q_D(DPlatformMediaPlayer);
    if (d->videoAvailable == videoAvailable) return;
    d->videoAvailable = videoAvailable;
}

void DPlatformMediaPlayer::seekableChanged(bool seekable)
{
    Q_D(DPlatformMediaPlayer);
    if (d->seekable == seekable) return;
    d->seekable = seekable;
    d->player->seekableChanged(seekable);
}


void DPlatformMediaPlayer::volumeChanged(int volume)
{
#if BUILD_Qt6
//Qt6 删除了音量和静音设置，需要通过其他方式实现
#else
    Q_D(DPlatformMediaPlayer);
    if (d->volume == volume) return;
    d->volume = volume;
    d->player->volumeChanged(volume);
#endif
}

void DPlatformMediaPlayer::mutedChanged(bool muted)
{
#if BUILD_Qt6
//Qt6 删除了音量和静音设置，需要通过其他方式实现
#else
    Q_D(DPlatformMediaPlayer);
    if (d->muted == muted) return;
    d->muted = muted;
    d->player->mutedChanged(muted);
#endif
}

void DPlatformMediaPlayer::playbackRateChanged(qreal rate)
{
    Q_D(DPlatformMediaPlayer);
    d->player->playbackRateChanged(rate);
}

void DPlatformMediaPlayer::bufferProgressChanged(float progress)
{
}

void DPlatformMediaPlayer::metaDataChanged()
{
    Q_D(DPlatformMediaPlayer);
    d->player->metaDataChanged();
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
    d->currentLoop = 0;
}

bool DPlatformMediaPlayer::doLoop()
{
    Q_D(DPlatformMediaPlayer);
    return isSeekable() && (d->loops < 0 || ++d->currentLoop < d->loops);
}

int DPlatformMediaPlayer::loops()
{
    Q_D(DPlatformMediaPlayer);
    return d->loops;
}

void DPlatformMediaPlayer::setLoops(int loops)
{
    Q_D(DPlatformMediaPlayer);
    if (d->loops == loops) return;
    d->loops = loops;
}
