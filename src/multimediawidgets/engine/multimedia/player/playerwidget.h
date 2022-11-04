// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QtWidgets>
#include <dtkmultimedia.h>

DMULTIMEDIA_BEGIN_NAMESPACE
class PlayerEngine;

class PlayerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlayerWidget(QWidget *parent = nullptr);
    virtual ~PlayerWidget();

    PlayerEngine &engine();
    void play(const QUrl &url);

protected:
    PlayerEngine *m_engine { nullptr };
};
DMULTIMEDIA_END_NAMESPACE
