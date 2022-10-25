// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"
#include "dgstplayerproxy.h"

#include <random>
#include <QtGui>
#include <QtGlobal>
#include <QMediaPlayer>
#include "dcompositemanager.h"
#include <QEventLoop>
#include "videosurface.h"

DMULTIMEDIA_BEGIN_NAMESPACE

enum AsyncReplyTag {
    SEEK,
    CHANNEL,
    SPEED
};

DGstPlayerProxy::DGstPlayerProxy(QObject *parent)
    :DPlayerBackend (parent)
{
    m_pPlayer = new QMediaPlayer(this);
    m_pVideoSurface = new VideoSurface;
    m_pPlayer->setVideoOutput(m_pVideoSurface);

    connect(m_pPlayer,&QMediaPlayer::stateChanged,this,&DGstPlayerProxy::slotStateChanged);
    connect(m_pPlayer,&QMediaPlayer::mediaStatusChanged,this,&DGstPlayerProxy::slotMediaStatusChanged);
    connect(m_pPlayer,&QMediaPlayer::positionChanged,this,&DGstPlayerProxy::slotPositionChanged);
    connect(m_pPlayer,SIGNAL(error(QMediaPlayer::Error)),this,SLOT(slotMediaError(QMediaPlayer::Error)));
    connect(m_pVideoSurface, &VideoSurface::frameAvailable, this, &DGstPlayerProxy::processFrame);
}


DGstPlayerProxy::~DGstPlayerProxy()
{
    if (DCompositeManager::get().composited()) {
        disconnect(this, &DGstPlayerProxy::stateChanged, nullptr, nullptr);
    }

    m_pVideoSurface->deleteLater();
    m_pVideoSurface = nullptr;
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
    m_currentImage = img;
}

void DGstPlayerProxy::setState(PlayState state)
{
    bool bRawFormat = false;

    if (m_state != state) {
        m_state = state;
        emit stateChanged();
    }
}

void DGstPlayerProxy::pollingEndOfPlayback()
{
    if (m_state != DPlayerBackend::Stopped) {
        stop();
        setState(DPlayerBackend::Stopped);
        return;
    }
}

const PlayingMovieInfo &DGstPlayerProxy::playingMovieInfo()
{
    return m_movieInfo;
}

void DGstPlayerProxy::slotStateChanged(QMediaPlayer::State newState)
{
    switch (newState) {
    case QMediaPlayer::StoppedState:
        setState(PlayState::Stopped);
        break;
     case QMediaPlayer::PlayingState:
        setState(PlayState::Playing);
        break;
    case QMediaPlayer::PausedState:
       setState(PlayState::Paused);
       break;
    }
}

void DGstPlayerProxy::slotMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    switch (status) {
    case QMediaPlayer::BufferedMedia:
        setState(PlayState::Playing);
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
    case QMediaPlayer::ServiceMissingError:
        emit sigMediaError();
        break;
    default:
        break;
    }
}

//void DGstPlayerProxy::processFrame(QVideoFrame &frame)
//{
//    frame.map(QAbstractVideoBuffer::ReadOnly);
//    QImage recvImage(frame.bits(), frame.width(), frame.height(), QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat()));
//    m_currentImage = recvImage;
//    emit gstImageShow(recvImage);
//    frame.unmap();
//}

void DGstPlayerProxy::savePlaybackPosition()
{
    if (state() == PlayState::Stopped) {
        return;
    }
}

void DGstPlayerProxy::setPlaySpeed(double dTimes)
{
    m_pPlayer->setPlaybackRate(dTimes);

    m_pPlayer->setPosition(m_pPlayer->position());   // 某些格式音频需要重新seek后才生效
}

void DGstPlayerProxy::volumeUp()
{
    if (volume() >= 200)
        return;

    changeVolume(volume() + 10);
}

void DGstPlayerProxy::volumeDown()
{
    if (volume() <= 0)
        return;

    changeVolume(volume() - 10);
}

void DGstPlayerProxy::changeVolume(int nVol)
{
    m_pPlayer->setVolume(nVol);
}

int DGstPlayerProxy::volume() const
{
    int nActualVol = m_pPlayer->volume();
    int nDispalyVol = static_cast<int>((nActualVol - 40) / 60.0 * 200.0);
    return nDispalyVol;
}

bool DGstPlayerProxy::muted() const
{
    return m_pPlayer->isMuted();
}

void DGstPlayerProxy::toggleMute()
{
    bool bMute = false;

    bMute = m_pPlayer->isMuted();
    m_pPlayer->setMuted(!bMute);
}

