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
class TableStructureDetector
{
public:
    explicit TableStructureDetector(OrtInferenceEngine *engine);
    ~TableStructureDetector();

    // 对图片做表格结构检测；成功返回 true 并填充 cells。
    bool detect(const QImage &image, QList<DetectedCell> &cells, QString &error);

    // 引擎已加载模型时返回 true。
    bool available() const;

    // ===== 以下为可单独测试的纯逻辑接口 =====

    // QImage -> NCHW float 张量（resize 到 inputSize，归一化 mean/std）。
    // 返回 CHW 顺序的 float 数据，shape = {1, 3, inputSize, inputSize}。
    static std::vector<float> preprocess(const QImage &image, int inputSize);

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
