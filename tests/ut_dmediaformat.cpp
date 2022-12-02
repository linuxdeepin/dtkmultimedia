// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkmultimedia.h"
#include "gtest/gtest.h"
#include <DMediaFormat>

DMULTIMEDIA_USE_NAMESPACE

class ut_DMediaFormat : public testing::Test
{
public:
    void SetUp() override
    {
        m_format = new DMediaFormat;
    }
    void TearDown() override
    {
        delete m_format;
        m_format = nullptr;
    }

public:
    DMediaFormat *m_format = nullptr;
};

TEST_F(ut_DMediaFormat, fileFormat)
{
    m_format->setFileFormat(DMediaFormat::MPEG4);
    EXPECT_EQ(DMediaFormat::MPEG4, m_format->fileFormat());
}

TEST_F(ut_DMediaFormat, videoCodec)
{
    m_format->setVideoCodec(DMediaFormat::VideoCodec::H265);
    EXPECT_EQ(DMediaFormat::VideoCodec::H265, m_format->videoCodec());
}

TEST_F(ut_DMediaFormat, audioCodec)
{
    m_format->setAudioCodec(DMediaFormat::AudioCodec::AC3);
    EXPECT_EQ(DMediaFormat::AudioCodec::AC3, m_format->audioCodec());
}

TEST_F(ut_DMediaFormat, isSupported)
{
    EXPECT_EQ(false, m_format->isSupported(DMediaFormat::Encode));
}

TEST_F(ut_DMediaFormat, supportedFileFormats)
{
    EXPECT_EQ(QList<DMediaFormat::FileFormat>(), m_format->supportedFileFormats(DMediaFormat::Encode));
}

TEST_F(ut_DMediaFormat, supportedVideoCodecs)
{
    EXPECT_EQ(QList<DMediaFormat::VideoCodec>(), m_format->supportedVideoCodecs(DMediaFormat::Encode));
}

TEST_F(ut_DMediaFormat, supportedAudioCodecs)
{
    EXPECT_EQ(QList<DMediaFormat::AudioCodec>(), m_format->supportedAudioCodecs(DMediaFormat::Encode));
}

TEST_F(ut_DMediaFormat, fileFormatName)
{
    EXPECT_EQ("MP3", m_format->fileFormatName(DMediaFormat::MP3));
}

TEST_F(ut_DMediaFormat, audioCodecName)
{
    EXPECT_EQ("MP3", m_format->audioCodecName(DMediaFormat::AudioCodec::MP3));
}

TEST_F(ut_DMediaFormat, videoCodecName)
{
    EXPECT_EQ("MPEG1", m_format->videoCodecName(DMediaFormat::VideoCodec::MPEG1));
}

TEST_F(ut_DMediaFormat, fileFormatDescription)
{
    EXPECT_EQ("Windows Media Video", m_format->fileFormatDescription(DMediaFormat::WMV));
}

TEST_F(ut_DMediaFormat, audioCodecDescription)
{
    EXPECT_EQ("Advanced Audio Codec (AAC)", m_format->audioCodecDescription(DMediaFormat::AudioCodec::AAC));
}

TEST_F(ut_DMediaFormat, videoCodecDescription)
{
    EXPECT_EQ("MPEG-4 Video", m_format->videoCodecDescription(DMediaFormat::VideoCodec::MPEG4));
}
