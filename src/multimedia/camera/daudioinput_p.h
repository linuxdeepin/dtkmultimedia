// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIOINPUTDEVICE_P_H
#define DAUDIOINPUTDEVICE_P_H
#include "daudioinput.h"
#if BUILD_Qt6
#include <QtMultimedia/QMediaRecorder>
#include <QtMultimedia/QMediaCaptureSession>
#else
#include <QAudioRecorder>
#endif

DMULTIMEDIA_BEGIN_NAMESPACE
class DAudioInputPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DAudioInput)
public:
    explicit DAudioInputPrivate(DAudioInput *parent = nullptr)
        : QObject(parent), q_ptr(parent)
    {
    }

private:
    DAudioInput *q_ptr;
#if BUILD_Qt6
    QMediaCaptureSession recorder;
#else
    QAudioRecorder recorder;
#endif
    float volume;
    bool muted;
    QString device;
};

DMULTIMEDIA_END_NAMESPACE

#endif   // DAUDIOINPUTDEVICE_P_H
