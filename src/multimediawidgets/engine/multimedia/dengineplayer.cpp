// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dengineplayer_p.h"
#include <DMediaPlayer>
#include <player/dplaylistmodel.h>
#include <player/playerengine.h>
#include <playerwidget.h>
DMULTIMEDIA_USE_NAMESPACE

DEnginePlayer::DEnginePlayer(QMediaPlayer *parent) : DPlatformMediaPlayer(parent), d_ptr(new DEnginePlayerPrivate(this))
{
    Q_D(DEnginePlayer);
    d->m_mediaPlayer = parent;
    d->m_pPlayer     = nullptr;
    d->m_audioOutput = nullptr;
    if(d->m_mediaPlayer) {
        (static_cast<DMediaPlayer *>(d->m_mediaPlayer))->setPlayer(this);
    }
}

DEnginePlayer::~DEnginePlayer()
{
    Q_D(DEnginePlayer);

    if(d->m_audioOutput) {
        delete d->m_audioOutput;
        d->m_audioOutput = nullptr;
    }
}

qint64 DEnginePlayer::duration() const
{
    Q_D(const DEnginePlayer);
    if(d->m_engine) {
        return d->m_engine->duration();
    }
    return 0;
}

qint64 DEnginePlayer::position() const
{
    Q_D(const DEnginePlayer);
    if(d->m_engine) {
        return d->m_engine->elapsed();
    }
    return 0;
}

void DEnginePlayer::setPosition(qint64 position)
{
    Q_D(DEnginePlayer);
    if(d->m_engine) {
        d->m_engine->seekAbsolute(position / 1000);
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
    Q_D(DEnginePlayer);
    if(d->m_engine) {
        d->m_engine->setPlaySpeed(rate);
    }
}

QUrl DEnginePlayer::media() const
{
    Q_D(const DEnginePlayer);
    return d->m_media;
}

const QIODevice *DEnginePlayer::mediaStream() const
{
    // TODO
    return nullptr;
}

void DEnginePlayer::setMedia(const QUrl &media, QIODevice *stream)
{
    Q_D(DEnginePlayer);
    d->m_media = media;
}

void DEnginePlayer::setVolume(float volume)
{
    changeVolume(volume);
}

void DEnginePlayer::setMuted(bool muted)
{
    Q_D(DEnginePlayer);
    if(d->m_engine) {
        d->m_engine->setMute(muted);
        d->m_audioOutput->setMuted(muted);
    }
}

void DEnginePlayer::play()
{
    Q_D(DEnginePlayer);
    if(!d->m_engine || !d->m_pPlayer) return;
    PlayerWidget *player = dynamic_cast<PlayerWidget *>(d->m_pPlayer);
    if(d->m_engine->isPlayableFile(d->m_media)) player->play(d->m_media);
}

void DEnginePlayer::pause()
{
    Q_D(DEnginePlayer);
    if(d->m_engine) {
        d->m_engine->pauseResume();
    }
}

void DEnginePlayer::stop()
{
    Q_D(DEnginePlayer);
    if(d->m_engine) {
        d->m_engine->stop();
    }
}

void DEnginePlayer::setVideoSink(DVideoSink *)
{
}

void DEnginePlayer::setPlayer(QWidget *Player)
{
    Q_D(DEnginePlayer);
    if(!Player) return;
    d->m_pPlayer         = Player;
    PlayerEngine *engine = &dynamic_cast<PlayerWidget *>(d->m_pPlayer)->engine();
    d->m_audioOutput     = new DAudioOutput(Player);
    d->m_engine          = engine;
    connect(engine, &PlayerEngine::durationChanged, this, &DEnginePlayer::durationChanged);
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
        connect(d->m_audioOutput, &DAudioOutput::volumeChanged, this, &DEnginePlayer::volumeChanged);
        connect(d->m_audioOutput, &DAudioOutput::mutedChanged, this, &DEnginePlayer::mutedChanged);
    });
}

void DEnginePlayer::setPlaySpeed(double times)
{
    Q_D(DEnginePlayer);
    if(d->m_engine) {
        d->m_engine->setPlaySpeed(times);
    }
}

