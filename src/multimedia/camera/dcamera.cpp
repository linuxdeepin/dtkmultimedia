// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcamera_p.h"
#include "dmediacapturesession.h"
#include <QDir>
#include <QLibraryInfo>
#include <QProcessEnvironment>
#ifdef BUILD_Qt6

#else
#include <QCameraExposure>
#endif
#include <QStandardPaths>
#include "datamanager.h"
#include "majorimageprocessingthread.h"
#include "dmediacapturesession.h"
extern "C" {
#include "LPF_V4L2.h"
#include "v4l2_core.h"
#include "camview.h"
#include "load_libs.h"
}

DMULTIMEDIA_USE_NAMESPACE

DCamera::DCamera(QCamera *parent)
    : QCamera(parent), d_ptr(new DCameraPrivate(this))
{
    Q_D(DCamera);
    d->initCamera();
}

#ifdef BUILD_Qt6
DCamera::DCamera(const QCameraDevice &cameraDevice, QObject *parent)
    : QCamera(cameraDevice, parent), d_ptr(new DCameraPrivate(this))
#else
DCamera::DCamera(const QCameraInfo &cameraInfo, QObject *parent)
    : QCamera(cameraInfo.deviceName().toLatin1(), parent), d_ptr(new DCameraPrivate(this))
#endif
{
    Q_D(DCamera);
    d->initCamera();
}

DCamera::~DCamera()
{
    Q_D(DCamera);
    if (isFfmpegEnv()) {
        d->cameraUnInit();
        if (d->imgPrcThread)
            delete d->imgPrcThread;
    }
    if (d->captureSession)
        delete d->captureSession;
}

void DCameraPrivate::initCamera()
{
    Q_Q(DCamera);
    bool bffmpegEnv = CheckFFmpegEnv();
    DataManager::instance()->setEncodeEnv(bffmpegEnv ? FFmpeg_Env : GStreamer_Env);
    if (bffmpegEnv) {
        cameraUnInit();
        isWayland = CheckWayland();
        initDynamicLibPath();
        gviewencoder_init();
        v4l2core_init();

        imgPrcThread = new MajorImageProcessingThread;
        imgPrcThread->setParent(this);
        imgPrcThread->setObjectName("MajorThread");
#ifndef __mips__
        connect(imgPrcThread, &MajorImageProcessingThread::sigYUVFrame, q, &DCamera::signalbuffer);
#endif
    }
    captureSession = new DMediaCaptureSession(q);
}
void DCameraPrivate::initDynamicLibPath()
{
    LoadLibNames tmp;
    QByteArray avcodec = libPath("libavcodec.so").toLatin1();
    tmp.chAvcodec = avcodec.data();
    QByteArray avformat = libPath("libavformat.so").toLatin1();
    tmp.chAvformat = avformat.data();
    QByteArray avutil = libPath("libavutil.so").toLatin1();
    tmp.chAvutil = avutil.data();
    QByteArray udev = libPath("libudev.so").toLatin1();
    tmp.chUdev = udev.data();
    QByteArray usb = libPath("libusb-1.0.so").toLatin1();
    tmp.chUsb = usb.data();
    QByteArray portaudio = libPath("libportaudio.so").toLatin1();
    tmp.chPortaudio = portaudio.data();
    QByteArray v4l2 = libPath("libv4l2.so").toLatin1();
    tmp.chV4l2 = v4l2.data();
    QByteArray ffmpegthumbnailer = libPath("libffmpegthumbnailer.so").toLatin1();
    tmp.chFfmpegthumbnailer = ffmpegthumbnailer.data();
    QByteArray swscale = libPath("libswscale.so").toLatin1();
    tmp.chSwscale = swscale.data();
    QByteArray swresample = libPath("libswresample.so").toLatin1();
    tmp.chSwresample = swresample.data();
    setLibNames(tmp);
}

QString DCameraPrivate::libPath(const QString &strlib)
{
    QDir dir;
    QString path = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    dir.setPath(path);
    QStringList list = dir.entryList(QStringList() << (strlib + "*"), QDir::NoDotAndDotDot | QDir::Files);   //filter name with strlib

    if (list.contains(strlib))
        return strlib;

    list.sort();
    if (list.size() > 0)
        return list.last();

    return "";
}

