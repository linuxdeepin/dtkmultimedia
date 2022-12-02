// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkmultimedia.h"
#include "gtest/gtest.h"
#include <DCamera>
#include <QVideoWidget>
#include <QThread>
#include <stubext.h>
extern "C" {
#include <linux/videodev2.h>
int start_encoder_thread();
void set_video_path(const char *path);
void set_video_name(const char *name);
int camInit(const char *devicename);
struct v4l2_device_list_t *get_device_list();
void v4l2core_prepare_new_resolution(struct v4l2_dev_t *vd, int new_width, int new_height);
void request_format_update(int bstatus);
v4l2_dev_t *get_v4l2_device_handler();
}

DMULTIMEDIA_USE_NAMESPACE

class ut_DCamera : public testing::Test
{
public:
    void SetUp() override
    {
        m_camera = new DCamera();
    }
    void TearDown() override
    {
        delete m_camera;
        m_camera = nullptr;
    }

public:
    DCamera *m_camera = nullptr;
};

TEST_F(ut_DCamera, takeOne)
{
    m_camera->takeOne("");
    EXPECT_EQ(0, m_camera->checkCamera());
}

TEST_F(ut_DCamera, takeVideo)
{
    stub_ext::StubExt stub;
    typedef int (*fptr)();
    typedef void (*fsetptr)(const char *);
    fptr ptr = (fptr)&start_encoder_thread;
    stub.set_lamda(ptr, [] {
        __DBG_STUB_INVOKE__
        return 0;
    });
    fsetptr pathaddr = (fsetptr)&set_video_path;
    stub.set_lamda(pathaddr, [] {
        __DBG_STUB_INVOKE__
        return;
    });
    fsetptr nameaddr = (fsetptr)&set_video_name;
    stub.set_lamda(nameaddr, [] {
        __DBG_STUB_INVOKE__
        return;
    });
    m_camera->takeVideo("");
    EXPECT_EQ(0, m_camera->checkCamera());
}

TEST_F(ut_DCamera, start)
{
    stub_ext::StubExt stub;
    typedef int (*fptr)(const char *);
    fptr addr = (fptr)&camInit;
    stub.set_lamda(addr, [] {
        __DBG_STUB_INVOKE__
        return (-10);
    });
    typedef QStringList (*fdevptr)();
    fdevptr devaddr = (fdevptr)&DCamera::devList;
    stub.set_lamda(devaddr, [] {
        __DBG_STUB_INVOKE__
        return QStringList() << "/dev/video0";
    });
    m_camera->start();
    EXPECT_EQ(1, m_camera->checkCamera());
}

TEST_F(ut_DCamera, stop)
{
    m_camera->stop();
    EXPECT_EQ(1, m_camera->checkCamera());
}

TEST_F(ut_DCamera, isFfmpegEnv)
{
    EXPECT_EQ(true, m_camera->isFfmpegEnv());
}

TEST_F(ut_DCamera, isWaylandEnv)
{
    EXPECT_EQ(false, m_camera->isWaylandEnv());
}

TEST_F(ut_DCamera, setViewfinder)
{
    QVideoWidget *viewfinder = new QVideoWidget(nullptr);
    m_camera->setViewfinder(viewfinder);
    EXPECT_EQ(1, m_camera->checkCamera());
    delete viewfinder;
}

TEST_F(ut_DCamera, captureSession)
{
    EXPECT_NE(nullptr, m_camera->captureSession());
}

TEST_F(ut_DCamera, devList)
{
    stub_ext::StubExt stub;
    typedef v4l2_device_list_t *(*fptr)();
    fptr addr = (fptr)&get_device_list;
    stub.set_lamda(addr, [] {
        __DBG_STUB_INVOKE__
        return nullptr;
    });
    EXPECT_EQ(QStringList(), m_camera->devList());
}

TEST_F(ut_DCamera, openDev)
{
    stub_ext::StubExt stub;
    typedef int (*fptr)(const char *);
    fptr addr = (fptr)&camInit;
    stub.set_lamda(addr, [] {
        __DBG_STUB_INVOKE__
        return (-10);
    });
    m_camera->openDev("");
    EXPECT_EQ(1, m_camera->checkCamera());
}

TEST_F(ut_DCamera, closeDev)
{
    m_camera->closeDev();
    EXPECT_EQ(1, m_camera->checkCamera());
}

TEST_F(ut_DCamera, yuvbuffer)
{
    stub_ext::StubExt stub;
    typedef bool (*fptr)();
    fptr ptr = (fptr)&QThread::isRunning;
    stub.set_lamda(ptr, [] {
        __DBG_STUB_INVOKE__
        return true;
    });
    uint width = 0, height = 0;
    EXPECT_EQ(QByteArray(), m_camera->yuvbuffer(width, height));
}

TEST_F(ut_DCamera, resolutionSettings)
{
    stub_ext::StubExt stub;
    typedef void (*fptr)(v4l2_dev_t *, int, int);
    fptr ptr = (fptr)&v4l2core_prepare_new_resolution;
    stub.set_lamda(ptr, [] {
        __DBG_STUB_INVOKE__
        return;
    });
    typedef void (*fupptr)(int);
    fupptr upptr = (fupptr)&request_format_update;
    stub.set_lamda(ptr, [] {
        __DBG_STUB_INVOKE__
        return;
    });
    QSize size(0, 0);
    m_camera->resolutionSettings(size);
    EXPECT_EQ(1, m_camera->checkCamera());
}

TEST_F(ut_DCamera, resolutions)
{
    EXPECT_EQ(QList<QSize>(), m_camera->resolutions());
}

TEST_F(ut_DCamera, setCameraCollectionFormat)
{
    m_camera->setCameraCollectionFormat(V4L2_PIX_FMT_MJPEG);
    EXPECT_EQ(1, m_camera->checkCamera());
}

TEST_F(ut_DCamera, setFilter)
{
    // warm(暖色)、cold(冷色)、black(黑白)、gray(灰度)、brown(褐色)、print(冲印)、classic(古典)、blues(蓝调)
    m_camera->setFilter("warm");
    EXPECT_EQ(1, m_camera->checkCamera());
}

TEST_F(ut_DCamera, setExposure)
{
    m_camera->setExposure(0);
    EXPECT_EQ(0, m_camera->exposure()->exposureCompensation());
}

TEST_F(ut_DCamera, supportedViewfinderPixelFormats)
{
    stub_ext::StubExt stub;
    typedef bool (*fptr)();
    fptr ptr = (fptr)&QThread::isRunning;
    stub.set_lamda(ptr, [] {
        __DBG_STUB_INVOKE__
        return true;
    });
    //v4l2_dev_t *get_v4l2_device_handler();
    typedef v4l2_dev_t *(*fgetptr)();
    fgetptr getptr = (fgetptr)&get_v4l2_device_handler;
    stub.set_lamda(getptr, [] {
        __DBG_STUB_INVOKE__
        return nullptr;
    });
    QList<uint32_t> sList = m_camera->supportedViewfinderPixelFormats();
    EXPECT_EQ(QList<uint32_t>(), sList);
}
