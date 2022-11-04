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
    return d->m_pPlayer;
}

QVideoWidget *DMediaPlayer::getVideoOutput()
{
    Q_D(DMediaPlayer);
    return d->m_videoWidget;
}

QMediaPlaylist *DMediaPlayer::getPlaylist()
{
    Q_D(DMediaPlayer);
    return d->m_pPlayList;
}

void DMediaPlayer::play()
{
    Q_D(DMediaPlayer);
    if(!d->m_pPlayer || !d->m_pPlayList || d->m_pPlayList->isEmpty()) return;
    QUrl url = d->m_pPlayList->currentMedia().canonicalUrl();
    if(url.isEmpty()) {
        d->m_pPlayList->setCurrentIndex(0);
        url = d->m_pPlayList->media(0).canonicalUrl();
    }
    if((d->m_pPlayer->media() != url) || d->m_pPlayer->state() == QMediaPlayer::StoppedState) {
        d->m_pPlayer->setMedia(url, nullptr);
        d->m_pPlayer->play();
    }
    else if(d->m_pPlayer->state() == QMediaPlayer::PausedState) {
        pause();
    }
}

void DMediaPlayer::pause()
{
    Q_D(DMediaPlayer);
    if(!d->m_pPlayer) return;
    d->m_pPlayer->pause();
}

void DMediaPlayer::stop()
{
    Q_D(DMediaPlayer);
    if(!d->m_pPlayer) return;
    d->m_pPlayer->stop();
}

void DMediaPlayer::setPosition(qint64 position)
{
    Q_D(DMediaPlayer);
    if(!d->m_pPlayer) return;
    d->m_pPlayer->setPosition(position);
}

void DMediaPlayer::setVolume(int volume)
{
    Q_D(DMediaPlayer);
    if(!d->m_pPlayer) return;
    d->m_pPlayer->setVolume(volume);
    //    QMediaPlayer::setVolume(volume);
}

void DMediaPlayer::setMuted(bool muted)
{
    Q_D(DMediaPlayer);
    if(!d->m_pPlayer) return;
    d->m_pPlayer->setMuted(muted);
}

void DMediaPlayer::setPlaybackRate(qreal rate)
{
    Q_D(DMediaPlayer);
    if(!d->m_pPlayer) return;
    d->m_pPlayer->setPlaybackRate(rate);
}

void DMediaPlayer::setMedia(const QMediaContent &media, QIODevice *stream)
{
    QMediaPlayer::setMedia(media, stream);
}

void DMediaPlayer::setPlaylist(QMediaPlaylist *playlist)
{
    Q_D(DMediaPlayer);
    d->m_pPlayList = playlist;
    connect(playlist, &QMediaPlaylist::currentIndexChanged, [=]() {
        if(!d->m_pPlayer) return;
        if(d->m_pPlayer->state() == QMediaPlayer::PlayingState) {
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
    d->m_videoWidget = videoWidget;
}

void DMediaPlayer::setPlayer(DPlatformMediaPlayer *player)
{
    Q_D(DMediaPlayer);
    d->m_pPlayer = player;
}
