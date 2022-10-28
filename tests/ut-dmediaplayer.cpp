// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mockdemo/mockdemo.h"
#include "gtest/gtest.h"
#include "3rdparty/cpp-stub/src/stub.h"
#include <DMediaPlayer>
#include "dtkmultimedia.h"

DMULTIMEDIA_USE_NAMESPACE

class ut_DMediaPlayer : public testing::Test
{
public:
    void SetUp() override
    {
        m_player = new DMediaPlayer;
    }
    void TearDown() override
    {
        delete m_player;
        m_player = nullptr;
    }
public:
    DMediaPlayer *m_player = nullptr;
};

TEST_F(ut_DMediaPlayer, play)
{
    m_player->play();
}

TEST_F(ut_DMediaPlayer, pause)
{
    m_player->pause();
}

TEST_F(ut_DMediaPlayer, stop)
{
    m_player->stop();
}

TEST_F(ut_DMediaPlayer, setVolume)
{
    int volume = 50;
    m_player->setVolume(volume);
}

TEST_F(ut_DMediaPlayer, setMuted)
{
    bool muted = true;
    m_player->setMuted(muted);
}


TEST_F(ut_DMediaPlayer, setPlaybackRate)
{
    qreal rate = 1.0;
    m_player->setPlaybackRate(rate);
}

TEST_F(ut_DMediaPlayer, setMedia)
{
    QMediaContent content(QUrl(""));
    m_player->setMedia(content, nullptr);
}

TEST_F(ut_DMediaPlayer, setPlaylist)
{
    QMediaPlaylist *playList = nullptr;
    m_player->setPlaylist(playList);
}

TEST_F(ut_DMediaPlayer, setNetworkConfigurations)
{
    QList<QNetworkConfiguration> configList;
    m_player->setNetworkConfigurations(configList);
}

TEST_F(ut_DMediaPlayer, setVideoOutput)
{
    QVideoWidget *videoWgt = nullptr;
    m_player->setVideoOutput(videoWgt);
}

TEST_F(ut_DMediaPlayer, setPlayer)
{
    DPlatformMediaPlayer *player = nullptr;
    m_player->setPlayer(player);
}
