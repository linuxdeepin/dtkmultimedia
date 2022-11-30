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
    void setCamera(const QCameraInfo &cameraInfo);
    void startCamera();
    void stopCamera();

    void record();
    void pause();
    void stop();
    void setMuted(bool);

    void toggleLock();
    void takeImage();
    void displayCaptureError(int, QCameraImageCapture::Error, const QString &errorString);

    void displayRecorderError();
    void displayCameraError();

    void updateCameraDevice(QAction *action);

    void updateCameraState(QCamera::State);
    void updateCaptureMode();
    void updateRecorderState(QMediaRecorder::State state);
    void setExposureCompensation(int index);

    void updateRecordTime();

    void processCapturedImage(int requestId, const QImage &img);
    void updateLockStatus(QCamera::LockStatus, QCamera::LockChangeReason);

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

    QImageEncoderSettings m_imageSettings;
    QAudioEncoderSettings m_audioSettings;
    QVideoEncoderSettings m_videoSettings;
    QString m_videoContainerFormat;
    bool m_isCapturingImage = false;
    bool m_applicationExiting = false;
    bool m_bStart = false;
    QOpenGLWidget *m_openglwidget;
};

#endif
