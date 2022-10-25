// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QMEDIAPLAYERCONTROL_H
#define QMEDIAPLAYERCONTROL_H

#include <QtMultimedia/qmediaplayer.h>
#include <QtMultimedia/qmediatimerange.h>
#include "dmediametadata.h"

#include <QtCore/qpair.h>
#include <dtkmultimedia.h>
DMULTIMEDIA_USE_NAMESPACE
DMULTIMEDIA_BEGIN_NAMESPACE

class QMediaStreamsControl;
class QPlatformAudioOutput;
class DMediaMetaData;
class DVideoSink {

};

class Q_MULTIMEDIA_EXPORT DPlatformMediaPlayer: public QObject
{
    Q_OBJECT
public:
    virtual ~DPlatformMediaPlayer();
    virtual QMediaPlayer::State state() const { return m_state; }
    virtual QMediaPlayer::MediaStatus mediaStatus() const { return m_status; };

    virtual qint64 duration() const = 0;

    virtual qint64 position() const = 0;
    virtual void setPosition(qint64 position) = 0;

    virtual float bufferProgress() const = 0;

    virtual bool isAudioAvailable() const { return m_audioAvailable; }
    virtual bool isVideoAvailable() const { return m_videoAvailable; }

    virtual bool isSeekable() const { return m_seekable; }

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

    virtual bool streamPlaybackSupported() const { return false; }

    virtual void setAudioOutput(QPlatformAudioOutput *) {}

//    virtual DMediaMetaData metaData() const { return {}; }

    virtual void setVideoSink(DVideoSink * /*sink*/) = 0;

    // media streams
    enum TrackType { VideoStream, AudioStream, SubtitleStream, NTrackTypes };

    virtual int trackCount(TrackType) { return 0; };
//    virtual DMediaMetaData trackMetaData(TrackType /*type*/, int /*streamNumber*/) { return DMediaMetaData(); }
    virtual int activeTrack(TrackType) { return -1; }
    virtual void setActiveTrack(TrackType, int /*streamNumber*/) {}

    void durationChanged(qint64 duration) {
        m_player->durationChanged(duration);
    }
    void positionChanged(qint64 position) { m_player->positionChanged(position); }
    void audioAvailableChanged(bool audioAvailable) {
        if (m_audioAvailable == audioAvailable)
            return;
        m_audioAvailable = audioAvailable;
//        player->hasAudioChanged(audioAvailable);
    }
    void videoAvailableChanged(bool videoAvailable) {
        if (m_videoAvailable == videoAvailable)
            return;
        m_videoAvailable = videoAvailable;
//        player->hasVideoChanged(videoAvailable);
    }
    void seekableChanged(bool seekable) {
        if (m_seekable == seekable)
            return;
        m_seekable = seekable;
        m_player->seekableChanged(seekable);
    }
    void volumeChanged(int volume) {
        if (m_volume == volume)
            return;
        m_volume = volume;
        m_player->volumeChanged(volume);
    }
    void mutedChanged(bool muted) {
        if (m_muted == muted)
            return;
        m_muted = muted;
        m_player->mutedChanged(muted);
    }

    void playbackRateChanged(qreal rate) { m_player->playbackRateChanged(rate); }
    void bufferProgressChanged(float progress) {/* player->bufferProgressChanged(progress);*/ }
    void metaDataChanged() { m_player->metaDataChanged(); }
    void tracksChanged() { /*player->tracksChanged();*/ }
    void activeTracksChanged() { /*player->activeTracksChanged(); */}

    void stateChanged(QMediaPlayer::State newState);
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);
    void error(int error, const QString &errorString);

    void resetCurrentLoop() { m_currentLoop = 0; }
    bool doLoop() {
        return isSeekable() && (m_loops < 0 || ++m_currentLoop < m_loops);
    }
    int loops() { return m_loops; }
    void setLoops(int loops) {
        if (m_loops == loops)
            return;
        m_loops = loops;
//        Q_EMIT player->loopsChanged();
    }

protected:
    explicit DPlatformMediaPlayer(QMediaPlayer *parent = nullptr)
        : m_player(parent), QObject(parent)
    {}
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

