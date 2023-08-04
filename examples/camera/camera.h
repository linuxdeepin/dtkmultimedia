// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CAMERA_H
#define CAMERA_H

#include <DCamera>
#include <DImageCapture>
#include <DMediaRecorder>
#include <DMediaCaptureSession>
#include <QScopedPointer>

#include <QMainWindow>
#ifdef BUILD_Qt6
#include <DAudioInput>
#include <QMediaDevices>
#include <QActionGroup>
#endif

#ifdef BUILD_Qt6
typedef  QImageCapture QCameraImageCapture;
#endif

class QOpenGLWidget;
QT_BEGIN_NAMESPACE
namespace Ui {
class Camera;
}
QT_END_NAMESPACE

DMULTIMEDIA_USE_NAMESPACE

class Camera : public QMainWindow
{
    Q_OBJECT

public:
    Camera();
    int dcamInit(int argc, char *argv[]);

private slots:
#ifdef BUILD_Qt6
    void setCamera(const QCameraDevice &cameraInfo);
#else
    void setCamera(const QCameraInfo &cameraInfo);
#endif
    void startCamera();
    void stopCamera();

    void record();
    void pause();
    void stop();
    void setMuted(bool);

#ifdef BUILD_Qt6

#else
    void toggleLock();
    void updateLockStatus(QCamera::LockStatus, QCamera::LockChangeReason);
#endif
    void takeImage();
    void displayCaptureError(int, QCameraImageCapture::Error, const QString &errorString);

    void displayRecorderError();
    void displayCameraError();

    void updateCameraDevice(QAction *action);

#ifdef BUILD_Qt6
    void updateCameraActive(bool active);
#else
    void updateCameraState(QCamera::State);
#endif
    void updateCaptureMode();
#ifdef BUILD_Qt6
    void updateRecorderState(QMediaRecorder::RecorderState state);
    void updateCameras();
#else
    void updateRecorderState(QMediaRecorder::State state);
#endif
    void setExposureCompensation(int index);

    void updateRecordTime();

    void processCapturedImage(int requestId, const QImage &img);

    void displayViewfinder();
    void displayCapturedImage();

    void readyForCapture(bool ready);
    void imageSaved(int id, const QString &fileName);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::Camera *ui;

    QScopedPointer<DCamera> m_camera;
    QScopedPointer<DImageCapture> m_imageCapture;
    QScopedPointer<DMediaRecorder> m_mediaRecorder;
    DMediaCaptureSession m_captureSession;
#ifdef BUILD_Qt6
    QMediaDevices m_devices;
    QScopedPointer<DAudioInput> m_audioInput;
    QActionGroup *m_videoDevicesGroup  = nullptr;

    bool m_doImageCapture = true;
#endif

    // 未使用
    // QImageEncoderSettings m_imageSettings;
    // QAudioEncoderSettings m_audioSettings;
    // QVideoEncoderSettings m_videoSettings;
    QString m_videoContainerFormat;
    bool m_isCapturingImage = false;
    bool m_applicationExiting = false;
    bool m_bStart = false;
    QOpenGLWidget *m_openglwidget;
};

#endif
