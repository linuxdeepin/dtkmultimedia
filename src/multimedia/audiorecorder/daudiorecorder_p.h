// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIORECORDER_P_H
#define DAUDIORECORDER_P_H

#include "daudiorecorder.h"
#include <daudioencoderinterface.h>

DMULTIMEDIA_BEGIN_NAMESPACE

class DAudioRecorderPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DAudioRecorder)
public:
    explicit DAudioRecorderPrivate(DAudioRecorder *parent = nullptr)
        : QObject(parent), q_ptr(parent)
    {
    }

signals:

public slots:

private:
    DAudioRecorder *q_ptr;
    DAudioEncoderInterface *encoderInterface { nullptr };
};
DMULTIMEDIA_END_NAMESPACE
#endif   // DAUDIORECORDER_P_H
