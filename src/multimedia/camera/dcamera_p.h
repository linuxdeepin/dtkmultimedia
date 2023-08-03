// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCAMERA_P_H
#define DCAMERA_P_H
#include "dcamera.h"
class MajorImageProcessingThread;

DMULTIMEDIA_BEGIN_NAMESPACE
class DCameraPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DCamera)
public:
    explicit DCameraPrivate(DCamera *parent = nullptr)
        : QObject(parent), q_ptr(parent)
    {
    }

private:
    void initDynamicLibPath();
    QString libPath(const QString &strlib);
    bool CheckFFmpegEnv();
    bool CheckWayland();
    int dcamInit();
    int dcamInit(const QString &sDevName);
    int cameraUnInit();
    void initCamera();

private:
    MajorImageProcessingThread *imgPrcThread = nullptr;
    bool bVideoStatus = false;
#ifdef BUILD_Qt6
    QWidget *viewfinder = nullptr;
#else
    QVideoWidget *viewfinder = nullptr;
#endif
    bool isWayland = false;
#ifdef BUILD_Qt6
    DCamera::State state = DCamera::UnloadedState;
#else
    QCamera::State state = QCamera::UnloadedState;
#endif
    DMediaCaptureSession *captureSession = nullptr;

private:
    DCamera *q_ptr;
};

DMULTIMEDIA_END_NAMESPACE

#endif   // DCAMERA_P_H
