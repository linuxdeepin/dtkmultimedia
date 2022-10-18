// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QMediaPlaylist_H
#define QMediaPlaylist_H

#include <QMediaPlaylist>


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


#endif
