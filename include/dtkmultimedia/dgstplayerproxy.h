// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DGSTPLAYERPROXY_H
#define DGSTPLAYERPROXY_H

#include "dplayerbackend.h"
#include <QMediaPlayer>

DMULTIMEDIA_BEGIN_NAMESPACE
class VideoSurface;
class DGstPlayerProxyPrivate;

class DGstPlayerProxy : public DPlayerBackend
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DGstPlayerProxy)

public:
    explicit DGstPlayerProxy(QObject *parent = 0);
    virtual ~DGstPlayerProxy();
    void firstInit() override;
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
    void updateSubStyle(const QString &font, int sz) override;
    void setSubCodepage(const QString &cp) override;
    QString subCodepage() override;
    void addSubSearchPath(const QString &path) override;
    bool loadSubtitle(const QFileInfo &fi) override;
    void toggleSubtitle() override;
    bool isSubVisible() override;
    void selectSubtitle(int id) override;
    void selectTrack(int id) override;
    void setSubDelay(double secs) override;
    double subDelay() const override;
    int aid() const override;
    int sid() const override;
    void changeSoundMode(SoundMode) override;
    void setVideoAspect(double r) override;
    double videoAspect() const override;
    int videoRotation() const override;
    void setVideoRotation(int degree) override;
    QImage takeScreenshot() const override;
    void burstScreenshot() override;
    void stopBurstScreenshot() override;
    QVariant getProperty(const QString &) const override;
    void setProperty(const QString &, const QVariant &) override;
    void nextFrame() override;
    void previousFrame() override;
    void makeCurrent() override;
    void changehwaccelMode(hwaccelMode hwaccelMode) override;

protected slots:
#if BUILD_Qt6
    void slotStateChanged(QMediaPlayer::PlaybackState newState);
#else
    void slotStateChanged(QMediaPlayer::State newState);
#endif
    void slotMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void slotPositionChanged(qint64 position);
    void slotMediaError(QMediaPlayer::Error error);

protected:
    QScopedPointer<DGstPlayerProxyPrivate> d_ptr;
};

DMULTIMEDIA_END_NAMESPACE

#endif /* ifndef DGSTPLAYERPROXY_H */
