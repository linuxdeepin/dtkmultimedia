// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DMEDIAPLAYER_H
#define DMEDIAPLAYER_H

#include <QMediaPlayer>
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
    QMediaPlaylist *getPlaylist();

public Q_SLOTS:
    void play();
    void pause();
    void stop();
    void setPosition(qint64 position);
    void setVolume(int volume);
    void setMuted(bool muted);
    void setPlaybackRate(qreal rate);
    void setMedia(const QMediaContent &media, QIODevice *stream = nullptr);
    void setPlaylist(QMediaPlaylist *playlist);
    void setNetworkConfigurations(const QList<QNetworkConfiguration> &configurations);
    void setVideoOutput(QVideoWidget *);
    void setPlayer(DPlatformMediaPlayer *player);

protected:
    QScopedPointer<DMediaPlayerPrivate> d_ptr;
};

DMULTIMEDIA_END_NAMESPACE

#endif
