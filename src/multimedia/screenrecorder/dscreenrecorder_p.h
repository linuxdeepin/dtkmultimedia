// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSCREENRECORDER_P_H
#define DSCREENRECORDER_P_H

#include "dscreenrecorder.h"
#include "videostreaminterface.h"

DMULTIMEDIA_BEGIN_NAMESPACE
class DScreenRecorderPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DScreenRecorder)
public:
    explicit DScreenRecorderPrivate(DScreenRecorder *parent = nullptr)
        : QObject(parent), q_ptr(parent)
    {
    }

private:
    DScreenRecorder *q_ptr;
    VideoStreamInterface *videoInterface { nullptr };
};
DMULTIMEDIA_END_NAMESPACE
#endif   // DSCREENRECORDER_P_H
