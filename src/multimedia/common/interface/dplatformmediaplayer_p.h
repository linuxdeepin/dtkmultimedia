// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLATFFORMMEDIAPLAYER_P_H
#define DPLATFFORMMEDIAPLAYER_P_H
#include "dplatformmediaplayer.h"
#include <QObject>

DMULTIMEDIA_BEGIN_NAMESPACE
class DPlatformMediaPlayerPrivate : public QObject {
    Q_OBJECT
    Q_DECLARE_PUBLIC(DPlatformMediaPlayer)
  public:
    explicit DPlatformMediaPlayerPrivate(DPlatformMediaPlayer *parent = nullptr) : QObject(parent), q_ptr(parent)
    {
    }

  private:
    DPlatformMediaPlayer *q_ptr;
    QMediaPlayer *m_player             = nullptr;
    QMediaPlayer::MediaStatus m_status = QMediaPlayer::NoMedia;
    QMediaPlayer::State m_state        = QMediaPlayer::StoppedState;
    bool m_seekable                    = false;
    bool m_videoAvailable              = false;
    bool m_audioAvailable              = false;
    int m_loops                        = 1;
    int m_currentLoop                  = 0;
    int m_volume                       = 0;
    int m_muted                        = false;
};
DMULTIMEDIA_END_NAMESPACE
#endif
