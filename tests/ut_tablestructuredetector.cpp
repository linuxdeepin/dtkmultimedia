// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "../src/tablerecognizer/TableStructureDetector.h"
#include "../src/tablerecognizer/OrtInferenceEngine.h"

#include <gtest/gtest.h>

D_TABLERECOGNIZER_USE_NAMESPACE

// 词表索引（与 TableStructureDetector.cpp 内 vocab() 一致，从 ONNX 模型元数据提取）：
// 0:<sos> 1:<thead> 2:</thead> 3:<tbody> 4:</tbody>
// 5:<tr> 6:</tr> 7:<td 8:> 9:</td>
// 10-28: colspan="2".."colspan="20"
// 29-47: rowspan="2".."rowspan="20"
// 48:<td></td> 49:<eos>
static std::vector<int> ids(std::initializer_list<int> lst)
{
    return std::vector<int>(lst);
}

// 解码一个 2x2 表格，验证行列。
// <tbody><tr><td><td></tr><tr><td><td></tr></tbody>
TEST(ut_TableStructureDetector, decodeSimpleGrid)
{
    std::vector<int> structIds = ids({3, 5, 7, 8, 7, 8, 6, 5, 7, 8, 7, 8, 6, 4});
    // 单元格按出现顺序的 bbox（4 值 = 4 个坐标，非按时间步）。
    std::vector<float> bboxes = {
        0.0f, 0.0f, 0.5f, 0.5f,
        0.5f, 0.0f, 1.0f, 0.5f,
        0.0f, 0.5f, 0.5f, 1.0f,
        0.5f, 0.5f, 1.0f, 1.0f,
    };

    QList<DetectedCell> cells = TableStructureDetector::decodeStructure(structIds, bboxes, QSize(100, 100), 4, false);
    ASSERT_EQ(cells.size(), 4);
    EXPECT_EQ(cells[0].row, 0);
    EXPECT_EQ(cells[0].col, 0);
    EXPECT_EQ(cells[1].row, 0);
    EXPECT_EQ(cells[1].col, 1);
    EXPECT_EQ(cells[2].row, 1);
    EXPECT_EQ(cells[2].col, 0);
    EXPECT_EQ(cells[3].row, 1);
    EXPECT_EQ(cells[3].col, 1);
    EXPECT_FLOAT_EQ(cells[0].bbox.left(), 0.0f);
    EXPECT_FLOAT_EQ(cells[3].bbox.right(), 100.0f);
}

// 使用 <td></td>(48) 自闭合空单元格解码 1x2 表格。
TEST(ut_TableStructureDetector, decodeSelfClosingCell)
{
    // <tbody><tr><td></td><td></td></tr></tbody>
    std::vector<int> structIds = ids({3, 5, 48, 48, 6, 4});
    std::vector<float> bboxes = {
        0.0f, 0.0f, 0.5f, 1.0f,
        0.5f, 0.0f, 1.0f, 1.0f,
    };

    QList<DetectedCell> cells = TableStructureDetector::decodeStructure(structIds, bboxes, QSize(50, 50), 4, false);
    ASSERT_EQ(cells.size(), 2);
    EXPECT_EQ(cells[0].col, 0);
    EXPECT_EQ(cells[1].col, 1);
}

// 解码含 colspan="2"(10) 的单元格，验证 span 与后续单元格列号。
TEST(ut_TableStructureDetector, decodeColspan)
{
    // <tbody><tr><td colspan="2"><td></tr></tbody>
    std::vector<int> structIds = ids({3, 5, 7, 10, 8, 7, 8, 6, 4});
    std::vector<float> bboxes = {
        0.0f, 0.0f, 1.0f, 0.5f,
        0.0f, 0.5f, 1.0f, 1.0f,
    };

    QList<DetectedCell> cells = TableStructureDetector::decodeStructure(structIds, bboxes, QSize(50, 50), 4, false);
    ASSERT_EQ(cells.size(), 2);
    EXPECT_EQ(cells[0].colSpan, 2);
    EXPECT_EQ(cells[0].col, 0);
    EXPECT_EQ(cells[1].col, 2);
}

