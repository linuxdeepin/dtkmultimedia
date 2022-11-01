// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DENGINEPLAYER_H
#define DENGINEPLAYER_H

#include <DPlatformMediaPlayer>
#include <DPlayerBackend>
#include <DAudioOutput>
#include <dtkmultimedia.h>
DMULTIMEDIA_BEGIN_NAMESPACE
class PlayerEngine;
struct MovieInfo {
    bool valid;
    QString title;
    QString fileType;
    QString resolution;
    QString filePath;
    QString creation;
    int rawRotate;
    qint64 fileSize;
    qint64 duration;
    int width = -1;
    int height = -1;
    int vCodecID;
    qint64 vCodeRate;
    int fps;
    float proportion;
    int aCodeID;
    qint64 aCodeRate;
    int aDigit;
    int channels;
    int sampling;
#ifdef _MOVIE_USE_
    QString strFmtName;
#endif
};
enum PlayMode {
    OrderPlay,
    ShufflePlay,
    SinglePlay,
    SingleLoop,
    ListLoop,
};
class Q_MULTIMEDIA_EXPORT DEnginePlayer : public DPlatformMediaPlayer
{
    Q_OBJECT
public:
    DEnginePlayer(QMediaPlayer *parent = nullptr);
    ~DEnginePlayer();
    virtual qint64 duration() const;
    virtual qint64 position() const;
    virtual void setPosition(qint64 position);
    virtual float bufferProgress() const ;
    virtual QMediaTimeRange availablePlaybackRanges() const;
    virtual qreal playbackRate() const;
    virtual void setPlaybackRate(qreal rate);
    virtual QUrl media() const;
    virtual const QIODevice *mediaStream() const;
    virtual void setMedia(const QUrl &media, QIODevice *stream);
    virtual void  setVolume (float volume);
    virtual void  setMuted (bool muted);
    virtual void play();
    virtual void pause();
    virtual void stop();
    virtual void setVideoSink(DVideoSink * /*sink*/);
    virtual void setPlayer(QWidget * Player);
    void setPlaySpeed(double times);
    void changeSoundMode(const DPlayerBackend::SoundMode &sm);
    void nextFrame();
    void previousFrame();
    void setDecodeModel(const DPlayerBackend::hwaccelMode &hwaccelMode);
    QImage takeScreenshot();
    void burstScreenshot();
    void setVideoRotation(int degree);
    void changeVolume(int val);
    void setMute(bool bMute);
    void seekAbsolute(int pos);
    void setPlayMode(const PlayMode &pm);
    PlayMode playMode() const;
    void playByName(const QUrl &url);
    bool loadSubtitle(const QFileInfo &fi);
    bool addPlayFile(const QUrl &url);
    const struct MovieInfo &movieInfo();
    DAudioOutput *audioOut();
    
public slots:
    void positionProxyChanged();

private:
    QWidget *m_pPlayer = nullptr;
    QMediaPlayer *m_mediaPlayer = nullptr;
    QUrl m_media;
    PlayerEngine *m_engine = nullptr;
    DAudioOutput *m_audioOutput = nullptr;
    MovieInfo m_movieInfo;
};
DMULTIMEDIA_END_NAMESPACE

#endif
