// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ppocrv5.h"

#include <ncnn/layer.h>
#include <ncnn/net.h>

#include <QDir>
#include <QProcessEnvironment>
#include <QThread>
#include <QtDebug>
#include <QSet>

DOCR_BEGIN_NAMESPACE

PaddleOCRAppV5::PaddleOCRAppV5()
{
#ifndef QT_NO_DEBUG
    currentPath = QString("%0/v5/").arg(OCR_MODEL_DIR);
#else
    QString fullPath;
    auto xdgDirs = QProcessEnvironment::systemEnvironment().value("XDG_DATA_DIRS").split(':');
    for (auto &xdgDir : xdgDirs) {
        fullPath = QString("%0/%1/v5/").arg(xdgDir).arg(OCR_MODEL_DIR);
        if (QFile::exists(fullPath)) {
            currentPath = fullPath;
            break;
        }
    }

    if (currentPath.isEmpty()) {
        fullPath = QString("/usr/share/%0/v5/").arg(OCR_MODEL_DIR);
        if (QFile::exists(fullPath)) {
            currentPath = fullPath;
        } else {
            qWarning() << "cannot find default model";
        }
    }
#endif
}

PaddleOCRAppV5::~PaddleOCRAppV5()
{
    resetNet();
}

void PaddleOCRAppV5::resetNet()
{
    delete detNet;
    delete recNet;

    detNet = nullptr;
    recNet = nullptr;

    keys.clear();

    needReset = false;
}

void PaddleOCRAppV5::initNet()
{
    if (currentPath.isEmpty()) {
        qWarning() << "cannot find default model";
        qWarning() << "model load failed";
    }

    if (detNet != nullptr && recNet != nullptr && !keys.empty()) {
        return;
    }

    QSet<int> gpuCanUseSet;
    for (auto &eachPair : hardwareUseInfos) {
        if (eachPair.first == Dtk::Ocr::HardwareID::GPU_Vulkan) {
            gpuCanUseSet.insert(eachPair.second);
        }
    }

#if BUILD_Qt6
    int gpuCount = ncnn::get_gpu_count();
    QList<int> gpuCanUse;
    for (QSet<int>::const_iterator it = gpuCanUseSet.constBegin(); it != gpuCanUseSet.constEnd(); ++it) {
        if (*it > 0 && *it <= gpuCount) {
            gpuCanUse.append(*it);
        }
    }
#else
    QList<int> gpuCanUse = gpuCanUseSet.toList();
    if (!gpuCanUse.isEmpty()) {
        int gpuCount = ncnn::get_gpu_count();
        for (int i = gpuCanUse.size() - 1; i >= 0; --i) {
            if (gpuCanUse[i] < 0 || gpuCanUse[i] >= gpuCount) {
                gpuCanUse.removeAt(i);
            }
        }
    }
#endif

    int maxThreads = QThread::idealThreadCount();
    if (maxThreads <= 0) {
        maxThreads = 1;
    }
    if (maxThreadsUsed > maxThreads) {
        maxThreadsUsed = maxThreads;
    }

    const QString dictSuffix = ".txt";

    ncnn::Option option;
    option.lightmode = true;
    option.use_int8_inference = false;
    option.num_threads = 1;

    if (detNet == nullptr) {
        detNet = new ncnn::Net;
        detNet->opt = option;
        detNet->opt.num_threads = static_cast<int>(maxThreadsUsed);
        detNet->load_param((currentPath + "det.ncnn.param").toStdString().c_str());
        detNet->load_model((currentPath + "det.ncnn.bin").toStdString().c_str());
    }

    if (recNet == nullptr) {
        recNet = new ncnn::Net;
        recNet->opt = option;

#if !defined(_loongarch) && !defined(__loongarch__) && !defined(__loongarch64)
        if (!gpuCanUse.empty()) {
            recNet->set_vulkan_device(gpuCanUse[0]);
            recNet->opt.use_vulkan_compute = true;
        }
#endif
        recNet->load_param((currentPath + "rec.ncnn.param").toStdString().c_str());
        recNet->load_model((currentPath + "rec.ncnn.bin").toStdString().c_str());
    }

    if (keys.empty()) {
        auto dictFilePath = currentPath + languageUsed + dictSuffix;
        QFile dictFile(dictFilePath);
        dictFile.open(QIODevice::ReadOnly);
        keys.push_back("#");
        while (1) {
            auto data = dictFile.readLine();
            if (data.isEmpty()) {
                break;
            } else {
                if (data.endsWith('\n')) {
                    data.remove(data.size() - 1, 1);
                }
                keys.push_back(data);
            }
        }
        keys.push_back(" ");
    }
}