void DGstPlayerProxy::setMute(bool bMute)
{
    m_pPlayer->setMuted(bMute);
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

QImage DGstPlayerProxy::takeScreenshot()
{
    return m_currentImage;
}

void DGstPlayerProxy::burstScreenshot()
{
    int nCurrentPos = static_cast<int>(m_pPlayer->position());
    int nDuration = static_cast<int>(m_pPlayer->duration() / 15);
    int nTime = 0;

    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<int> uniform_dist(0, nDuration);
    //m_listBurstPoints.clear();
    for (int i = 0; i < 15; i++) {
        //m_listBurstPoints.append(nDuration * i + uniform_dist(g));
        nTime = nDuration * i + uniform_dist(g) - 200;
        m_pPlayer->setPosition(nTime);
        QEventLoop loop;
        QTimer::singleShot(200, &loop, SLOT(quit()));
        loop.exec();
        emit notifyScreenshot(m_currentImage, nTime/1000);
    }

    m_pPlayer->setPosition(nCurrentPos);
}

void DGstPlayerProxy::stopBurstScreenshot()
{

}

QVariant DGstPlayerProxy::getProperty(const QString &)
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

void DGstPlayerProxy::initMember()
{
    m_nBurstStart = 0;

    m_bInBurstShotting = false;
    m_posBeforeBurst = false;
    m_bExternalSubJustLoaded = false;
    m_bConnectStateChange = false;
    m_bPauseOnStart = false;
    m_bInited = false;
    m_bHwaccelAuto = false;
    m_bLastIsSpecficFormat = false;

    m_listBurstPoints.clear();
    m_mapWaitSet.clear();
    m_vecWaitCommand.clear();

    m_pConfig = nullptr;
}

void DGstPlayerProxy::play()
{
    bool bRawFormat = false;

//    if (0 < dynamic_cast<PlayerEngine *>(m_pParentWidget)->getplaylist()->size()) {
//        PlayItemInfo currentInfo = dynamic_cast<PlayerEngine *>(m_pParentWidget)->getplaylist()->currentInfo();
//        bRawFormat = currentInfo.mi.isRawFormat();
//    }
  
    if (m_file.isLocalFile()) {
        QString strFilePath = QFileInfo(m_file.toLocalFile()).absoluteFilePath();
        m_pPlayer->setMedia(QMediaContent(QUrl::fromLocalFile(strFilePath)));
    } else {
        m_pPlayer->setMedia(QMediaContent(m_file));
    }
    m_pPlayer->play();
}

void DGstPlayerProxy::pauseResume()
{
    if (m_state == PlayState::Playing){
        m_pPlayer->pause();
    }else if (m_state == PlayState::Paused) {
        m_pPlayer->play();
    }
}

void DGstPlayerProxy::stop()
{
    m_pPlayer->stop();
}

int DGstPlayerProxy::volumeCorrection(int displayVol)
{
    int realVol = 0;
    if (DCompositeManager::get().check_wayland_env()) {
        //>100时，mpv按照显示音量：mpv 10：5的比例调节音量
        realVol = displayVol > 100 ? 100 + (displayVol - 100) / 10 * 5 : displayVol;
    } else {
        realVol = static_cast<int>((displayVol / 200.0) * 60.0 + 40);
    }
    return (realVol == 40 ? 0 : realVol);
}

void DGstPlayerProxy::seekForward(int nSecs)
{
    qint64 nPosition = 0;

    nPosition = m_pPlayer->position();
    nPosition = nPosition + nSecs*1000;

    if (state() != PlayState::Stopped) {
        m_pPlayer->setPosition(nPosition);
    }
}

void DGstPlayerProxy::seekBackward(int nSecs)
{
    qint64 nPosition = 0;

    nPosition = m_pPlayer->position();
    nPosition = nPosition - nSecs*1000;

    if (state() != PlayState::Stopped) {
        m_pPlayer->setPosition(nPosition);
    }
}

void DGstPlayerProxy::seekAbsolute(int nPos)
{
    if (state() != PlayState::Stopped)
        m_pPlayer->setPosition(nPos*1000);
}

QSize DGstPlayerProxy::videoSize() const
{
//    PlayItemInfo currentInfo;
//    if (0 < dynamic_cast<PlayerEngine *>(m_pParentWidget)->getplaylist()->size()) {
//         currentInfo = dynamic_cast<PlayerEngine *>(m_pParentWidget)->getplaylist()->currentInfo();
//    }

//    return QSize(currentInfo.mi.width, currentInfo.mi.height);
    return QSize(0, 0);
}

qint64 DGstPlayerProxy::duration() const
{
//    PlayItemInfo currentInfo;
//    if (0 < dynamic_cast<PlayerEngine *>(m_pParentWidget)->getplaylist()->size()) {
//         currentInfo = dynamic_cast<PlayerEngine *>(m_pParentWidget)->getplaylist()->currentInfo();
//    }
//    if(currentInfo.mi.duration > 0){
//        return currentInfo.mi.duration;
//    } else {
//        return m_pPlayer->duration()/1000;
//    }
    return m_pPlayer->duration()/1000;
}


qint64 DGstPlayerProxy::elapsed() const
{
    return m_pPlayer->position()/1000;
}

void DGstPlayerProxy::updatePlayingMovieInfo()
{
}

DMULTIMEDIA_END_NAMESPACE // end of namespace

