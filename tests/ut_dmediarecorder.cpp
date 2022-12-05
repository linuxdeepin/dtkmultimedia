// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkmultimedia.h"
#include "gtest/gtest.h"
#include <DMediaRecorder>
#include <DCamera>
#include <QTimer>
#include <stubext.h>

DMULTIMEDIA_USE_NAMESPACE

class ut_DMediaRecorder : public testing::Test
{
public:
    void SetUp() override
    {
        m_camera = new DCamera();
        m_mediaRecorder = new DMediaRecorder(m_camera);
    }
    void TearDown() override
    {
        delete m_camera;
        m_camera = nullptr;
        delete m_mediaRecorder;
        m_mediaRecorder = nullptr;
    }

public:
    DCamera *m_camera = nullptr;
    DMediaRecorder *m_mediaRecorder = nullptr;
};

TEST_F(ut_DMediaRecorder, isAvailable)
{
    EXPECT_EQ(true, m_mediaRecorder->isAvailable());
}

TEST_F(ut_DMediaRecorder, outputLocation)
{
    m_mediaRecorder->setOutputLocation(QUrl::fromLocalFile("/home/xxx/Videos"));
    EXPECT_EQ(QUrl::fromLocalFile("/home/xxx/Videos"), m_mediaRecorder->outputLocation());
}

TEST_F(ut_DMediaRecorder, actualLocation)
{
    m_mediaRecorder->setOutputLocation(QUrl::fromLocalFile("/home/xxx/Videos"));
    EXPECT_EQ(QUrl::fromLocalFile("/home/xxx/Videos"), m_mediaRecorder->actualLocation());
}

TEST_F(ut_DMediaRecorder, recorderState)
{
    EXPECT_EQ(QMediaRecorder::StoppedState, m_mediaRecorder->recorderState());
}

TEST_F(ut_DMediaRecorder, error)
{
    EXPECT_EQ(QMediaRecorder::NoError, m_mediaRecorder->error());
}

TEST_F(ut_DMediaRecorder, errorString)
{
    stub_ext::StubExt stub;
    typedef QString (*fptr)();
    fptr ptr = (fptr)(&QMediaRecorder::errorString);
    stub.set_lamda(ptr, [] {
        __DBG_STUB_INVOKE__
        return "ut_DMediaRecorder";
    });
    EXPECT_EQ("ut_DMediaRecorder", m_mediaRecorder->errorString());
}

TEST_F(ut_DMediaRecorder, duration)
{
    stub_ext::StubExt stub;
    typedef bool (*fptr)();
    fptr ptr = (fptr)(&DCamera::isFfmpegEnv);
    stub.set_lamda(ptr, [] {
        __DBG_STUB_INVOKE__
        return true;
    });
    EXPECT_EQ(0, m_mediaRecorder->duration());
}

TEST_F(ut_DMediaRecorder, mediaFormat)
{
    DMediaFormat mediaFormat(DMediaFormat::MPEG4);
    m_mediaRecorder->setMediaFormat(mediaFormat);
    EXPECT_EQ(mediaFormat, m_mediaRecorder->mediaFormat());
}

TEST_F(ut_DMediaRecorder, encodingMode)
{
    m_mediaRecorder->setEncodingMode(DMediaRecorder::ConstantQualityEncoding);
    EXPECT_EQ(DMediaRecorder::ConstantQualityEncoding, m_mediaRecorder->encodingMode());
}

TEST_F(ut_DMediaRecorder, quality)
{
    m_mediaRecorder->setQuality(DMediaRecorder::NormalQuality);
    EXPECT_EQ(DMediaRecorder::NormalQuality, m_mediaRecorder->quality());
}

TEST_F(ut_DMediaRecorder, videoResolution)
{
    m_mediaRecorder->setVideoResolution(QSize(800, 600));
    EXPECT_EQ(QSize(800, 600), m_mediaRecorder->videoResolution());
}

TEST_F(ut_DMediaRecorder, videoFrameRate)
{
    m_mediaRecorder->setVideoFrameRate(1.f);
    EXPECT_EQ(1.f, m_mediaRecorder->videoFrameRate());
}

