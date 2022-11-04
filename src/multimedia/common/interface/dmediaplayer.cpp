// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dmediaplayer_p.h"
#include "dmediaplaylist.h"
#include "dplatformmediaplayer.h"

DMULTIMEDIA_USE_NAMESPACE

DMediaPlayer::DMediaPlayer(QObject *parent)
    : QMediaPlayer(parent), d_ptr(new DMediaPlayerPrivate(this))
{
}

DMediaPlayer::~DMediaPlayer()
{
}

DPlatformMediaPlayer *DMediaPlayer::getPlayer()
{
    Q_D(DMediaPlayer);
    return d->pPlayer;
}

QVideoWidget *DMediaPlayer::getVideoOutput()
{
    Q_D(DMediaPlayer);
    return d->videoWidget;
}

QMediaPlaylist *DMediaPlayer::getPlaylist()
{
    Q_D(DMediaPlayer);
    return d->pPlayList;
}

void DMediaPlayer::play()
{
    Q_D(DMediaPlayer);
    if (!d->pPlayer || !d->pPlayList || d->pPlayList->isEmpty()) return;
    QUrl url = d->pPlayList->currentMedia().canonicalUrl();
    if (url.isEmpty()) {
        d->pPlayList->setCurrentIndex(0);
        url = d->pPlayList->media(0).canonicalUrl();
    }
    if ((d->pPlayer->media() != url) || d->pPlayer->state() == QMediaPlayer::StoppedState) {
        d->pPlayer->setMedia(url, nullptr);
        d->pPlayer->play();
    } else if (d->pPlayer->state() == QMediaPlayer::PausedState) {
        pause();
    }
}

void DMediaPlayer::pause()
{
    Q_D(DMediaPlayer);
    if (!d->pPlayer) return;
    d->pPlayer->pause();
}

void DMediaPlayer::stop()
{
    Q_D(DMediaPlayer);
    if (!d->pPlayer) return;
    d->pPlayer->stop();
}

void DMediaPlayer::setPosition(qint64 position)
{
    Q_D(DMediaPlayer);
    if (!d->pPlayer) return;
    d->pPlayer->setPosition(position);
}

void DMediaPlayer::setVolume(int volume)
{
    Q_D(DMediaPlayer);
    if (!d->pPlayer) return;
    d->pPlayer->setVolume(volume);
}

void DMediaPlayer::setMuted(bool muted)
{
    Q_D(DMediaPlayer);
    if (!d->pPlayer) return;
    d->pPlayer->setMuted(muted);
}

void DMediaPlayer::setPlaybackRate(qreal rate)
{
    Q_D(DMediaPlayer);
    if (!d->pPlayer) return;
    d->pPlayer->setPlaybackRate(rate);
}

void DMediaPlayer::setMedia(const QMediaContent &media, QIODevice *stream)
{
    QMediaPlayer::setMedia(media, stream);
}

void DMediaPlayer::setPlaylist(QMediaPlaylist *playlist)
{
    Q_D(DMediaPlayer);
    d->pPlayList = playlist;
    connect(playlist, &QMediaPlaylist::currentIndexChanged, [=]() {
        if (!d->pPlayer) return;
        if (d->pPlayer->state() == QMediaPlayer::PlayingState) {
            play();
        }
    });
}

void DMediaPlayer::setNetworkConfigurations(const QList<QNetworkConfiguration> &configurations)
{
    QMediaPlayer::setNetworkConfigurations(configurations);
}

void DMediaPlayer::setVideoOutput(QVideoWidget *videoWidget)
{
    Q_D(DMediaPlayer);
    d->videoWidget = videoWidget;
}

void DMediaPlayer::setPlayer(DPlatformMediaPlayer *player)
{
    Q_D(DMediaPlayer);
    d->pPlayer = player;
}
