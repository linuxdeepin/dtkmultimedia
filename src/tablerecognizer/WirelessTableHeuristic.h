// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WIRELESSTABLEHEURISTIC_H
#define WIRELESSTABLEHEURISTIC_H

#include "dtablerecognizer_global.h"
#include "tabletypes.h"

#include <QImage>
#include <QList>
#include <QPair>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

// 无线框/弱线表格启发式（H3）。
//
// 通用问题：SLANet 训练偏向有线表，无线框表格结构预测出错且「自信地给错」，
// 既有 img2table 降级依赖霍夫线检测，无线表无线同样无效。
//
// 修复方向：当主路径对无线/弱线表不可信时，改由 OCR 文本框的对齐关系构建表格
// 结构——按文本框纵向聚类成行、按水平空白间隙切列、由行列网格构建单元格。
// 仅产出结构（row/col/colSpan/rowSpan/bbox），文字仍交由 CellTextMapper 统一映射，
// 避免与 OCR 文本拼接（M3）耦合。
class WirelessTableHeuristic
{
public:
    // 由 OCR 文本框对齐关系构建表格单元格（无线框/弱线场景）。
    // 成功返回 true 并填充 cells（text 留空，交由 CellTextMapper 映射）。
    bool build(const QSize &imageSize, const QList<OcrTextBox> &ocrBoxes,
               QList<DetectedCell> &cells, QString &error);

    bool available() const;

    // ===== 以下为可单独测试的纯逻辑接口 =====

    // 估算图片的表格线密度（0~1）：形态学提取水平/垂直长线像素占比。
    // 密度低 -> 无线/弱线表；密度高 -> 有线表。供 runPipeline 决策是否启用无线启发式。
    static float lineDensity(const QImage &image);

    // 文本框纵向聚类成行：按中心 y 排序后按阈值分簇。返回每行所含文本框索引
    // （行按从上到下、行内按从左到右排序）。
    static QList<QList<int>> clusterRows(const QList<OcrTextBox> &boxes, qreal yTolerance);

    // 由全部文本框的水平区间按空白间隙切列：返回每列的 (xMin, xMax)（从左到右）。
    // gapTolerance 为判定同列的最大水平间距（<= 该宽度视为同列，> 则分列）。
    static QList<QPair<qreal, qreal>> splitColumns(const QList<OcrTextBox> &boxes, qreal gapTolerance);
};

D_TABLERECOGNIZER_END_NAMESPACE

#endif // WIRELESSTABLEHEURISTIC_H