TEST_F(ut_DMediaRecorder, videoBitRate)
{
    m_mediaRecorder->setVideoBitRate(10);
    EXPECT_EQ(10, m_mediaRecorder->videoBitRate());
}

TEST_F(ut_DMediaRecorder, audioBitRate)
{
    m_mediaRecorder->setAudioBitRate(10);
    EXPECT_EQ(10, m_mediaRecorder->audioBitRate());
}

TEST_F(ut_DMediaRecorder, audioChannelCount)
{
    m_mediaRecorder->setAudioChannelCount(2);
    EXPECT_EQ(2, m_mediaRecorder->audioChannelCount());
}

TEST_F(ut_DMediaRecorder, audioSampleRate)
{
    m_mediaRecorder->setAudioSampleRate(10);
    EXPECT_EQ(10, m_mediaRecorder->audioSampleRate());
}

TEST_F(ut_DMediaRecorder, metaData)
{
    DMediaMetaData data;
    data.insert(DMediaMetaData::Title, "test1");
    m_mediaRecorder->setMetaData(data);
    DMediaMetaData adddata;
    adddata.insert(DMediaMetaData::Author, "test2");
    m_mediaRecorder->addMetaData(adddata);
    EXPECT_EQ("test1", m_mediaRecorder->metaData().value(DMediaMetaData::Title));
    EXPECT_EQ("test2", m_mediaRecorder->metaData().value(DMediaMetaData::Author));
}

TEST_F(ut_DMediaRecorder, captureSession)
{
    stub_ext::StubExt stub;
    typedef DMediaCaptureSession *(*fptr)();
    fptr ptr = (fptr)(&DCamera::captureSession);
    stub.set_lamda(ptr, [] {
        __DBG_STUB_INVOKE__
        return nullptr;
    });
    EXPECT_EQ(nullptr, m_mediaRecorder->captureSession());
}

TEST_F(ut_DMediaRecorder, platformRecoder)
{
    EXPECT_EQ(nullptr, m_mediaRecorder->platformRecoder());
}

TEST_F(ut_DMediaRecorder, record)
{
    stub_ext::StubExt stub;
    typedef bool (*fptrcheck)();
    typedef void (*fptrtake)(const QString &);
    fptrcheck ptrcheck = (fptrcheck)&DCamera::isFfmpegEnv;
    stub.set_lamda(ptrcheck, [] {
        __DBG_STUB_INVOKE__
        return true;
    });
    fptrtake ptrtake = (fptrtake)&DCamera::takeVideo;
    stub.set_lamda(ptrtake, [] {
        __DBG_STUB_INVOKE__
        return;
    });
    m_mediaRecorder->record();
    EXPECT_EQ(QMediaRecorder::StoppedState, m_mediaRecorder->state());
}

TEST_F(ut_DMediaRecorder, pause)
{
    stub_ext::StubExt stub;
    typedef bool (*fptrcheck)();
    fptrcheck ptrcheck = (fptrcheck)&DCamera::isFfmpegEnv;
    stub.set_lamda(ptrcheck, [] {
        __DBG_STUB_INVOKE__
        return true;
    });
    m_mediaRecorder->pause();
    EXPECT_EQ(QMediaRecorder::StoppedState, m_mediaRecorder->state());
}

TEST_F(ut_DMediaRecorder, stop)
{
    stub_ext::StubExt stub;
    typedef bool (*fptrcheck)();
    typedef void (*fptrtake)(const QString &);
    fptrcheck ptrcheck = (fptrcheck)&DCamera::isFfmpegEnv;
    stub.set_lamda(ptrcheck, [] {
        __DBG_STUB_INVOKE__
        return true;
    });
    fptrtake ptrtake = (fptrtake)&DCamera::takeVideo;
    stub.set_lamda(ptrtake, [] {
        __DBG_STUB_INVOKE__
        return;
    });
    m_mediaRecorder->stop();
    EXPECT_EQ(QMediaRecorder::StoppedState, m_mediaRecorder->state());
}
