// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DGSTPLAYERPROXY_P_H
#define DGSTPLAYERPROXY_P_H

#include "dgstplayerproxy.h"
#include <QObject>

DMULTIMEDIA_BEGIN_NAMESPACE
class DGstPlayerProxyPrivate : public QObject {
    Q_OBJECT
    Q_DECLARE_PUBLIC(DGstPlayerProxy)
  public:
    explicit DGstPlayerProxyPrivate(DGstPlayerProxy *parent = nullptr)
        : QObject(parent), q_ptr(parent)
    {
    }

  protected:
    void initMember();

  private:
    void updatePlayingMovieInfo();
    void setState(DPlayerBackend::PlayState state);
    int volumeCorrection(int);

  private:
    DGstPlayerProxy *q_ptr;
    QMediaPlayer *m_pPlayer;
    VideoSurface *m_pVideoSurface;
    PlayingMovieInfo m_movieInfo;
    QVariant m_posBeforeBurst;
    QList<qint64> m_listBurstPoints;
    qint64 m_nBurstStart;
    bool m_bInBurstShotting;
    bool m_bExternalSubJustLoaded;
    bool m_bConnectStateChange;
    bool m_bPauseOnStart;
    bool m_bInited;
    bool m_bHwaccelAuto;
    bool m_bLastIsSpecficFormat;
    QMap<QString, QVariant> m_mapWaitSet;
    QVector<QVariant> m_vecWaitCommand;
    QMap<QString, QString> *m_pConfig;
    QImage m_currentImage;
};

DMULTIMEDIA_END_NAMESPACE
#endif
