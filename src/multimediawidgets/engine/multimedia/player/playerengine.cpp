// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "playerengine.h"
#include "config.h"
#include "dgstplayerglwidget.h"
#include "dguiapplicationhelper.h"
#include "dmpvglwidget.h"
#include "dplaylistmodel.h"
#include "eventlogutils.h"
#include "filefilter.h"
#include "onlinesub.h"
#include <DCompositeManager>
#include <DGstPlayerProxy>
#include <iostream>

#include <QPainterPath>


#include "drecentmanager.h"
DCORE_USE_NAMESPACE
DGUI_USE_NAMESPACE

DMULTIMEDIA_USE_NAMESPACE

const QStringList PlayerEngine::audio_filetypes = {"*.mp3",
    "*.wav",
    "*.wma",
    "*.m4a",
    "*.aac",
    "*.ac3",
    "*.ape",
    "*.flac",
    "*.ra",
    "*.mka",
    "*.dts",
    "*.opus",
    "*.amr"};
const QStringList PlayerEngine::video_filetypes = {
    "*.3g2",
    "*.3ga",
    "*.3gp",
    "*.3gp2",
    "*.3gpp",
    "*.amv",
    "*.asf",
    "*.asx",
    "*.avf",
    "*.avi",
    "*.bdm",
    "*.bdmv",
    "*.bik",
    "*.clpi",
    "*.cpi",
    "*.dat",
    "*.divx",
    "*.drc",
    "*.dv",
    "*.dvr-ms",
    "*.f4v",
    "*.flv",
    "*.gvi",
    "*.gxf",
    "*.hdmov",
    "*.hlv",
    "*.iso",
    "*.letv",
    "*.lrv",
    "*.m1v",
    "*.m2p",
    "*.m2t",
    "*.m2ts",
    "*.m2v",
    "*.m3u",
    "*.m3u8",
    "*.m4v",
    "*.mkv",
    "*.moov",
    "*.mov",
    "*.mov",
    "*.mp2",
    "*.mp2v",
    "*.mp4",
    "*.mp4v",
    "*.mpe",
    "*.mpeg",
    "*.mpeg1",
    "*.mpeg2",
    "*.mpeg4",
    "*.mpg",
    "*.mpl",
    "*.mpls",
    "*.mpv",
    "*.mpv2",
    "*.mqv",
    "*.mts",
    "*.mts",
    "*.mtv",
    "*.mxf",
    "*.mxg",
    "*.nsv",
    "*.nuv",
    "*.ogg",
    "*.ogm",
    "*.ogv",
    "*.ogx",
    "*.ps",
    "*.qt",
    "*.qtvr",
    "*.ram",
    "*.rec",
    "*.rm",
    "*.rm",
    "*.rmj",
    "*.rmm",
    "*.rms",
    "*.rmvb",
    "*.rmx",
    "*.rp",
    "*.rpl",
    "*.rv",
    "*.rvx",
    "*.thp",
    "*.tod",
    "*.tp",
    "*.trp",
    "*.ts",
    "*.tts",
    "*.txd",
    "*.vcd",
    "*.vdr",
    "*.vob",
    "*.vp8",
    "*.vro",
    "*.webm",
    "*.wm",
    "*.wmv",
    "*.wtv",
    "*.xesc",
    "*.xspf",
    "*.ogg",
};

const QStringList PlayerEngine::subtitle_suffixs = {
    "ass", "sub", "srt", "aqt", "jss", "gsub", "ssf", "ssa", "smi", "usf", "idx"};

