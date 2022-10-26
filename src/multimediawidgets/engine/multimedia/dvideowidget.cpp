// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dvideowidget.h"
#include <QVBoxLayout>
#include <playerwidget.h>
#include <playerengine.h>
#include <dengineplayer.h>

DMULTIMEDIA_USE_NAMESPACE

DVideoWidget::DVideoWidget(QWidget *parent, DPlatformMediaPlayer *mediaPlayer):
    QVideoWidget (parent), m_mediaPlayer(mediaPlayer)
{
    setlocale(LC_NUMERIC, "C");
    auto l = new QVBoxLayout(this);
    m_player = new PlayerWidget;
    m_player->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    l->addWidget(m_player);
    setLayout(l);
    connect(this, &QVideoWidget::fullScreenChanged, this, &DVideoWidget::slotFullScreenChanged);

    if(m_mediaPlayer) {
        ((DEnginePlayer *)m_mediaPlayer)->setPlayer(m_player);
    }
}

DVideoWidget::~DVideoWidget()
{

}

QWidget *DVideoWidget::getPlayer()
{
    return m_player;
}

void DVideoWidget::setPlatformMediaPlayer(DPlatformMediaPlayer *mediaPlayer)
{
    m_mediaPlayer = mediaPlayer;
    if(m_mediaPlayer) {
        ((DEnginePlayer *)m_mediaPlayer)->setPlayer(m_player);
    }
}

void DVideoWidget::showEvent(QShowEvent *pEvent)
{
    m_player->show();
}

void DVideoWidget::slotFullScreenChanged(bool fullScreen)
{
    m_player->raise();
    m_player->show();
}


