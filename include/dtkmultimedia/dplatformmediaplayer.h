// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLATFFORMMEDIAPLAYER_H
#define DPLATFFORMMEDIAPLAYER_H

#include "dmediametadata.h"
#include <QtMultimedia/qmediaplayer.h>
#include <QtMultimedia/qmediatimerange.h>
#include <dtkmultimedia.h>

DMULTIMEDIA_BEGIN_NAMESPACE

class QMediaStreamsControl;
class QPlatformAudioOutput;
class DVideoSink
{
};

class DPlatformMediaPlayerPrivate;
class Q_MULTIMEDIA_EXPORT DPlatformMediaPlayer : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DPlatformMediaPlayer)

public:
    virtual ~DPlatformMediaPlayer();
#if BUILD_Qt6
    virtual QMediaPlayer::PlaybackState playbackState() const;
#else
    virtual QMediaPlayer::State state() const;
#endif
    virtual QMediaPlayer::MediaStatus mediaStatus() const;
    virtual qint64 duration() const = 0;
    virtual qint64 position() const = 0;
    virtual void setPosition(qint64 position) = 0;
    virtual float bufferProgress() const = 0;
    virtual bool isAudioAvailable() const;
    virtual bool isVideoAvailable() const;
    virtual bool isSeekable() const;
    virtual QMediaTimeRange availablePlaybackRanges() const = 0;
    virtual qreal playbackRate() const = 0;
    virtual void setPlaybackRate(qreal rate) = 0;
    virtual QUrl media() const = 0;
    virtual const QIODevice *mediaStream() const = 0;
    virtual void setMedia(const QUrl &media, QIODevice *stream) = 0;
    virtual void setVolume(float volume) = 0;
    virtual void setMuted(bool muted) = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual bool streamPlaybackSupported() const;
    virtual void setAudioOutput(QPlatformAudioOutput *);
    virtual DMediaMetaData metaData() const;
    virtual void setVideoSink(DVideoSink *) = 0;
    enum TrackType { VideoStream,
                     AudioStream,
                     SubtitleStream,
                     NTrackTypes };
    virtual int trackCount(TrackType);
    virtual DMediaMetaData trackMetaData(TrackType, int);
    virtual int activeTrack(TrackType);
    virtual void setActiveTrack(TrackType, int);
    void durationChanged(qint64 duration);
    void positionChanged(qint64 position);
    void audioAvailableChanged(bool audioAvailable);
    void videoAvailableChanged(bool videoAvailable);
    void seekableChanged(bool seekable);
    void volumeChanged(int volume);
    void mutedChanged(bool muted);
    void playbackRateChanged(qreal rate);
    void bufferProgressChanged(float progress);
    void metaDataChanged();
    void tracksChanged();
    void activeTracksChanged();
#if BUILD_Qt6
    void playbackStateChanged(QMediaPlayer::PlaybackState newState);
#else
    void stateChanged(QMediaPlayer::State newState);
#endif
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);
    void error(int error, const QString &errorString);
    void resetCurrentLoop();
    bool doLoop();
    int loops();
    void setLoops(int loops);

protected:
    explicit DPlatformMediaPlayer(QMediaPlayer *parent = nullptr);

protected:
    QScopedPointer<DPlatformMediaPlayerPrivate> d_ptr;
};

DMULTIMEDIA_END_NAMESPACE

#endif   //
