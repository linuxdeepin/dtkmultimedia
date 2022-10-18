// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEnginePlayer_H
#define DEnginePlayer_H

#include <dplatformmediaplayer.h>
#include <player/player_backend.h>
#include <player/playlist_model.h>
#include <player/player_engine.h>
#include "daudioout.h"

using namespace dmr;

class Q_MULTIMEDIA_EXPORT DEnginePlayer : public DPlatformMediaPlayer
{
    Q_OBJECT
public:
    DEnginePlayer(QMediaPlayer *parent = nullptr);
    ~DEnginePlayer();
public:
    virtual qint64 duration() const;
    virtual qint64 position() const;
    virtual void setPosition(qint64 position);
    virtual float bufferProgress() const ;
    virtual QMediaTimeRange availablePlaybackRanges() const;
    virtual qreal playbackRate() const;
    virtual void setPlaybackRate(qreal rate);
    virtual QUrl media() const;
    virtual const QIODevice *mediaStream() const;
    virtual void setMedia(const QUrl &media, QIODevice *stream);
    virtual void  setVolume (float volume);
    virtual void  setMuted (bool muted);
    virtual void play();
    virtual void pause();
    virtual void stop();

    virtual void setVideoSink(DVideoSink * /*sink*/);
    virtual void setPlayer(QWidget * Player);


public:
    void setPlaySpeed(double times);
    void changeSoundMode(const Backend::SoundMode &sm);
    void nextFrame();
    void previousFrame();
    void setDecodeModel(const Backend::hwaccelMode &hwaccelMode);
    QImage takeScreenshot();
    void burstScreenshot();
    void setVideoRotation(int degree);
    void changeVolume(int val);
    void setMute(bool bMute);
    void seekAbsolute(int pos);
    void setPlayMode(const PlaylistModel::PlayMode &pm);
    void playByName(const QUrl &url);
    bool loadSubtitle(const QFileInfo &fi);
    PlaylistModel *getplaylist();
    bool addPlayFile(const QUrl &url);
    const struct MovieInfo &movieInfo();

public slots:
    void positionProxyChanged();

private:
    QWidget *m_pPlayer = nullptr;
    QUrl m_media;
    PlayerEngine *m_engine;
    DAudioOutput *m_audioOutput;
};


#endif
