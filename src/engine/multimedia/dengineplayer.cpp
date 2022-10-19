// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dengineplayer.h"
#include <player_widget.h>


DEnginePlayer::DEnginePlayer(QMediaPlayer *parent)
    :DPlatformMediaPlayer(parent)
{
    m_pPlayer = nullptr;
    m_audioOutput = nullptr;
}

DEnginePlayer::~DEnginePlayer()
{

}

qint64 DEnginePlayer::duration() const
{
    if(m_engine) {
        return m_engine->duration();
    }
    return 0;
}

qint64 DEnginePlayer::position() const
{
    if(m_engine) {
        return m_engine->elapsed();
    }
    return 0;
}

void DEnginePlayer::setPosition(qint64 position)
{
    if(m_engine) {
        m_engine->seekAbsolute(position/1000);
    }
}

float DEnginePlayer::bufferProgress() const
{
    return 0.f;
}

QMediaTimeRange DEnginePlayer::availablePlaybackRanges() const
{
    return QMediaTimeRange();
}

qreal DEnginePlayer::playbackRate() const
{
    return 0;
}

void DEnginePlayer::setPlaybackRate(qreal rate)
{
    if(m_engine) {
        m_engine->setPlaySpeed(rate);
    }
}

QUrl DEnginePlayer::media() const
{
    return m_media;
}

const QIODevice *DEnginePlayer::mediaStream() const
{

}

void DEnginePlayer::setMedia(const QUrl &media, QIODevice *stream)
{
    m_media = media;
}

void DEnginePlayer::setVolume(float volume)
{
    changeVolume(volume);
}

void DEnginePlayer::setMuted(bool muted)
{
    if(m_engine) {
        m_engine->setMute(muted);
        m_audioOutput->setMuted(muted);
    }
}

void DEnginePlayer::play()
{
    if(!m_engine || !m_pPlayer) return;
    PlayerWidget *player = dynamic_cast<PlayerWidget *>(m_pPlayer);
    if (m_engine->isPlayableFile(m_media))
        player->play(m_media);
}

void DEnginePlayer::pause()
{
    if(m_engine) {
        m_engine->pauseResume();
    }
}

void DEnginePlayer::stop()
{
    if(m_engine) {
        m_engine->stop();
    }
}

void DEnginePlayer::setVideoSink(DVideoSink *)
{

}

void DEnginePlayer::setPlayer(QWidget *Player)
{
    if(!Player) return;
    m_pPlayer = Player;
    PlayerEngine *engine = &dynamic_cast<PlayerWidget *>(m_pPlayer)->engine();
    m_audioOutput = new DAudioOutput(Player);
    m_engine = engine;
    connect(engine, &PlayerEngine::elapsedChanged, this, &DEnginePlayer::positionProxyChanged);
    connect(engine, &PlayerEngine::stateChanged, [=]() {
        switch(engine->state()) {
        case PlayerEngine::Playing:
            stateChanged(QMediaPlayer::PlayingState);
            break;
        case PlayerEngine::Idle:
            stateChanged(QMediaPlayer::StoppedState);
            break;
        case PlayerEngine::Paused:
            stateChanged(QMediaPlayer::PausedState);
            break;
        }
        connect(m_audioOutput, &DAudioOutput::volumeChanged, this,  &DEnginePlayer::volumeChanged);
        connect(m_audioOutput, &DAudioOutput::mutedChanged, this,  &DEnginePlayer::mutedChanged);
    });
    //    connect(m_pPlayer, &PlayerEngine::positionChanged, this, &DEnginePlayer::positionChanged);
}

void DEnginePlayer::setPlaySpeed(double times)
{
    if(m_engine) {
        m_engine->setPlaySpeed(times);
    }
}

void DEnginePlayer::changeSoundMode(const Backend::SoundMode &sm)
{
    if(m_engine) {
        m_engine->changeSoundMode(sm);
    }
}

void DEnginePlayer::nextFrame()
{
    if(m_engine) {
        m_engine->nextFrame();
    }
}

void DEnginePlayer::previousFrame()
{
    if(m_engine) {
        m_engine->previousFrame();
    }
}

void DEnginePlayer::setDecodeModel(const Backend::hwaccelMode &hwaccelMode)
{
    if(m_engine) {
        m_engine->changehwaccelMode(hwaccelMode);
    }
}

QImage DEnginePlayer::takeScreenshot()
{
    if(m_engine) {
        m_engine->takeScreenshot();
    }
}

void DEnginePlayer::burstScreenshot()
{
    if(m_engine) {
        m_engine->burstScreenshot();
    }
}

void DEnginePlayer::setVideoRotation(int degree)
{
    if(m_engine) {
        m_engine->setVideoRotation(degree);
    }
}

void DEnginePlayer::changeVolume(int val)
{
    if(m_engine) {
        m_engine->changeVolume(val);
        m_audioOutput->setVolume(val);
    }
}

void DEnginePlayer::setMute(bool bMute)
{
    if(m_engine) {
        m_engine->setMute(bMute);
        m_audioOutput->setMuted(bMute);
    }
}

void DEnginePlayer::seekAbsolute(int pos)
{
    if(m_engine) {
        m_engine->seekAbsolute(pos);
    }
}

void DEnginePlayer::setPlayMode(const PlaylistModel::PlayMode &pm)
{
    if(m_engine) {
        m_engine->playlist().setPlayMode(pm);
    }
}

void DEnginePlayer::playByName(const QUrl &url)
{
    if(m_engine) {
        m_engine->playByName(url);
    }
}

bool DEnginePlayer::loadSubtitle(const QFileInfo &fi)
{
    if(m_engine) {
        m_engine->loadSubtitle(fi);
    }
}

PlaylistModel *DEnginePlayer::getplaylist()
{
    if(m_engine) {
        m_engine->getplaylist();
    }
}

bool DEnginePlayer::addPlayFile(const QUrl &url)
{
    if(m_engine) {
        m_engine->addPlayFile(url);
    }
}

const MovieInfo &DEnginePlayer::movieInfo()
{
//    PlayerEngine *engine = &dynamic_cast<PlayerWidget *>(m_pPlayer)->engine();
//    if(engine) {
//        engine->movieInfo();
//    }
}

void DEnginePlayer::positionProxyChanged()
{
    if(!m_pPlayer) return;
    PlayerEngine *engine = &dynamic_cast<PlayerWidget *>(m_pPlayer)->engine();
    qint64 nElapsed = engine->elapsed() * 1000;
    positionChanged(nElapsed);
}

//void DEnginePlayer::updateDuration()
//{
//    PlayerEngine *engine = &dynamic_cast<PlayerWidget *>(m_pPlayer)->engine();
////    emit durationChanged(engine->duration());
//}