PlayerEngine::PlayerEngine(QWidget *parent) : QWidget(parent)
{
    m_bAudio            = false;
    m_stopRunningThread = false;
    m_nDuration         = 0;
    m_pVideoWidget      = nullptr;

    if(DCompositeManager::isMpvExists()) {
        m_current = new DMpvProxy(this);
        connect(((DMpvProxy *) m_current), &DMpvProxy::notifyCreateOpenGL, this, &PlayerEngine::createOPenGLWgt);
    }
    else {
        m_current      = new DGstPlayerProxy(this);
        m_pVideoWidget = new DGstPlayerGLWidget(this);
        showOpenGLWgt(m_pVideoWidget);
    }
    if(m_current) {
        connect(m_current, &DPlayerBackend::stateChanged, this, &PlayerEngine::onBackendStateChanged);
        connect(m_current, &DPlayerBackend::tracksChanged, this, &PlayerEngine::tracksChanged);
        connect(m_current, &DPlayerBackend::elapsedChanged, this, &PlayerEngine::elapsedChanged);
        connect(m_current, &DPlayerBackend::fileLoaded, this, &PlayerEngine::fileLoaded);
        connect(m_current, &DPlayerBackend::muteChanged, this, &PlayerEngine::muteChanged);
        connect(m_current, &DPlayerBackend::volumeChanged, this, &PlayerEngine::volumeChanged);
        connect(m_current, &DPlayerBackend::sidChanged, this, &PlayerEngine::sidChanged);
        connect(m_current, &DPlayerBackend::aidChanged, this, &PlayerEngine::aidChanged);
        connect(m_current, &DPlayerBackend::videoSizeChanged, this, &PlayerEngine::videoSizeChanged);
        connect(m_current, &DPlayerBackend::notifyScreenshot, this, &PlayerEngine::notifyScreenshot);
        connect(m_current, &DPlayerBackend::mpvErrorLogsChanged, this, &PlayerEngine::mpvErrorLogsChanged);
        connect(m_current, &DPlayerBackend::mpvWarningLogsChanged, this, &PlayerEngine::mpvWarningLogsChanged);
        connect(m_current, &DPlayerBackend::urlpause, this, &PlayerEngine::urlpause);
        connect(m_current, &DPlayerBackend::sigMediaError, this, &PlayerEngine::sigMediaError);
        m_current->setWinID(winId());
    }

#ifdef BUILD_Qt6

#else
    connect(&m_networkConfigMng,
        &QNetworkConfigurationManager::onlineStateChanged,
        this,
        &PlayerEngine::onlineStateChanged);
#endif
    connect(
        &OnlineSubtitle::get(), &OnlineSubtitle::subtitlesDownloadedFor, this, &PlayerEngine::onSubtitlesDownloaded);

    m_playlist = new DPlaylistModel(this);
    connect(m_playlist,
        &DPlaylistModel::asyncAppendFinished,
        this,
        &PlayerEngine::onPlaylistAsyncAppendFinished,
        Qt::DirectConnection);
    connect(m_playlist, &DPlaylistModel::updateDuration, this, &PlayerEngine::updateDuration);
}

PlayerEngine::~PlayerEngine()
{
    m_stopRunningThread = true;
    m_current->stop();
    FileFilter::instance()->stopThread();
    if(m_current) {
        disconnect(m_current, nullptr, nullptr, nullptr);
        delete m_current;
        m_current = nullptr;
    }

    if(m_playlist) {
        disconnect(m_playlist, nullptr, nullptr, nullptr);
        delete m_playlist;
        m_playlist = nullptr;
    }
    qInfo() << __func__;
}

bool PlayerEngine::createOPenGLWgt(MpvHandle handle)
{
    m_pVideoWidget = new DMpvGLWidget(this, handle);
    showOpenGLWgt(m_pVideoWidget);
}

