// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DGSTPLAYERPROXY_P_H
#define DGSTPLAYERPROXY_P_H

#include "dgstplayerproxy.h"
#include <QObject>
#ifdef BUILD_Qt6
#include <QAudioOutput>
#endif

DMULTIMEDIA_BEGIN_NAMESPACE
class DGstPlayerProxyPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DGstPlayerProxy)
public:
    explicit DGstPlayerProxyPrivate(DGstPlayerProxy *parent = nullptr)
        : QObject(parent), q_ptr(parent)
    {
        initMember();
    }

protected:
    void initMember();

private:
    void updatePlayingMovieInfo();
    void setState(DPlayerBackend::PlayState state);
    int volumeCorrection(int);

private:
    DGstPlayerProxy *q_ptr;
    QMediaPlayer *pPlayer = nullptr;
    VideoSurface *pVideoSurface = nullptr;
#ifdef BUILD_Qt6
    QAudioOutput *pAudioOutput = nullptr;
#endif
    PlayingMovieInfo movieInfo;
    QVariant posBeforeBurst;
    QList<qint64> listBurstPoints;
    qint64 nBurstStart = 0;
    bool bInBurstShotting = false;
    bool bExternalSubJustLoaded = false;
    bool bConnectStateChange = false;
    bool bPauseOnStart = false;
    bool bInited = false;
    bool bHwaccelAuto = false;
    bool bLastIsSpecficFormat = false;
    QMap<QString, QVariant> mapWaitSet;
    QVector<QVariant> vecWaitCommand;
    QImage currentImage;
};

DMULTIMEDIA_END_NAMESPACE
#endif
