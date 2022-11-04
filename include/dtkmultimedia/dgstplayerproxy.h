// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DGSTPLAYERPROXY_H
#define DGSTPLAYERPROXY_H

#include "dplayerbackend.h"
#include <QMediaContent>
#include <QMediaPlayer>
#include <QVideoFrame>

DMULTIMEDIA_BEGIN_NAMESPACE
class VideoSurface;
class DGstPlayerProxyPrivate;

class DGstPlayerProxy : public DPlayerBackend {
    Q_OBJECT
    Q_DECLARE_PRIVATE(DGstPlayerProxy)

  public:
    explicit DGstPlayerProxy(QObject *parent = 0);
    virtual ~DGstPlayerProxy();
    void firstInit();
    void updateRoundClip(bool roundClip);
    void setCurrentFrame(const QImage &img);
    const PlayingMovieInfo &playingMovieInfo() override;
    bool isPlayable() const override;
    void pollingEndOfPlayback();
    void pollingStartOfPlayback();
    qint64 duration() const override;
    qint64 elapsed() const override;
    QSize videoSize() const override;
    void setPlaySpeed(double dTimes) override;
    void savePlaybackPosition() override;

  public slots:
    void play() override;
    void pauseResume() override;
    void stop() override;
    void seekForward(int nSecs) override;
    void seekBackward(int nSecs) override;
    void seekAbsolute(int nPos) override;
    void volumeUp() override;
    void volumeDown() override;
    void changeVolume(int nVol) override;
    int volume() const override;
    bool muted() const override;
    void toggleMute() override;
    void setMute(bool bMute) override;
    void updateSubStyle(const QString &font, int sz);
    void setSubCodepage(const QString &cp);
    QString subCodepage();
    void addSubSearchPath(const QString &path);
    bool loadSubtitle(const QFileInfo &fi);
    void toggleSubtitle();
    bool isSubVisible();
    void selectSubtitle(int id);
    void selectTrack(int id);
    void setSubDelay(double secs);
    double subDelay() const;
    int aid() const;
    int sid() const;
    void changeSoundMode(SoundMode);
    void setVideoAspect(double r);
    double videoAspect() const;
    int videoRotation() const;
    void setVideoRotation(int degree);
    QImage takeScreenshot() const;
    void burstScreenshot();
    void stopBurstScreenshot();
    QVariant getProperty(const QString &) const;
    void setProperty(const QString &, const QVariant &);
    void nextFrame();
    void previousFrame();
    void makeCurrent();
    void changehwaccelMode(hwaccelMode hwaccelMode);

  protected slots:
    void slotStateChanged(QMediaPlayer::State newState);
    void slotMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void slotPositionChanged(qint64 position);
    void slotMediaError(QMediaPlayer::Error error);

  protected:
    QScopedPointer<DGstPlayerProxyPrivate> d_ptr;
};

DMULTIMEDIA_END_NAMESPACE

#endif /* ifndef DGSTPLAYERPROXY_H */
