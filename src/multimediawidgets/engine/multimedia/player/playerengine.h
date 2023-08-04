// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLAYER_ENINE_H
#define DPLAYER_ENINE_H

#include "dplayerbackend.h"
#include <DMpvProxy>
#ifdef BUILD_Qt6
#include <QtNetwork>
#include <QtOpenGLWidgets/QOpenGLWidget>
#else
#include <QNetworkConfigurationManager>
#endif
#include <QtWidgets>
#include <dplaylistmodel.h>
#include <dtkmultimedia.h>
#include <onlinesub.h>

DMULTIMEDIA_BEGIN_NAMESPACE
class DPlaylistModel;
struct PlayingMovieInfo;

class PlayerEngine : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qint64 duration READ duration)
    Q_PROPERTY(qint64 elapsed READ elapsed NOTIFY elapsedChanged)
    Q_PROPERTY(QSize videoSize READ videoSize NOTIFY videoSizeChanged)
    Q_PROPERTY(bool paused READ paused)

    Q_PROPERTY(CoreState state READ state NOTIFY stateChanged)
public:
    enum CoreState {
        Idle,
        Playing,
        Paused,
    };
    Q_ENUM(CoreState)

    // filetypes supported by mpv:
    // https://github.com/mpv-player/mpv/blob/master/player/external_files.c
    const static QStringList audio_filetypes;
    const static QStringList video_filetypes;

    const static QStringList subtitle_suffixs;

    /* backend like mpv will asynchronously report end of playback.
     * there are situations when we need to see the end-event before
     * proceed (e.g playlist next)
     */
    void waitLastEnd();

    friend class DPlaylistModel;

    explicit PlayerEngine(QWidget *parent);
    virtual ~PlayerEngine();

    // only the last dvd device set
    void setDVDDevice(const QString &path);
    // add by heyi
    //第一次播放需要初库始化函数指针
    void firstInit();

    bool addPlayFile(const QUrl &url);
    // return collected valid urls
    QList<QUrl> addPlayDir(const QDir &dir);
    // returned list contains only accepted valid items
    QList<QUrl> addPlayFiles(const QList<QUrl> &urls);
    /**
     * @brief addPlayFiles 添加播放文件
     * @param 文件集合
     * @return 返回已添加成功的文件
     */
    QList<QUrl> addPlayFiles(const QList<QString> &lstFile);
    /**
     * @brief addPlayFs 在线程中运行添加文件
     * @param 文件集合
     */
    void addPlayFs(const QList<QString> &lstFile);
    /**
     * @brief isPlayableFile 判断一个文件是否可以播放
     * @param url 文件url
     * @return 是否可以播放
     */
    bool isPlayableFile(const QUrl &url);
    /**
     * @brief isPlayableFile 判断一个文件是否可以播放
     * @param url 文件路径
     * @return 是否可以播放
     */
    bool isPlayableFile(const QString &name);
    static bool isAudioFile(const QString &name);
    static bool isSubtitle(const QString &name);

    // only supports (+/-) 0, 90, 180, 270
    int videoRotation() const;
    void setVideoRotation(int degree);

    void setVideoAspect(double r);
    double videoAspect() const;

    qint64 duration() const;
    qint64 elapsed() const;
    QSize videoSize() const;
    const struct ModeMovieInfo &movieInfo();

    bool paused();
    CoreState state();
    const PlayingMovieInfo &playingMovieInfo();
    void setPlaySpeed(double times);

    void loadOnlineSubtitle(const QUrl &url);
    bool loadSubtitle(const QFileInfo &fi);
    void toggleSubtitle();
    bool isSubVisible();
    void selectSubtitle(int id);   // id into PlayingMovieInfo.subs
    int sid();
    void setSubDelay(double secs);
    double subDelay() const;
    void updateSubStyle(const QString &font, int sz);
    void setSubCodepage(const QString &cp);
    QString subCodepage();
    void addSubSearchPath(const QString &path);

    void selectTrack(int id);   // id into PlayingMovieInfo.audios
    int aid();

    void changeSoundMode(DPlayerBackend::SoundMode sm);
    int volume() const;
    bool muted() const;

    void changehwaccelMode(DPlayerBackend::hwaccelMode hwaccelMode);

    DPlaylistModel &playlist() const
    {
        return *m_playlist;
    }

    DPlayerBackend *getMpvProxy();

    DPlaylistModel *getplaylist()
    {
        return m_playlist;
    }

    QImage takeScreenshot();
    void burstScreenshot();
    void stopBurstScreenshot();

    void savePlaybackPosition();

    void nextFrame();
    void previousFrame();
    void makeCurrent();

    void setBackendProperty(const QString &, const QVariant &);
    QVariant getBackendProperty(const QString &);

    void toggleRoundedClip(bool roundClip);
    bool currFileIsAudio();

signals:
    void tracksChanged();
    void elapsedChanged();
    void durationChanged(qint64 duration);
    void videoSizeChanged();
    void stateChanged();
    void fileLoaded();
    void muteChanged();
    void volumeChanged();
    void sidChanged();
    void aidChanged();
    void subCodepageChanged();

    void loadOnlineSubtitlesFinished(const QUrl &url, bool success);
    void mpvFunsLoadOver();
    void positionProxyChanged(const qint64 &position) const;
    void notifyScreenshot(const QImage &frame, qint64 time);

    void playlistChanged();

    void onlineStateChanged(const bool isOnline);
    void mpvErrorLogsChanged(const QString prefix, const QString text);
    void mpvWarningLogsChanged(const QString prefix, const QString text);
    void urlpause(bool status);

    void siginitthumbnailseting();
    void updateDuration();
    void sigInvalidFile(QString strFileName);

    void sigMediaError();
    void finishedAddFiles(QList<QUrl>);

public slots:
    void play();
    void pauseResume();
    void stop();

    void prev();
    void next();
    void playSelected(int id);
    void playByName(const QUrl &url);
    void clearPlaylist();

    void seekForward(int secs);
    void seekBackward(int secs);
    void seekAbsolute(int pos);

    void volumeUp();
    void volumeDown();
    void changeVolume(int val);
    void toggleMute();
    void setMute(bool bMute);

protected slots:
    void onBackendStateChanged();
    void requestPlay(int id);
    void onSubtitlesDownloaded(const QUrl &url, const QList<QString> &filenames, OnlineSubtitle::FailReason);
    void onPlaylistAsyncAppendFinished(const QList<PlayItemInfo> &);
    void processFrame(QVideoFrame &frame);

protected:
    DPlaylistModel *m_playlist { nullptr };
    CoreState m_state { CoreState::Idle };
    DPlayerBackend *m_current { nullptr };
    QUrl m_pendingPlayReq;
    bool m_playingRequest { false };
    bool m_bMpvFunsLoad { false };
    void savePreviousMovieState();
    void paintEvent(QPaintEvent *e) override;
    bool createOPenGLWgt(MpvHandle handle);
    bool showOpenGLWgt(QOpenGLWidget *pVideoWidget);

private:
#ifdef BUILD_Qt6

#else
    QNetworkConfigurationManager m_networkConfigMng;
#endif
    bool m_bAudio;
    bool m_stopRunningThread;
    QOpenGLWidget *m_pVideoWidget;
    qint64 m_nDuration;
};
DMULTIMEDIA_END_NAMESPACE

#endif /* ifndef DPLAYER_ENINE_H */