bool PlayerEngine::showOpenGLWgt(QOpenGLWidget *pVideoWidget)
{
    if(DCompositeManager::isMpvExists()) {
        DMpvGLWidget *glwgt = (DMpvGLWidget *) pVideoWidget;
        connect(m_current, &DPlayerBackend::stateChanged, [=]() {
            glwgt->setPlaying(m_current->state() != DPlayerBackend::PlayState::Stopped);
            glwgt->setRawFormatFlag(getplaylist()->currentInfo().mi.isRawFormat());
            glwgt->update();
        });
#ifdef __x86_64__
        connect(m_current, &DPlayerBackend::elapsedChanged, [=]() {
            if(m_nDuration != duration()) {
                m_nDuration = duration();
                emit durationChanged(m_nDuration * 1000);
            }
            glwgt->updateMovieProgress(m_nDuration, elapsed());
            glwgt->update();
        });
#endif
#if defined(USE_DXCB)
        glwgt->toggleRoundedClip(false);
#endif
    }
    else {
        DGstPlayerGLWidget *glwgt = (DGstPlayerGLWidget *) pVideoWidget;
        connect(m_current, &DPlayerBackend::stateChanged, [=]() {
            glwgt->setPlaying(m_current->state() != DPlayerBackend::PlayState::Stopped);
            glwgt->setRawFormatFlag(getplaylist()->currentInfo().mi.isRawFormat());
            glwgt->update();
        });
#ifdef __x86_64__
        connect(m_current, &DPlayerBackend::elapsedChanged, [=]() {
            if(m_nDuration != duration()) {
                m_nDuration = duration();
                emit durationChanged(m_nDuration * 1000);
            }
            glwgt->updateMovieProgress(m_nDuration, elapsed());
            glwgt->update();
        });
        connect(m_current, &DPlayerBackend::processFrame, this, &PlayerEngine::processFrame);

#endif
#if defined(USE_DXCB)
        glwgt->toggleRoundedClip(false);
#endif
    }
    QVBoxLayout *pLayout = new QVBoxLayout(this);
    pLayout->setContentsMargins(0, 0, 0, 0);
    pLayout->addWidget(pVideoWidget);
    setLayout(pLayout);
    pVideoWidget->show();
}

void PlayerEngine::processFrame(QVideoFrame &frame)
{
    DGstPlayerGLWidget *glwgt = (DGstPlayerGLWidget *) m_pVideoWidget;
    DGstPlayerProxy *proxy    = (DGstPlayerProxy *) m_current;
#ifdef BUILD_Qt6
    frame.map(QVideoFrame::ReadOnly);
    //// bits().......
    QImage recvImage(
        frame.bits(frame.planeCount()), frame.width(), frame.height(), QVideoFrameFormat::imageFormatFromPixelFormat(frame.pixelFormat()));
#else
    frame.map(QAbstractVideoBuffer::ReadOnly);
    QImage recvImage(
        frame.bits(), frame.width(), frame.height(), QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat()));
#endif
    proxy->setCurrentFrame(recvImage);
    glwgt->setVideoTex(recvImage);
    glwgt->repaint();
    frame.unmap();
}

bool PlayerEngine::isPlayableFile(const QUrl &url)
{
    if(FileFilter::instance()->isMediaFile(url)) {
        return true;
    }
    else {
        if(url.isLocalFile()) {
            emit sigInvalidFile(QFileInfo(url.toLocalFile()).fileName());
        }
        return false;
    }
}

static QStringList suffixes;

bool PlayerEngine::isPlayableFile(const QString &name)
{
    QUrl url = FileFilter::instance()->fileTransfer(name);

    if(FileFilter::instance()->isMediaFile(url)) {
        return true;
    }

    if(url.isLocalFile()) {
        emit sigInvalidFile(QFileInfo(url.toLocalFile()).fileName());
        return false;
    }
    return false;
}

bool PlayerEngine::isAudioFile(const QString &name)
{
    QUrl url = FileFilter::instance()->fileTransfer(name);

    return FileFilter::instance()->isAudio(url);
}

bool PlayerEngine::isSubtitle(const QString &name)
{
    QUrl url = FileFilter::instance()->fileTransfer(name);

    return FileFilter::instance()->isSubtitle(url);
}


void PlayerEngine::waitLastEnd()
{
    if(DMpvProxy *mpv = dynamic_cast<DMpvProxy *>(m_current)) {
        mpv->pollingEndOfPlayback();
    }
    else if(DGstPlayerProxy *qtPlayer = dynamic_cast<DGstPlayerProxy *>(m_current)) {
        qtPlayer->pollingEndOfPlayback();
    }
}

