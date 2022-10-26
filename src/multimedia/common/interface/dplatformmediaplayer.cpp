// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dplatformmediaplayer.h"
#include "qmediaplayer.h"

DMULTIMEDIA_USE_NAMESPACE

DPlatformMediaPlayer::~DPlatformMediaPlayer()
{
}

DPlatformMediaPlayer::DPlatformMediaPlayer(QMediaPlayer *parent)
        : m_player(parent), QObject(parent)
{

}

void DPlatformMediaPlayer::stateChanged(QMediaPlayer::State newState)
{
    if (newState == m_state)
        return;
    m_state = newState;
    m_player->stateChanged(newState);
}

void DPlatformMediaPlayer::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (m_status == status)
        return;
    m_status = status;
}


QMediaPlayer::State DPlatformMediaPlayer::state() const 
{
        return m_state;
}

QMediaPlayer::MediaStatus DPlatformMediaPlayer::mediaStatus() const 
{ 
    return m_status; 
}

bool DPlatformMediaPlayer::isAudioAvailable() const 
{ 
    return m_audioAvailable;
}

bool DPlatformMediaPlayer::isVideoAvailable() const 
{ 
    return m_videoAvailable; 
}

bool DPlatformMediaPlayer::isSeekable() const 
{ 
    return m_seekable; 
}

bool DPlatformMediaPlayer::streamPlaybackSupported() const 
{ 
    return false; 
}

void DPlatformMediaPlayer::setAudioOutput(QPlatformAudioOutput *) 
{

}

DMediaMetaData DPlatformMediaPlayer::metaData() const
{ 
    return {};
}

int DPlatformMediaPlayer::trackCount(TrackType) 
{ 
    return 0;
}

DMediaMetaData DPlatformMediaPlayer::trackMetaData(TrackType , int ) 
{ 
    return DMediaMetaData(); 
}

int DPlatformMediaPlayer::activeTrack(TrackType) 
{ 
    return -1; 
}

void DPlatformMediaPlayer::setActiveTrack(TrackType, int ) 
{

}

void DPlatformMediaPlayer::durationChanged(qint64 duration) 
{
    m_player->durationChanged(duration);
}

void DPlatformMediaPlayer::positionChanged(qint64 position) 
{ 
    m_player->positionChanged(position); 
}

void DPlatformMediaPlayer::audioAvailableChanged(bool audioAvailable) 
{
    if (m_audioAvailable == audioAvailable)
        return;
    m_audioAvailable = audioAvailable;
}

void DPlatformMediaPlayer::videoAvailableChanged(bool videoAvailable) 
{
    if (m_videoAvailable == videoAvailable)
        return;
    m_videoAvailable = videoAvailable;
}

void DPlatformMediaPlayer::seekableChanged(bool seekable) 
{
    if (m_seekable == seekable)
        return;
    m_seekable = seekable;
    m_player->seekableChanged(seekable);
}

void DPlatformMediaPlayer::volumeChanged(int volume) 
{
    if (m_volume == volume)
        return;
    m_volume = volume;
    m_player->volumeChanged(volume);
}

void DPlatformMediaPlayer::mutedChanged(bool muted) 
{
    if (m_muted == muted)
        return;
    m_muted = muted;
    m_player->mutedChanged(muted);
}

void DPlatformMediaPlayer::playbackRateChanged(qreal rate) 
{ 
    m_player->playbackRateChanged(rate); 
}

void DPlatformMediaPlayer::bufferProgressChanged(float progress) 
{ 

}

void DPlatformMediaPlayer::metaDataChanged() 
{ 
    m_player->metaDataChanged(); 
}

void DPlatformMediaPlayer::tracksChanged() 
{

}

void DPlatformMediaPlayer::activeTracksChanged() 
{

}


void DPlatformMediaPlayer::error(int error, const QString &errorString)
{

}

void DPlatformMediaPlayer::resetCurrentLoop() 
{ 
    m_currentLoop = 0; 
}

bool DPlatformMediaPlayer::doLoop() 
{
    return isSeekable() && (m_loops < 0 || ++m_currentLoop < m_loops);
}

int DPlatformMediaPlayer::loops() 
{ 
    return m_loops; 
}

void DPlatformMediaPlayer::setLoops(int loops) 
{
    if (m_loops == loops)
        return;
    m_loops = loops;
}
