// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkmultimedia.h"
#include "gtest/gtest.h"
#include <QPoint>
#include <DScreenRecorder>

DMULTIMEDIA_USE_NAMESPACE

class ut_DScreenRecorder : public testing::Test
{
public:
    void SetUp() override
    {
        m_screenRecorder = new DScreenRecorder;
    }
    void TearDown() override
    {
        delete m_screenRecorder;
        m_screenRecorder = nullptr;
    }

public:
    DScreenRecorder *m_screenRecorder = nullptr;
};

TEST_F(ut_DScreenRecorder, setCodec)
{
    DScreenRecorder::VCodecID codec = DScreenRecorder::CODEC_ID_MPEG4;
    m_screenRecorder->setCodec(codec);
    EXPECT_EQ(codec, m_screenRecorder->codec());
}

TEST_F(ut_DScreenRecorder, setPixfmt)
{
    DScreenRecorder::PixFormatID format = DScreenRecorder::PIX_FMT_ARGB;
    m_screenRecorder->setPixfmt(format);
    EXPECT_EQ(format, m_screenRecorder->pixfmt());
}

TEST_F(ut_DScreenRecorder, setBitRate)
{
    int bitRateV = 34000;
    m_screenRecorder->setBitRate(bitRateV);
    EXPECT_EQ(bitRateV, m_screenRecorder->bitRate());
}

TEST_F(ut_DScreenRecorder, setFrameRate)
{
    int rate = 60;
    m_screenRecorder->setFrameRate(rate);
    EXPECT_EQ(rate, m_screenRecorder->frameRate());
}

TEST_F(ut_DScreenRecorder, setResolution)
{
    QSize size(1920, 1080);
    m_screenRecorder->setResolution(size.width(), size.height());
    EXPECT_EQ(size, m_screenRecorder->resolution());
}

TEST_F(ut_DScreenRecorder, setTopLeft)
{
    QPoint p(1920, 1080);
    m_screenRecorder->setTopLeft(p.x(), p.y());
    EXPECT_EQ(p, m_screenRecorder->topLeft());
}
