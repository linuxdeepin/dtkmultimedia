// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PPOCRV5_H
#define PPOCRV5_H

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

class PaddleOCRAppV5 : public Dtk::Ocr::DOcrPluginInterface
{
public:
    PaddleOCRAppV5();
    ~PaddleOCRAppV5() override;
    static inline QString name() {
        return "PPOCR_V5";
    }

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
    void resetNet();
    void initNet();

    std::vector<std::vector<std::vector<int>>> detect(const cv::Mat &src, float thresh, float boxThresh, float unclipRatio);
    std::pair<std::string, std::vector<int>> ctcDecode(const std::vector<float> &recNetOutputData, int h, int w);
    void rec(const std::vector<cv::Mat> &detectImg);
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

    QImage imageCache;
    QList<Dtk::Ocr::HardwareID> supportHardwares = {Dtk::Ocr::HardwareID::CPU_Any, Dtk::Ocr::HardwareID::GPU_Vulkan};
    QList<QPair<Dtk::Ocr::HardwareID, int>> hardwareUseInfos;
    QString languageUsed = "zh-Hans_en";
    int maxThreadsUsed = 1;
    QStringList supportLanguages = {"zh-Hans_en"};

    QList<Dtk::Ocr::TextBox> allTextBoxes;
    QString allResult;
    QVector<QString> boxesResult;
};

DOCR_END_NAMESPACE

#endif
