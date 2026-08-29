// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "OrtInferenceEngine.h"

#include <onnxruntime_cxx_api.h>

#include <QDebug>
#include <QFileInfo>
#include <cstdio>
#include <unistd.h>
#include <thread>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

namespace {
// RAII：临时重定向 stderr 到 /dev/null，抑制 ONNX schema 重复注册告警。
// 这些告警由底层 onnx 库在 Session 构造时直接输出到 stderr，不受 ORT 日志级别
// 控制。无论 Ort::Session 构造是否抛异常，作用域结束时析构都恢复 stderr 并关闭
// fd，避免异常路径下 stderr 永久指向 /dev/null 及 fd 泄漏。
class StderrSuppressor
{
public:
    StderrSuppressor()
        : m_savedFd(-1)
        , m_devnull(nullptr)
    {
        fflush(stderr);
        m_savedFd = dup(fileno(stderr));
        if (m_savedFd < 0)
            return;
        m_devnull = fopen("/dev/null", "w");
        if (m_devnull)
            dup2(fileno(m_devnull), fileno(stderr));
    }
    ~StderrSuppressor()
    {
        fflush(stderr);
        if (m_devnull) {
            fclose(m_devnull);
            m_devnull = nullptr;
        }
        if (m_savedFd >= 0) {
            dup2(m_savedFd, fileno(stderr));
            close(m_savedFd);
            m_savedFd = -1;
        }
    }
    StderrSuppressor(const StderrSuppressor &) = delete;
    StderrSuppressor &operator=(const StderrSuppressor &) = delete;

private:
    int m_savedFd;
    FILE *m_devnull;
};
} // namespace

class OrtInferenceEngine::Impl
{
public:
    // 使用 FATAL 级别抑制 ORT 运行时日志。
    // ONNX schema 重复注册告警来自底层 onnx 库（输出到 stderr），
    // 需在创建 Session 时临时重定向 stderr（见 loadModel）。
    Ort::Env env{ORT_LOGGING_LEVEL_FATAL, "dtk6tablerecognizer"};
    std::unique_ptr<Ort::Session> session;
    Ort::AllocatorWithDefaultOptions allocator;
    QStringList inputNames;
    QStringList outputNames;
    QString lastError;
    bool loaded = false;
};

OrtInferenceEngine::OrtInferenceEngine()
    : d(std::make_unique<Impl>())
{
}

OrtInferenceEngine::~OrtInferenceEngine() = default;

bool OrtInferenceEngine::loadModel(const QString &modelPath)
{
    d->loaded = false;
    d->session.reset();
    d->lastError.clear();
    d->inputNames.clear();
    d->outputNames.clear();

    if (modelPath.isEmpty()) {
        d->lastError = QStringLiteral("模型路径为空");
        return false;
    }
    const QFileInfo info(modelPath);
    if (!info.exists() || !info.isFile()) {
        d->lastError = QStringLiteral("模型文件不存在: %1").arg(modelPath);
        return false;
    }

    try {
        Ort::SessionOptions sessionOptions;
        sessionOptions.SetLogSeverityLevel(4);

        // ORT 线程数配置：优先 TABLEREC_ORT_THREADS 环境变量，默认取物理核数。
        // 三平台统一口径：bench CLI 通过设置此环境变量控制线程数。
        {
            int numThreads = static_cast<int>(std::thread::hardware_concurrency());
            if (numThreads < 1)
                numThreads = 1;
            const QByteArray envThreads = qgetenv("TABLEREC_ORT_THREADS");
            if (!envThreads.isEmpty()) {
                bool ok = false;
                const int envVal = envThreads.toInt(&ok);
                if (ok && envVal > 0)
                    numThreads = envVal;
            }
            sessionOptions.SetIntraOpNumThreads(numThreads);
        }

        // 临时重定向 stderr 到 /dev/null，抑制 ONNX schema 重复注册告警。
        // 这些告警由底层 onnx 库在 Session 构造时直接输出到 stderr，
        // 不受 ORT 日志级别控制（SetLogSeverityLevel 仅影响 ORT 自身日志）。
        // RAII 作用域：StderrSuppressor 构造时重定向 stderr，析构时恢复，
        // 异常路径下亦保证 stderr 恢复与 fd 释放。
        {
            StderrSuppressor suppressor;
            d->session = std::make_unique<Ort::Session>(d->env, modelPath.toUtf8().constData(),
                                                        sessionOptions);
        }

        const size_t inCount = d->session->GetInputCount();
        for (size_t i = 0; i < inCount; ++i) {
            auto name = d->session->GetInputNameAllocated(i, d->allocator);
            d->inputNames << QString::fromUtf8(name.get());
        }
        const size_t outCount = d->session->GetOutputCount();
        for (size_t i = 0; i < outCount; ++i) {
            auto name = d->session->GetOutputNameAllocated(i, d->allocator);
            d->outputNames << QString::fromUtf8(name.get());
        }
        d->loaded = true;
        return true;
    } catch (const Ort::Exception &e) {
        d->lastError = QStringLiteral("ORT 加载失败: %1").arg(QString::fromUtf8(e.what()));
        d->session.reset();
        return false;
    } catch (const std::exception &e) {
        d->lastError = QStringLiteral("ORT 加载异常: %1").arg(QString::fromUtf8(e.what()));
        d->session.reset();
        return false;
    }
}

