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
