// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DMEDIAPLAYLIST_H
#define DMEDIAPLAYLIST_H

#include <QMediaPlaylist>
#include <dtkmultimedia.h>
DMULTIMEDIA_BEGIN_NAMESPACE

class Q_MULTIMEDIA_EXPORT DMediaPlaylist : public QMediaPlaylist
{
    Q_OBJECT
public:
    DMediaPlaylist(QMediaPlaylist *parent = nullptr);
    ~DMediaPlaylist();
public Q_SLOTS:
    void shuffle();
    void next();
    void previous();
    void setCurrentIndex(int index);
};
DMULTIMEDIA_END_NAMESPACE

#endif
