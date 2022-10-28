// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mockdemo/mockdemo.h"
#include "gtest/gtest.h"
#include "3rdparty/cpp-stub/src/stub.h"
#include <DAudioOutput>
#include "dtkmultimedia.h"

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
    m_audioOut->setDevice(device);
}

TEST_F(ut_DAudioOutput, setVolume)
{
    float volume = 50.f;
    m_audioOut->setVolume(volume);
}

TEST_F(ut_DAudioOutput, setMuted)
{
    bool muted = false;
    m_audioOut->setMuted(muted);
}

TEST_F(ut_DAudioOutput, device)
{
    m_audioOut->device();
}

TEST_F(ut_DAudioOutput, volume)
{
    EXPECT_EQ(0.f, m_audioOut->volume());
}

TEST_F(ut_DAudioOutput, isMuted)
{
    EXPECT_EQ(false, m_audioOut->isMuted());
}
