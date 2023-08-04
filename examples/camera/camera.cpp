// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "camera.h"
#ifdef BUILD_Qt6
#include "ui_camera_qt6.h"
#else
#include "ui_camera_qt5.h"
#endif

#ifdef BUILD_Qt6
#include <QTimer>
#include <QKeyEvent>
#include <QEvent>
#include <QSurfaceFormat>
#include <QList>
#else
#include <QMediaService>
#include <QCameraViewfinder>
#include <QCameraInfo>
#endif
#include <QMediaRecorder>
#include <QMediaMetaData>

#include <QMessageBox>
#include <QPalette>

#include <QtWidgets>
#include "previewopenglwidget.h"

#ifndef BUILD_Qt6
Q_DECLARE_METATYPE(QCameraInfo)
#endif
DMULTIMEDIA_USE_NAMESPACE
Camera::Camera()
    : ui(new Ui::Camera)
{
    ui->setupUi(this);

    QActionGroup *m_videoDevicesGroup = new QActionGroup(this);
    m_videoDevicesGroup->setExclusive(true);

#ifdef BUILD_Qt6
    m_audioInput.reset(new DAudioInput);

    m_videoDevicesGroup = new QActionGroup(this);
    m_videoDevicesGroup->setExclusive(true);
    updateCameras();
    connect(&m_devices, &QMediaDevices::videoInputsChanged, this, &Camera::updateCameras);

    connect(m_videoDevicesGroup, &QActionGroup::triggered, this, &Camera::updateCameraDevice);
    connect(ui->captureWidget, &QTabWidget::currentChanged, this, &Camera::updateCaptureMode);

    setCamera(QMediaDevices::defaultVideoInput());

#else
    const QList<QCameraInfo> availableCameras = QCameraInfo::availableCameras();
    for (const QCameraInfo &cameraInfo : availableCameras) {
        QAction *videoDeviceAction = new QAction(cameraInfo.description(), m_videoDevicesGroup);
        videoDeviceAction->setCheckable(true);
        videoDeviceAction->setData(QVariant::fromValue(cameraInfo));
        if (cameraInfo == QCameraInfo::defaultCamera())
            videoDeviceAction->setChecked(true);

        ui->menuDevices->addAction(videoDeviceAction);
    }

    connect(ui->captureWidget, &QTabWidget::currentChanged, this, &Camera::updateCaptureMode);

    setCamera(QCameraInfo::defaultCamera());
#endif
}

