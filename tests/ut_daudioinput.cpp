// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkmultimedia.h"
#include "gtest/gtest.h"
#include <DAudioInput>

DMULTIMEDIA_USE_NAMESPACE

class ut_DAudioInput : public testing::Test
{
public:
    void SetUp() override
    {
        m_audioinput = new DAudioInput("");
    }
    void TearDown() override
    {
        delete m_audioinput;
        m_audioinput = nullptr;
    }

public:
    DAudioInput *m_audioinput = nullptr;
};

TEST_F(ut_DAudioInput, device)
{
    m_audioinput->setDevice("test");
    EXPECT_EQ("test", m_audioinput->device());
}

TEST_F(ut_DAudioInput, volume)
{
    float volume = 1.f;
    m_audioinput->setVolume(volume);
    EXPECT_EQ(volume, m_audioinput->volume());
}

TEST_F(ut_DAudioInput, isMuted)
{
    bool mute = true;
    m_audioinput->setMuted(mute);
    EXPECT_EQ(mute, m_audioinput->isMuted());
}
