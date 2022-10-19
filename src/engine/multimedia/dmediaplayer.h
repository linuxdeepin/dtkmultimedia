// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DMediaPlayer_H
#define DMediaPlayer_H

#include <QMediaPlayer>

class DPlatformMediaPlayer;
class DMediaPlaylist;
class Q_MULTIMEDIA_EXPORT DMediaPlayer : public QMediaPlayer
{
    Q_OBJECT
public:
    DMediaPlayer(QObject *parent = nullptr);
    ~DMediaPlayer();

public Q_SLOTS:
    void play();
    void pause();
    void stop();

    void setPosition(qint64 position);
    void setVolume(int volume);
    void setMuted(bool muted);

    void setPlaybackRate(qreal rate);

    void setMedia(const QMediaContent &media, QIODevice *stream = nullptr);
    void setPlaylist(DMediaPlaylist *playlist);

    void setNetworkConfigurations(const QList<QNetworkConfiguration> &configurations);
    void setVideoOutput(QVideoWidget *);

private:
    DPlatformMediaPlayer *m_pPlayer = nullptr;
    QVideoWidget *m_videoWidget = nullptr;
    QMediaPlaylist *m_pPlayList = nullptr;
};


#endif
