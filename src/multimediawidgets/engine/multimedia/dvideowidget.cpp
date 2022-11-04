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
    d->mediaPlayer = mediaPlayer;
    auto l           = new QVBoxLayout(this);
    d->player      = new PlayerWidget;
    d->player->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    l->addWidget(d->player);
    setLayout(l);
    connect(this, &QVideoWidget::fullScreenChanged, this, &DVideoWidget::slotFullScreenChanged);

    if(d->mediaPlayer) {
        static_cast<DEnginePlayer *>(d->mediaPlayer)->setPlayer(d->player);
    }
}

DVideoWidget::~DVideoWidget()
{
}

QWidget *DVideoWidget::getPlayer()
{
    Q_D(DVideoWidget);
    return d->player;
}

void DVideoWidget::setPlatformMediaPlayer(DPlatformMediaPlayer *mediaPlayer)
{
    Q_D(DVideoWidget);
    d->mediaPlayer = mediaPlayer;
    if(d->mediaPlayer) {
        ((DEnginePlayer *) d->mediaPlayer)->setPlayer(d->player);
    }
}

void DVideoWidget::showEvent(QShowEvent *pEvent)
{
    Q_D(DVideoWidget);
    d->player->show();
}

void DVideoWidget::slotFullScreenChanged(bool fullScreen)
{
    Q_D(DVideoWidget);
    d->player->raise();
    d->player->show();
}
