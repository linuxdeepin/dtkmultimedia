// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLATFFORMMEDIAPLAYER_H
#define DPLATFFORMMEDIAPLAYER_H

#include <QtMultimedia/qmediaplayer.h>
#include <QtMultimedia/qmediatimerange.h>
#include <QtCore/qpair.h>
#include "dmediametadata.h"
#include <dtkmultimedia.h>

DMULTIMEDIA_BEGIN_NAMESPACE

class QMediaStreamsControl;
class QPlatformAudioOutput;
class DVideoSink {

};

class Q_MULTIMEDIA_EXPORT DPlatformMediaPlayer: public QObject
{
    Q_OBJECT
public:
    virtual ~DPlatformMediaPlayer();
    virtual QMediaPlayer::State state() const;
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
    virtual void  setVolume (float volume) = 0;
    virtual void  setMuted (bool muted) = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual bool streamPlaybackSupported() const;
    virtual void setAudioOutput(QPlatformAudioOutput *);
    virtual DMediaMetaData metaData() const;
    virtual void setVideoSink(DVideoSink * ) = 0;
    enum TrackType { VideoStream, AudioStream, SubtitleStream, NTrackTypes };
    virtual int trackCount(TrackType);
    virtual DMediaMetaData trackMetaData(TrackType , int );
    virtual int activeTrack(TrackType);
    virtual void setActiveTrack(TrackType, int ) ;
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
    void stateChanged(QMediaPlayer::State newState);
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);
    void error(int error, const QString &errorString);
    void resetCurrentLoop();
    bool doLoop();
    int loops();
    void setLoops(int loops);

protected:
    explicit DPlatformMediaPlayer(QMediaPlayer *parent = nullptr);

private:
    QMediaPlayer *m_player = nullptr;
    QMediaPlayer::MediaStatus m_status = QMediaPlayer::NoMedia;
    QMediaPlayer::State m_state = QMediaPlayer::StoppedState;
    bool m_seekable = false;
    bool m_videoAvailable = false;
    bool m_audioAvailable = false;
    int m_loops = 1;
    int m_currentLoop = 0;
    int m_volume = 0;
    int m_muted = false;
};

DMULTIMEDIA_END_NAMESPACE


#endif  //

