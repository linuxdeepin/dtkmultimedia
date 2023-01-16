// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkmultimedia.h"
#include "gtest/gtest.h"
#include <DMultiOperateInterface>
#include <QIODevice>
#include <QProcess>
#include <QSize>
#include <QFileInfo>
#include <QDir>

DMULTIMEDIA_USE_NAMESPACE

class ut_DMultiOperateInterface : public testing::Test
{
public:
    void SetUp() override
    {
        m_operateInt = new DMultiOperateInterface;
    }
    void TearDown() override
    {
        delete m_operateInt;
        m_operateInt = nullptr;
    }

public:
    DMultiOperateInterface *m_operateInt = nullptr;
};

TEST_F(ut_DMultiOperateInterface, showVideoMuxList)
{
    QStringList sList = m_operateInt->showVideoMuxList(DMultiOperateInterface::h264);
    EXPECT_EQ(true, sList.contains("mp4"));
}

TEST_F(ut_DMultiOperateInterface, showAudioMuxList)
{
    QStringList sList = m_operateInt->showAudioMuxList(DMultiOperateInterface::mp3);
    EXPECT_EQ(true, sList.contains("mp4"));
}

TEST_F(ut_DMultiOperateInterface, nameOfMuxer)
{
    QString sName = m_operateInt->nameOfMuxer(DMultiOperateInterface::mkv);
    EXPECT_EQ("mkv", sName);
}

TEST_F(ut_DMultiOperateInterface, nameOfVideoCodec)
{
    QString sName = m_operateInt->nameOfVideoCodec(DMultiOperateInterface::h264);
    EXPECT_EQ("h264", sName);
}

TEST_F(ut_DMultiOperateInterface, nameOfAudioCodec)
{
    QString sName = m_operateInt->nameOfAudioCodec(DMultiOperateInterface::aac);
    EXPECT_EQ("aac", sName);
}

TEST_F(ut_DMultiOperateInterface, setOutFilePath)
{
    m_operateInt->setOutFilePath("/home/data");
    EXPECT_EQ("/home/data", m_operateInt->outFilePath());
}

TEST_F(ut_DMultiOperateInterface, showFileInfo)
{
    EXPECT_EQ("", m_operateInt->showFileInfo("/home/test.mp4"));
}

TEST_F(ut_DMultiOperateInterface, showVideoInfo)
{
    EXPECT_EQ("", m_operateInt->showVideoInfo("/home/test.mp4"));
}

TEST_F(ut_DMultiOperateInterface, showAudioInfo)
{
    EXPECT_EQ("", m_operateInt->showAudioInfo("/home/test.mp4"));
}

TEST_F(ut_DMultiOperateInterface, showSubtitleInfo)
{
    EXPECT_EQ("", m_operateInt->showSubtitleInfo("/home/test.mp4"));
}

TEST_F(ut_DMultiOperateInterface, videoConvert)
{
    QString strDesc;
    m_operateInt->setOutFilePath("/home/data");
    EXPECT_EQ(false, m_operateInt->videoConvert("/home/test.mp4", DMultiOperateInterface::vp8, DMultiOperateInterface::webm, "test", strDesc));
    QFileInfo info;
    info.setFile(QDir("/home/data"), "test."+m_operateInt->nameOfMuxer(DMultiOperateInterface::webm));
    if(info.exists()) {
        QFile::remove(info.absoluteFilePath());
    }
}

TEST_F(ut_DMultiOperateInterface, audioConvert)
{
    QString strDesc;
    m_operateInt->setOutFilePath("/home/data");
    QString sOutFilename = "test";
    EXPECT_EQ(false, m_operateInt->audioConvert("/home/test.mp4", DMultiOperateInterface::aac, DMultiOperateInterface::webm, "test", strDesc));
    QFileInfo info;
    info.setFile(QDir("/home/data"), "test."+m_operateInt->nameOfMuxer(DMultiOperateInterface::webm));
    if(info.exists()) {
        QFile::remove(info.absoluteFilePath());
    }
}

TEST_F(ut_DMultiOperateInterface, muxerConvert)
{
    QString strDesc;
    m_operateInt->setOutFilePath("/home/data");
    EXPECT_EQ(false, m_operateInt->muxerConvert("/home/test.mp4",DMultiOperateInterface::webm, "test", strDesc));
    QFileInfo info;
    info.setFile(QDir("/home/data"), "test."+m_operateInt->nameOfMuxer(DMultiOperateInterface::webm));
    if(info.exists()) {
        QFile::remove(info.absoluteFilePath());
    }
}