#ifdef BUILD_Qt6
void Camera::setCamera(const QCameraDevice &cameraInfo)
#else
void Camera::setCamera(const QCameraInfo &cameraInfo)
#endif
{
#ifdef BUILD_Qt6
    m_camera.reset(new DCamera(cameraInfo));
#else
    m_camera.reset(new DCamera());
#endif

    if (m_camera->isWaylandEnv()) {
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        format.setDefaultFormat(format);
    }

#ifdef BUILD_Qt6
    connect(m_camera.data(), &QCamera::activeChanged, this, &Camera::updateCameraActive);
    connect(m_camera.data(), &QCamera::errorOccurred, this, &Camera::displayCameraError);

    if (!m_mediaRecorder) {
        m_mediaRecorder.reset(new DMediaRecorder(m_camera.data()));
        m_camera->captureSession()->setRecorder(m_mediaRecorder.data());
        connect(m_mediaRecorder.data(), &QMediaRecorder::recorderStateChanged, this, &Camera::updateRecorderState);
    }

    m_imageCapture.reset(new DImageCapture(m_camera.data()));
    m_camera->captureSession()->setImageCapture(m_imageCapture.data());
    m_camera->captureSession()->setAudioInput(m_audioInput.get());
#else
    connect(m_camera.data(), &QCamera::stateChanged, this, &Camera::updateCameraState);
    connect(m_camera.data(), QOverload<QCamera::Error>::of(&QCamera::error), this, &Camera::displayCameraError);

    m_mediaRecorder.reset(new DMediaRecorder(m_camera.data()));
    connect(m_mediaRecorder.data(), &QMediaRecorder::stateChanged, this, &Camera::updateRecorderState);

     m_imageCapture.reset(new DImageCapture(m_camera.data()));
#endif

    connect(m_mediaRecorder.data(), &QMediaRecorder::durationChanged, this, &Camera::updateRecordTime);

#ifdef BUILD_Qt6
    connect(m_mediaRecorder.data(), &QMediaRecorder::errorChanged, this, &Camera::displayRecorderError);
#else
    connect(m_mediaRecorder.data(), QOverload<QMediaRecorder::Error>::of(&QMediaRecorder::error),
            this, &Camera::displayRecorderError);

    m_mediaRecorder->setMetaData(QMediaMetaData::Title, QVariant(QLatin1String("Test Title")));
#endif

    connect(ui->exposureCompensation, &QAbstractSlider::valueChanged, this, &Camera::setExposureCompensation);

#ifdef BUILD_Qt6
    updateCameraActive(m_camera->isActive());
    updateRecorderState(m_mediaRecorder->recorderState());

    connect(m_imageCapture.data(), &QImageCapture::readyForCaptureChanged, this, &Camera::readyForCapture);
    connect(m_imageCapture.data(), &QImageCapture::imageCaptured, this, &Camera::processCapturedImage);
    connect(m_imageCapture.data(), &QImageCapture::imageSaved, this, &Camera::imageSaved);
    connect(m_imageCapture.data(), &QImageCapture::errorOccurred, this, &Camera::displayCaptureError);
    readyForCapture(m_imageCapture->isReadyForCapture());

    updateCaptureMode();

    if (m_camera->cameraFormat().isNull()) {
        auto formats = cameraInfo.videoFormats();
        if (!formats.isEmpty()) {
            // Choose a decent camera format: Maximum resolution at at least 30 FPS
            // we use 29 FPS to compare against as some cameras report 29.97 FPS...
            QCameraFormat bestFormat;
            for (const auto &fmt : formats) {
                if (bestFormat.maxFrameRate() < 29 && fmt.maxFrameRate() > bestFormat.maxFrameRate())
                    bestFormat = fmt;
                else if (bestFormat.maxFrameRate() == fmt.maxFrameRate() &&
                         bestFormat.resolution().width()*bestFormat.resolution().height() <
                             fmt.resolution().width()*fmt.resolution().height())
                    bestFormat = fmt;
            }

            m_camera->setCameraFormat(bestFormat);
            m_mediaRecorder->setVideoFrameRate(bestFormat.maxFrameRate());
        }
    }
#else
    updateCameraState(m_camera->state());
    updateLockStatus(m_camera->lockStatus(), QCamera::UserRequest);
    updateRecorderState(m_mediaRecorder->state());

    connect(m_imageCapture.data(), &QCameraImageCapture::readyForCaptureChanged, this, &Camera::readyForCapture);
    connect(m_imageCapture.data(), &QCameraImageCapture::imageCaptured, this, &Camera::processCapturedImage);
    connect(m_imageCapture.data(), &QCameraImageCapture::imageSaved, this, &Camera::imageSaved);
    connect(m_imageCapture.data(), QOverload<int, QCameraImageCapture::Error, const QString &>::of(&QCameraImageCapture::error),
            this, &Camera::displayCaptureError);

    connect(m_camera.data(), QOverload<QCamera::LockStatus, QCamera::LockChangeReason>::of(&QCamera::lockStatusChanged),
            this, &Camera::updateLockStatus);

    ui->captureWidget->setTabEnabled(0, (m_camera->isCaptureModeSupported(QCamera::CaptureStillImage)));
    ui->captureWidget->setTabEnabled(1, (m_camera->isCaptureModeSupported(QCamera::CaptureVideo)));
    updateCaptureMode();
#endif

    if (m_camera->isFfmpegEnv()) {
        QWidget *pWgt = new QWidget(ui->viewfinder);
        QVBoxLayout *pLay = new QVBoxLayout;
        QVBoxLayout *pwgtLay = new QVBoxLayout;
        m_openglwidget = new PreviewOpenglWidget(pWgt, m_camera->isWaylandEnv());
        pwgtLay->addWidget(m_openglwidget);
        pWgt->setLayout(pwgtLay);
        pLay->addWidget(pWgt);
        ui->viewfinder->setLayout(pLay);
        connect(m_camera.data(), &DCamera::signalbuffer, (PreviewOpenglWidget *)m_openglwidget, &PreviewOpenglWidget::slotShowYuv);
        m_camera->setViewfinder(ui->viewfinder);
    } else {
        m_camera->setViewfinder(ui->viewfinder);
    }

    m_camera->start();
}

void Camera::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        return;

    switch (event->key()) {
#ifdef BUILD_Qt6
    displayViewfinder();
    event->accept();
    break;
    case Qt::Key_Camera:
        if (m_doImageCapture) {
            takeImage();
        } else {
            if (m_mediaRecorder->recorderState() == QMediaRecorder::RecordingState)
                stop();
            else
                record();
        }
        event->accept();
        break;
#else
    case Qt::Key_CameraFocus:
        displayViewfinder();
        m_camera->searchAndLock();
        event->accept();
        break;
    case Qt::Key_Camera:
        if (m_camera->captureMode() == QCamera::CaptureStillImage) {
            takeImage();
        } else {
            if (m_mediaRecorder->state() == QMediaRecorder::RecordingState)
                stop();
            else
                record();
        }
        event->accept();
        break;
#endif
    default:
        QMainWindow::keyPressEvent(event);
    }
}

