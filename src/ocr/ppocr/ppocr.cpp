// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ppocr.h"

#include <ncnn/layer.h>
#include <ncnn/net.h>

#include <QDir>
#include <QProcessEnvironment>
#include <QThread>
#include <QtDebug>
#include <QSet>

DOCR_BEGIN_NAMESPACE

PaddleOCRApp::PaddleOCRApp()
{
#ifndef QT_NO_DEBUG
    currentPath = OCR_MODEL_DIR;
#else
    QString fullPath;
    auto xdgDirs = QProcessEnvironment::systemEnvironment().value("XDG_DATA_DIRS").split(':');
    for (auto &xdgDir : xdgDirs) {
        fullPath = xdgDir + '/' + OCR_MODEL_DIR;
        if (QFile::exists(fullPath)) {
            currentPath = fullPath;
            break;
        }
    }

    if (currentPath.isEmpty()) {
        fullPath = "/usr/share/" OCR_MODEL_DIR;
        if (QFile::exists(fullPath)) {
            currentPath = fullPath;
        } else {
            qWarning() << "cannot find default model";
        }
    }
#endif
}

PaddleOCRApp::~PaddleOCRApp()
{
    resetNet();
}

void PaddleOCRApp::resetNet()
{
    delete detNet;
    delete recNet;

    detNet = nullptr;
    recNet = nullptr;

    keys.clear();

    needReset = false;
}

void PaddleOCRApp::initNet()
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

    const QString paramSuffix = ".param.bin";
    const QString binSuffix = ".bin";
    const QString dictSuffix = ".txt";

    ncnn::Option option;
    option.lightmode = true;
    option.use_int8_inference = false;
    option.num_threads = 1;

    if (detNet == nullptr) {
        auto detModelPathPrefix = currentPath + "det";

        detNet = new ncnn::Net;
        detNet->opt = option;
        detNet->opt.num_threads = static_cast<int>(maxThreadsUsed);
        detNet->load_param_bin((detModelPathPrefix + paramSuffix).toStdString().c_str());
        detNet->load_model((detModelPathPrefix + binSuffix).toStdString().c_str());
    }

    if (recNet == nullptr) {
        auto recModelPathPrefix = currentPath + "rec_" + languageUsed;

        recNet = new ncnn::Net;
        recNet->opt = option;

        if (!gpuCanUse.empty()) {
            recNet->set_vulkan_device(gpuCanUse[0]);
            recNet->opt.use_vulkan_compute = true;
        }

        recNet->load_param_bin((recModelPathPrefix + paramSuffix).toStdString().c_str());
        recNet->load_model((recModelPathPrefix + binSuffix).toStdString().c_str());
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
PaddleOCRApp::detect(const cv::Mat &src, float thresh, float boxThresh, float unclipRatio)
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

    ncnn::Mat in_pad = ncnn::Mat::from_pixels(resize_img.data, ncnn::Mat::PIXEL_RGB, resizeW, resizeH);

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
    cv::Mat bit_map;
    cv::threshold(cbuf_map, bit_map, static_cast<double>(threshold), 255, cv::THRESH_BINARY);
    cv::Mat dilation_map;
    cv::Mat dila_ele = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
    cv::dilate(bit_map, dilation_map, dila_ele);

    if (needBreak) {
        return std::vector<std::vector<std::vector<int>>>();
    }

    auto result = postProcessor.BoxesFromBitmap(pred_map, dilation_map, boxThresh, unclipRatio, false);

    if (needBreak) {
        return std::vector<std::vector<std::vector<int>>>();
    }

    result = postProcessor.FilterTagDetRes(result, ratio_h, ratio_w, src);

    return result;
}

std::pair<std::string, std::vector<int>> PaddleOCRApp::ctcDecode(const std::vector<float> &recNetOutputData, int h, int w)
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

QList<Dtk::Ocr::TextBox>
PaddleOCRApp::lengthToBox(const std::vector<int> &lengths, QPointF basePoint, float rectHeight, float ratio)
{
    QList<Dtk::Ocr::TextBox> result;
    float currentPos = basePoint.x();
    float yAxisT = basePoint.y();
    float yAxisB = basePoint.y() + rectHeight;
    for (auto &eachLen : lengths) {
        Dtk::Ocr::TextBox temp;
        temp.angle = 0;
        temp.points.push_back(QPointF(currentPos, yAxisT));
        temp.points.push_back(QPointF(currentPos + eachLen * 4 / ratio, yAxisT));
        temp.points.push_back(QPointF(currentPos + eachLen * 4 / ratio, yAxisB));
        temp.points.push_back(QPointF(currentPos, yAxisB));
        result.push_back(temp);
        currentPos += eachLen * 4 / ratio;
    }
    return result;
}

