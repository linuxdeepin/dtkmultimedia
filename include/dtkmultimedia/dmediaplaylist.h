// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DMEDIAPLAYLIST_H
#define DMEDIAPLAYLIST_H

#if BUILD_Qt6
#include <QObject>
#else
#include <QMediaPlaylist>
#endif

#include <dtkmultimedia.h>
DMULTIMEDIA_BEGIN_NAMESPACE

#if BUILD_Qt6
class  DMediaPlaylist : public QObject
#else
class Q_MULTIMEDIA_EXPORT DMediaPlaylist : public QMediaPlaylist
#endif
{
    Q_OBJECT
public:
#if BUILD_Qt6
    DMediaPlaylist(QObject *parent = nullptr);
    void addMedia(const QUrl &content) {};
    void addMedia(const QList<QUrl> &items){};

    void load(const QUrl &location, const char *format = nullptr) {};
    int mediaCount(){return 1;};

    DMediaPlaylist *getPlaylist(){};
#else
    DMediaPlaylist(QMediaPlaylist *parent = nullptr);
#endif
    ~DMediaPlaylist();

public Q_SLOTS:
    void shuffle();
    void next();
    void previous();
    void setCurrentIndex(int index);
};
DMULTIMEDIA_END_NAMESPACE

#endif