bool DCameraPrivate::CheckFFmpegEnv()
{
    QDir dir;
    QString path = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    dir.setPath(path);
    QStringList list = dir.entryList(QStringList() << (QString("libavcodec") + "*"), QDir::NoDotAndDotDot | QDir::Files);
    if (list.contains("libavcodec.so")) {
        return true;
    }
    return false;
}

bool DCameraPrivate::CheckWayland()
{
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        return true;
    } else
        return false;
}

int DCameraPrivate::dcamInit()
{
    Q_Q(DCamera);
    QStringList sList = q->devList();
    if (sList.isEmpty()) return -1;
    return dcamInit(sList.first());
}

int DCameraPrivate::dcamInit(const QString &sDevName)
{
    int nRet = camInit(sDevName.toLatin1().data());

    if (nRet == E_OK) {
        if (imgPrcThread) {
            imgPrcThread->init();
            imgPrcThread->start();
        }
        DataManager::instance()->setdevStatus(CAM_CANUSE);
    } else if (nRet == E_FORMAT_ERR) {
        v4l2_dev_t *vd = get_v4l2_device_handler();

        if (vd != nullptr)
            close_v4l2_device_handler();

        qWarning() << "camInit failed";
        DataManager::instance()->setdevStatus(CAM_CANNOT_USE);
    } else {
        v4l2_dev_t *vd = get_v4l2_device_handler();

        if (vd != nullptr)
            close_v4l2_device_handler();

        DataManager::instance()->setdevStatus(NOCAM);
    }
    return nRet;
}

int DCameraPrivate::cameraUnInit()
{
    if (imgPrcThread) {
        imgPrcThread->stop();
        imgPrcThread->wait();
    }
    camUnInit();
}

void DCamera::takeOne(const QString &location)
{
    Q_D(DCamera);
    if (location.isEmpty() || location.isNull())
        d->imgPrcThread->m_strPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + QDir::separator() + "uosimage.jpg";
    d->imgPrcThread->m_bTake = true;
}

void DCamera::takeVideo(const QString &location)
{
    Q_D(DCamera);
    if (!d->bVideoStatus) {
        if (location.isEmpty() || location.isNull()) {
            set_video_path(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation).toLatin1().data());
            set_video_name(DataManager::instance()->getstrFileName().toStdString().c_str());
        } else {
            set_video_path(QFileInfo(location).path().toLatin1().data());
            set_video_name(QFileInfo(location).fileName().toLatin1().data());
        }
        start_encoder_thread();
        d->bVideoStatus = true;
    } else {
        if (video_capture_get_save_video() == 1) {
            set_video_time_capture(0);
            stop_encoder_thread();
        }
        reset_video_timer();
        d->bVideoStatus = false;
    }
}

void DCamera::start()
{
    Q_D(DCamera);
    if (isFfmpegEnv()) {
#ifdef BUILD_Qt6
        if (isActive()) {
            return;
        }
        d->dcamInit();
        setActive(true);
        emit QCamera::activeChanged(true);
        setstate(DCamera::ActiveState);
#else
        if (d->state == QCamera::ActiveState) {
            return;
        }
        d->dcamInit();
        setstate(QCamera::ActiveState);
#endif
    } else {
        QCamera::start();
    }
}

void DCamera::stop()
{
    Q_D(DCamera);
    if (isFfmpegEnv()) {
        d->cameraUnInit();
    } else {
        QCamera::stop();
    }
#ifdef BUILD_Qt6
    setActive(false);
#else
    setstate(QCamera::UnloadedState);
#endif
}

bool DCamera::isFfmpegEnv()
{
//    return false;
    return DataManager::instance()->encodeEnv() == FFmpeg_Env;
}

bool DCamera::isWaylandEnv()
{
    Q_D(DCamera);
    return d->isWayland;
}
#ifdef BUILD_Qt6
void DCamera::setViewfinder(QWidget *viewfinder)
#else
void DCamera::setViewfinder(QVideoWidget *viewfinder)
#endif
{
    Q_D(DCamera);
    if (isFfmpegEnv()) {
        d->viewfinder = viewfinder;
    } else {
#ifdef BUILD_Qt6
        // Qt6已移除setViewfinder接口
        QVideoWidget *wigdet = new QVideoWidget(viewfinder);
        wigdet->resize(429, 334);
        d->captureSession->setVideoOutput(wigdet);
        // d->captureSession->setVideoOutput(viewfinder);
        d->captureSession->setCamera(this);
#else
        QCamera::setViewfinder(viewfinder);
#endif
    }
}

