// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "config.h"
#include "dcompositemanager.h"
#include "dgstplayerproxy_p.h"
#include "videosurface.h"
#include <QEventLoop>
#include <QMediaPlayer>
#include <QtGlobal>
#include <QtGui>
#include <random>

DMULTIMEDIA_BEGIN_NAMESPACE

enum AsyncReplyTag { SEEK,
                     CHANNEL,
                     SPEED };

DGstPlayerProxy::DGstPlayerProxy(QObject *parent)
    : DPlayerBackend(parent), d_ptr(new DGstPlayerProxyPrivate(this))
{
    Q_D(DGstPlayerProxy);
    d->pPlayer = new QMediaPlayer(this);
    d->pVideoSurface = new VideoSurface;
    d->pPlayer->setVideoOutput(d->pVideoSurface);

#ifdef BUILD_Qt6
    connect(d->pPlayer, &QMediaPlayer::playbackStateChanged, this, &DGstPlayerProxy::slotStateChanged);
#else
    connect(d->pPlayer, &QMediaPlayer::stateChanged, this, &DGstPlayerProxy::slotStateChanged);
#endif
    connect(d->pPlayer, &QMediaPlayer::mediaStatusChanged, this, &DGstPlayerProxy::slotMediaStatusChanged);
    connect(d->pPlayer, &QMediaPlayer::positionChanged, this, &DGstPlayerProxy::slotPositionChanged);
    connect(d->pPlayer, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(slotMediaError(QMediaPlayer::Error)));
    connect(d->pVideoSurface, &VideoSurface::frameAvailable, this, &DGstPlayerProxy::processFrame);
}

DGstPlayerProxy::~DGstPlayerProxy()
{
    Q_D(DGstPlayerProxy);
    if (DCompositeManager::get().composited()) {
        disconnect(this, &DGstPlayerProxy::stateChanged, nullptr, nullptr);
    }

    d->pVideoSurface->deleteLater();
    d->pVideoSurface = nullptr;
}

void DGstPlayerProxy::firstInit()
{
}

void DGstPlayerProxy::updateRoundClip(bool roundClip)
{
    Q_UNUSED(roundClip);
}

void DGstPlayerProxy::setCurrentFrame(const QImage &img)
{
    Q_D(DGstPlayerProxy);
    d->currentImage = img;
}

void DGstPlayerProxyPrivate::setState(DPlayerBackend::PlayState state)
{
    Q_Q(DGstPlayerProxy);
    if (q->state() != state) {
        q->setState(state);
        emit q->stateChanged();
    }
}

void DGstPlayerProxy::pollingEndOfPlayback()
{
    Q_D(DGstPlayerProxy);
    if (state() != DPlayerBackend::Stopped) {
        stop();
        d->setState(DPlayerBackend::Stopped);
        return;
    }
}

const PlayingMovieInfo &DGstPlayerProxy::playingMovieInfo()
{
    Q_D(DGstPlayerProxy);
    return d->movieInfo;
}

bool DGstPlayerProxy::isPlayable() const
{
    return true;
}

#ifdef BUILD_Qt6
void DGstPlayerProxy::slotStateChanged(QMediaPlayer::PlaybackState newState)
#else
void DGstPlayerProxy::slotStateChanged(QMediaPlayer::State newState)
#endif
{
    Q_D(DGstPlayerProxy);
    switch (newState) {
    case QMediaPlayer::StoppedState:
        d->setState(PlayState::Stopped);
        break;
    case QMediaPlayer::PlayingState:
        d->setState(PlayState::Playing);
        break;
    case QMediaPlayer::PausedState:
        d->setState(PlayState::Paused);
        break;
    }
}

void DGstPlayerProxy::slotMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    Q_D(DGstPlayerProxy);
    switch (status) {
    case QMediaPlayer::BufferedMedia:
        d->setState(PlayState::Playing);
        emit fileLoaded();
        break;
    default:
        break;
    }
}

void DGstPlayerProxy::slotPositionChanged(qint64 position)
{
    Q_UNUSED(position);
    emit elapsedChanged();
}

void DGstPlayerProxy::slotMediaError(QMediaPlayer::Error error)
{
    switch (error) {
    case QMediaPlayer::ResourceError:
    case QMediaPlayer::FormatError:
    case QMediaPlayer::NetworkError:
    case QMediaPlayer::AccessDeniedError:
#ifndef BUILD_Qt6
    case QMediaPlayer::ServiceMissingError:
#endif
        emit sigMediaError();
        break;
    default:
        break;
    }
}