std::vector<std::vector<std::vector<int>>>
PaddleOCRAppV5::detect(const cv::Mat &src, float thresh, float boxThresh, float unclipRatio)
{
    int resizeH = src.rows;
    int resizeW = src.cols;

    float ratio2 = 1.f;
    if (std::min(resizeW, resizeH) < 64) {
        if (resizeH > resizeW) {
            ratio2 = 64.0f / resizeW;
        } else {
            ratio2 = 64.0f / resizeH;
        }
    }
    resizeH = int(resizeH * ratio2);
    resizeW = int(resizeW * ratio2);

    resizeH = int(round(float(resizeH) / 32) * 32);
    resizeW = int(round(float(resizeW) / 32) * 32);

    float ratio_h = float(resizeH) / float(src.rows);
    float ratio_w = float(resizeW) / float(src.cols);
    cv::Mat resize_img;
    cv::resize(src, resize_img, cv::Size(resizeW, resizeH));

    ncnn::Mat in_pad = ncnn::Mat::from_pixels(resize_img.data, ncnn::Mat::PIXEL_BGR, resizeW, resizeH); //CHW,BGR
    const float meanValues[3] = {0.485f * 255, 0.456f * 255, 0.406f * 255};
    const float normValues[3] = {1.0f / 0.229f / 255.0f, 1.0f / 0.224f / 255.0f, 1.0f / 0.225f / 255.0f};

    in_pad.substract_mean_normalize(meanValues, normValues);
    ncnn::Extractor extractor = detNet->create_extractor();
    extractor.input(0, in_pad);
    ncnn::Mat out;
    extractor.extract(detNet->output_indexes()[0], out);

    if (needBreak) {
        return std::vector<std::vector<std::vector<int>>>();
    }

    std::vector<float> pred;
    std::vector<unsigned char> cbuf;

    for (int q = 0; q < out.c; q++) {
        const float *ptr = out.channel(q);
        for (int y = 0; y < out.h; y++) {
            for (int x = 0; x < out.w; x++) {
                pred.push_back(ptr[x]);
                cbuf.push_back(static_cast<unsigned char>(ptr[x] * 255.0f));
            }
            ptr += out.w;
        }
    }

    if (needBreak) {
        return std::vector<std::vector<std::vector<int>>>();
    }

    cv::Mat cbuf_map(out.h, out.w, CV_8UC1, static_cast<unsigned char *>(cbuf.data()));
    cv::Mat pred_map(out.h, out.w, CV_32F, static_cast<float *>(pred.data()));

    const float threshold = thresh * 255.0f;
    cv::Mat mask;
    {
        cv::Mat bit_map;
        cv::threshold(cbuf_map, bit_map, static_cast<double>(threshold), 255, cv::THRESH_BINARY);
        mask = bit_map;
    }

    if (needBreak) {
        return std::vector<std::vector<std::vector<int>>>();
    }

    auto result = postProcessor.BoxesFromBitmap(pred_map, mask, boxThresh, unclipRatio, false, true);

    if (needBreak) {
        return std::vector<std::vector<std::vector<int>>>();
    }

    result = postProcessor.FilterTagDetRes(result, ratio_h, ratio_w, src);
    return result;
}

std::pair<std::string, std::vector<int>> PaddleOCRAppV5::ctcDecode(const std::vector<float> &recNetOutputData, int h, int w)
{
    std::string text;
    std::vector<int> baseSize;
    int currentSize = 0;
    int status = 0;
    size_t lastIndex = 0;

    for (int i = 0; i < h; i++) {
        size_t maxIndex = 0;
        maxIndex = utilityTool.argmax(recNetOutputData.begin() + i * w, recNetOutputData.begin() + i * w + w);
        ++currentSize;
        if (maxIndex > 0 && (i == 0 || maxIndex != lastIndex)) {
            text.append(keys[static_cast<int>(maxIndex)].toStdString());

            if (status == 0) {
                status = 1;
            } else {
                baseSize.push_back(currentSize - 1);
                currentSize = 1;
            }
        }
        lastIndex = maxIndex;
    }
    baseSize.push_back(currentSize);
    return std::make_pair(text, baseSize);
}

