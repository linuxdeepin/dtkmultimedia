// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkocr.h"
#include "gtest/gtest.h"
#include <DOcr>
#include <QDateTime>
#include <QFile>
#include <QImage>
#include <cmath>
#include <cstring>

DOCR_USE_NAMESPACE

class ut_DOcrPluginInterface : public testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

public:
    static constexpr double floatPrecision = 0.01;
    DOcr *m_ocr = nullptr;
};

void ut_DOcrPluginInterface::SetUp()
{
    m_ocr = new DOcr;
}

void ut_DOcrPluginInterface::TearDown()
{
    delete m_ocr;
    m_ocr = nullptr;
}

TEST_F(ut_DOcrPluginInterface, installedPluginNames)
{
    auto names = m_ocr->installedPluginNames();
    ASSERT_EQ(names == QStringList{"ocrplugin"}, true);
}
/*
TEST_F(ut_DOcrPluginInterface, loadPlugin)
{
    ASSERT_EQ(m_ocr->loadDefaultPlugin(), true);
    ASSERT_EQ(m_ocr->loadPlugin("ocrplugin"), true);
    ASSERT_EQ(m_ocr->loadPlugin("ocrplugin"), true);
    ASSERT_EQ(m_ocr->loadPlugin("ocrplugin"), true);
    ASSERT_EQ(m_ocr->loadDefaultPlugin(), true);
    ASSERT_EQ(m_ocr->loadPlugin("ocrplugin"), true);
    ASSERT_EQ(m_ocr->loadPlugin("1233211234567"), false);
}

TEST_F(ut_DOcrPluginInterface, pluginReady)
{
    ASSERT_EQ(m_ocr->pluginReady(), false);
    m_ocr->loadDefaultPlugin();
    ASSERT_EQ(m_ocr->pluginReady(), true);
    m_ocr->loadPlugin("ocrplugin");
    ASSERT_EQ(m_ocr->pluginReady(), true);
    m_ocr->loadPlugin("ocrplugin");
    ASSERT_EQ(m_ocr->pluginReady(), true);
}

TEST_F(ut_DOcrPluginInterface, hardwareSupportList)
{
    auto list = m_ocr->hardwareSupportList();
    ASSERT_EQ(list.isEmpty(), true);

    m_ocr->loadPlugin("ocrplugin");
    list = m_ocr->hardwareSupportList();
    ASSERT_EQ((list == QList<Dtk::Ocr::HardwareID>{Dtk::Ocr::CPU_X86, Dtk::Ocr::GPU_NVIDIA}), true);
}

TEST_F(ut_DOcrPluginInterface, setUseHardware)
{
    ASSERT_EQ(m_ocr->setUseHardware({}), false);

    m_ocr->loadPlugin("ocrplugin");
    ASSERT_EQ(m_ocr->setUseHardware({{Dtk::Ocr::CPU_X86, 1}}), true);
    ASSERT_EQ(m_ocr->setUseHardware({{Dtk::Ocr::GPU_Vulkan, 0}}), false);
}

TEST_F(ut_DOcrPluginInterface, setUseMaxThreadsCount)
{
    ASSERT_EQ(m_ocr->setUseMaxThreadsCount(1), false);

    m_ocr->loadPlugin("ocrplugin");
    ASSERT_EQ(m_ocr->setUseMaxThreadsCount(2), true);
    ASSERT_EQ(m_ocr->setUseMaxThreadsCount(-1), false);
}

TEST_F(ut_DOcrPluginInterface, authKeys)
{
    ASSERT_EQ(m_ocr->authKeys() == QStringList(), true);

    m_ocr->loadPlugin("ocrplugin");
    auto keys = m_ocr->authKeys();
    ASSERT_EQ((keys == QStringList{"Product ID", "CDKEY"}), true);
}

TEST_F(ut_DOcrPluginInterface, setAuth)
{
    ASSERT_EQ(m_ocr->setAuth({"123"}), false);

    m_ocr->loadPlugin("ocrplugin");
    ASSERT_EQ(m_ocr->setAuth({"1", "2"}), false);
    ASSERT_EQ(m_ocr->setAuth({"15", "8"}), true);
}

TEST_F(ut_DOcrPluginInterface, imageFileSupportFormats)
{
    ASSERT_EQ(m_ocr->imageFileSupportFormats() == QStringList(), true);

    m_ocr->loadPlugin("ocrplugin");
    auto formats = m_ocr->imageFileSupportFormats();
    ASSERT_EQ((formats == QStringList{"jpg", "png"}), true);
}

TEST_F(ut_DOcrPluginInterface, setImageFile)
{
    ASSERT_EQ(m_ocr->setImageFile("123"), false);

    m_ocr->loadPlugin("ocrplugin");
    uint randomPixData = QDateTime::currentMSecsSinceEpoch() % 2;
    QImage image(QSize(100, 100), QImage::Format::Format_RGB888);
    image.fill(randomPixData);
    QString imageFileName("dtkocrtest.bmp");
    image.save(imageFileName, "bmp");

    bool success = m_ocr->setImageFile(imageFileName);

    QFile::remove(imageFileName);

    auto cachedImage = m_ocr->imageCached().convertToFormat(QImage::Format_RGB888);

    ASSERT_EQ(success, true);
    ASSERT_EQ(cachedImage == image, true);
}

TEST_F(ut_DOcrPluginInterface, setImage)
{
    uint randomPixData = QDateTime::currentMSecsSinceEpoch() % 2;
    QImage image(QSize(100, 100), QImage::Format::Format_RGB888);
    image.fill(randomPixData);

    ASSERT_EQ(m_ocr->setImage(image), false);
    ASSERT_EQ(m_ocr->imageCached() == image, false);
    m_ocr->loadPlugin("ocrplugin");
    ASSERT_EQ(m_ocr->setImage(image), true);
    ASSERT_EQ(m_ocr->imageCached() == image, true);
}

TEST_F(ut_DOcrPluginInterface, pluginExpandParam)
{
    ASSERT_EQ(m_ocr->pluginExpandParam("") == QString(""), true);
    ASSERT_EQ(m_ocr->setPluginExpandParam("123", "321"), false);

    m_ocr->loadPlugin("ocrplugin");
    ASSERT_EQ(m_ocr->pluginExpandParam("") == QString(""), true);
    ASSERT_EQ(m_ocr->pluginExpandParam("GPU memory usage") == QString("200MB"), true);
    ASSERT_EQ(m_ocr->setPluginExpandParam("123", "321"), false);
    ASSERT_EQ(m_ocr->setPluginExpandParam("GPU memory usage", "500"), true);
    ASSERT_EQ(m_ocr->pluginExpandParam("GPU memory usage") == QString("200MB"), false);
    ASSERT_EQ(m_ocr->pluginExpandParam("GPU memory usage") == QString("500MB"), true);
    ASSERT_EQ(m_ocr->pluginExpandParam("") == QString(""), true);
}

TEST_F(ut_DOcrPluginInterface, analyze)
{
    ASSERT_EQ(m_ocr->analyze(), false);

    m_ocr->loadPlugin("ocrplugin");
    ASSERT_EQ(m_ocr->analyze(), true);
}

TEST_F(ut_DOcrPluginInterface, breakAnalyze)
{
    ASSERT_EQ(m_ocr->breakAnalyze(), false);

    m_ocr->loadPlugin("ocrplugin");
    ASSERT_EQ(m_ocr->breakAnalyze(), false);
}

TEST_F(ut_DOcrPluginInterface, isRunning)
{
    ASSERT_EQ(m_ocr->isRunning(), false);

    m_ocr->loadPlugin("ocrplugin");
    ASSERT_EQ(m_ocr->isRunning(), false);
}

TEST_F(ut_DOcrPluginInterface, languageSupport)
{
    ASSERT_EQ(m_ocr->languageSupport() == QStringList(), true);

    m_ocr->loadPlugin("ocrplugin");
    auto languages = m_ocr->languageSupport();
    ASSERT_EQ((languages == QStringList{"zh_Ancient", "zh_Modern"}), true);
}

TEST_F(ut_DOcrPluginInterface, setLanguage)
{
    ASSERT_EQ(m_ocr->setLanguage("zh_Ancient"), false);

    m_ocr->loadPlugin("ocrplugin");
    ASSERT_EQ(m_ocr->setLanguage("zh_Ancient"), true);
    ASSERT_EQ(m_ocr->setLanguage("en"), false);
}

TEST_F(ut_DOcrPluginInterface, textBoxes)
{
    ASSERT_EQ(m_ocr->textBoxes().isEmpty(), true);

    m_ocr->loadPlugin("ocrplugin");
    auto boxes = m_ocr->textBoxes();

    ASSERT_EQ(boxes.size() == 1, true);
    ASSERT_EQ(boxes[0].points.size() == 4, true);
    ASSERT_LT(std::fabs(boxes[0].angle - 10.1), floatPrecision);
}

TEST_F(ut_DOcrPluginInterface, charBoxes)
{
    ASSERT_EQ(m_ocr->charBoxes(1).isEmpty(), true);

    m_ocr->loadPlugin("ocrplugin");
    ASSERT_EQ(m_ocr->charBoxes(1).isEmpty(), true);

    auto boxes = m_ocr->charBoxes(0);
    ASSERT_EQ(boxes.size() == 1, true);
    ASSERT_EQ(boxes[0].points.size() == 4, true);
    ASSERT_LT(std::fabs(boxes[0].angle - 1.01), floatPrecision);
}

TEST_F(ut_DOcrPluginInterface, simpleResult)
{
    ASSERT_EQ(m_ocr->simpleResult().isEmpty(), true);

    m_ocr->loadPlugin("ocrplugin");
    auto result = m_ocr->simpleResult();
    ASSERT_EQ((result == "1233211234567"), true);
}

TEST_F(ut_DOcrPluginInterface, resultFromBox)
{
    ASSERT_EQ(m_ocr->resultFromBox(1).isEmpty(), true);

    m_ocr->loadPlugin("ocrplugin");
    ASSERT_EQ(m_ocr->resultFromBox(1).isEmpty(), true);
    ASSERT_EQ(m_ocr->resultFromBox(0) == "1234567321123", true);
}
*/
