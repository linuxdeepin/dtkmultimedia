// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkmultimedia.h"
#include "gtest/gtest.h"
#include <DImageCapture>
#include <DCamera>
#include <DMediaCaptureSession>
#include <QCameraImageCapture>
#include <DMediaMetaData>
#include <stubext.h>

DMULTIMEDIA_USE_NAMESPACE

class ut_DImageCapture : public testing::Test
{
public:
    void SetUp() override
    {
        m_camera = new DCamera();
        m_imageCap = new DImageCapture(m_camera);
    }
    void TearDown() override
    {
        delete m_camera;
        m_camera = nullptr;
        delete m_imageCap;
        m_imageCap = nullptr;
    }

public:
    DImageCapture *m_imageCap = nullptr;
    DCamera *m_camera = nullptr;
};

TEST_F(ut_DImageCapture, isAvailable)
{
    stub_ext::StubExt stub;
    typedef bool (*fptr)();
    fptr ptr1 = (fptr)(&DCamera::isFfmpegEnv);
    stub.set_lamda(ptr1, [] {
        __DBG_STUB_INVOKE__
        return false;
    });
    fptr ptr2 = (fptr)(&QCameraImageCapture::isAvailable);
    stub.set_lamda(ptr2, [] {
        __DBG_STUB_INVOKE__
        return true;
    });
    EXPECT_EQ(true, m_imageCap->isAvailable());
}

TEST_F(ut_DImageCapture, captureSession)
{
    stub_ext::StubExt stub;
    typedef DMediaCaptureSession *(*fptr)();
    fptr ptr = (fptr)(&DCamera::captureSession);
    stub.set_lamda(ptr, [] {
        __DBG_STUB_INVOKE__
        return nullptr;
    });
    EXPECT_EQ(nullptr, m_imageCap->captureSession());
}

TEST_F(ut_DImageCapture, error)
{
    EXPECT_EQ(QCameraImageCapture::NoError, m_imageCap->error());
}

TEST_F(ut_DImageCapture, errorString)
{
    stub_ext::StubExt stub;
    typedef QString (*fptr)();
    fptr ptr = (fptr)(&QCameraImageCapture::errorString);
    stub.set_lamda(ptr, [] {
        __DBG_STUB_INVOKE__
        return "";
    });
    EXPECT_EQ("", m_imageCap->errorString());
}

TEST_F(ut_DImageCapture, isReadyForCapture)
{
    stub_ext::StubExt stub;
    typedef bool (*fptr)();
    fptr ptr = (fptr)(&QCameraImageCapture::isReadyForCapture);
    stub.set_lamda(ptr, [] {
        __DBG_STUB_INVOKE__
        return true;
    });
    EXPECT_EQ(true, m_imageCap->isReadyForCapture());
}

TEST_F(ut_DImageCapture, fileFormat)
{
    m_imageCap->setFileFormat(DImageCapture::JPEG);
    EXPECT_EQ(DImageCapture::JPEG, m_imageCap->fileFormat());
}

TEST_F(ut_DImageCapture, supportedFormats)
{
    EXPECT_EQ(QList<DImageCapture::FileFormat>(), m_imageCap->supportedFormats());
}

TEST_F(ut_DImageCapture, fileFormatName)
{
    EXPECT_EQ("JPEG", m_imageCap->fileFormatName(DImageCapture::JPEG));
}

TEST_F(ut_DImageCapture, fileFormatDescription)
{
    EXPECT_EQ("JPEG", m_imageCap->fileFormatDescription(DImageCapture::JPEG));
}

TEST_F(ut_DImageCapture, resolution)
{
    m_imageCap->setResolution(1920, 1080);
    EXPECT_EQ(QSize(1920, 1080), m_imageCap->resolution());
}

TEST_F(ut_DImageCapture, quality)
{
    m_imageCap->setQuality(DImageCapture::NormalQuality);
    EXPECT_EQ(DImageCapture::NormalQuality, m_imageCap->quality());
}

TEST_F(ut_DImageCapture, metaData)
{
    DMediaMetaData data;
    data.insert(DMediaMetaData::Title, "test1");
    m_imageCap->setMetaData(data);
    DMediaMetaData adddata;
    adddata.insert(DMediaMetaData::Author, "test2");
    m_imageCap->addMetaData(adddata);
    EXPECT_EQ("test1", m_imageCap->metaData().value(DMediaMetaData::Title));
    EXPECT_EQ("test2", m_imageCap->metaData().value(DMediaMetaData::Author));
}

TEST_F(ut_DImageCapture, captureToFile)
{
    stub_ext::StubExt stub;
    typedef bool (*fptr)();
    fptr ptr = (fptr)(&DCamera::isFfmpegEnv);
    stub.set_lamda(ptr, [] {
        __DBG_STUB_INVOKE__
        return true;
    });
    typedef void (*fptrtake)(const QString &);
    fptrtake ptrtake = (fptrtake)(&DCamera::takeOne);
    stub.set_lamda(ptrtake, [] {
        __DBG_STUB_INVOKE__
        return;
    });
    EXPECT_EQ(0, m_imageCap->captureToFile("/home/xxx/filename"));
}

TEST_F(ut_DImageCapture, capture)
{
    stub_ext::StubExt stub;
    typedef bool (*fptr)();
    fptr ptr = (fptr)(&DCamera::isFfmpegEnv);
    stub.set_lamda(ptr, [] {
        __DBG_STUB_INVOKE__
        return true;
    });
    typedef void (*fptrtake)(const QString &);
    fptrtake ptrtake = (fptrtake)(&DCamera::takeOne);
    stub.set_lamda(ptrtake, [] {
        __DBG_STUB_INVOKE__
        return;
    });
    EXPECT_EQ(0, m_imageCap->capture("/home/xxx/filename"));
}
