// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DMEDIACAPTURESESSION_H
#define DMEDIACAPTURESESSION_H

#if BUILD_Qt6
#include <QtMultimedia/QMediaCaptureSession>
#else
#include <QMediaObject>
#endif

#include "dtkmultimedia.h"

DMULTIMEDIA_BEGIN_NAMESPACE
class DCamera;
class DImageCapture;
class DAudioInput;
class DMediaRecorder;
class DVideoSink;
class DAudioOutput;
class DMediaCaptureSessionPrivate;
#ifdef BUILD_Qt6
class Q_MULTIMEDIA_EXPORT DMediaCaptureSession : public QMediaCaptureSession
#else
class Q_MULTIMEDIA_EXPORT DMediaCaptureSession : public QObject
#endif
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DMediaCaptureSession)
public:
    explicit DMediaCaptureSession(QObject *parent = nullptr);
    ~DMediaCaptureSession();

public:
#ifndef BUILD_Qt6
    DAudioInput *audioInput() const;
    void setAudioInput(DAudioInput *input);

    DCamera *camera() const;
    void setCamera(DCamera *camera);

    DImageCapture *imageCapture();
    void setImageCapture(DImageCapture *imageCapture);


    DMediaRecorder *recorder();
    void setRecorder(DMediaRecorder *recorder);


    void setVideoOutput(QObject *output);
    QObject *videoOutput() const;

    void setVideoSink(DVideoSink *sink);
    DVideoSink *videoSink() const;

    void setAudioOutput(DAudioOutput *output);
    DAudioOutput *audioOutput() const;

    DMediaCaptureSession *platformSession() const;

Q_SIGNALS:
    void audioInputChanged();
    void cameraChanged();
    void imageCaptureChanged();
    void recorderChanged();
    void videoOutputChanged();
    void audioOutputChanged();
#endif

protected:
    QScopedPointer<DMediaCaptureSessionPrivate> d_ptr;
};
DMULTIMEDIA_END_NAMESPACE

#endif
