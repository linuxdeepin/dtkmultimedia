// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ORTINFERENCEENGINE_H
#define ORTINFERENCEENGINE_H

#include "dtablerecognizer_global.h"

#include <QString>
#include <QStringList>
#include <vector>
#include <cstdint>
#include <memory>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

// ONNX Runtime C++ 封装，仅用于加载 SLANet_plus ONNX 模型进行结构推理。
class D_TABLERECOGNIZER_EXPORT OrtInferenceEngine
{
public:
    OrtInferenceEngine();
    ~OrtInferenceEngine();

    // 加载 ONNX 模型文件；成功返回 true。
    bool loadModel(const QString &modelPath);
    bool isLoaded() const;

    // 以 NCHW 单输入张量推理，返回每个输出节点的 float 序列。
    // inputShape 为输入张量形状（含 batch 维）。
    // outShapes（可选）接收每个输出节点的张量形状。
    std::vector<std::vector<float>> run(const std::vector<float> &input,
                                        const std::vector<int64_t> &inputShape,
                                        std::vector<std::vector<int64_t>> *outShapes = nullptr);

    // 返回模型输入名称列表（loadModel 成功后有效）。
    QStringList inputNames() const;
    // 返回模型输出名称列表（loadModel 成功后有效）。
    QStringList outputNames() const;

    // 失败分类（与 lastError() 配合使用；lastError() 为详细诊断文本）。
    enum class Failure {
        None = 0,
        ModelPathEmpty,   // 模型路径为空
        ModelNotFound,    // 模型文件不存在
        LoadFailed,       // Session 构建/加载抛异常
        NotLoaded,        // 未加载模型就调用 run()
        InvalidInput,     // 输入张量为空
        InferenceFailed,  // run() 抛异常（ORT 或 std 异常）
    };

    QString lastError() const;
    Failure lastFailure() const;

private:
    class Impl;
    std::unique_ptr<Impl> d;
};

D_TABLERECOGNIZER_END_NAMESPACE

#endif // ORTINFERENCEENGINE_H
