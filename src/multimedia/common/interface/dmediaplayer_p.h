// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DMEDIAPLAYER_P_H
#define DMEDIAPLAYER_P_H
#include "dmediaplayer.h"
#include <QObject>

DMULTIMEDIA_BEGIN_NAMESPACE
class DMediaPlayerPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DMediaPlayer)
public:
    explicit DMediaPlayerPrivate(DMediaPlayer *parent = nullptr)
        : QObject(parent), q_ptr(parent)
    {
    }

private:
    DMediaPlayer *q_ptr;
    DPlatformMediaPlayer *pPlayer = nullptr;
    QVideoWidget *videoWidget = nullptr;

#if BUILD_Qt6
   DMediaPlaylist *pPlayList = nullptr;    //QMediaPlaylist已移除
#else

    QMediaPlaylist *pPlayList = nullptr;
#endif
};
DMULTIMEDIA_END_NAMESPACE
#endif