#ifdef BUILD_Qt6
void DCamera::setstate(State newState)
{
    Q_D(DCamera);
    if (d->state != newState) {
        d->state = newState;
        emit DCamera::stateChanged(newState);
    }
}
#else
void DCamera::setstate(QCamera::State newState)
{
    Q_D(DCamera);
    if (d->state != newState) {
        d->state = newState;
        emit QCamera::stateChanged(newState);
    }
}
#endif

DMediaCaptureSession *DCamera::captureSession() const
{
    Q_D(const DCamera);
    return d->captureSession;
}

int DCamera::checkCamera()
{
    return (int)DataManager::instance()->getdevStatus();
}

QStringList DCamera::devList()
{
    QStringList sList;
    v4l2_device_list_t *devlist = get_device_list();
    if (!devlist) return sList;
    for (int i = 0; i < devlist->num_devices; i++) {
        char *devName = devlist->list_devices->device;
        sList << devName;
    }
    return sList;
}

void DCamera::openDev(const QString &sDevName)
{
    Q_D(DCamera);
    d->dcamInit(sDevName);
}

void DCamera::closeDev()
{
    Q_D(DCamera);
    d->cameraUnInit();
}

QByteArray DCamera::yuvbuffer(uint &width, uint &height)
{
    Q_D(const DCamera);
    QByteArray data;
    if (d->imgPrcThread->isRunning()) {
        width = d->imgPrcThread->getResolution().width();
        height = d->imgPrcThread->getResolution().height();
        data.append((char *)d->imgPrcThread->getbuffer(), width * height * 1.5);
    }
    return data;
}

void DCamera::resolutionSettings(const QSize &size)
{
    v4l2core_prepare_new_resolution(get_v4l2_device_handler(), size.width(), size.height());
    request_format_update(1);
}

QList<QSize> DCamera::resolutions()
{
    Q_D(const DCamera);
    QList<QSize> lSize;
    if (d->imgPrcThread->isRunning()) {
        v4l2_dev_t *my_vd = get_v4l2_device_handler();
        config_t *my_config = config_get();
        if (my_config && my_vd) {
            struct v4l2_frmsizeenum fsize;
            memset(&fsize, 0, sizeof(fsize));
            fsize.index = 0;
            fsize.pixel_format = my_config->format;
            while (!xioctl(my_vd->fd, VIDIOC_ENUM_FRAMESIZES, &fsize)) {
                qInfo() << (QSize(fsize.discrete.width, fsize.discrete.height));
                lSize.prepend(QSize(fsize.discrete.width, fsize.discrete.height));
                fsize.index++;
            }
        }
    }
    return lSize;
}

void DCamera::setCameraCollectionFormat(const uint32_t &pixelformat)
{
    config_t *my_config = config_get();
    if (my_config) {
        my_config->format = pixelformat;
    }
}

void DCamera::setFilter(const QString &filter)
{
    Q_D(const DCamera);
    if (d->imgPrcThread->isRunning()) {
        d->imgPrcThread->setFilter(filter);
    }
}

void DCamera::setExposure(const int &exposure)
{
    Q_D(const DCamera);
    if (d->imgPrcThread->isRunning()) {
        d->imgPrcThread->setExposure(exposure);
    }
#ifdef BUILD_Qt6
    QCamera::setExposureCompensation(exposure);
#else
    QCamera::exposure()->setExposureCompensation(exposure);
#endif
}

QList<uint32_t> DCamera::supportedViewfinderPixelFormats()
{
    Q_D(const DCamera);
    QList<uint32_t> lFormats;
    if (d->imgPrcThread->isRunning()) {
        struct v4l2_fmtdesc ffmt;
        memset(&ffmt, 0, sizeof(ffmt));
        ffmt.index = 0;
        ffmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        v4l2_dev_t *my_vd = get_v4l2_device_handler();
        if (my_vd) {
            while (!xioctl(my_vd->fd, VIDIOC_ENUM_FMT, &ffmt)) {
                qInfo() << (char *)ffmt.description;
                lFormats.append(ffmt.pixelformat);
                ffmt.index++;
            }
        }
    }
    return lFormats;
}