void PlayerEngine::onBackendStateChanged()
{
    if(!m_current) return;

    CoreState old = m_state;
    switch(m_current->state()) {
    case DPlayerBackend::PlayState::Playing:
        m_state = CoreState::Playing;
        if(m_playlist->count() > 0) {
            m_bAudio = currFileIsAudio();
        }
        if(old == CoreState::Idle) emit siginitthumbnailseting();
        break;
    case DPlayerBackend::PlayState::Paused:
        m_state = CoreState::Paused;
        break;
    case DPlayerBackend::PlayState::Stopped:
        m_state = CoreState::Idle;
        break;
    }

    if(old != m_state) emit stateChanged();

    auto systemEnv           = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = systemEnv.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY  = systemEnv.value(QStringLiteral("WAYLAND_DISPLAY"));
    if(XDG_SESSION_TYPE == QLatin1String("wayland") ||
        WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        if(m_state == CoreState::Idle) {
            QPalette pal(qApp->palette());
            this->setAutoFillBackground(true);
            this->setPalette(pal);
        }
        else {
            QPalette pal(this->palette());
#ifdef BUILD_Qt6
            // Background/Foreground已移除，用WindowText/Window替代
            pal.setColor(QPalette::Window, Qt::black);
#else
            pal.setColor(QPalette::Background, Qt::black);
#endif
            this->setAutoFillBackground(true);
            this->setPalette(pal);
        }
    }
}

PlayerEngine::CoreState PlayerEngine::state()
{
    auto old = m_state;
    switch(m_current->state()) {
    case DPlayerBackend::PlayState::Playing:
        m_state = CoreState::Playing;
        break;
    case DPlayerBackend::PlayState::Paused:
        m_state = CoreState::Paused;
        break;
    case DPlayerBackend::PlayState::Stopped:
        m_state = CoreState::Idle;
        break;
    }

    if(old != m_state) {
        qWarning() << "###### state mismatch" << old << m_state;
        emit stateChanged();
    }
    return m_state;
}

const PlayingMovieInfo &PlayerEngine::playingMovieInfo()
{
    static PlayingMovieInfo empty;

    if(!m_current) return empty;
    return m_current->playingMovieInfo();
}

int PlayerEngine::aid()
{
    if(state() == CoreState::Idle) {
        return 0;
    }
    if(!m_current) return 0;

    return m_current->aid();
}

int PlayerEngine::sid()
{
    if(state() == CoreState::Idle) {
        return 0;
    }
    if(!m_current) return 0;

    return m_current->sid();
}

