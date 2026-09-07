// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TABLESTRUCTUREDETECTOR_H
#define TABLESTRUCTUREDETECTOR_H

#include "dtablerecognizer_global.h"
#include "tabletypes.h"

#include <QImage>
#include <QList>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

class OrtInferenceEngine;

// SLANet_plus 表格结构检测：预处理 -> ORT 推理 -> 结构 token 解码 -> 单元格 bbox。
class D_TABLERECOGNIZER_EXPORT TableStructureDetector
{
public:
    explicit TableStructureDetector(OrtInferenceEngine *engine);
    ~TableStructureDetector();

    // 对图片做表格结构检测；成功返回 true 并填充 cells。
    // confidence（可选出参）接收结构 token 的 softmax 均值置信度（0~1）：
    // 越低表示模型对结构越不自信，供 runPipeline 门控降级使用（M1 置信度门）。
    bool detect(const QImage &image, QList<DetectedCell> &cells, QString &error,
                float *confidence = nullptr);

    // 引擎已加载模型时返回 true。
    bool available() const;

    // ===== 以下为可单独测试的纯逻辑接口 =====

    // QImage -> NCHW float 张量（resize 到 inputSize，归一化 mean/std）。
    // 返回 CHW 顺序的 float 数据，shape = {1, 3, inputSize, inputSize}。
    static std::vector<float> preprocess(const QImage &image, int inputSize);

    // 计算 [T, V] 结构 logits 的逐时间步 softmax 最大概率均值（置信度）。
    // ids 为已 argmax 的 token 序列（用于界定有效时间步范围）。
    // 返回 0~1：1 表示模型在每个时间步都唯一确信，越低越不自信。
    static float meanMaxConfidence(const std::vector<float> &logits, int V, int T,
                                   const std::vector<int> &ids);

    // 结构 token id 序列 + bbox 浮点序列 -> DetectedCell 列表。
    // bboxStride 为每个单元格的浮点数个数（4 表示 [x1,y1,x2,y2]，
    // 8 表示 4 个多边形顶点 [x1,y1,x2,y2,x3,y3,x4,y4]）。
    // bboxPerTimeStep 为 true 时按时间步索引 bbox（PaddleOCR SLANet 约定），
    // 为 false 时按单元格计数器索引 bbox（单元测试约定）。
    static QList<DetectedCell> decodeStructure(const std::vector<int> &structureIds,
                                               const std::vector<float> &bboxes,
                                               const QSize &imageSize,
                                               int bboxStride = 4,
                                               bool bboxPerTimeStep = false);

    // 结构词表（50 tokens，与模型导出一致）。
    static QStringList vocabulary();

private:
    OrtInferenceEngine *m_engine;
};

D_TABLERECOGNIZER_END_NAMESPACE

#endif // TABLESTRUCTUREDETECTOR_H