void DEnginePlayer::changeSoundMode(const DPlayerBackend::SoundMode &sm)
{
    Q_D(DEnginePlayer);
    if(d->m_engine) {
        d->m_engine->changeSoundMode(sm);
    }
}

void DEnginePlayer::nextFrame()
{
    Q_D(DEnginePlayer);
    if(d->m_engine) {
        d->m_engine->nextFrame();
    }
}

void DEnginePlayer::previousFrame()
{
    Q_D(DEnginePlayer);
    if(d->m_engine) {
        d->m_engine->previousFrame();
    }
}

void DEnginePlayer::setDecodeModel(const DPlayerBackend::hwaccelMode &hwaccelMode)
{
    Q_D(DEnginePlayer);
    if(d->m_engine) {
        d->m_engine->changehwaccelMode(hwaccelMode);
    }
}

QImage DEnginePlayer::takeScreenshot()
{
    Q_D(DEnginePlayer);
    if(d->m_engine) {
        return d->m_engine->takeScreenshot();
    }
    return QImage();
}

void DEnginePlayer::burstScreenshot()
{
    Q_D(DEnginePlayer);
    if(d->m_engine) {
        d->m_engine->burstScreenshot();
    }
}

void DEnginePlayer::setVideoRotation(int degree)
{
    Q_D(DEnginePlayer);
    if(d->m_engine) {
        d->m_engine->setVideoRotation(degree);
    }
}

void DEnginePlayer::changeVolume(int val)
{
    Q_D(DEnginePlayer);
    if(d->m_engine) {
        d->m_engine->changeVolume(val);
        d->m_audioOutput->setVolume(val);
    }
}

void DEnginePlayer::setMute(bool bMute)
{
    Q_D(DEnginePlayer);
    if(d->m_engine) {
        d->m_engine->setMute(bMute);
        d->m_audioOutput->setMuted(bMute);
    }
}

void DEnginePlayer::seekAbsolute(int pos)
{
    Q_D(DEnginePlayer);
    if(d->m_engine) {
        d->m_engine->seekAbsolute(pos);
    }
}

void DEnginePlayer::setPlayMode(const PlayMode &pm)
{
    Q_D(DEnginePlayer);
    if(d->m_engine) {
        d->m_engine->playlist().setPlayMode((DPlaylistModel::PlayMode) pm);
    }
}

PlayMode DEnginePlayer::playMode() const
{
    Q_D(const DEnginePlayer);
    if(d->m_engine) {
        return static_cast<PlayMode>(d->m_engine->playlist().playMode());
    }
}

void DEnginePlayer::playByName(const QUrl &url)
{
    Q_D(const DEnginePlayer);
    if(d->m_engine) {
        d->m_engine->playByName(url);
    }
}

bool DEnginePlayer::loadSubtitle(const QFileInfo &fi)
{
    Q_D(const DEnginePlayer);
    if(d->m_engine) {
        return d->m_engine->loadSubtitle(fi);
    }
    return false;
}

bool DEnginePlayer::addPlayFile(const QUrl &url)
{
    Q_D(const DEnginePlayer);
    if(d->m_engine) {
        return d->m_engine->addPlayFile(url);
    }
    return false;
}

const MovieInfo &DEnginePlayer::movieInfo()
{
    Q_D(DEnginePlayer);
    PlayerEngine *engine = &dynamic_cast<PlayerWidget *>(d->m_pPlayer)->engine();
    if(engine) {
        memcpy(&d->m_movieInfo, &(engine->movieInfo()), sizeof(MovieInfo));
        return d->m_movieInfo;
    }
    return d->m_movieInfo = MovieInfo();
}

DAudioOutput *DEnginePlayer::audioOut()
{
    Q_D(DEnginePlayer);
    return d->m_audioOutput;
}

void DEnginePlayer::positionProxyChanged()
{
    Q_D(DEnginePlayer);
    if(!d->m_pPlayer) return;
    PlayerEngine *engine = &dynamic_cast<PlayerWidget *>(d->m_pPlayer)->engine();
    qint64 nElapsed      = engine->elapsed() * 1000;
    positionChanged(nElapsed);
}