void PaddleOCRAppV5::rec(const std::vector<cv::Mat> &detectImg)
{
    size_t size = detectImg.size();
    allResult.clear();
    std::vector<std::string> allResultVec(detectImg.size());
    boxesResult.resize(size);

#pragma omp parallel for num_threads(maxThreadsUsed)
    for (size_t i = 0; i < size; ++i) {
        if (needBreak) {
            continue;
        }

        cv::Mat stdMat;
        {
            // input size [1,3,48, ?] 320 <= ? <= 3200
            const int recImgH = 48;
            const int recImgW = 320;
            float max_wh_ratio = recImgW / (float)recImgH;
            float ratio = static_cast<float>(detectImg[i].cols) / static_cast<float>(detectImg[i].rows);
            max_wh_ratio = std::max(max_wh_ratio, ratio);

            int imgW = int(ceilf(recImgH * max_wh_ratio));
            if (imgW > 3200) {
                imgW = 3200;
                cv::resize(detectImg[i], stdMat, cv::Size(3200, recImgH), 0, 0, cv::INTER_LINEAR);
            } else {
                int resized_w = int(ceilf(recImgH * ratio));
                resized_w = resized_w > imgW ? imgW : resized_w;
                cv::resize(detectImg[i], stdMat, cv::Size(resized_w, recImgH), 0, 0, cv::INTER_LINEAR);
            }
            // Fill in 127, which is approximately equal to fill in 0 after being processed by subtract_cean_normalization
            cv::copyMakeBorder(stdMat, stdMat, 0, 0, 0, int(imgW - stdMat.cols), cv::BORDER_CONSTANT, {127, 127, 127});
        }
        if (needBreak) {
            continue;
        }

        //stdMat HWC,BGR
        ncnn::Mat input = ncnn::Mat::from_pixels(stdMat.data, ncnn::Mat::PIXEL_BGR, stdMat.cols, stdMat.rows);
        const float mean_vals[3] = {127.5, 127.5, 127.5};
        const float norm_vals[3] = {1.0f / 127.5f, 1.0f / 127.5f, 1.0f / 127.5f};
        input.substract_mean_normalize(mean_vals, norm_vals);
        auto outIndexes = recNet->output_indexes();
        ncnn::Extractor extractor = recNet->create_extractor();
        if (recNet->opt.use_vulkan_compute) {
            if (maxThreadsUsed > 1 && i % maxThreadsUsed != 1) {
                extractor.set_vulkan_compute(false);
            }
        }

        extractor.input(0, input);
        ncnn::Mat out;
        extractor.extract(outIndexes[outIndexes.size() - 1], out);

        if (needBreak) {
            continue;
        }

        float *floatArray = static_cast<float *>(out.data);
        std::vector<float> recNetOutputData(floatArray, floatArray + out.h * out.w);

        auto ctcResult = ctcDecode(recNetOutputData, out.h, out.w);
        auto baseSize = ctcResult.second;
        auto box = allTextBoxes[i];

        // v5 is not support char boxes
#pragma omp critical
        {
            allResultVec[i] = ctcResult.first;
            boxesResult[i] = ctcResult.first.c_str();
        }

        if (needBreak) {
            continue;
        }
    }

    for (const auto &eachResult : allResultVec) {
        allResult += eachResult.c_str();
        allResult += "\n";
    }
}

bool PaddleOCRAppV5::setUseHardware(const QList<QPair<Dtk::Ocr::HardwareID, int>> &hardwareUsed)
{
    for (const auto &pair : hardwareUsed) {
        if (!supportHardwares.contains(pair.first)) {
            return false;
        }
    }

    needReset = true;
    hardwareUseInfos = hardwareUsed;
    return true;
}

QList<Dtk::Ocr::HardwareID> PaddleOCRAppV5::hardwareSupportList()
{
    return supportHardwares;
}

bool PaddleOCRAppV5::setUseMaxThreadsCount(int n)
{
    needReset = true;
    maxThreadsUsed = n;
    return true;
}

QStringList PaddleOCRAppV5::imageFileSupportFormats() const
{
    return {"BMP", "JPEG", "PNG", "PBM", "PGM", "PPM"};
}

bool PaddleOCRAppV5::setImage(const QImage &image)
{
    if (inRunning) {
        qWarning() << "analyze is running";
        return false;
    }
    imageCache = image;
    return true;
}

bool PaddleOCRAppV5::setImageFile(const QString &filePath)
{
    if (inRunning) {
        qWarning() << "analyze is running";
        return false;
    }
    imageCache = QImage(filePath);
    return !imageCache.isNull();
}

QStringList PaddleOCRAppV5::languageSupport() const
{
    return supportLanguages;
}

bool PaddleOCRAppV5::setLanguage(const QString &language)
{
    if (!supportLanguages.contains(language)) {
        return false;
    } else {
        languageUsed = language;
        needReset = true;
        return true;
    }
}

