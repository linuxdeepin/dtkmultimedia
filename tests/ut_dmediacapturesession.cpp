// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkmultimedia.h"
#include "gtest/gtest.h"
#include <DMediaCaptureSession>
#include <DAudioInput>
#include <DAudioOutput>
#include <DCamera>
#include <DImageCapture>
#include <DMediaRecorder>
#include <DPlatformMediaPlayer>

DMULTIMEDIA_USE_NAMESPACE

class ut_DMediaCaptureSession : public testing::Test
{
public:
    void SetUp() override
    {
        m_session = new DMediaCaptureSession;
    }
    void TearDown() override
    {
        delete m_session;
        m_session = nullptr;
    }

public:
    DMediaCaptureSession *m_session = nullptr;
};

TEST_F(ut_DMediaCaptureSession, audioInput)
{
    DAudioInput *aInput = new DAudioInput("test");
    m_session->setAudioInput(aInput);
    EXPECT_EQ(aInput, m_session->audioInput());
    delete aInput;
}

TEST_F(ut_DMediaCaptureSession, camera)
{
    DCamera *camera = new DCamera();
    m_session->setCamera(camera);
    EXPECT_EQ(camera, m_session->camera());
    delete camera;
}

TEST_F(ut_DMediaCaptureSession, imageCapture)
{
    DImageCapture *cap = new DImageCapture();
    m_session->setImageCapture(cap);
    EXPECT_EQ(cap, m_session->imageCapture());
    delete cap;
}

TEST_F(ut_DMediaCaptureSession, recorder)
{
    DMediaRecorder *recoder = new DMediaRecorder();
    m_session->setRecorder(recoder);
    EXPECT_EQ(recoder, m_session->recorder());
    delete recoder;
}

TEST_F(ut_DMediaCaptureSession, videoOutput)
{
    QObject *videoOutput = new QObject();
    m_session->setVideoOutput(videoOutput);
    EXPECT_EQ(videoOutput, m_session->videoOutput());
    delete videoOutput;
}

TEST_F(ut_DMediaCaptureSession, videoSink)
{
    DVideoSink *sink = new DVideoSink();
    m_session->setVideoSink(sink);
    EXPECT_EQ(sink, m_session->videoSink());
    delete sink;
}

TEST_F(ut_DMediaCaptureSession, platformSession)
{
    EXPECT_EQ(nullptr, m_session->platformSession());
}

TEST_F(ut_DMediaCaptureSession, audioOutput)
{
    DAudioOutput *aOutput = new DAudioOutput();
    m_session->setAudioOutput(aOutput);
    EXPECT_EQ(aOutput, m_session->audioOutput());
    delete aOutput;
}
