// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DVideoWidget_H
#define DVideoWidget_H

#include <QVideoWidget>
#include <dtkmultimedia.h>
#include <DPlatformMediaPlayer>

DMULTIMEDIA_BEGIN_NAMESPACE
class Q_MULTIMEDIA_EXPORT DVideoWidget : public QVideoWidget
{
    Q_OBJECT
public:
    DVideoWidget(QWidget *parent = nullptr, DPlatformMediaPlayer *mediaPlayer = nullptr);
    ~DVideoWidget();
    QWidget *getPlayer();
    void setPlatformMediaPlayer(DPlatformMediaPlayer *mediaPlayer = nullptr);
protected:
    void showEvent(QShowEvent *pEvent) override;
public Q_SLOTS:
    void slotFullScreenChanged(bool fullScreen);
private:
    QWidget *m_player {nullptr};
    DPlatformMediaPlayer *m_mediaPlayer;
};

DMULTIMEDIA_END_NAMESPACE
#endif