// 解码含 rowspan="2"(29) 的单元格，验证跨行跳列。
TEST(ut_TableStructureDetector, decodeRowspan)
{
    // <tbody><tr><td rowspan="2"><td></tr><tr><td></tr></tbody>
    std::vector<int> structIds = ids({3, 5, 7, 29, 8, 7, 8, 6, 5, 7, 8, 6, 4});
    std::vector<float> bboxes = {
        0.0f, 0.0f, 0.5f, 1.0f,
        0.5f, 0.0f, 1.0f, 0.5f,
        0.5f, 0.5f, 1.0f, 1.0f,
    };

    QList<DetectedCell> cells = TableStructureDetector::decodeStructure(structIds, bboxes, QSize(50, 50), 4, false);
    ASSERT_EQ(cells.size(), 3);
    EXPECT_EQ(cells[0].rowSpan, 2);
    EXPECT_EQ(cells[0].row, 0);
    EXPECT_EQ(cells[0].col, 0);
    EXPECT_EQ(cells[1].row, 0);
    EXPECT_EQ(cells[1].col, 1);
    // 第二行的单元格应跳过被 rowspan 覆盖的列 0，落在 col 1。
    EXPECT_EQ(cells[2].row, 1);
    EXPECT_EQ(cells[2].col, 1);
}

// 验证 8 值 bbox stride（4 个多边形顶点）的解码。
TEST(ut_TableStructureDetector, decodeBboxStride8)
{
    // <tbody><tr><td><td></tr></tbody>
    std::vector<int> structIds = ids({3, 5, 7, 8, 7, 8, 6, 4});
    std::vector<float> bboxes = {
        0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f,
        0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f,
    };

    QList<DetectedCell> cells = TableStructureDetector::decodeStructure(structIds, bboxes, QSize(100, 100), 8, false);
    ASSERT_EQ(cells.size(), 2);
    EXPECT_FLOAT_EQ(cells[0].bbox.left(), 0.0f);
    EXPECT_FLOAT_EQ(cells[0].bbox.right(), 50.0f);
    EXPECT_FLOAT_EQ(cells[1].bbox.left(), 50.0f);
    EXPECT_FLOAT_EQ(cells[1].bbox.right(), 100.0f);
}

// 验证 available() 在无引擎/未加载模型时返回 false。
TEST(ut_TableStructureDetector, notAvailableWithoutLoadedModel)
{
    OrtInferenceEngine engine;
    TableStructureDetector detector(&engine);
    EXPECT_FALSE(detector.available());
}

// 验证词表大小为 50（与模型 V=50 一致）。
TEST(ut_TableStructureDetector, vocabularySize)
{
    EXPECT_FALSE(TableStructureDetector::vocabulary().isEmpty());
    EXPECT_EQ(TableStructureDetector::vocabulary().size(), 50);
}

// ===== M1：meanMaxConfidence 置信度计算 =====

// 每个时间步 logits 为 one-hot（某一类远大于其他）时，softmax 最大概率接近 1，均值≈1。
TEST(ut_TableStructureDetector, confidenceHighOnOneHotLogits)
{
    // V=4, T=2。每个时间步第 0 类 logits=10，其余=0 → softmax 近似 [1,0,0,0]。
    std::vector<float> logits = {
        10.0f, 0.0f, 0.0f, 0.0f,
        10.0f, 0.0f, 0.0f, 0.0f,
    };
    std::vector<int> ids = {0, 0};
    const float conf = TableStructureDetector::meanMaxConfidence(logits, 4, 2, ids);
    EXPECT_GT(conf, 0.99f);
}

// 每个时间步 logits 均匀（全 0）时，softmax 均匀=1/V，均值=1/V。
TEST(ut_TableStructureDetector, confidenceLowOnUniformLogits)
{
    std::vector<float> logits = {
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
    };
    std::vector<int> ids = {0, 1};
    const float conf = TableStructureDetector::meanMaxConfidence(logits, 4, 2, ids);
    EXPECT_FLOAT_EQ(conf, 0.25f);   // 1/V = 1/4
}

// ids 长度小于 T 时，仅按 ids.size() 个时间步计算均值（与 argmax 序列实际范围对齐）。
TEST(ut_TableStructureDetector, confidenceUsesIdCountWhenShorterThanT)
{
    // T=3 但 ids 只有 2 个（eos 截断后的常见情形）。第 1 步 one-hot、第 2 步均匀。
    std::vector<float> logits = {
        10.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
    };
    std::vector<int> ids = {0, 1};
    const float conf = TableStructureDetector::meanMaxConfidence(logits, 4, 3, ids);
    // 均值 = (1.0 + 0.25) / 2 = 0.625
    EXPECT_NEAR(conf, 0.625f, 1e-4f);
}

