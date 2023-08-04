// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dmediaplaylist.h"


DMULTIMEDIA_USE_NAMESPACE

#if BUILD_Qt6
DMediaPlaylist::DMediaPlaylist(QObject *parent)
    :QObject(parent)
#else
DMediaPlaylist::DMediaPlaylist(QMediaPlaylist *parent)
    :QMediaPlaylist(parent)
#endif
{

}

DMediaPlaylist::~DMediaPlaylist()
{

}

void DMediaPlaylist::shuffle()
{
#if BUILD_Qt6

#else
    QMediaPlaylist::shuffle();
#endif
}

void DMediaPlaylist::next()
{
#if BUILD_Qt6

#else
    QMediaPlaylist::next();
#endif
}

void DMediaPlaylist::previous()
{
#if BUILD_Qt6

#else
    QMediaPlaylist::previous();
#endif
}

void DMediaPlaylist::setCurrentIndex(int index)
{
#if BUILD_Qt6

#else
    QMediaPlaylist::setCurrentIndex(index);
#endif
}