void PlayerEngine::onSubtitlesDownloaded(
    const QUrl &url, const QList<QString> &filenames, OnlineSubtitle::FailReason reason)
{
    reason = OnlineSubtitle::FailReason::NoError;

    if(state() == CoreState::Idle) {
        return;
    }
    if(!m_current) return;

    if(playlist().currentInfo().url != url) return;

    bool res = false;

    for(auto &filename : filenames) {
#ifdef BUILD_Qt6
        if(true == m_current->loadSubtitle(QFileInfo(filename))) {
#else
        if(true == m_current->loadSubtitle(filename)) {
#endif
            res = true;
        }
        else {
            QFile::remove(filename);
        }
    }

    emit loadOnlineSubtitlesFinished(url, res);
}

bool PlayerEngine::loadSubtitle(const QFileInfo &fi)
{
    if(state() == CoreState::Idle) {
        return true;
    }
    if(!m_current) return true;

    const auto &pmf = m_current->playingMovieInfo();
    auto pif        = playlist().currentInfo();
    int i           = 0;
    for(const auto &sub : pmf.subs) {
        if(sub["external"].toBool()) {
            auto path = sub["external-filename"].toString();
            if(path == fi.canonicalFilePath()) {
                this->selectSubtitle(i);
                return true;
            }
        }
        ++i;
    }

    if(m_current->loadSubtitle(fi)) {
        return true;
    }
    return false;
}

void PlayerEngine::loadOnlineSubtitle(const QUrl &url)
{
    if(state() == CoreState::Idle) {
        return;
    }
    if(!m_current) return;

    OnlineSubtitle::get().requestSubtitle(url);
}

void PlayerEngine::setPlaySpeed(double times)
{
    if(!m_current) return;
    m_current->setPlaySpeed(times);
}

void PlayerEngine::setSubDelay(double secs)
{
    if(!m_current) return;

    m_current->setSubDelay(secs + m_current->subDelay());
}

double PlayerEngine::subDelay() const
{
    if(!m_current) return 0.0;
    return m_current->subDelay();
}

QString PlayerEngine::subCodepage()
{
    if(m_current->subCodepage().isEmpty()) {
        return "auto";
    }
    else {
        return m_current->subCodepage();
    }
}

void PlayerEngine::setSubCodepage(const QString &cp)
{
    if(!m_current) return;
    m_current->setSubCodepage(cp);
}

void PlayerEngine::addSubSearchPath(const QString &path)
{
    if(!m_current) return;
    m_current->addSubSearchPath(path);
}

void PlayerEngine::updateSubStyle(const QString &font, int sz)
{
    if(!m_current) return;
    m_current->updateSubStyle(font, sz / 2);
}

void PlayerEngine::selectSubtitle(int id)
{
    if(!m_current) return;
    if(state() != CoreState::Idle) {
        const auto &pmf = m_current->playingMovieInfo();
        if(id >= pmf.subs.size()) return;
        auto sid = pmf.subs[id]["id"].toInt();
        m_current->selectSubtitle(sid);
    }
}

bool PlayerEngine::isSubVisible()
{
    if(state() == CoreState::Idle) {
        return false;
    }
    if(!m_current) return false;

    return m_current->isSubVisible();
}

void PlayerEngine::toggleSubtitle()
{
    if(!m_current) return;
    m_current->toggleSubtitle();
}

void PlayerEngine::selectTrack(int id)
{
    if(!m_current) return;
    m_current->selectTrack(id);
}

void PlayerEngine::volumeUp()
{
    if(!m_current) return;
    m_current->volumeUp();
}

void PlayerEngine::changeVolume(int val)
{
    if(!m_current) return;
    m_current->changeVolume(val);
}

void PlayerEngine::volumeDown()
{
    if(!m_current) return;
    m_current->volumeDown();
}

int PlayerEngine::volume() const
{
    if(!m_current) return 100;
    return m_current->volume();
}

bool PlayerEngine::muted() const
{
    if(!m_current) return false;
    return m_current->muted();
}

void PlayerEngine::changehwaccelMode(DPlayerBackend::hwaccelMode hwaccelMode)
{
    if(!m_current) return;
    return m_current->changehwaccelMode(hwaccelMode);
}

DPlayerBackend *PlayerEngine::getMpvProxy()
{
    return m_current;
}

void PlayerEngine::toggleMute()
{
    if(!m_current) return;
    if(!m_bMpvFunsLoad) {
        emit mpvFunsLoadOver();
        m_bMpvFunsLoad = true;
    }

    m_current->toggleMute();
    emit volumeChanged();
}

void PlayerEngine::setMute(bool bMute)
{
    m_current->setMute(bMute);
}

void PlayerEngine::savePreviousMovieState()
{
    savePlaybackPosition();
}

void PlayerEngine::paintEvent(QPaintEvent *e)
{
    QRect rect = this->rect();
    QPainter p(this);

    if(!DCompositeManager::get().composited() || utils::check_wayland_env()) {
        if(m_state != Idle && m_bAudio) {
            p.fillRect(rect, QBrush(QColor(0, 0, 0)));
        }
        else {
            QImage icon = QIcon::fromTheme("deepin-movie").pixmap(130, 130).toImage();
            ;
            QPixmap pix = QPixmap::fromImage(icon);
            QPointF pos = rect.center() - QPoint(pix.width() / 2, pix.height() / 2) / devicePixelRatioF();

            if(DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
                p.fillRect(rect, QBrush(QColor(255, 255, 255)));
                p.drawPixmap(pos, pix);
            }
            else {
                p.fillRect(rect, QBrush(QColor(0, 0, 0)));
                p.drawPixmap(pos, pix);
            }
        }
    }
    return QWidget::paintEvent(e);
}

void PlayerEngine::requestPlay(int id)
{
    if(!m_current) return;
    if(id >= m_playlist->count()) return;

    const auto &item = m_playlist->items()[id];
    m_current->setPlayFile(item.url);

    DRecentData data;
    data.appName = "Deepin Movie";
    data.appExec = "deepin-movie";
    DRecentManager::addItem(item.url.toLocalFile(), data);

    if(m_current->isPlayable()) {
        m_current->play();
    }

    QJsonObject obj{{"tid", EventLogUtils::StartPlaying},
        {"version", VERSION},
        {"successful", item.url.isLocalFile() ? "true" : ""},
        {"type", currFileIsAudio() ? "audio" : "video"},
        {"origin", item.url.isLocalFile() ? "local" : "http"},
        {"encapsulation_format", item.mi.fileType},
        {"coding_format", utils::videoIndex2str(item.mi.vCodecID)}};

    EventLogUtils::get().writeLogs(obj);
}

void PlayerEngine::savePlaybackPosition()
{
    if(!m_current) return;
    m_current->savePlaybackPosition();
}

void PlayerEngine::nextFrame()
{
    if(!m_current) return;
    m_current->nextFrame();
}

void PlayerEngine::previousFrame()
{
    if(!m_current) return;
    m_current->previousFrame();
}
void PlayerEngine::makeCurrent()
{
    if(!m_pVideoWidget) return;
    m_pVideoWidget->makeCurrent();
}

void PlayerEngine::play()
{
    if(!m_current || !m_playlist->count()) return;

    if(state() == CoreState::Paused && getBackendProperty("keep-open").toBool() &&
        getBackendProperty("eof-reached").toBool()) {
        stop();
        next();
    }
    else if(state() == CoreState::Idle) {
        next();
    }
}

void PlayerEngine::prev()
{
    if(m_playingRequest) return;
    m_playingRequest = true;
    savePreviousMovieState();
    m_playlist->playPrev(true);
    m_playingRequest = false;
}

void PlayerEngine::next()
{
    if(m_playingRequest) return;
    m_playingRequest = true;
    savePreviousMovieState();
    m_playlist->playNext(true);
    m_playingRequest = false;
}

void PlayerEngine::onPlaylistAsyncAppendFinished(const QList<PlayItemInfo> &pil)
{
    if(m_pendingPlayReq.isValid()) {
        auto id = m_playlist->indexOf(m_pendingPlayReq);
        if(pil.size() && m_pendingPlayReq.scheme() == "playlist") {
            id = m_playlist->indexOf(pil[0].url);
        }

        if(id >= 0) {
            m_playlist->changeCurrent(id);
            m_pendingPlayReq = QUrl();
        }
        else {
            qInfo() << __func__ << "id is:" << id;
        }
    }
    else {
        qInfo() << __func__ << m_pendingPlayReq;
    }
}

void PlayerEngine::playByName(const QUrl &url)
{
    savePreviousMovieState();
    int id = m_playlist->indexOf(url);
    qInfo() << __func__ << url << "id:" << id;
    if(id >= 0) {
        m_playlist->changeCurrent(id);
    }
    else {
        m_pendingPlayReq = url;
    }
}

void PlayerEngine::playSelected(int id)
{
    qInfo() << __func__ << id;
    savePreviousMovieState();
    m_playlist->changeCurrent(id);
}

void PlayerEngine::clearPlaylist()
{
    m_playlist->clear();
}

void PlayerEngine::pauseResume()
{
    if(!m_current) return;
    if(m_state == CoreState::Idle) return;

    m_current->pauseResume();
}

void PlayerEngine::stop()
{
    if(!m_current) return;
    m_current->stop();
}

bool PlayerEngine::paused()
{
    return m_state == CoreState::Paused;
}

QImage PlayerEngine::takeScreenshot()
{
    return m_current->takeScreenshot();
}

void PlayerEngine::burstScreenshot()
{
    m_current->burstScreenshot();
}

void PlayerEngine::stopBurstScreenshot()
{
    m_current->stopBurstScreenshot();
}

void PlayerEngine::seekForward(int secs)
{
    if(state() == CoreState::Idle) return;

    static int lastElapsed = 0;

    if(elapsed() == lastElapsed) return;
    m_current->seekForward(secs);
}

void PlayerEngine::seekBackward(int secs)
{
    if(state() == CoreState::Idle) return;

    if(elapsed() - abs(secs) <= 0) {
        m_current->seekBackward(static_cast<int>(elapsed()));
    }
    else {
        m_current->seekBackward(secs);
    }
}


void PlayerEngine::seekAbsolute(int pos)
{
    if(state() == CoreState::Idle) return;

    m_current->seekAbsolute(pos);
}

void PlayerEngine::setDVDDevice(const QString &path)
{
    if(!m_current) {
        return;
    }
    m_current->setDVDDevice(path);
}

bool PlayerEngine::addPlayFile(const QUrl &url)
{
    QUrl realUrl;

    realUrl = FileFilter::instance()->fileTransfer(url.toString());
    if(!isPlayableFile(realUrl)) return false;

    m_playlist->append(realUrl);
    return true;
}

QList<QUrl> PlayerEngine::addPlayDir(const QDir &dir)
{
    QList<QUrl> valids = FileFilter::instance()->filterDir(dir);

    struct {
        bool operator()(const QUrl &fi1, const QUrl &fi2) const
        {
#ifdef BUILD_Qt6
            static QRegularExpression rd("\\d+");
            QRegularExpressionMatch match, match2;

            int pos           = 0;
            QString fileName1 = QFileInfo(fi1.toLocalFile()).fileName();
            QString fileName2 = QFileInfo(fi2.toLocalFile()).fileName();

            match = rd.match(fileName1, pos);
            match2 = rd.match(fileName2, pos);
            while(match.hasMatch()) {
                auto inc = match.capturedLength();
                auto id1 = fileName1.mid(pos, inc);

                auto pos2 = match2.capturedStart(1);
                if(pos == pos2) {
                    auto id2 = fileName2.mid(pos, match2.capturedLength());
                    if(id1 != id2) {
                        bool ok1, ok2;
                        bool v = id1.toInt(&ok1) < id2.toInt(&ok2);
                        if(ok1 && ok2) return v;
                        return id1.localeAwareCompare(id2) < 0;
                    }
                }

                pos += inc;
                match = rd.match(fileName1, pos);
                match2 = rd.match(fileName2, pos);
            }

            return fileName1.localeAwareCompare(fileName2) < 0;
#else
            static QRegExp rd("\\d+");

            int pos           = 0;
            QString fileName1 = QFileInfo(fi1.toLocalFile()).fileName();
            QString fileName2 = QFileInfo(fi2.toLocalFile()).fileName();
            while((pos = rd.indexIn(fileName1, pos)) != -1) {
                auto inc = rd.matchedLength();
                auto id1 = fileName1.midRef(pos, inc);

                auto pos2 = rd.indexIn(fileName2, pos);
                if(pos == pos2) {
                    auto id2 = fileName2.midRef(pos, rd.matchedLength());
                    if(id1 != id2) {
                        bool ok1, ok2;
                        bool v = id1.toInt(&ok1) < id2.toInt(&ok2);
                        if(ok1 && ok2) return v;
                        return id1.localeAwareCompare(id2) < 0;
                    }
                }

                pos += inc;
            }

            return fileName1.localeAwareCompare(fileName2) < 0;
#endif
        }
    } SortByDigits;

    std::sort(valids.begin(), valids.end(), SortByDigits);
    valids = addPlayFiles(valids);
    m_playlist->appendAsync(valids);

    return valids;
}

QList<QUrl> PlayerEngine::addPlayFiles(const QList<QUrl> &urls)
{
    qInfo() << __func__;
    QList<QUrl> valids;

    for(QUrl url : urls) {
        if(m_stopRunningThread) break;
        if(isPlayableFile(url)) valids << url;
    }

    m_playlist->appendAsync(valids);

    return valids;
}

QList<QUrl> PlayerEngine::addPlayFiles(const QList<QString> &lstFile)
{
    qInfo() << __func__;
    QList<QUrl> valids;
    QUrl realUrl;

    for(QString strFile : lstFile) {
        realUrl = FileFilter::instance()->fileTransfer(strFile);
        if(QFileInfo(realUrl.path()).isDir()) {
            if(realUrl.isLocalFile()) valids << FileFilter::instance()->filterDir(QDir(realUrl.path()));
        }
        else {
            valids << realUrl;
        }
    }

    return addPlayFiles(valids);
}

void PlayerEngine::addPlayFs(const QList<QString> &lstFile)
{
    qInfo() << __func__;
    QList<QUrl> valids;
    QUrl realUrl;

    for(QString strFile : lstFile) {
        realUrl = FileFilter::instance()->fileTransfer(strFile);
        if(QFileInfo(realUrl.path()).isDir()) {
            if(realUrl.isLocalFile()) valids << FileFilter::instance()->filterDir(QDir(realUrl.path()));
        }
        else {
            valids << realUrl;
        }
    }

    if(valids.isEmpty()) {
        blockSignals(false);
        return;
    }
    QList<QUrl> addFiles = addPlayFiles(valids);
    blockSignals(false);
    emit finishedAddFiles(addFiles);
}

qint64 PlayerEngine::duration() const
{
    if(!m_current) return 0;
    return m_current->duration();
}

QSize PlayerEngine::videoSize() const
{
    if(!m_current) return {0, 0};
    return m_current->videoSize();
}

const ModeMovieInfo &PlayerEngine::movieInfo()
{
    if(!m_playlist) return ModeMovieInfo();
    return m_playlist->currentInfo().mi;
}

qint64 PlayerEngine::elapsed() const
{
    if(!m_current) return 0;
    if(!m_playlist) return 0;
    if(m_playlist->count() == 0) return 0;
    if(m_playlist->current() < 0) return 0;
    qint64 nDuration = m_current->duration();
    qint64 nElapsed  = m_current->elapsed();
    if(nElapsed < 0) nElapsed = 0;
    if(nElapsed > nDuration) nElapsed = nDuration;
    return nElapsed;
}

void PlayerEngine::setVideoAspect(double r)
{
    if(m_current) m_current->setVideoAspect(r);
}

double PlayerEngine::videoAspect() const
{
    if(!m_current) return 0.0;
    return m_current->videoAspect();
}

int PlayerEngine::videoRotation() const
{
    if(!m_current) return 0;
    return m_current->videoRotation();
}

void PlayerEngine::setVideoRotation(int degree)
{
    if(m_current) m_current->setVideoRotation(degree);
}

void PlayerEngine::changeSoundMode(DPlayerBackend::SoundMode sm)
{
    if(m_current) m_current->changeSoundMode(sm);
}


void PlayerEngine::setBackendProperty(const QString &name, const QVariant &val)
{
    if(m_current) {
        m_current->setProperty(name, val);
    }
}

QVariant PlayerEngine::getBackendProperty(const QString &name)
{
    if(m_current) {
        return m_current->getProperty(name);
    }
    return QVariant();
}

void PlayerEngine::toggleRoundedClip(bool roundClip)
{
    DMpvProxy *pMpvProxy = nullptr;

    pMpvProxy = dynamic_cast<DMpvProxy *>(m_current);
    if(!pMpvProxy) {
        dynamic_cast<DGstPlayerProxy *>(m_current)->updateRoundClip(roundClip);
    }
}

bool PlayerEngine::currFileIsAudio()
{
    bool bAudio = false;
    PlayItemInfo pif;

    if(m_playlist->count() > 0) {
        pif = m_playlist->currentInfo();
    }

    if(DCompositeManager::isMpvExists()) {
        bAudio = pif.thumbnail.isNull() && pif.url.isLocalFile();
    }
    else {
        bAudio = isAudioFile(pif.url.toString());
    }

    return bAudio;
}
