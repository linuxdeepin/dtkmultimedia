// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "../src/tablerecognizer/CellTextMapper.h"
#include "../src/tablerecognizer/tabletypes.h"

#include <gtest/gtest.h>

#include <QRectF>

D_TABLERECOGNIZER_USE_NAMESPACE

TEST(ut_CellTextMapper, centerPointAssignment)
{
    QList<DetectedCell> cells;
    DetectedCell c0;
    c0.row = 0;
    c0.col = 0;
    c0.bbox = QRectF(0, 0, 100, 100);
    DetectedCell c1;
    c1.row = 0;
    c1.col = 1;
    c1.bbox = QRectF(100, 0, 100, 100);
    cells << c0 << c1;

    QList<OcrTextBox> boxes;
    OcrTextBox b0;
    b0.bbox = QRectF(10, 10, 20, 20);
    b0.text = QStringLiteral("A");
    OcrTextBox b1;
    b1.bbox = QRectF(150, 40, 20, 20);
    b1.text = QStringLiteral("B");
    boxes << b0 << b1;

    CellTextMapper mapper;
    mapper.map(cells, boxes);

    ASSERT_EQ(cells.size(), 2);
    EXPECT_EQ(cells[0].text.toStdString(), "A");
    EXPECT_EQ(cells[1].text.toStdString(), "B");
}

TEST(ut_CellTextMapper, multipleBoxesConcatenatedLeftToRightTopToBottom)
{
    QList<DetectedCell> cells;
    DetectedCell cell;
    cell.bbox = QRectF(0, 0, 200, 100);
    cells << cell;

    QList<OcrTextBox> boxes;
    OcrTextBox right;
    right.bbox = QRectF(120, 10, 30, 30);
    right.text = QStringLiteral("B");
    OcrTextBox left;
    left.bbox = QRectF(10, 10, 30, 30);
    left.text = QStringLiteral("A");
    OcrTextBox below;
    below.bbox = QRectF(10, 60, 30, 30);
    below.text = QStringLiteral("C");
    boxes << right << left << below;

    CellTextMapper mapper;
    mapper.map(cells, boxes);

    ASSERT_EQ(cells.size(), 1);
    EXPECT_EQ(cells[0].text.toStdString(), "ABC");
}

TEST(ut_CellTextMapper, iouFallbackForCrossBoundaryBox)
{
    QList<DetectedCell> cells;
    DetectedCell c0;
    c0.bbox = QRectF(0, 0, 100, 100);
    DetectedCell c1;
    c1.bbox = QRectF(100, 0, 100, 100);
    cells << c0 << c1;

    // 框跨两格，中心在 c0，但故意偏移让中心落在边界外，
    // 主要验证 IoU 兜底不会崩溃且能归入最大重叠格。
    QList<OcrTextBox> boxes;
    OcrTextBox box;
    box.bbox = QRectF(80, 40, 60, 30);   // 中心 (110,55)，落入 c1
    box.text = QStringLiteral("X");
    boxes << box;

    CellTextMapper mapper;
    mapper.map(cells, boxes);

    ASSERT_EQ(cells.size(), 2);
    EXPECT_EQ(cells[1].text.toStdString(), "X");
    EXPECT_TRUE(cells[0].text.isEmpty());
}

TEST(ut_CellTextMapper, noBoxesLeavesTextEmpty)
{
    QList<DetectedCell> cells;
    DetectedCell c;
    c.bbox = QRectF(0, 0, 50, 50);
    cells << c;

    QList<OcrTextBox> boxes;
    CellTextMapper mapper;
    mapper.map(cells, boxes);

    ASSERT_EQ(cells.size(), 1);
    EXPECT_TRUE(cells[0].text.isEmpty());
}

TEST(ut_CellTextMapper, lowOverlapBoxDropped)
{
    QList<DetectedCell> cells;
    DetectedCell c0;
    c0.bbox = QRectF(0, 0, 100, 100);
    cells << c0;

    QList<OcrTextBox> boxes;
    OcrTextBox box;
    box.bbox = QRectF(200, 200, 30, 30);   // 完全不相交
    box.text = QStringLiteral("Y");
    boxes << box;

    CellTextMapper mapper;
    mapper.map(cells, boxes);

    ASSERT_EQ(cells.size(), 1);
    EXPECT_TRUE(cells[0].text.isEmpty());
}
