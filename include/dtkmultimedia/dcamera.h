// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCAMERA_H
#define DCAMERA_H

#include <QCameraInfo>
#include "dtkmultimedia.h"

DMULTIMEDIA_BEGIN_NAMESPACE
class DCameraPrivate;
class DMediaCaptureSession;
class Q_MULTIMEDIA_EXPORT DCamera : public QCamera
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DCamera)

public:
    explicit DCamera(QCamera *parent = nullptr);
    explicit DCamera(const QCameraInfo &cameraInfo, QObject *parent = nullptr);
    ~DCamera();
    void takeOne(const QString &location);
    void takeVideo(const QString &location);
    void start();
    void stop();
    bool isFfmpegEnv();
    bool isWaylandEnv();
    void setViewfinder(QVideoWidget *viewfinder);
    void setstate(QCamera::State newState);
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
    void stateChanged(State);

protected:
    QScopedPointer<DCameraPrivate> d_ptr;
};
DMULTIMEDIA_END_NAMESPACE

#endif