void DGstPlayerProxy::savePlaybackPosition()
{
    if (state() == PlayState::Stopped) {
        return;
    }
}

void DGstPlayerProxy::setPlaySpeed(double dTimes)
{
    Q_D(DGstPlayerProxy);
    d->pPlayer->setPlaybackRate(dTimes);

    d->pPlayer->setPosition(d->pPlayer->position());
}

void DGstPlayerProxy::volumeUp()
{
    if (volume() >= 200) return;

    changeVolume(volume() + 10);
}

void DGstPlayerProxy::volumeDown()
{
    if (volume() <= 0) return;

    changeVolume(volume() - 10);
}

void DGstPlayerProxy::changeVolume(int nVol)
{
    Q_D(DGstPlayerProxy);
#ifdef BUILD_Qt6
    //// Qt6 QMediaPlayer没有setVolume

#else
    d->pPlayer->setVolume(nVol);
#endif
}

int DGstPlayerProxy::volume() const
{
    Q_D(const DGstPlayerProxy);
#ifdef BUILD_Qt6
    //// Qt6 QMediaPlayer没有volume()
    int nActualVol = 100;
#else
    int nActualVol = d->pPlayer->volume();
#endif
    int nDispalyVol = static_cast<int>((nActualVol - 40) / 60.0 * 200.0);
    return nDispalyVol;
}

bool DGstPlayerProxy::muted() const
{
    Q_D(const DGstPlayerProxy);
#ifdef BUILD_Qt6
    //// Qt6 QMediaPlayer没有 isMuted

#else
    return d->pPlayer->isMuted();
#endif
}

void DGstPlayerProxy::toggleMute()
{
    Q_D(DGstPlayerProxy);
    bool bMute = false;

#ifdef BUILD_Qt6
    //// Qt6 QMediaPlayer没有 setMuted

#else
    bMute = d->pPlayer->isMuted();
    d->pPlayer->setMuted(!bMute);
#endif
}

void DGstPlayerProxy::setMute(bool bMute)
{
    Q_D(DGstPlayerProxy);

#ifdef BUILD_Qt6
    //// Qt6 QMediaPlayer没有 setMuted

#else
    d->pPlayer->setMuted(bMute);
#endif
}

void DGstPlayerProxy::updateSubStyle(const QString &font, int sz)
{
    Q_UNUSED(font);
    Q_UNUSED(sz);
}

void DGstPlayerProxy::setSubCodepage(const QString &cp)
{
    Q_UNUSED(cp);
}

QString DGstPlayerProxy::subCodepage()
{
    return QString();
}

void DGstPlayerProxy::addSubSearchPath(const QString &path)
{
    Q_UNUSED(path);
}

bool DGstPlayerProxy::loadSubtitle(const QFileInfo &fi)
{
    Q_UNUSED(fi);
    return false;
}

void DGstPlayerProxy::toggleSubtitle()
{
}

bool DGstPlayerProxy::isSubVisible()
{
    return false;
}

void DGstPlayerProxy::selectSubtitle(int id)
{
    Q_UNUSED(id);
}

void DGstPlayerProxy::selectTrack(int id)
{
    Q_UNUSED(id);
}

void DGstPlayerProxy::setSubDelay(double secs)
{
    Q_UNUSED(secs);
}

double DGstPlayerProxy::subDelay() const
{
    return .0;
}

int DGstPlayerProxy::aid() const
{
    return 0;
}

int DGstPlayerProxy::sid() const
{
    return 0;
}

void DGstPlayerProxy::changeSoundMode(DPlayerBackend::SoundMode)
{
}

void DGstPlayerProxy::setVideoAspect(double r)
{
    Q_UNUSED(r);
}

double DGstPlayerProxy::videoAspect() const
{
    return 0.0;
}

int DGstPlayerProxy::videoRotation() const
{
    return 0;
}

void DGstPlayerProxy::setVideoRotation(int degree)
{
    Q_UNUSED(degree);
}

QImage DGstPlayerProxy::takeScreenshot() const
{
    Q_D(const DGstPlayerProxy);
    return d->currentImage;
}

void DGstPlayerProxy::burstScreenshot()
{
    Q_D(DGstPlayerProxy);
    int nCurrentPos = static_cast<int>(d->pPlayer->position());
    int nDuration = static_cast<int>(d->pPlayer->duration() / 15);
    int nTime = 0;

    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<int> uniform_dist(0, nDuration);
    for (int i = 0; i < 15; i++) {
        nTime = nDuration * i + uniform_dist(g) - 200;
        d->pPlayer->setPosition(nTime);
        QEventLoop loop;
        QTimer::singleShot(200, &loop, SLOT(quit()));
        loop.exec();
        emit notifyScreenshot(d->currentImage, nTime / 1000);
    }

    d->pPlayer->setPosition(nCurrentPos);
}

