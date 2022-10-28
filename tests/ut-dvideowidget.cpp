// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mockdemo/mockdemo.h"
#include "gtest/gtest.h"
#include "3rdparty/cpp-stub/src/stub.h"
#include <DVideoWidget>
#include <DMediaPlayer>
#include <DMediaPlaylist>
#include <DEnginePlayer>
#include "dtkmultimedia.h"

DMULTIMEDIA_USE_NAMESPACE

class ut_DVideoWidget : public testing::Test
{
public:
    void SetUp() override
    {
        m_player = new DMediaPlayer(NULL);
        m_playlist = new DMediaPlaylist();
        m_player->setPlaylist(m_playlist);
        m_videoWgt = new DVideoWidget(NULL, new DEnginePlayer(m_player));
        m_player->setVideoOutput(m_videoWgt);
    }
    void TearDown() override
    {
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
    QWidget *m_testWgt = nullptr;
};

TEST_F(ut_DVideoWidget, getPlayer)
{
    ASSERT_NE(nullptr, m_videoWgt->getPlayer());
}

TEST_F(ut_DVideoWidget, setPlatformMediaPlayer)
{
    m_videoWgt->setPlatformMediaPlayer(nullptr);
}

TEST_F(ut_DVideoWidget, slotFullScreenChanged)
{
    m_videoWgt->slotFullScreenChanged(false);
}