TEST_F(ut_DMultiOperateInterface, adjustRate)
{
    QString strDesc;
    m_operateInt->setOutFilePath("/home/data");
    EXPECT_EQ(false, m_operateInt->adjustRate("/home/test.mp4", 964, 3856, 2000, DMultiOperateInterface::mp4, "test", strDesc));
    QFileInfo info;
    info.setFile(QDir("/home/data"), "test."+m_operateInt->nameOfMuxer(DMultiOperateInterface::mp4));
    if(info.exists()) {
        QFile::remove(info.absoluteFilePath());
    }
}

TEST_F(ut_DMultiOperateInterface, changeResolution)
{
    QString strDesc;
    m_operateInt->setOutFilePath("/home/data");
    EXPECT_EQ(false, m_operateInt->changeResolution("/home/test.mp4", 800, 600, DMultiOperateInterface::mp4, "test", strDesc));
    QFileInfo info;
    info.setFile(QDir("/home/data"), "test."+m_operateInt->nameOfMuxer(DMultiOperateInterface::mp4));
    if(info.exists()) {
        QFile::remove(info.absoluteFilePath());
    }
}

TEST_F(ut_DMultiOperateInterface, changeResolutionSize)
{
    QString strDesc;
    m_operateInt->setOutFilePath("/home/data");
    EXPECT_EQ(false, m_operateInt->changeResolution("/home/test.mp4", QSize(800, 600), DMultiOperateInterface::mp4, "test", strDesc));
    QFileInfo info;
    info.setFile(QDir("/home/data"), "test."+m_operateInt->nameOfMuxer(DMultiOperateInterface::mp4));
    if(info.exists()) {
        QFile::remove(info.absoluteFilePath());
    }
}

TEST_F(ut_DMultiOperateInterface, audioStreamFile)
{
    QString strDesc;
    m_operateInt->setOutFilePath("/home/data");
    EXPECT_EQ(false, m_operateInt->audioStreamFile("/home/test.mp4",  DMultiOperateInterface::mp4, "test", strDesc));
    QFileInfo info;
    info.setFile(QDir("/home/data"), "test."+m_operateInt->nameOfMuxer(DMultiOperateInterface::mp4));
    if(info.exists()) {
        QFile::remove(info.absoluteFilePath());
    }
}

TEST_F(ut_DMultiOperateInterface, videoStreamFile)
{
    QString strDesc;
    m_operateInt->setOutFilePath("/home/data");
    EXPECT_EQ(false, m_operateInt->videoStreamFile("/home/test.mp4",  DMultiOperateInterface::mp4, "test", strDesc));
    QFileInfo info;
    info.setFile(QDir("/home/data"), "test."+m_operateInt->nameOfMuxer(DMultiOperateInterface::mp4));
    if(info.exists()) {
        QFile::remove(info.absoluteFilePath());
    }
}

TEST_F(ut_DMultiOperateInterface, addAudioforVideo)
{
    QString strDesc;
    m_operateInt->setOutFilePath("/home/data");
    EXPECT_EQ(false, m_operateInt->addAudioforVideo("/home/testVideo.mp4",  "/home/testAudio.mp4",  DMultiOperateInterface::mp4, "test", strDesc));
    QFileInfo info;
    info.setFile(QDir("/home/data"), "test."+m_operateInt->nameOfMuxer(DMultiOperateInterface::mp4));
    if(info.exists()) {
        QFile::remove(info.absoluteFilePath());
    }
}

TEST_F(ut_DMultiOperateInterface, cutVideoToDuration)
{
    QString strDesc, startTime = "10", durationTime = "10";
    m_operateInt->setOutFilePath("/home/data");
    EXPECT_EQ(false, m_operateInt->cutVideoToDuration("/home/testVideo.mp4", startTime, durationTime, DMultiOperateInterface::mp4, "test", strDesc));
    QFileInfo info;
    info.setFile(QDir("/home/data"), "test."+m_operateInt->nameOfMuxer(DMultiOperateInterface::mp4));
    if(info.exists()) {
        QFile::remove(info.absoluteFilePath());
    }
}

TEST_F(ut_DMultiOperateInterface, cutVideoToEnd)
{
    QString strDesc, startTime = "0", EndTime = "10";
    m_operateInt->setOutFilePath("/home/data");
    EXPECT_EQ(false, m_operateInt->cutVideoToEnd("/home/testVideo.mp4", startTime, EndTime, DMultiOperateInterface::mp4, "test", strDesc));
    QFileInfo info;
    info.setFile(QDir("/home/data"), "test."+m_operateInt->nameOfMuxer(DMultiOperateInterface::mp4));
    if(info.exists()) {
        QFile::remove(info.absoluteFilePath());
    }
}
