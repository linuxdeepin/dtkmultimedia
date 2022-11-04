// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLAYERBACKEND_H
#define DPLAYERBACKEND_H

#include <QVideoFrame>
#include <QtGui>
#include <dtkmultimedia.h>

DMULTIMEDIA_BEGIN_NAMESPACE

using SubtitleInfo = QMap<QString, QVariant>;
using AudioInfo = QMap<QString, QVariant>;

struct PlayingMovieInfo
{
    QList<SubtitleInfo> subs;
    QList<AudioInfo> audios;
};

class DPlayerBackendPrivate;
class DPlayerBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qint64 duration READ duration)
    Q_PROPERTY(qint64 elapsed READ elapsed NOTIFY elapsedChanged)
    Q_PROPERTY(QSize videoSize READ videoSize NOTIFY videoSizeChanged)
    Q_PROPERTY(bool paused READ paused)
    Q_PROPERTY(PlayState state READ state NOTIFY stateChanged)
    Q_DECLARE_PRIVATE(DPlayerBackend)

public:
    enum PlayState { Playing,
                     Paused,
                     Stopped };
    Q_ENUM(PlayState)

    enum DebugLevel { Info,
                      Debug,
                      Verbose };
    Q_ENUM(DebugLevel)

    enum SoundMode { Stereo,
                     Left,
                     Right };
    Q_ENUM(SoundMode)

    enum hwaccelMode { hwaccelAuto = 0,
                       hwaccelOpen,
                       hwaccelClose };
    Q_ENUM(hwaccelMode)

    DPlayerBackend(QObject *parent = nullptr);
    virtual ~DPlayerBackend();

    virtual void setPlayFile(const QUrl &url);
    virtual void setDVDDevice(const QString &path);
    virtual bool isPlayable() const = 0;
    virtual qint64 duration() const;
    virtual qint64 elapsed() const;
    virtual QSize videoSize() const = 0;
    virtual bool paused();
    virtual PlayState state() const;
    virtual void setState(PlayState state);
    virtual const PlayingMovieInfo &playingMovieInfo() = 0;
    virtual void setPlaySpeed(double times) = 0;
    virtual void savePlaybackPosition() = 0;
    virtual void updateSubStyle(const QString &font, int sz) = 0;
    virtual void setSubCodepage(const QString &cp) = 0;
    virtual QString subCodepage() = 0;
    virtual void addSubSearchPath(const QString &path) = 0;
    virtual void firstInit() = 0;
    virtual bool loadSubtitle(const QFileInfo &fi) = 0;
    virtual void toggleSubtitle() = 0;
    virtual bool isSubVisible() = 0;
    virtual void selectSubtitle(int id) = 0;
    virtual void selectTrack(int id) = 0;
    virtual void setSubDelay(double secs) = 0;
    virtual double subDelay() const = 0;
    virtual int aid() const = 0;
    virtual int sid() const = 0;
    virtual void changeSoundMode(SoundMode) = 0;
    virtual int volume() const = 0;
    virtual bool muted() const = 0;
    virtual void setVideoAspect(double r) = 0;
    virtual double videoAspect() const = 0;
    virtual int videoRotation() const = 0;
    virtual void setVideoRotation(int degree) = 0;
    virtual QImage takeScreenshot() const = 0;
    virtual void burstScreenshot() = 0;
    virtual void stopBurstScreenshot() = 0;
    virtual QVariant getProperty(const QString &) const = 0;
    virtual void setProperty(const QString &, const QVariant &) = 0;
    virtual void nextFrame() = 0;
    virtual void previousFrame() = 0;
    virtual void makeCurrent() = 0;
    virtual void changehwaccelMode(hwaccelMode hwaccelMode) = 0;
    static void setDebugLevel(DebugLevel lvl);
    static DebugLevel debugLevel();
    QUrl urlFile() const;

Q_SIGNALS:
    void tracksChanged();
    void elapsedChanged();
    void videoSizeChanged();
    void stateChanged();
    void fileLoaded();
    void muteChanged();
    void volumeChanged();
    void sidChanged();
    void aidChanged();
    void processFrame(QVideoFrame &frame);
    void notifyScreenshot(const QImage &frame, qint64 time);
    void mpvErrorLogsChanged(const QString prefix, const QString text);
    void mpvWarningLogsChanged(const QString prefix, const QString text);
    void urlpause(bool status);
    void sigMediaError();

public slots:
    virtual void play() = 0;
    virtual void pauseResume() = 0;
    virtual void stop() = 0;
    virtual void seekForward(int secs) = 0;
    virtual void seekBackward(int secs) = 0;
    virtual void seekAbsolute(int) = 0;
    virtual void volumeUp() = 0;
    virtual void volumeDown() = 0;
    virtual void changeVolume(int val) = 0;
    virtual void toggleMute() = 0;
    virtual void setMute(bool bMute) = 0;
    virtual void setWinID(const qint64 &winID);
    qint64 winID() const;

protected:
    QScopedPointer<DPlayerBackendPrivate> d_ptr;
};
DMULTIMEDIA_END_NAMESPACE

#endif /* ifndef DPLAYERBACKEND_H */