void DGstPlayerProxy::stopBurstScreenshot()
{
}

QVariant DGstPlayerProxy::getProperty(const QString &) const
{
    return 0;
}

void DGstPlayerProxy::setProperty(const QString &, const QVariant &)
{
}

void DGstPlayerProxy::nextFrame()
{
}

void DGstPlayerProxy::previousFrame()
{
}

void DGstPlayerProxy::makeCurrent()
{
}

void DGstPlayerProxy::changehwaccelMode(DPlayerBackend::hwaccelMode hwaccelMode)
{
    Q_UNUSED(hwaccelMode);
}

void DGstPlayerProxyPrivate::initMember()
{

    nBurstStart = 0;
    bInBurstShotting = false;
    posBeforeBurst = false;
    bExternalSubJustLoaded = false;
    bConnectStateChange = false;
    bPauseOnStart = false;
    bInited = false;
    bHwaccelAuto = false;
    bLastIsSpecficFormat = false;
    listBurstPoints.clear();
    mapWaitSet.clear();
    vecWaitCommand.clear();
}

void DGstPlayerProxy::play()
{
    Q_D(DGstPlayerProxy);

#ifdef BUILD_Qt6
    //// 需要确认setMedia与setSource功能是否相同
    if (urlFile().isLocalFile()) {
        QString strFilePath = QFileInfo(urlFile().toLocalFile()).absoluteFilePath();
        d->pPlayer->setSource(QUrl::fromLocalFile(strFilePath));
    } else {
        d->pPlayer->setSource(urlFile());
    }
#else
    if (urlFile().isLocalFile()) {
        QString strFilePath = QFileInfo(urlFile().toLocalFile()).absoluteFilePath();
        d->pPlayer->setMedia(QMediaContent(QUrl::fromLocalFile(strFilePath)));
    } else {
        d->pPlayer->setMedia(QMediaContent(urlFile()));
    }
#endif
    d->pPlayer->play();
}

void DGstPlayerProxy::pauseResume()
{
    Q_D(DGstPlayerProxy);
    if (state() == PlayState::Playing) {
        d->pPlayer->pause();
    } else if (state() == PlayState::Paused) {
        d->pPlayer->play();
    }
}

void DGstPlayerProxy::stop()
{
    Q_D(DGstPlayerProxy);
    d->pPlayer->stop();
}

int DGstPlayerProxyPrivate::volumeCorrection(int displayVol)
{
    int realVol = 0;
    if (DCompositeManager::get().check_wayland_env()) {
        realVol = displayVol > 100 ? 100 + (displayVol - 100) / 10 * 5 : displayVol;
    } else {
        realVol = static_cast<int>((displayVol / 200.0) * 60.0 + 40);
    }
    return (realVol == 40 ? 0 : realVol);
}

void DGstPlayerProxy::seekForward(int nSecs)
{
    Q_D(DGstPlayerProxy);
    qint64 nPosition = 0;

    nPosition = d->pPlayer->position();
    nPosition = nPosition + nSecs * 1000;

    if (state() != PlayState::Stopped) {
        d->pPlayer->setPosition(nPosition);
    }
}

void DGstPlayerProxy::seekBackward(int nSecs)
{
    Q_D(DGstPlayerProxy);
    qint64 nPosition = 0;

    nPosition = d->pPlayer->position();
    nPosition = nPosition - nSecs * 1000;

    if (state() != PlayState::Stopped) {
        d->pPlayer->setPosition(nPosition);
    }
}

void DGstPlayerProxy::seekAbsolute(int nPos)
{
    Q_D(DGstPlayerProxy);
    if (state() != PlayState::Stopped) d->pPlayer->setPosition(nPos * 1000);
}

QSize DGstPlayerProxy::videoSize() const
{
    return QSize(0, 0);
}

qint64 DGstPlayerProxy::duration() const
{
    Q_D(const DGstPlayerProxy);
    return d->pPlayer->duration() / 1000;
}

qint64 DGstPlayerProxy::elapsed() const
{
    Q_D(const DGstPlayerProxy);
    return d->pPlayer->position() / 1000;
}

void DGstPlayerProxyPrivate::updatePlayingMovieInfo()
{
}

DMULTIMEDIA_END_NAMESPACE   // end of namespace
