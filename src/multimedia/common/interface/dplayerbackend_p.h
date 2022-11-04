// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DPLAYERBACKEND_P_H
#define DPLAYERBACKEND_P_H
#include "dplayerbackend.h"
#include <QObject>

DMULTIMEDIA_BEGIN_NAMESPACE
class DPlayerBackendPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DPlayerBackend)
public:
    explicit DPlayerBackendPrivate(DPlayerBackend *parent = nullptr)
        : QObject(parent), q_ptr(parent)
    {
    }

private:
    DPlayerBackend *q_ptr;

protected:
    DPlayerBackend::PlayState state { DPlayerBackend::PlayState::Stopped };
    QString dvdDevice;
    QUrl file;
    static DPlayerBackend::DebugLevel debugLevel;
    qint64 winId = -1;
};
DMULTIMEDIA_END_NAMESPACE
#endif
