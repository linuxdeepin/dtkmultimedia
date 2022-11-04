// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkmultimedia.h"
#include "gtest/gtest.h"
#include <DMediaPlayer>
#include <DMediaPlaylist>

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
//此单元测试测试播放器中播放指针为空时覆盖。
TEST_F(ut_DMediaPlayer, play)
{
    m_player->play();
    EXPECT_EQ(nullptr, m_player->getPlayer());
}

TEST_F(ut_DMediaPlayer, pause)
{
    m_player->pause();
    EXPECT_EQ(nullptr, m_player->getPlayer());
}

TEST_F(ut_DMediaPlayer, stop)
{
    m_player->stop();
    EXPECT_EQ(nullptr, m_player->getPlayer());
}

TEST_F(ut_DMediaPlayer, setVolume)
{
    int volume = 50;
    m_player->setVolume(volume);
    EXPECT_EQ(nullptr, m_player->getPlayer());
}

TEST_F(ut_DMediaPlayer, setMuted)
{
    bool muted = true;
    m_player->setMuted(muted);
    EXPECT_EQ(nullptr, m_player->getPlayer());
}

TEST_F(ut_DMediaPlayer, setPlaybackRate)
{
    qreal rate = 1.0;
    m_player->setPlaybackRate(rate);
    EXPECT_EQ(nullptr, m_player->getPlayer());
}

TEST_F(ut_DMediaPlayer, setMedia)
{
    QMediaContent content(QUrl(""));
    m_player->setMedia(content, nullptr);
    EXPECT_EQ(content, m_player->media());
}

TEST_F(ut_DMediaPlayer, setPlaylist)
{
    DMediaPlaylist *playList = new DMediaPlaylist();
    m_player->setPlaylist(playList);
    EXPECT_EQ(playList, m_player->getPlaylist());
    if (playList) delete playList;
}

TEST_F(ut_DMediaPlayer, setNetworkConfigurations)
{
    QList<QNetworkConfiguration> configList;
    m_player->setNetworkConfigurations(configList);
    QNetworkConfiguration config = m_player->currentNetworkConfiguration();
    EXPECT_EQ(false, config.isValid());
}

TEST_F(ut_DMediaPlayer, setVideoOutput)
{
    QVideoWidget *videoWgt = nullptr;
    m_player->setVideoOutput(videoWgt);
    EXPECT_EQ(nullptr, m_player->getVideoOutput());
}

TEST_F(ut_DMediaPlayer, setPlayer)
{
    DPlatformMediaPlayer *player = nullptr;
    m_player->setPlayer(player);
    EXPECT_EQ(nullptr, m_player->getPlayer());
}
