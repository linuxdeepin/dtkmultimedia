// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcamera_p.h"
#include "dmediacapturesession.h"
#include <QDir>
#include <QLibraryInfo>
#include <QProcessEnvironment>

DMULTIMEDIA_USE_NAMESPACE

DCamera::DCamera(QCamera *parent)
    : QCamera(parent), d_ptr(new DCameraPrivate(this))
{
    Q_D(DCamera);
    d->initCamera();
}

DCamera::DCamera(const QCameraInfo &cameraInfo, QObject *parent)
    : QCamera(cameraInfo.deviceName().toLatin1(), parent), d_ptr(new DCameraPrivate(this))
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
    //TODO
}
void DCameraPrivate::initDynamicLibPath()
{
    //TODO
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
    //TODO
    return 0;
}

int DCameraPrivate::cameraUnInit()
{
    //TODO
    return 0;
}

void DCamera::takeOne(const QString &location)
{
    Q_D(DCamera);
    //TODO
}

void DCamera::takeVideo(const QString &location)
{
    Q_D(DCamera);
    //TODO
}

void DCamera::start()
{
    Q_D(DCamera);
    if (isFfmpegEnv()) {
        if (d->state == QCamera::ActiveState) {
            return;
        }
        d->dcamInit();
        setstate(QCamera::ActiveState);
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
    setstate(QCamera::UnloadedState);
}

bool DCamera::isFfmpegEnv()
{
    //TODO
    return false;
}

bool DCamera::isWaylandEnv()
{
    Q_D(DCamera);
    return d->isWayland;
}

void DCamera::setViewfinder(QVideoWidget *viewfinder)
{
    Q_D(DCamera);
    if (isFfmpegEnv()) {
        d->viewfinder = viewfinder;
    } else {
        QCamera::setViewfinder(viewfinder);
    }
}

void DCamera::setstate(QCamera::State newState)
{
    Q_D(DCamera);
    if (d->state != newState) {
        d->state = newState;
        emit QCamera::stateChanged(newState);
    }
}

DMediaCaptureSession *DCamera::captureSession() const
{
    Q_D(const DCamera);
    return d->captureSession;
}

int DCamera::checkCamera()
{
    //TODO
    return 0;
}

QStringList DCamera::devList()
{
    QStringList sList;
    //TODO
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
    width = 0;
    height = 0;
    //TODO
    return QByteArray();
}

void DCamera::resolutionSettings(const QSize &size)
{
    //TODO
}

QList<QSize> DCamera::resolutions()
{
    Q_D(const DCamera);
    QList<QSize> lSize;
    //TODO
    return lSize;
}

void DCamera::setCameraCollectionFormat(const uint32_t &pixelformat)
{
    //TODO
}

void DCamera::setFilter(const QString &filter)
{
    Q_D(const DCamera);
    //TODO
}

void DCamera::setExposure(const int &exposure)
{
    Q_D(const DCamera);
    //TODO
    QCamera::exposure()->setExposureCompensation(exposure);
}

QList<uint32_t> DCamera::supportedViewfinderPixelFormats()
{
    Q_D(const DCamera);
    QList<uint32_t> lFormats;
    //TODO
    return lFormats;
}
