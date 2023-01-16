// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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

class ut_DOcr : public testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

    QImage simpleTestImage();

public:
    static constexpr double floatPrecision = 0.01;
    DOcr *m_ocr = nullptr;
};

void ut_DOcr::SetUp()
{
    m_ocr = new DOcr;
    m_ocr->loadDefaultPlugin();
}

void ut_DOcr::TearDown()
{
    delete m_ocr;
    m_ocr = nullptr;
}

QImage ut_DOcr::simpleTestImage()
{
    constexpr int before = 113;
    constexpr int after = 788;
    const unsigned char monoImageData[] = {
        0xe1, 0xff, 0x7e, 0x37, 0xa0, 0xff, 0x86, 0x0c, 0x10, 0xe1, 0xdd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0xff,
        0x7d, 0xdb, 0xaf, 0xff, 0xb2, 0xfd, 0xf7, 0x7b, 0xcd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0xff, 0x7d, 0xdb,
        0xaf, 0xff, 0xba, 0xfd, 0xf7, 0x7b, 0xcd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0xff, 0x7d, 0xdb, 0x61, 0xff,
        0xba, 0x1c, 0x37, 0x7b, 0xd5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0xff, 0x79, 0xdd, 0x6f, 0xff, 0xba, 0xfd,
        0xf0, 0xfb, 0xd5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0xff, 0x7d, 0xdd, 0x6f, 0xff, 0xba, 0xfd, 0xf7, 0xfb,
        0xd5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0xff, 0x7d, 0xdc, 0xef, 0xff, 0xb2, 0xfd, 0xf7, 0xfb, 0xd9, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xe1, 0xff, 0x06, 0x3e, 0xe0, 0xff, 0x86, 0x0c, 0x17, 0xe1, 0xd9};

    unsigned char imgData[before + after + sizeof(monoImageData)];
    std::memset(imgData, 0xff, sizeof(imgData));
    std::memcpy(imgData + before, monoImageData, sizeof(monoImageData));

    QImage image(imgData, 128, 64, QImage::Format_Mono);
    return image.copy();
}

TEST_F(ut_DOcr, pluginReady)
{
    ASSERT_EQ(m_ocr->pluginReady(), true);
}

TEST_F(ut_DOcr, hardwareSupportList)
{
    decltype(m_ocr->hardwareSupportList()) list{Dtk::Ocr::HardwareID::CPU_Any, Dtk::Ocr::HardwareID::GPU_Vulkan};
    ASSERT_EQ(m_ocr->hardwareSupportList() == list, true);
}

TEST_F(ut_DOcr, setUseHardware)
{
    bool success = m_ocr->setUseHardware({{Dtk::Ocr::HardwareID::CPU_Any, 0}});
    ASSERT_EQ(success, true);

    success = m_ocr->setUseHardware({{Dtk::Ocr::HardwareID::GPU_Loongson, 0}});
    ASSERT_EQ(success, false);
}

TEST_F(ut_DOcr, setUseMaxThreadsCount)
{
    ASSERT_EQ(m_ocr->setUseMaxThreadsCount(1), true);
}

TEST_F(ut_DOcr, authKeys)
{
    decltype(m_ocr->authKeys()) keys{};
    ASSERT_EQ(m_ocr->authKeys() == keys, true);
}

TEST_F(ut_DOcr, setAuth)
{
    ASSERT_EQ(m_ocr->setAuth({}), false);
}

TEST_F(ut_DOcr, imageFileSupportFormats)
{
    decltype(m_ocr->imageFileSupportFormats()) formats{"BMP", "JPEG", "PNG", "PBM", "PGM", "PPM"};
    ASSERT_EQ(m_ocr->imageFileSupportFormats() == formats, true);
}

TEST_F(ut_DOcr, setImageFile)
{
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

TEST_F(ut_DOcr, setImage)
{
    uint randomPixData = QDateTime::currentMSecsSinceEpoch() % 2;
    QImage image(QSize(100, 100), QImage::Format::Format_RGB888);
    image.fill(randomPixData);

    ASSERT_EQ(m_ocr->setImage(image), true);
    ASSERT_EQ(m_ocr->imageCached() == image, true);
}

TEST_F(ut_DOcr, pluginExpandParam)
{
    ASSERT_EQ(m_ocr->pluginExpandParam("") == QString(""), true);
}

TEST_F(ut_DOcr, setPluginExpandParam)
{
    ASSERT_EQ(m_ocr->setPluginExpandParam("key", "12345"), false);
}

TEST_F(ut_DOcr, analyze)
{
    ASSERT_EQ(m_ocr->analyze(), false);
}

TEST_F(ut_DOcr, breakAnalyze)
{
    ASSERT_EQ(m_ocr->breakAnalyze(), false);
}

TEST_F(ut_DOcr, isRunning)
{
    ASSERT_EQ(m_ocr->isRunning(), false);
}

TEST_F(ut_DOcr, languageSupport)
{
    decltype(m_ocr->languageSupport()) languages{"zh-Hans_en", "zh-Hant_en", "en"};
    ASSERT_EQ(m_ocr->languageSupport() == languages, true);
}

TEST_F(ut_DOcr, setLanguage)
{
    ASSERT_EQ(m_ocr->setLanguage("f983hdojh;o"), false);
    ASSERT_EQ(m_ocr->setLanguage("zh-Hans_en"), true);
}

TEST_F(ut_DOcr, textBoxes)
{
    auto testImage = simpleTestImage();
    m_ocr->setImage(testImage);
    m_ocr->analyze();

    auto boxes = m_ocr->textBoxes();
    ASSERT_EQ(boxes.size(), 1);

    auto box = boxes[0];
    ASSERT_EQ(box.points.size(), 4);
    ASSERT_EQ(box.angle, 0);

    ASSERT_LT(std::fabs(box.points[0].y() - box.points[1].y()), floatPrecision);
    ASSERT_LT(std::fabs(box.points[1].x() - box.points[2].x()), floatPrecision);
    ASSERT_LT(std::fabs(box.points[2].y() - box.points[3].y()), floatPrecision);
    ASSERT_LT(std::fabs(box.points[3].x() - box.points[0].x()), floatPrecision);
}

TEST_F(ut_DOcr, charBoxes)
{
    auto testImage = simpleTestImage();
    m_ocr->setImage(testImage);
    m_ocr->analyze();

    auto boxes = m_ocr->charBoxes(0);

    ASSERT_GT(boxes.size(), 2);

    for (int i = 1; i != boxes.size(); ++i) {
        ASSERT_EQ(boxes[i].points.size(), 4);
        ASSERT_LT(std::fabs(boxes[i].points[0].y() - boxes[i - 1].points[0].y()), floatPrecision);
    }
}

TEST_F(ut_DOcr, simpleResult)
{
    auto testImage = simpleTestImage();
    m_ocr->setImage(testImage);
    m_ocr->analyze();

    auto result = m_ocr->simpleResult();
    QString correctResult("I LOVE DEEPIN\n");

    ASSERT_EQ(result == correctResult, true);
}

TEST_F(ut_DOcr, resultFromBox)
{
    auto testImage = simpleTestImage();
    m_ocr->setImage(testImage);
    m_ocr->analyze();

    ASSERT_EQ(m_ocr->textBoxes().size(), 1);
    auto result = m_ocr->resultFromBox(0);
    QString correctResult("I LOVE DEEPIN");

    ASSERT_EQ(result == correctResult, true);
}
