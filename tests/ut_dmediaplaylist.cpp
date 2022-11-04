// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkmultimedia.h"
#include "gtest/gtest.h"
#include <DMediaPlaylist>

DMULTIMEDIA_USE_NAMESPACE

class ut_DMediaPlaylist : public testing::Test
{
public:
    void SetUp() override
    {
        m_playList = new DMediaPlaylist;
    }
    void TearDown() override
    {
        delete m_playList;
        m_playList = nullptr;
    }

public:
    DMediaPlaylist *m_playList = nullptr;
};
//此单元测试测试播放器中播放列表为空。
TEST_F(ut_DMediaPlaylist, shuffle)
{
    m_playList->shuffle();
    EXPECT_EQ(-1, m_playList->currentIndex());
}

TEST_F(ut_DMediaPlaylist, next)
{
    m_playList->next();
    EXPECT_EQ(-1, m_playList->currentIndex());
}

TEST_F(ut_DMediaPlaylist, previous)
{
    m_playList->previous();
    EXPECT_EQ(-1, m_playList->currentIndex());
}

TEST_F(ut_DMediaPlaylist, setCurrentIndex)
{
    m_playList->setCurrentIndex(-1);
    EXPECT_EQ(-1, m_playList->currentIndex());
}
