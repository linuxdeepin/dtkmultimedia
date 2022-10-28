// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mockdemo/mockdemo.h"
#include "gtest/gtest.h"
#include "3rdparty/cpp-stub/src/stub.h"
#include <DMediaPlaylist>
#include "dtkmultimedia.h"

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

TEST_F(ut_DMediaPlaylist, shuffle)
{
    m_playList->shuffle();
}

TEST_F(ut_DMediaPlaylist, next)
{
    m_playList->next();
}

TEST_F(ut_DMediaPlaylist, previous)
{
    m_playList->previous();
}

TEST_F(ut_DMediaPlaylist, setCurrentIndex)
{
    m_playList->setCurrentIndex(-1);
}

