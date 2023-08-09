// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dmediacapturesession_p.h"

DMULTIMEDIA_USE_NAMESPACE

DMediaCaptureSession::DMediaCaptureSession(QObject *parent)
#ifdef BUILD_Qt6
    : QMediaCaptureSession(parent), d_ptr(new DMediaCaptureSessionPrivate(this))
#else
    : QObject(parent), d_ptr(new DMediaCaptureSessionPrivate(this))
#endif
{
}

DMediaCaptureSession::~DMediaCaptureSession()
{
}

#ifndef BUILD_Qt6
DAudioInput *DMediaCaptureSession::audioInput() const
{
    Q_D(const DMediaCaptureSession);
    return d->audioInput;
}

void DMediaCaptureSession::setAudioInput(DAudioInput *input)
{
    Q_D(DMediaCaptureSession);
    if (d->audioInput == input)
        return;
    d_ptr->audioInput = input;
    emit audioInputChanged();
}

DCamera *DMediaCaptureSession::camera() const
{
    Q_D(const DMediaCaptureSession);
    return d->camera;
}

void DMediaCaptureSession::setCamera(DCamera *camera)
{
    Q_D(DMediaCaptureSession);
    if (d->camera == camera)
        return;
    d_ptr->camera = camera;
    emit cameraChanged();
}

DImageCapture *DMediaCaptureSession::imageCapture()
{
    Q_D(const DMediaCaptureSession);
    return d->imageCapture;
}

void DMediaCaptureSession::setImageCapture(DImageCapture *imageCapture)
{
    Q_D(DMediaCaptureSession);
    if (d->imageCapture == imageCapture)
        return;
    d_ptr->imageCapture = imageCapture;
    emit imageCaptureChanged();
}


DMediaRecorder *DMediaCaptureSession::recorder()
{
    Q_D(const DMediaCaptureSession);
    return d->recorder;
}

void DMediaCaptureSession::setRecorder(DMediaRecorder *recorder)
{
    Q_D(DMediaCaptureSession);

#ifdef BUILD_Qt6

#else
    if (d->recorder == recorder)
        return;
    d_ptr->recorder = recorder;
#endif
    emit recorderChanged();
}

void DMediaCaptureSession::setVideoOutput(QObject *output)
{
    Q_D(DMediaCaptureSession);
    if (d->videoOutput == output)
        return;
    d_ptr->videoOutput = output;
    emit videoOutputChanged();
}

QObject *DMediaCaptureSession::videoOutput() const
{
    Q_D(const DMediaCaptureSession);
    return d->videoOutput;
}

void DMediaCaptureSession::setVideoSink(DVideoSink *sink)
{
    Q_D(DMediaCaptureSession);
    if (d->videoSink == sink)
        return;
    d_ptr->videoSink = sink;
}

DVideoSink *DMediaCaptureSession::videoSink() const
{
    Q_D(const DMediaCaptureSession);
    return d->videoSink;
}

void DMediaCaptureSession::setAudioOutput(DAudioOutput *output)
{
    Q_D(DMediaCaptureSession);
    if (d->audioOutput == output)
        return;
    d_ptr->audioOutput = output;
    emit audioOutputChanged();
}

DAudioOutput *DMediaCaptureSession::audioOutput() const
{
    Q_D(const DMediaCaptureSession);
    return d->audioOutput;
}

DMediaCaptureSession *DMediaCaptureSession::platformSession() const
{
    //todo
    return NULL;
}
#endif
