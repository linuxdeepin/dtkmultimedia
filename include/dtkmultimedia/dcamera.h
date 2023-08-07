// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCAMERA_H
#define DCAMERA_H

#if BUILD_Qt6
#include <QCamera>
#include <QCameraDevice>
#include <QtMultimediaWidgets/QVideoWidget>
#include <DMediaRecorder>
#include <DImageCapture>
#else
#include <QCameraInfo>
#endif

#include "dtkmultimedia.h"

DMULTIMEDIA_BEGIN_NAMESPACE

class DCameraPrivate;
class DMediaCaptureSession;
class Q_MULTIMEDIA_EXPORT DCamera : public QCamera
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DCamera)

public:
#ifdef BUILD_Qt6
    enum State {
        UnloadedState,
        LoadedState,
        ActiveState
    };
#endif

    explicit DCamera(QCamera *parent = nullptr);
#ifdef BUILD_Qt6
    explicit DCamera(const QCameraDevice& cameraDevice, QObject *parent = nullptr);
#else
    explicit DCamera(const QCameraInfo &cameraInfo, QObject *parent = nullptr);
#endif

    ~DCamera();
    void takeOne(const QString &location);
    void takeVideo(const QString &location);
    void start();
    void stop();
    bool isFfmpegEnv();
    bool isWaylandEnv();

#ifdef BUILD_Qt6
    void setViewfinder(QWidget *viewfinder);
#else
    void setViewfinder(QVideoWidget *viewfinder);
#endif
#ifdef BUILD_Qt6
    void setActive(bool active) { m_isActive = active; };
    void setstate(State newState);
//    void setRecorder(DMediaRecorder *recorder);
//    void setImageCapture(DImageCapture *imageCapture);
#else
    void setstate(QCamera::State newState);
#endif
    int checkCamera();
    DMediaCaptureSession *captureSession() const;
    QStringList devList();
    void openDev(const QString &sDevName);
    void closeDev();
    QByteArray yuvbuffer(uint &width, uint &height);
    void resolutionSettings(const QSize &size);
    QList<QSize> resolutions();
    void setCameraCollectionFormat(const uint32_t &pixelformat);
    void setFilter(const QString &filter);
    void setExposure(const int &exposure);
    QList<uint32_t> supportedViewfinderPixelFormats();

Q_SIGNALS:
    void signalbuffer(uchar *yuv, uint width, uint height);
#ifdef BUILD_Qt6
    void stateChanged(State);
#else
    void stateChanged(State);
#endif

protected:
    QScopedPointer<DCameraPrivate> d_ptr;

#ifdef BUILD_Qt6
    bool m_isActive;
#endif
};
DMULTIMEDIA_END_NAMESPACE

#endif
