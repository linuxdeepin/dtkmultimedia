// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PPOCR_H
#define PPOCR_H

#include "docrplugininterface.h"
#include "postprocess_op.h"
#include "utility.h"

#include <opencv2/opencv.hpp>

#include <QImage>

#include <atomic>
#include <utility>

namespace ncnn {
class Net;
}

DOCR_BEGIN_NAMESPACE

class PaddleOCRApp : public Dtk::Ocr::DOcrPluginInterface
{
public:
    PaddleOCRApp();
    ~PaddleOCRApp() override;

    QList<Dtk::Ocr::HardwareID> hardwareSupportList() override;
    bool setUseHardware(const QList<QPair<Dtk::Ocr::HardwareID, int>> &hardwareUsed) override;
    bool setUseMaxThreadsCount(int n) override;
    QStringList imageFileSupportFormats() const override;
    bool setImageFile(const QString &filePath) override;
    bool setImage(const QImage &image) override;
    QImage imageCached() const override;
    bool analyze() override;
    bool breakAnalyze() override;
    bool isRunning() const override;
    QStringList languageSupport() const override;
    bool setLanguage(const QString &language) override;
    QList<Dtk::Ocr::TextBox> textBoxes() const override;
    QList<Dtk::Ocr::TextBox> charBoxes(int index) const override;
    QString simpleResult() const override;
    QString resultFromBox(int index) const override;

private:
    QString currentPath;
    std::atomic_bool needReset = false;
    std::atomic_bool needBreak = false;
    std::atomic_bool inRunning = false;
    ncnn::Net *detNet = nullptr;
    ncnn::Net *recNet = nullptr;
    QStringList keys;
    PaddleOCR::PostProcessor postProcessor;
    PaddleOCR::Utility utilityTool;
    void resetNet();
    void initNet();

    std::vector<std::vector<std::vector<int>>> detect(const cv::Mat &src, float thresh, float boxThresh, float unclipRatio);
    std::pair<std::string, std::vector<int>> ctcDecode(const std::vector<float> &recNetOutputData, int h, int w);
    void rec(const std::vector<cv::Mat> &detectImg);
    QList<Dtk::Ocr::TextBox> lengthToBox(const std::vector<int> &lengths, QPointF basePoint, float rectHeight, float ratio);

    QImage imageCache;
    QStringList supportLanguages = {"zh-Hans_en", "zh-Hant_en", "en"};
    QList<Dtk::Ocr::HardwareID> supportHardwares = {Dtk::Ocr::HardwareID::CPU_Any, Dtk::Ocr::HardwareID::GPU_Vulkan};
    QList<QPair<Dtk::Ocr::HardwareID, int>> hardwareUseInfos;
    QString languageUsed = "zh-Hans_en";
    int maxThreadsUsed = 1;

    QList<Dtk::Ocr::TextBox> allTextBoxes;
    QVector<QList<Dtk::Ocr::TextBox>> allCharBoxes;
    QString allResult;
    QVector<QString> boxesResult;
};

DOCR_END_NAMESPACE

#endif