// 退化输入返回 0（V<=0 / 空 logits）。
TEST(ut_TableStructureDetector, confidenceZeroOnDegenerateInput)
{
    std::vector<int> ids = {0};
    EXPECT_FLOAT_EQ(TableStructureDetector::meanMaxConfidence({}, 4, 1, ids), 0.0f);
    std::vector<float> logits = {1.0f, 0.0f};
    EXPECT_FLOAT_EQ(TableStructureDetector::meanMaxConfidence(logits, 0, 1, ids), 0.0f);
    EXPECT_FLOAT_EQ(TableStructureDetector::meanMaxConfidence(logits, 2, 0, ids), 0.0f);
}

// ids 越界的时间步被跳过，不影响其他时间步。
TEST(ut_TableStructureDetector, confidenceSkipsOutOfRangeIds)
{
    std::vector<float> logits = {
        10.0f, 0.0f, 0.0f, 0.0f,
        10.0f, 0.0f, 0.0f, 0.0f,
    };
    std::vector<int> ids = {0, 99};   // 第 2 步 id 越界，被跳过 → 均值=1.0/1=1.0
    const float conf = TableStructureDetector::meanMaxConfidence(logits, 4, 2, ids);
    EXPECT_GT(conf, 0.99f);
}

// ===== M1 v2：输出类型自适应（概率直读 vs logits softmax） =====

// 概率输入（值全在 [0,1] 且行和≈1）→ 直接取 row[argmax]，不再 softmax。
// 模拟 SLANet_plus 真实输出：概率 0.9993 → 直读得 0.9993（不被双重 softmax 压成 ~0.0525）。
TEST(ut_TableStructureDetector, confidenceProbabilityDirectRead)
{
    // V=4, T=2。概率分布：[0.9993, 0.0002, 0.0003, 0.0002]，argmax=0
    std::vector<float> probs = {
        0.9993f, 0.0002f, 0.0003f, 0.0002f,
        0.9991f, 0.0003f, 0.0004f, 0.0002f,
    };
    std::vector<int> ids = {0, 0};
    const float conf = TableStructureDetector::meanMaxConfidence(probs, 4, 2, ids);
    // 概率直读：均值 = (0.9993 + 0.9991) / 2 ≈ 0.9992，远高于双重 softmax 的 ~0.0525
    EXPECT_GT(conf, 0.99f);
    EXPECT_NEAR(conf, 0.9992f, 1e-3f);
}

// 概率输入但置信度较低 → 直读得低值（不被 softmax 扭曲）。
TEST(ut_TableStructureDetector, confidenceProbabilityLowConfidence)
{
    // V=4, T=1。概率分布：[0.3, 0.25, 0.25, 0.2]，argmax=0 → 直读 0.3
    std::vector<float> probs = {
        0.3f, 0.25f, 0.25f, 0.2f,
    };
    std::vector<int> ids = {0};
    const float conf = TableStructureDetector::meanMaxConfidence(probs, 4, 1, ids);
    EXPECT_NEAR(conf, 0.3f, 1e-4f);
}

// 原始 logits 输入（值 > 1 或行和 ≠ 1）→ 自动走 softmax 路径。
TEST(ut_TableStructureDetector, confidenceLogitsTriggersSoftmax)
{
    // V=4, T=1。logits=[2, 0, 0, 0]，值 > 1 → 检测为 logits → softmax → ~0.88
    std::vector<float> logits = {
        2.0f, 0.0f, 0.0f, 0.0f,
    };
    std::vector<int> ids = {0};
    const float conf = TableStructureDetector::meanMaxConfidence(logits, 4, 1, ids);
    // softmax([2,0,0,0]) = e^2/(e^2+3) ≈ 0.7113 → 均值=0.7113
    EXPECT_NEAR(conf, 0.7113f, 1e-3f);
}

// 自动判定：概率输入即使值全在 [0,1] 但行和 ≠ 1 → 视为 logits → softmax。
TEST(ut_TableStructureDetector, confidenceAutoDetectNonProbabilityRow)
{
    // V=4, T=1。值全在 [0,1] 但行和=0.5 ≠ 1 → 视为 logits → softmax
    std::vector<float> vals = {
        0.5f, 0.0f, 0.0f, 0.0f,
    };
    std::vector<int> ids = {0};
    const float conf = TableStructureDetector::meanMaxConfidence(vals, 4, 1, ids);
    // softmax([0.5,0,0,0]) = e^0.5/(e^0.5+3) ≈ 0.3547 → 均值=0.3547
    EXPECT_NEAR(conf, 0.3547f, 1e-3f);
}
