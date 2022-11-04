// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkmultimedia.h"
#include "gtest/gtest.h"
#include <DEnginePlayer>
#include <DMediaPlayer>
#include <DMediaPlaylist>
#include <DVideoWidget>

DMULTIMEDIA_USE_NAMESPACE

class ut_DEnginePlayer : public testing::Test
{
public:
    void SetUp() override
    {
        m_player = new DMediaPlayer();
        m_playlist = new DMediaPlaylist();
        m_player->setPlaylist(m_playlist);
        m_enginePlayer = new DEnginePlayer(m_player);
        m_videoWgt = new DVideoWidget(nullptr, m_enginePlayer);
        m_player->setVideoOutput(m_videoWgt);
    }
    void TearDown() override
    {
        delete m_enginePlayer;
        m_enginePlayer = nullptr;
        delete m_videoWgt;
        m_videoWgt = nullptr;
        delete m_playlist;
        m_playlist = nullptr;
        delete m_player;
        m_player = nullptr;
    }

public:
    DVideoWidget *m_videoWgt = nullptr;
    DMediaPlayer *m_player = nullptr;
    DMediaPlaylist *m_playlist = nullptr;
    DEnginePlayer *m_enginePlayer = nullptr;
};

TEST_F(ut_DEnginePlayer, duration)
{
    EXPECT_EQ(0, m_enginePlayer->duration());
}

TEST_F(ut_DEnginePlayer, position)
{
    EXPECT_EQ(0, m_enginePlayer->position());
}

TEST_F(ut_DEnginePlayer, setPosition)
{
    qint64 position = 0;
    m_enginePlayer->setPosition(position);
    EXPECT_EQ(0, m_enginePlayer->position());
}

TEST_F(ut_DEnginePlayer, bufferProgress)
{
    EXPECT_EQ(0.f, m_enginePlayer->bufferProgress());
}

TEST_F(ut_DEnginePlayer, availablePlaybackRanges)
{
    EXPECT_EQ(QMediaTimeRange(), m_enginePlayer->availablePlaybackRanges());
}

TEST_F(ut_DEnginePlayer, playbackRate)
{
    EXPECT_EQ(0, m_enginePlayer->playbackRate());
}

TEST_F(ut_DEnginePlayer, setPlaybackRate)
{
    qreal rate = 0.0;
    m_enginePlayer->setPlaybackRate(rate);
    EXPECT_EQ(0, m_enginePlayer->playbackRate());
}

TEST_F(ut_DEnginePlayer, setMedia)
{
    QUrl url = QUrl("test");
    m_enginePlayer->setMedia(url, nullptr);
    EXPECT_EQ(url, m_enginePlayer->media());
}

TEST_F(ut_DEnginePlayer, mediaStream)
{
    QUrl url = QUrl("test");
    EXPECT_EQ(nullptr, m_enginePlayer->mediaStream());
}

TEST_F(ut_DEnginePlayer, setVolume)
{
    float volume = 50.f;
    m_enginePlayer->setVolume(volume);
    DAudioOutput *audioOutput = m_enginePlayer->audioOut();
    EXPECT_EQ(volume, audioOutput->volume());
}

TEST_F(ut_DEnginePlayer, setMuted)
{
    bool muted = false;
    m_enginePlayer->setMuted(muted);
    DAudioOutput *audioOutput = m_enginePlayer->audioOut();
    EXPECT_EQ(muted, audioOutput->isMuted());
}

TEST_F(ut_DEnginePlayer, play)
{
    m_enginePlayer->setMedia(QUrl("test"), nullptr);
    m_enginePlayer->play();
    EXPECT_EQ(QMediaPlayer::StoppedState, m_enginePlayer->state());
}

TEST_F(ut_DEnginePlayer, pause)
{
    m_enginePlayer->pause();
    EXPECT_EQ(QMediaPlayer::StoppedState, m_enginePlayer->state());
}

TEST_F(ut_DEnginePlayer, stop)
{
    m_enginePlayer->stop();
    EXPECT_EQ(QMediaPlayer::StoppedState, m_enginePlayer->state());
}

TEST_F(ut_DEnginePlayer, setVideoSink)
{
    m_enginePlayer->setVideoSink(nullptr);
    EXPECT_EQ(QMediaPlayer::StoppedState, m_enginePlayer->state());
}

TEST_F(ut_DEnginePlayer, changeSoundMode)
{
    DPlayerBackend::SoundMode mode = DPlayerBackend::Left;
    m_enginePlayer->changeSoundMode(mode);
    EXPECT_EQ(QMediaPlayer::StoppedState, m_enginePlayer->state());
}

TEST_F(ut_DEnginePlayer, nextFrame)
{
    m_enginePlayer->nextFrame();
    EXPECT_EQ(QMediaPlayer::StoppedState, m_enginePlayer->state());
}

TEST_F(ut_DEnginePlayer, previousFrame)
{
    m_enginePlayer->previousFrame();
    EXPECT_EQ(QMediaPlayer::StoppedState, m_enginePlayer->state());
}

TEST_F(ut_DEnginePlayer, setDecodeModel)
{
    DPlayerBackend::hwaccelMode mode = DPlayerBackend::hwaccelAuto;
    m_enginePlayer->setDecodeModel(mode);
    EXPECT_EQ(QMediaPlayer::StoppedState, m_enginePlayer->state());
}

TEST_F(ut_DEnginePlayer, takeScreenshot)
{
    EXPECT_EQ(QImage(), m_enginePlayer->takeScreenshot());
}

TEST_F(ut_DEnginePlayer, burstScreenshot)
{
    m_enginePlayer->burstScreenshot();
    EXPECT_EQ(QMediaPlayer::StoppedState, m_enginePlayer->state());
}

TEST_F(ut_DEnginePlayer, setVideoRotation)
{
    int degree = 90;
    m_enginePlayer->setVideoRotation(90);
    EXPECT_EQ(QMediaPlayer::StoppedState, m_enginePlayer->state());
}

TEST_F(ut_DEnginePlayer, changeVolume)
{
    int val = 90;
    m_enginePlayer->changeVolume(val);
    DAudioOutput *audioOutput = m_enginePlayer->audioOut();
    EXPECT_EQ(val, audioOutput->volume());
}

TEST_F(ut_DEnginePlayer, seekAbsolute)
{
    int pos = 0;
    m_enginePlayer->seekAbsolute(pos);
    EXPECT_EQ(pos, m_enginePlayer->position());
}

TEST_F(ut_DEnginePlayer, setPlayMode)
{
    PlayMode mode = PlayMode::ShufflePlay;
    m_enginePlayer->setPlayMode(mode);
    EXPECT_EQ(mode, m_enginePlayer->playMode());
}

TEST_F(ut_DEnginePlayer, playByName)
{
    m_enginePlayer->playByName(QUrl("test"));
    EXPECT_EQ(QMediaPlayer::StoppedState, m_enginePlayer->state());
}

TEST_F(ut_DEnginePlayer, loadSubtitle)
{
    QFileInfo info;
    EXPECT_TRUE(m_enginePlayer->loadSubtitle(info));
}

TEST_F(ut_DEnginePlayer, addPlayFile)
{
    EXPECT_TRUE(m_enginePlayer->addPlayFile(QUrl("test")));
}

TEST_F(ut_DEnginePlayer, movieInfo)
{
    MovieInfo info = m_enginePlayer->movieInfo();
    EXPECT_EQ(false, info.valid);
}
