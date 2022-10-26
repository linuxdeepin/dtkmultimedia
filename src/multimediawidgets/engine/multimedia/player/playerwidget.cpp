// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "playerwidget.h"
#include "filefilter.h"
#include <playerengine.h>

DMULTIMEDIA_BEGIN_NAMESPACE

PlayerWidget::PlayerWidget(QWidget *parent)
    : QWidget (parent)
{
    utils::first_check_wayland_env();
    m_engine = new PlayerEngine(this);
    auto *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(m_engine);
    setLayout(l);
}

PlayerWidget::~PlayerWidget()
{
}

PlayerEngine &PlayerWidget::engine()
{
    return *m_engine;
}

void PlayerWidget::play(const QUrl &url)
{
    QUrl realUrl;
    realUrl = FileFilter::instance()->fileTransfer(url.toString());

    if (!realUrl.isValid())
        return;

    if (!m_engine->addPlayFile(realUrl)) {
        return;
    }
    m_engine->playByName(realUrl);
}

DMULTIMEDIA_END_NAMESPACE
