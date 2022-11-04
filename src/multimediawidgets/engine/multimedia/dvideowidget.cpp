// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dvideowidget_p.h"
#include <DCompositeManager>
#include <QVBoxLayout>
#include <dengineplayer.h>
#include <playerengine.h>
#include <playerwidget.h>

DMULTIMEDIA_USE_NAMESPACE

DVideoWidget::DVideoWidget(QWidget *parent, DPlatformMediaPlayer *mediaPlayer)
    : QVideoWidget(parent), d_ptr(new DVideoWidgetPrivate(this))
{
    Q_D(DVideoWidget);
    setlocale(LC_NUMERIC, "C");
    if(DCompositeManager::get().first_check_wayland_env()) {
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
        setenv("PULSE_PROP_media.role", "video", 1);
#ifndef __x86_64__
        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        format.setDefaultFormat(format);
#endif
    }
    d->m_mediaPlayer = mediaPlayer;
    auto l           = new QVBoxLayout(this);
    d->m_player      = new PlayerWidget;
    d->m_player->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    l->addWidget(d->m_player);
    setLayout(l);
    connect(this, &QVideoWidget::fullScreenChanged, this, &DVideoWidget::slotFullScreenChanged);

    if(d->m_mediaPlayer) {
        static_cast<DEnginePlayer *>(d->m_mediaPlayer)->setPlayer(d->m_player);
    }
}

DVideoWidget::~DVideoWidget()
{
}

QWidget *DVideoWidget::getPlayer()
{
    Q_D(DVideoWidget);
    return d->m_player;
}

void DVideoWidget::setPlatformMediaPlayer(DPlatformMediaPlayer *mediaPlayer)
{
    Q_D(DVideoWidget);
    d->m_mediaPlayer = mediaPlayer;
    if(d->m_mediaPlayer) {
        ((DEnginePlayer *) d->m_mediaPlayer)->setPlayer(d->m_player);
    }
}

void DVideoWidget::showEvent(QShowEvent *pEvent)
{
    Q_D(DVideoWidget);
    d->m_player->show();
}

void DVideoWidget::slotFullScreenChanged(bool fullScreen)
{
    Q_D(DVideoWidget);
    d->m_player->raise();
    d->m_player->show();
}
