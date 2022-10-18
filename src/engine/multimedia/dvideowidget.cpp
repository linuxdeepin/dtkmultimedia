// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dvideowidget.h"
#include <QVBoxLayout>
#include <player_widget.h>
#include <player_engine.h>







DVideoWidget::DVideoWidget(QWidget *parent):
    QVideoWidget (parent)
{
    setlocale(LC_NUMERIC, "C");
    auto l = new QVBoxLayout(this);
    m_player = new dmr::PlayerWidget;
    m_player->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    l->addWidget(m_player);
    setLayout(l);
    connect(this, &QVideoWidget::fullScreenChanged, this, &DVideoWidget::slotFullScreenChanged);
}

DVideoWidget::~DVideoWidget()
{

}

QWidget *DVideoWidget::getPlayer()
{
    return m_player;
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


