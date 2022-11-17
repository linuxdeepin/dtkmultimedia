// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkmultimedia.h"
#include "gtest/gtest.h"
#include <QThread>
#include <DAudioRecorder>

DMULTIMEDIA_USE_NAMESPACE

class ut_DAudioRecorder : public testing::Test
{
public:
    void SetUp() override
    {
        m_audioRecorder = new DAudioRecorder;
    }
    void TearDown() override
    {
        delete m_audioRecorder;
        m_audioRecorder = nullptr;
    }

public:
    DAudioRecorder *m_audioRecorder = nullptr;
};

TEST_F(ut_DAudioRecorder, setCodec)
{
    DAudioRecorder::ACodecID codec = DAudioRecorder::CODEC_ID_AAC;
    m_audioRecorder->setCodec(codec);
    EXPECT_EQ(codec, m_audioRecorder->codec());
}

TEST_F(ut_DAudioRecorder, setBitRate)
{
    int bitRateV = 34000;
    m_audioRecorder->setBitRate(bitRateV);
    EXPECT_EQ(bitRateV, m_audioRecorder->bitRate());
}

TEST_F(ut_DAudioRecorder, setChannelCount)
{
    DAudioRecorder::AChannelsID count = DAudioRecorder::CHANNELS_ID_MONO;
    m_audioRecorder->setChannelCount(count);
    EXPECT_EQ(count, m_audioRecorder->channelCount());
}

TEST_F(ut_DAudioRecorder, setAudioInput)
{
    QString device = "default";
    m_audioRecorder->setAudioInput(device);
    EXPECT_EQ(device, m_audioRecorder->audioInput());
}

TEST_F(ut_DAudioRecorder, setSampleRate)
{
    int sampleR = 36000;
    m_audioRecorder->setSampleRate(sampleR);
    EXPECT_EQ(sampleR, m_audioRecorder->sampleRate());
}