bool PaddleOCRAppV5::analyze()
{
    inRunning = true;

    if (imageCache.isNull()) {
        inRunning = false;
        qWarning() << "imageCache is not valid";
        return false;
    }

    if (needReset) {
        resetNet();
    }
    initNet();

    auto image = imageCache.convertToFormat(QImage::Format_RGB888);
    image = image.rgbSwapped(); //BGR
    cv::Mat matrix(image.height(), image.width(), CV_8UC3, image.bits(), image.bytesPerLine()); //HWC,BGR

    do {
        auto boxes = detect(matrix, 0.3f, 0.6f, 1.5f);
        if (needBreak) {
            break;
        }

        std::sort(
            boxes.begin(), boxes.end(), [](const std::vector<std::vector<int>> &boxL, const std::vector<std::vector<int>> &boxR) {
                int x_collect_L[4] = {boxL[0][0], boxL[1][0], boxL[2][0], boxL[3][0]};
                int y_collect_L[4] = {boxL[0][1], boxL[1][1], boxL[2][1], boxL[3][1]};
                int x_collect_R[4] = {boxR[0][0], boxR[1][0], boxR[2][0], boxR[3][0]};
                int y_collect_R[4] = {boxR[0][1], boxR[1][1], boxR[2][1], boxR[3][1]};
                int y_L = *std::min_element(y_collect_L, y_collect_L + 4);
                int height_L = *std::max_element(y_collect_L, y_collect_L + 4) - y_L;
                int y_R = *std::min_element(y_collect_R, y_collect_R + 4);
                int height_R = *std::max_element(y_collect_R, y_collect_R + 4) - y_R;
                if (y_R - y_L > height_R / 3.0f * 2.0f) {
                    return true;
                } else if (y_L - y_R > height_L / 3.0f * 2.0f) {
                    return false;
                }
                int x_L = *std::min_element(x_collect_L, x_collect_L + 4);
                int x_R = *std::min_element(x_collect_R, x_collect_R + 4);
                if (x_L < x_R) {
                    return true;
                } else {
                    return false;
                }
            });

        if (needBreak) {
            break;
        }

        allTextBoxes.clear();
        for (auto &eachBox : boxes) {
            eachBox[0][1] = std::min(eachBox[0][1], eachBox[1][1]);
            eachBox[1][1] = std::min(eachBox[0][1], eachBox[1][1]);
            eachBox[2][1] = std::max(eachBox[2][1], eachBox[3][1]);
            eachBox[3][1] = std::max(eachBox[2][1], eachBox[3][1]);
            eachBox[0][0] = std::min(eachBox[0][0], eachBox[3][0]);
            eachBox[3][0] = std::min(eachBox[0][0], eachBox[3][0]);
            eachBox[1][0] = std::max(eachBox[1][0], eachBox[2][0]);
            eachBox[2][0] = std::max(eachBox[1][0], eachBox[2][0]);

            Dtk::Ocr::TextBox temp;
            temp.points.push_back(QPointF(eachBox[0][0], eachBox[0][1]));
            temp.points.push_back(QPointF(eachBox[1][0], eachBox[1][1]));
            temp.points.push_back(QPointF(eachBox[2][0], eachBox[2][1]));
            temp.points.push_back(QPointF(eachBox[3][0], eachBox[3][1]));
            temp.angle = 0;
            allTextBoxes.push_back(temp);
        }

        if (needBreak) {
            break;
        }

        std::vector<cv::Mat> images;
        std::transform(
            boxes.begin(), boxes.end(), std::back_inserter(images), [this, matrix](const std::vector<std::vector<int>> &box) {
                return utilityTool.GetRotateCropImage(matrix, box);
            });

        if (needBreak) {
            break;
        }

        rec(images);
    } while (0);

    if (needBreak) {
        allTextBoxes.clear();
        allResult.clear();
        boxesResult.clear();
        needBreak = false;

        inRunning = false;
        return false;
    } else {
        for (int i = 0; i != boxesResult.size(); ++i) {
            if (boxesResult[i].isEmpty()) {
                boxesResult.removeAt(i);
                allTextBoxes.removeAt(i);
                --i;
            }
        }

        inRunning = false;
        return !allTextBoxes.empty();
    }
}

bool PaddleOCRAppV5::isRunning() const
{
    return inRunning;
}

QImage PaddleOCRAppV5::imageCached() const
{
    if (imageCache.isNull()) {
        qWarning() << "no image cached";
    }

    return imageCache;
}

bool PaddleOCRAppV5::breakAnalyze()
{
    if (!needBreak && isRunning()) {
        needBreak = true;
        return true;
    } else {
        return false;
    }
}

QList<Dtk::Ocr::TextBox> PaddleOCRAppV5::textBoxes() const
{
    return allTextBoxes;
}

QList<Dtk::Ocr::TextBox> PaddleOCRAppV5::charBoxes(int index) const
{
    Q_UNUSED(index)
    return {};
}

QString PaddleOCRAppV5::simpleResult() const
{
    return allResult;
}

QString PaddleOCRAppV5::resultFromBox(int index) const
{
    return boxesResult.at(index);
}

DOCR_END_NAMESPACE
