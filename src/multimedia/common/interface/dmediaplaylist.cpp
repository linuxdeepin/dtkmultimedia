// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dmediaplaylist.h"


DMULTIMEDIA_USE_NAMESPACE




DMediaPlaylist::DMediaPlaylist(QMediaPlaylist *parent)
    :QMediaPlaylist(parent)
{

}

DMediaPlaylist::~DMediaPlaylist()
{

}

void DMediaPlaylist::shuffle()
{
    QMediaPlaylist::shuffle();
}

void DMediaPlaylist::next()
{
    QMediaPlaylist::next();
}

void DMediaPlaylist::previous()
{
    QMediaPlaylist::previous();
}

void DMediaPlaylist::setCurrentIndex(int index)
{
    QMediaPlaylist::setCurrentIndex(index);
}