bool OrtInferenceEngine::isLoaded() const
{
    return d && d->loaded;
}

std::vector<std::vector<float>> OrtInferenceEngine::run(const std::vector<float> &input,
                                                        const std::vector<int64_t> &inputShape,
                                                        std::vector<std::vector<int64_t>> *outShapes)
{
    std::vector<std::vector<float>> outputs;
    if (!isLoaded()) {
        d->lastError = QStringLiteral("模型未加载");
        return outputs;
    }
    if (input.empty() || inputShape.empty()) {
        d->lastError = QStringLiteral("输入张量为空");
        return outputs;
    }

    try {
        Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            memoryInfo, const_cast<float *>(input.data()), input.size(),
            inputShape.data(), inputShape.size());

        // 持久保存 QByteArray，保证 Run 调用期间 const char* 缓冲存活，避免悬垂指针。
        std::vector<QByteArray> inUtf8;
        std::vector<const char *> inNames;
        inUtf8.reserve(d->inputNames.size());
        for (const QString &n : d->inputNames) {
            inUtf8.push_back(n.toUtf8());
            inNames.push_back(inUtf8.back().constData());
        }
        std::vector<QByteArray> outUtf8;
        std::vector<const char *> outNames;
        outUtf8.reserve(d->outputNames.size());
        for (const QString &n : d->outputNames) {
            outUtf8.push_back(n.toUtf8());
            outNames.push_back(outUtf8.back().constData());
        }

        auto outputTensors = d->session->Run(Ort::RunOptions{nullptr}, inNames.data(),
                                             &inputTensor, 1, outNames.data(), outNames.size());

        for (auto &tensor : outputTensors) {
            const auto &typeInfo = tensor.GetTensorTypeAndShapeInfo();
            const auto shape = typeInfo.GetShape();
            const size_t total = typeInfo.GetElementCount();
            const float *data = tensor.GetTensorData<float>();
            outputs.emplace_back(data, data + total);
            if (outShapes)
                outShapes->emplace_back(shape.begin(), shape.end());
        }
        d->lastError.clear();
    } catch (const Ort::Exception &e) {
        d->lastError = QStringLiteral("ORT 推理失败: %1").arg(QString::fromUtf8(e.what()));
    } catch (const std::exception &e) {
        d->lastError = QStringLiteral("ORT 推理异常: %1").arg(QString::fromUtf8(e.what()));
    }
    return outputs;
}

QStringList OrtInferenceEngine::inputNames() const
{
    return d ? d->inputNames : QStringList();
}

QStringList OrtInferenceEngine::outputNames() const
{
    return d ? d->outputNames : QStringList();
}

QString OrtInferenceEngine::lastError() const
{
    return d ? d->lastError : QString();
}

D_TABLERECOGNIZER_END_NAMESPACE
