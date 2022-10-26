// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DVIDEOWIDGET_H
#define DVIDEOWIDGET_H

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
public Q_SLOTS:
    void slotFullScreenChanged(bool fullScreen);
protected:
    void showEvent(QShowEvent *pEvent) override;
private:
    QWidget *m_player {nullptr};
    DPlatformMediaPlayer *m_mediaPlayer;
};

DMULTIMEDIA_END_NAMESPACE
#endif
