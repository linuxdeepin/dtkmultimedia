// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkmultimedia.h"
#include "gtest/gtest.h"
#include <DAudioOutput>

DMULTIMEDIA_USE_NAMESPACE

class ut_DAudioOutput : public testing::Test
{
public:
    void SetUp() override
    {
        m_audioOut = new DAudioOutput;
    }
    void TearDown() override
    {
        delete m_audioOut;
        m_audioOut = nullptr;
    }

public:
    DAudioOutput *m_audioOut = nullptr;
};

TEST_F(ut_DAudioOutput, setDevice)
{
    DAudioDevice device;
    device.setDevice("test");
    m_audioOut->setDevice(device);
    EXPECT_EQ("test", m_audioOut->device().device());
}

TEST_F(ut_DAudioOutput, setVolume)
{
    float volume = 50.f;
    m_audioOut->setVolume(volume);
    EXPECT_EQ(volume, m_audioOut->volume());
}

TEST_F(ut_DAudioOutput, setMuted)
{
    bool muted = false;
    m_audioOut->setMuted(muted);
    EXPECT_EQ(muted, m_audioOut->isMuted());
}

TEST_F(ut_DAudioOutput, device)
{
    DAudioDevice device;
    device.setDevice("testdevice");
    m_audioOut->setDevice(device);
    EXPECT_EQ("testdevice", m_audioOut->device().device());
}

TEST_F(ut_DAudioOutput, volume)
{
    float volume = 100.f;
    m_audioOut->setVolume(volume);
    EXPECT_EQ(volume, m_audioOut->volume());
}

TEST_F(ut_DAudioOutput, isMuted)
{
    bool muted = true;
    m_audioOut->setMuted(muted);
    EXPECT_EQ(muted, m_audioOut->isMuted());
}
