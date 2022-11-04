// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLATFFORMMEDIAPLAYER_P_H
#define DPLATFFORMMEDIAPLAYER_P_H
#include "dplatformmediaplayer.h"
#include <QObject>

DMULTIMEDIA_BEGIN_NAMESPACE
class DPlatformMediaPlayerPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DPlatformMediaPlayer)
public:
    explicit DPlatformMediaPlayerPrivate(DPlatformMediaPlayer *parent = nullptr)
        : QObject(parent), q_ptr(parent)
    {
    }

private:
    DPlatformMediaPlayer *q_ptr;
    QMediaPlayer *player = nullptr;
    QMediaPlayer::MediaStatus status = QMediaPlayer::NoMedia;
    QMediaPlayer::State state = QMediaPlayer::StoppedState;
    bool seekable = false;
    bool videoAvailable = false;
    bool audioAvailable = false;
    int loops = 1;
    int currentLoop = 0;
    int volume = 0;
    int muted = false;
};
DMULTIMEDIA_END_NAMESPACE
#endif
