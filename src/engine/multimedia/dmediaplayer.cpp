// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dmediaplayer.h"
#include <dengineplayer.h>
#include <dmediaplaylist.h>
#include <dvideowidget.h>
#include <player_widget.h>
#include <player_engine.h>
using namespace dmr;







DMediaPlayer::DMediaPlayer(QObject *parent)
    :QMediaPlayer(parent)
{
    m_pPlayer = new DEnginePlayer(this);
    connect(m_pPlayer, &DEnginePlayer::durationChanged, this, &DMediaPlayer::durationChanged);
    connect(m_pPlayer, &DEnginePlayer::positionChanged, this, &DMediaPlayer::positionChanged);
    connect(m_pPlayer, &DEnginePlayer::audioAvailableChanged, this, &QMediaPlayer::audioAvailableChanged);
    connect(m_pPlayer, &DEnginePlayer::videoAvailableChanged, this, &QMediaPlayer::videoAvailableChanged);
    connect(m_pPlayer, &DEnginePlayer::volumeChanged, this, &QMediaPlayer::volumeChanged);
    connect(m_pPlayer, &DEnginePlayer::mutedChanged, this, &QMediaPlayer::mutedChanged);
    connect(m_pPlayer, &DEnginePlayer::seekableChanged, this, &QMediaPlayer::seekableChanged);
    connect(m_pPlayer, &DEnginePlayer::playbackRateChanged, this, &QMediaPlayer::playbackRateChanged);
    connect(m_pPlayer, &DEnginePlayer::stateChanged, this, &QMediaPlayer::stateChanged);
//    connect(m_pPlayer, &DEnginePlayer::bufferStatusChanged, this, &QMediaPlayer::bufferStatusChanged);
}

DMediaPlayer::~DMediaPlayer()
{

}

void DMediaPlayer::play()
{
    QUrl url = QMediaPlayer::currentMedia().canonicalUrl();
    if((m_pPlayer->media() != url) ||
            m_pPlayer->state() == QMediaPlayer::StoppedState) {
        m_pPlayer->setMedia(url, nullptr);
        m_pPlayer->play();
    } else if(m_pPlayer->state() == QMediaPlayer::PausedState) {
        pause();
    }

//    QMediaPlayer::play();
}

void DMediaPlayer::pause()
{
    m_pPlayer->pause();
//    QMediaPlayer::pause();
}

void DMediaPlayer::stop()
{
    m_pPlayer->stop();
//    QMediaPlayer::stop();
}

void DMediaPlayer::setPosition(qint64 position)
{
    m_pPlayer->setPosition(position);
//    QMediaPlayer::setPosition(position);
}

void DMediaPlayer::setVolume(int volume)
{
    m_pPlayer->setVolume(volume);
//    QMediaPlayer::setVolume(volume);
}

void DMediaPlayer::setMuted(bool muted)
{
    m_pPlayer->setMuted(muted);
//    QMediaPlayer::setMuted(muted);
}

void DMediaPlayer::setPlaybackRate(qreal rate)
{
    m_pPlayer->setPlaybackRate(rate);
    QMediaPlayer::setPlaybackRate(rate);
}

void DMediaPlayer::setMedia(const QMediaContent &media, QIODevice *stream)
{
    QMediaPlayer::setMedia(media, stream);
}

void DMediaPlayer::setPlaylist(DMediaPlaylist *playlist)
{
    QMediaPlayer::setPlaylist(playlist);
}

void DMediaPlayer::setNetworkConfigurations(const QList<QNetworkConfiguration> &configurations)
{
    QMediaPlayer::setNetworkConfigurations(configurations);
}

void DMediaPlayer::setVideoOutput(QVideoWidget *videoWidget)
{
    m_videoWidget = videoWidget;
    QWidget *player = dynamic_cast<DVideoWidget *>(videoWidget)->getPlayer();
    dynamic_cast<DEnginePlayer *>(m_pPlayer)->setPlayer(player);
//    PlayerEngine *engine = &dynamic_cast<PlayerWidget *>(player)->engine();
//    QObject::connect(engine, &PlayerEngine::updateDuration, this, &DMediaPlayer::durationChanged);

//    QMediaPlayer::setVideoOutput(videoWidget);
}
