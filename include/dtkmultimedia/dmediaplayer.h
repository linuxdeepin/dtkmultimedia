// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DMEDIAPLAYER_H
#define DMEDIAPLAYER_H

#include <QMediaPlayer>
#ifdef BUILD_Qt6
#include <QtMultimediaWidgets/QVideoWidget>
#endif
#include <dtkmultimedia.h>
DMULTIMEDIA_BEGIN_NAMESPACE

class DPlatformMediaPlayer;
class DMediaPlaylist;
class DMediaPlayerPrivate;
class Q_MULTIMEDIA_EXPORT DMediaPlayer : public QMediaPlayer
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DMediaPlayer)

public:
    DMediaPlayer(QObject *parent = nullptr);
    ~DMediaPlayer();
    DPlatformMediaPlayer *getPlayer();
    QVideoWidget *getVideoOutput();

#if BUILD_Qt6
    DMediaPlaylist *getPlaylist();    ////播放列表需要自己实现
#else
    QMediaPlaylist *getPlaylist();
#endif

public Q_SLOTS:
    void play();
    void pause();
    void stop();
    void setPosition(qint64 position);
    void setVolume(int volume);
    void setMuted(bool muted);
    void setPlaybackRate(qreal rate);
#if BUILD_Qt6
    ////....以下接口在Qt6中已移除，需要自己实现
//    void setMedia(const QMediaContent &media, QIODevice *stream = nullptr); ////
//    void setPlaylist(QMediaPlaylist *playlist); ////删掉了
//    void setNetworkConfigurations(const QList<QNetworkConfiguration> &configurations);  ////删掉了
    void setVideoOutput(QObject *);
    bool isMetaDataAvailable() {return true;};
#else
    void setMedia(const QMediaContent &media, QIODevice *stream = nullptr);
    void setPlaylist(QMediaPlaylist *playlist);
    void setNetworkConfigurations(const QList<QNetworkConfiguration> &configurations);
    void setVideoOutput(QVideoWidget *);
#endif
    void setPlayer(DPlatformMediaPlayer *player);


protected:
    QScopedPointer<DMediaPlayerPrivate> d_ptr;
};

DMULTIMEDIA_END_NAMESPACE

#endif
