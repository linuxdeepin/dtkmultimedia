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
    d->mediaPlayer = parent;
    d->pPlayer     = nullptr;
    d->audioOutput = nullptr;
    if(d->mediaPlayer) {
        (static_cast<DMediaPlayer *>(d->mediaPlayer))->setPlayer(this);
    }
}

DEnginePlayer::~DEnginePlayer()
{
    Q_D(DEnginePlayer);

    if(d->audioOutput) {
        delete d->audioOutput;
        d->audioOutput = nullptr;
    }
}

qint64 DEnginePlayer::duration() const
{
    Q_D(const DEnginePlayer);
    if(d->engine) {
        return d->engine->duration();
    }
    return 0;
}

qint64 DEnginePlayer::position() const
{
    Q_D(const DEnginePlayer);
    if(d->engine) {
        return d->engine->elapsed();
    }
    return 0;
}

void DEnginePlayer::setPosition(qint64 position)
{
    Q_D(DEnginePlayer);
    if(d->engine) {
        d->engine->seekAbsolute(position / 1000);
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
    if(d->engine) {
        d->engine->setPlaySpeed(rate);
    }
}

QUrl DEnginePlayer::media() const
{
    Q_D(const DEnginePlayer);
    return d->media;
}

const QIODevice *DEnginePlayer::mediaStream() const
{
    // TODO
    return nullptr;
}

void DEnginePlayer::setMedia(const QUrl &media, QIODevice *stream)
{
    Q_D(DEnginePlayer);
    d->media = media;
}

void DEnginePlayer::setVolume(float volume)
{
    changeVolume(volume);
}

void DEnginePlayer::setMuted(bool muted)
{
    Q_D(DEnginePlayer);
    if(d->engine) {
        d->engine->setMute(muted);
        d->audioOutput->setMuted(muted);
    }
}

void DEnginePlayer::play()
{
    Q_D(DEnginePlayer);
    if(!d->engine || !d->pPlayer) return;
    PlayerWidget *player = dynamic_cast<PlayerWidget *>(d->pPlayer);
    if(d->engine->isPlayableFile(d->media)) player->play(d->media);
}

void DEnginePlayer::pause()
{
    Q_D(DEnginePlayer);
    if(d->engine) {
        d->engine->pauseResume();
    }
}

void DEnginePlayer::stop()
{
    Q_D(DEnginePlayer);
    if(d->engine) {
        d->engine->stop();
    }
}

void DEnginePlayer::setVideoSink(DVideoSink *)
{
}

void DEnginePlayer::setPlayer(QWidget *Player)
{
    Q_D(DEnginePlayer);
    if(!Player) return;
    d->pPlayer         = Player;
    PlayerEngine *engine = &dynamic_cast<PlayerWidget *>(d->pPlayer)->engine();
    d->audioOutput     = new DAudioOutput(Player);
    d->engine          = engine;
    connect(engine, &PlayerEngine::durationChanged, this, &DEnginePlayer::durationChanged);
    connect(engine, &PlayerEngine::elapsedChanged, this, &DEnginePlayer::positionProxyChanged);
    connect(engine, &PlayerEngine::stateChanged, [=]() {
#ifdef BUILD_Qt6
        switch(engine->state()) {
        case PlayerEngine::Playing:
            playbackStateChanged(QMediaPlayer::PlayingState);
            break;
        case PlayerEngine::Idle:
            playbackStateChanged(QMediaPlayer::StoppedState);
            break;
        case PlayerEngine::Paused:
            playbackStateChanged(QMediaPlayer::PausedState);
            break;
        }
#else
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
#endif
        connect(d->audioOutput, &DAudioOutput::volumeChanged, this, &DEnginePlayer::volumeChanged);
        connect(d->audioOutput, &DAudioOutput::mutedChanged, this, &DEnginePlayer::mutedChanged);
    });
}

void DEnginePlayer::setPlaySpeed(double times)
{
    Q_D(DEnginePlayer);
    if(d->engine) {
        d->engine->setPlaySpeed(times);
    }
}

void DEnginePlayer::changeSoundMode(const DPlayerBackend::SoundMode &sm)
{
    Q_D(DEnginePlayer);
    if(d->engine) {
        d->engine->changeSoundMode(sm);
    }
}

void DEnginePlayer::nextFrame()
{
    Q_D(DEnginePlayer);
    if(d->engine) {
        d->engine->nextFrame();
    }
}

void DEnginePlayer::previousFrame()
{
    Q_D(DEnginePlayer);
    if(d->engine) {
        d->engine->previousFrame();
    }
}

void DEnginePlayer::setDecodeModel(const DPlayerBackend::hwaccelMode &hwaccelMode)
{
    Q_D(DEnginePlayer);
    if(d->engine) {
        d->engine->changehwaccelMode(hwaccelMode);
    }
}

QImage DEnginePlayer::takeScreenshot()
{
    Q_D(DEnginePlayer);
    if(d->engine) {
        return d->engine->takeScreenshot();
    }
    return QImage();
}

void DEnginePlayer::burstScreenshot()
{
    Q_D(DEnginePlayer);
    if(d->engine) {
        d->engine->burstScreenshot();
    }
}

void DEnginePlayer::setVideoRotation(int degree)
{
    Q_D(DEnginePlayer);
    if(d->engine) {
        d->engine->setVideoRotation(degree);
    }
}

void DEnginePlayer::changeVolume(int val)
{
    Q_D(DEnginePlayer);
    if(d->engine) {
        d->engine->changeVolume(val);
        d->audioOutput->setVolume(val);
    }
}

void DEnginePlayer::setMute(bool bMute)
{
    Q_D(DEnginePlayer);
    if(d->engine) {
        d->engine->setMute(bMute);
        d->audioOutput->setMuted(bMute);
    }
}

void DEnginePlayer::seekAbsolute(int pos)
{
    Q_D(DEnginePlayer);
    if(d->engine) {
        d->engine->seekAbsolute(pos);
    }
}

void DEnginePlayer::setPlayMode(const PlayMode &pm)
{
    Q_D(DEnginePlayer);
    if(d->engine) {
        d->engine->playlist().setPlayMode((DPlaylistModel::PlayMode) pm);
    }
}

PlayMode DEnginePlayer::playMode() const
{
    Q_D(const DEnginePlayer);
    if(d->engine) {
        return static_cast<PlayMode>(d->engine->playlist().playMode());
    }
}

void DEnginePlayer::playByName(const QUrl &url)
{
    Q_D(const DEnginePlayer);
    if(d->engine) {
        d->engine->playByName(url);
    }
}

bool DEnginePlayer::loadSubtitle(const QFileInfo &fi)
{
    Q_D(const DEnginePlayer);
    if(d->engine) {
        return d->engine->loadSubtitle(fi);
    }
    return false;
}

bool DEnginePlayer::addPlayFile(const QUrl &url)
{
    Q_D(const DEnginePlayer);
    if(d->engine) {
        return d->engine->addPlayFile(url);
    }
    return false;
}

const MovieInfo &DEnginePlayer::movieInfo()
{
    Q_D(DEnginePlayer);
    PlayerEngine *engine = &dynamic_cast<PlayerWidget *>(d->pPlayer)->engine();
    if(engine) {
        memcpy(&d->movieInfo, &(engine->movieInfo()), sizeof(MovieInfo));
        return d->movieInfo;
    }
    return d->movieInfo = MovieInfo();
}

DAudioOutput *DEnginePlayer::audioOut()
{
    Q_D(DEnginePlayer);
    return d->audioOutput;
}

void DEnginePlayer::positionProxyChanged()
{
    Q_D(DEnginePlayer);
    if(!d->pPlayer) return;
    PlayerEngine *engine = &dynamic_cast<PlayerWidget *>(d->pPlayer)->engine();
    qint64 nElapsed      = engine->elapsed() * 1000;
    positionChanged(nElapsed);
}