void Camera::keyReleaseEvent(QKeyEvent *event)
{
#ifdef BUILD_Qt6
    QMainWindow::keyReleaseEvent(event);
#else
    if (event->isAutoRepeat())
        return;

    switch (event->key()) {
    case Qt::Key_CameraFocus:
        m_camera->unlock();
        break;
    default:
        QMainWindow::keyReleaseEvent(event);
    }
#endif
}

void Camera::updateRecordTime()
{
    QString str = QString("Recorded %1 sec").arg(m_mediaRecorder->duration() / 1000);
    ui->statusbar->showMessage(str);
}

void Camera::processCapturedImage(int requestId, const QImage &img)
{
    Q_UNUSED(requestId);
    QImage scaledImage = img.scaled(ui->viewfinder->size(),
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation);

    ui->lastImagePreviewLabel->setPixmap(QPixmap::fromImage(scaledImage));

    // Display captured image for 4 seconds.
    displayCapturedImage();
    QTimer::singleShot(4000, this, &Camera::displayViewfinder);
}

void Camera::record()
{
    m_mediaRecorder->record();
    updateRecordTime();
}

void Camera::pause()
{
    m_mediaRecorder->pause();
}

void Camera::stop()
{
    m_mediaRecorder->stop();
}

void Camera::setMuted(bool muted)
{
#ifdef BUILD_Qt6
//    m_captureSession.audioInput()->setMuted(muted);
#else
    m_mediaRecorder->setMuted(muted);
#endif
}

#ifdef BUILD_Qt6



#else
void Camera::toggleLock()
{
    switch (m_camera->lockStatus()) {
    case QCamera::Searching:
    case QCamera::Locked:
        m_camera->unlock();
        break;
    case QCamera::Unlocked:
        m_camera->searchAndLock();
    }
}

void Camera::updateLockStatus(QCamera::LockStatus status, QCamera::LockChangeReason reason)
{
    QColor indicationColor = Qt::black;

    switch (status) {
    case QCamera::Searching:
        indicationColor = Qt::yellow;
        ui->statusbar->showMessage(tr("Focusing..."));
        ui->lockButton->setText(tr("Focusing..."));
        break;
    case QCamera::Locked:
        indicationColor = Qt::darkGreen;
        ui->lockButton->setText(tr("Unlock"));
        ui->statusbar->showMessage(tr("Focused"), 2000);
        break;
    case QCamera::Unlocked:
        indicationColor = reason == QCamera::LockFailed ? Qt::red : Qt::black;
        ui->lockButton->setText(tr("Focus"));
        if (reason == QCamera::LockFailed)
            ui->statusbar->showMessage(tr("Focus Failed"), 2000);
    }

    QPalette palette = ui->lockButton->palette();
    palette.setColor(QPalette::ButtonText, indicationColor);
    ui->lockButton->setPalette(palette);
}
#endif

void Camera::takeImage()
{
    if (!m_camera->isFfmpegEnv()) {
        m_isCapturingImage = true;
    }
#ifdef BUILD_Qt6
     m_imageCapture->captureToFile();
#else
    m_imageCapture->capture();
#endif
}

void Camera::displayCaptureError(int id, const QCameraImageCapture::Error error, const QString &errorString)
{
    Q_UNUSED(id);
    Q_UNUSED(error);
    QMessageBox::warning(this, tr("Image Capture Error"), errorString);
    m_isCapturingImage = false;
}

void Camera::startCamera()
{
    m_camera->start();
}

void Camera::stopCamera()
{
    m_camera->stop();
}

void Camera::updateCaptureMode()
{
    int tabIndex = ui->captureWidget->currentIndex();

#ifdef BUILD_Qt6
    m_doImageCapture = (tabIndex == 0);
#else
    QCamera::CaptureModes captureMode = tabIndex == 0 ? QCamera::CaptureStillImage : QCamera::CaptureVideo;

    if (m_camera->isCaptureModeSupported(captureMode))
        m_camera->setCaptureMode(captureMode);
#endif
}

