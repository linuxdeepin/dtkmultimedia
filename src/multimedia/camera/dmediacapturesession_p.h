// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DMEDIACAPTURESESSION_P_H
#define DMEDIACAPTURESESSION_P_H

#include <QObject>
#include <dmediacapturesession.h>
#include "dplatformmediaplayer.h"

DMULTIMEDIA_BEGIN_NAMESPACE

class DMediaCaptureSessionPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DMediaCaptureSession)

public:
    explicit DMediaCaptureSessionPrivate(DMediaCaptureSession *parent = nullptr)
        : QObject(parent), q_ptr(parent)
    {
    }

private:
    DMediaCaptureSession *q_ptr;
    DAudioInput *audioInput = nullptr;
    DCamera *camera = nullptr;
    DImageCapture *imageCapture = nullptr;
    DMediaRecorder *recorder = nullptr;
    QObject *videoOutput = nullptr;
    DVideoSink *videoSink = nullptr;
    DAudioOutput *audioOutput = nullptr;
};

DMULTIMEDIA_END_NAMESPACE

#endif   // DMEDIACAPTURESESSION_P_H
