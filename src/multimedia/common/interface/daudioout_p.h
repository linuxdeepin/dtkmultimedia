// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DAUDIOOUT_P_H
#define DAUDIOOUT_P_H

#include "daudioout.h"
#include <QObject>

DMULTIMEDIA_BEGIN_NAMESPACE

class DAudioOutputPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DAudioOutput)
public:
    explicit DAudioOutputPrivate(DAudioOutput *parent = nullptr)
        : QObject(parent), q_ptr(parent)
    {
    }

private:
    DAudioOutput *q_ptr;
    DAudioDevice audioDevice;
    bool bMuted = false;
    float fVolume = 0.f;
};
DMULTIMEDIA_END_NAMESPACE
#endif
