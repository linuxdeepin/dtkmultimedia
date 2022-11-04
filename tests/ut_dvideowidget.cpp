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

class ut_DVideoWidget : public testing::Test
{
public:
    void SetUp() override
    {
        m_player = new DMediaPlayer();
        m_playlist = new DMediaPlaylist();
        m_enginePlayer = new DEnginePlayer(m_player);
        m_player->setPlaylist(m_playlist);
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

TEST_F(ut_DVideoWidget, getPlayer)
{
    ASSERT_NE(nullptr, m_videoWgt->getPlayer());
}

TEST_F(ut_DVideoWidget, setPlatformMediaPlayer)
{
    DEnginePlayer *player = new DEnginePlayer(m_player);
    m_videoWgt->setPlatformMediaPlayer(player);
    EXPECT_NE(nullptr, m_videoWgt->getPlayer());
    if (player) delete player;
}

TEST_F(ut_DVideoWidget, slotFullScreenChanged)
{
    m_videoWgt->slotFullScreenChanged(false);
    EXPECT_EQ(false, m_videoWgt->isVisible());
}
