// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DMEDIACAPTURESESSION_H
#define DMEDIACAPTURESESSION_H

#include <QMediaObject>
#include "dtkmultimedia.h"

DMULTIMEDIA_BEGIN_NAMESPACE
class DCamera;
class DImageCapture;
class DAudioInput;
class DMediaRecorder;
class DVideoSink;
class DAudioOutput;
class DMediaCaptureSessionPrivate;
class Q_MULTIMEDIA_EXPORT DMediaCaptureSession : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DMediaCaptureSession)
public:
    explicit DMediaCaptureSession(QObject *parent = nullptr);
    ~DMediaCaptureSession();

public:
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

protected:
    QScopedPointer<DMediaCaptureSessionPrivate> d_ptr;
};
DMULTIMEDIA_END_NAMESPACE

#endif