void PaddleOCRApp::rec(const std::vector<cv::Mat> &detectImg)
{
    size_t size = detectImg.size();
    allResult.clear();
    std::vector<std::string> allResultVec(detectImg.size());
    boxesResult.resize(size);
    allCharBoxes.resize(size);

#pragma omp parallel for num_threads(maxThreadsUsed)
    for (size_t i = 0; i < size; ++i) {
        if (needBreak) {
            continue;
        }

        //输入图片固定高度32
        float ratio = static_cast<float>(detectImg[i].cols) / static_cast<float>(detectImg[i].rows);
        int imgW = static_cast<int>(32 * ratio);
        int resize_w;
        if (ceilf(32 * ratio) > imgW)
            resize_w = imgW;
        else
            resize_w = static_cast<int>(ceilf(32 * ratio));

        cv::Mat stdMat;
        cv::resize(detectImg[i], stdMat, cv::Size(resize_w, 32), 0, 0, cv::INTER_LINEAR);
        cv::copyMakeBorder(stdMat, stdMat, 0, 0, 0, int(imgW - stdMat.cols), cv::BORDER_CONSTANT, {127, 127, 127});

        float realRatio = static_cast<float>(stdMat.cols) / detectImg[i].cols;

        if (needBreak) {
            continue;
        }

        ncnn::Mat input = ncnn::Mat::from_pixels(stdMat.data, ncnn::Mat::PIXEL_RGB, stdMat.cols, stdMat.rows);
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

        allResultVec[i] = ctcResult.first;
        boxesResult[i] = ctcResult.first.c_str();
        auto box = allTextBoxes[i];
        auto baseSize = ctcResult.second;
        auto currentCharBox = lengthToBox(baseSize, allTextBoxes[i].points[0], box.points[2].y() - box.points[0].y(), realRatio);
        allCharBoxes[i] = currentCharBox;

        if (needBreak) {
            continue;
        }
    }

    for (const auto &eachResult : allResultVec) {
        allResult += eachResult.c_str();
        allResult += "\n";
    }
}

bool PaddleOCRApp::setUseHardware(const QList<QPair<Dtk::Ocr::HardwareID, int>> &hardwareUsed)
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

QList<Dtk::Ocr::HardwareID> PaddleOCRApp::hardwareSupportList()
{
    return supportHardwares;
}

bool PaddleOCRApp::setUseMaxThreadsCount(int n)
{
    needReset = true;
    maxThreadsUsed = n;
    return true;
}

QStringList PaddleOCRApp::imageFileSupportFormats() const
{
    return {"BMP", "JPEG", "PNG", "PBM", "PGM", "PPM"};
}

bool PaddleOCRApp::setImage(const QImage &image)
{
    if (inRunning) {
        qWarning() << "analyze is running";
        return false;
    }
    imageCache = image;
    return true;
}

bool PaddleOCRApp::setImageFile(const QString &filePath)
{
    if (inRunning) {
        qWarning() << "analyze is running";
        return false;
    }
    imageCache = QImage(filePath);
    return !imageCache.isNull();
}

QStringList PaddleOCRApp::languageSupport() const
{
    return supportLanguages;
}

bool PaddleOCRApp::setLanguage(const QString &language)
{
    if (!supportLanguages.contains(language)) {
        return false;
    } else {
        languageUsed = language;
        needReset = true;
        return true;
    }
}

bool PaddleOCRApp::analyze()
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
    image = image.rgbSwapped();
    cv::Mat matrix(image.height(), image.width(), CV_8UC3, image.bits(), image.bytesPerLine());

    do {
        auto boxes = detect(matrix, 0.3f, 0.5f, 1.6f);

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
        allCharBoxes.clear();
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
                allCharBoxes.removeAt(i);
                --i;
            }
        }

        inRunning = false;
        return !allTextBoxes.empty();
    }
}

bool PaddleOCRApp::isRunning() const
{
    return inRunning;
}

QImage PaddleOCRApp::imageCached() const
{
    if (imageCache.isNull()) {
        qWarning() << "no image cached";
    }

    return imageCache;
}

bool PaddleOCRApp::breakAnalyze()
{
    if (!needBreak && isRunning()) {
        needBreak = true;
        return true;
    } else {
        return false;
    }
}

QList<Dtk::Ocr::TextBox> PaddleOCRApp::textBoxes() const
{
    return allTextBoxes;
}

QList<Dtk::Ocr::TextBox> PaddleOCRApp::charBoxes(int index) const
{
    if (index >= allCharBoxes.size()) {
        return QList<Dtk::Ocr::TextBox>();
    } else {
        return allCharBoxes.at(index);
    }
}

QString PaddleOCRApp::simpleResult() const
{
    return allResult;
}

QString PaddleOCRApp::resultFromBox(int index) const
{
    return boxesResult.at(index);
}

DOCR_END_NAMESPACE