#ifdef BUILD_Qt6
void Camera::updateCameraActive(bool active)
{
    if (active) {
        ui->actionStartCamera->setEnabled(false);
        ui->actionStopCamera->setEnabled(true);
        ui->captureWidget->setEnabled(true);
        //ui->actionSettings->setEnabled(true);
    } else {
        ui->actionStartCamera->setEnabled(true);
        ui->actionStopCamera->setEnabled(false);
        ui->captureWidget->setEnabled(false);
//        ui->actionSettings->setEnabled(false);
    }
}
#else
void Camera::updateCameraState(QCamera::State state)
{
    switch (state) {
    case QCamera::ActiveState:
        ui->actionStartCamera->setEnabled(false);
        ui->actionStopCamera->setEnabled(true);
        ui->captureWidget->setEnabled(true);
        break;
    case QCamera::UnloadedState:
    case QCamera::LoadedState:
        ui->actionStartCamera->setEnabled(true);
        ui->actionStopCamera->setEnabled(false);
        ui->captureWidget->setEnabled(false);
    }
}
#endif

#ifdef BUILD_Qt6
void Camera::updateRecorderState(QMediaRecorder::RecorderState state)
{
    switch (state) {
    case QMediaRecorder::StoppedState:
        ui->recordButton->setEnabled(true);
//        ui->pauseButton->setEnabled(true);
        ui->stopButton->setEnabled(false);
//        ui->metaDataButton->setEnabled(true);
        break;
    case QMediaRecorder::PausedState:
        ui->recordButton->setEnabled(true);
//        ui->pauseButton->setEnabled(false);
        ui->stopButton->setEnabled(true);
//        ui->metaDataButton->setEnabled(false);
        break;
    case QMediaRecorder::RecordingState:
        ui->recordButton->setEnabled(false);
//        ui->pauseButton->setEnabled(true);
        ui->stopButton->setEnabled(true);
//        ui->metaDataButton->setEnabled(false);
        break;
    }
}
 void Camera::updateCameras()
 {
     ui->menuDevices->clear();
     const QList<QCameraDevice> availableCameras = QMediaDevices::videoInputs();
     for (const QCameraDevice &cameraDevice : availableCameras) {
         QAction *videoDeviceAction = new QAction(cameraDevice.description(), m_videoDevicesGroup);
         videoDeviceAction->setCheckable(true);
         videoDeviceAction->setData(QVariant::fromValue(cameraDevice));
         if (cameraDevice == QMediaDevices::defaultVideoInput())
             videoDeviceAction->setChecked(true);

         ui->menuDevices->addAction(videoDeviceAction);
     }
 }
#else
void Camera::updateRecorderState(QMediaRecorder::State state)
{
    switch (state) {
    case QMediaRecorder::StoppedState:
        ui->recordButton->setEnabled(true);
        ui->stopButton->setEnabled(false);
        break;
    case QMediaRecorder::PausedState:
        ui->recordButton->setEnabled(true);
        ui->stopButton->setEnabled(true);
        break;
    case QMediaRecorder::RecordingState:
        ui->recordButton->setEnabled(false);
        ui->stopButton->setEnabled(true);
        break;
    }
}
#endif

void Camera::setExposureCompensation(int index)
{
#ifdef BUILD_Qt6
    m_camera->setExposureCompensation(index*0.5);
#else
    m_camera->exposure()->setExposureCompensation(index * 0.5);
#endif
}

void Camera::displayRecorderError()
{
#ifdef BUILD_Qt6
    if (m_mediaRecorder->error() != QMediaRecorder::NoError)
        QMessageBox::warning(this, tr("Capture Error"), m_mediaRecorder->errorString());
#else
    QMessageBox::warning(this, tr("Capture Error"), m_mediaRecorder->errorString());
#endif
}

void Camera::displayCameraError()
{
#ifdef BUILD_Qt6
    if (m_camera->error() != QCamera::NoError)
            QMessageBox::warning(this, tr("Camera Error"), m_camera->errorString());
#else
    QMessageBox::warning(this, tr("Camera Error"), m_camera->errorString());
#endif
}

void Camera::updateCameraDevice(QAction *action)
{
#ifdef BUILD_Qt6
    setCamera(qvariant_cast<QCameraDevice>(action->data()));
#else
    setCamera(qvariant_cast<QCameraInfo>(action->data()));
#endif
}

void Camera::displayViewfinder()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void Camera::displayCapturedImage()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void Camera::readyForCapture(bool ready)
{
    ui->takeImageButton->setEnabled(ready);
}

void Camera::imageSaved(int id, const QString &fileName)
{
    Q_UNUSED(id);
    ui->statusbar->showMessage(tr("Captured \"%1\"").arg(QDir::toNativeSeparators(fileName)));

    m_isCapturingImage = false;
    if (m_applicationExiting)
        close();
}

void Camera::closeEvent(QCloseEvent *event)
{
    if (m_isCapturingImage) {
        setEnabled(false);
        m_applicationExiting = true;
        event->ignore();
    } else {
        event->accept();
    }
}
