// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DMULTIOPERATEINTER_P_H
#define DMULTIOPERATEINTER_P_H
#include "dmultioperateinterface.h"
#if BUILD_Qt6
#include <QtMultimedia/QMediaRecorder>
#include <QtMultimedia/QMediaCaptureSession>
#else
#include <QAudioRecorder>
#endif

DMULTIMEDIA_BEGIN_NAMESPACE
class DMultiOperateProcess;
class DMultiOperateInterfacePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DMultiOperateInterface)
public:
    explicit DMultiOperateInterfacePrivate(DMultiOperateInterface *parent = nullptr)
        : QObject(parent), q_ptr(parent)
    {
    }

private:
    DMultiOperateInterface *q_ptr;
    DMultiOperateProcess *videoInterface { nullptr };
};

DMULTIMEDIA_END_NAMESPACE

#endif   // DMULTIOPERATEINTER_P_H
