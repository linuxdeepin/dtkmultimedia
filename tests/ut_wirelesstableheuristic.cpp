// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "../src/tablerecognizer/WirelessTableHeuristic.h"
#include "../src/tablerecognizer/tabletypes.h"

#include <gtest/gtest.h>

#include <QColor>
#include <QImage>
#include <QPainter>
#include <QPen>

D_TABLERECOGNIZER_USE_NAMESPACE

// ===== clusterRows：纵向聚类成行 =====

// 同一文本行（中心 y 接近）归入一行，跨行分簇。
TEST(ut_WirelessTableHeuristic, clusterRowsGroupsSameLine)
{
    QList<OcrTextBox> boxes;
    OcrTextBox a;   // 行0 左
    a.bbox = QRectF(10, 10, 20, 10);
    OcrTextBox b;   // 行0 右
    b.bbox = QRectF(100, 12, 20, 10);
    OcrTextBox c;   // 行1 左
    c.bbox = QRectF(10, 60, 20, 10);
    OcrTextBox d;   // 行1 右
    d.bbox = QRectF(100, 62, 20, 10);
    boxes << a << b << c << d;

    // yTolerance=5：行0 中心 y≈15/17 在 5 内同行；行1 中心 y≈65/67 同行；行间距 ~50 分簇。
    QList<QList<int>> rows = WirelessTableHeuristic::clusterRows(boxes, 5.0);
    ASSERT_EQ(rows.size(), 2);
    ASSERT_EQ(rows[0].size(), 2);
    ASSERT_EQ(rows[1].size(), 2);
    // 行内按 x 升序：行0 [a(0), b(1)]，行1 [c(2), d(3)]。
    EXPECT_EQ(rows[0][0], 0);
    EXPECT_EQ(rows[0][1], 1);
    EXPECT_EQ(rows[1][0], 2);
    EXPECT_EQ(rows[1][1], 3);
}

// 行按从上到下排序（输入乱序时仍按 y 升序输出）。
TEST(ut_WirelessTableHeuristic, clusterRowsSortedTopToBottom)
{
    QList<OcrTextBox> boxes;
    OcrTextBox top;   // y=5
    top.bbox = QRectF(10, 5, 20, 10);
    OcrTextBox bottom;   // y=80
    bottom.bbox = QRectF(10, 80, 20, 10);
    boxes << bottom << top;   // 故意倒序输入

    QList<QList<int>> rows = WirelessTableHeuristic::clusterRows(boxes, 5.0);
    ASSERT_EQ(rows.size(), 2);
    EXPECT_EQ(rows[0].first(), 1);   // top（输入索引1）在第一行
    EXPECT_EQ(rows[1].first(), 0);   // bottom（输入索引0）在第二行
}

// 空输入返回空。
TEST(ut_WirelessTableHeuristic, clusterRowsEmpty)
{
    QList<OcrTextBox> boxes;
    EXPECT_TRUE(WirelessTableHeuristic::clusterRows(boxes, 5.0).isEmpty());
}

// ===== splitColumns：水平空白间隙切列 =====

// 两列：左列两框 x 重叠归一列，右列一框独立，中间空白分列。
TEST(ut_WirelessTableHeuristic, splitColumnsTwoColumns)
{
    QList<OcrTextBox> boxes;
    OcrTextBox a;   // 左列上
    a.bbox = QRectF(10, 10, 30, 10);
    OcrTextBox b;   // 左列下（与 a x 重叠）
    b.bbox = QRectF(15, 60, 30, 10);
    OcrTextBox c;   // 右列
    c.bbox = QRectF(120, 10, 30, 10);
    boxes << a << b << c;

    // gapTolerance=5：a/b 重叠同列；c 与 b 右端(45) 间距 75 > 5 分列。
    QList<QPair<qreal, qreal>> cols = WirelessTableHeuristic::splitColumns(boxes, 5.0);
    ASSERT_EQ(cols.size(), 2);
    EXPECT_NEAR(cols[0].first, 10.0, 1e-3);   // 左列 [10, 45]
    EXPECT_NEAR(cols[0].second, 45.0, 1e-3);
    EXPECT_NEAR(cols[1].first, 120.0, 1e-3);   // 右列 [120, 150]
    EXPECT_NEAR(cols[1].second, 150.0, 1e-3);
}

// gapTolerance 较大时，邻近两框被合并为同列。
TEST(ut_WirelessTableHeuristic, splitColumnsMergesNearbyByTolerance)
{
    QList<OcrTextBox> boxes;
    OcrTextBox a;   // [10, 40]
    a.bbox = QRectF(10, 10, 30, 10);
    OcrTextBox b;   // [50, 80] 与 a 间距 10
    b.bbox = QRectF(50, 60, 30, 10);
    boxes << a << b;

    // gapTolerance=15：10 <= 40+15 → 同列；gapTolerance=5：50 > 45 → 分列。
    EXPECT_EQ(WirelessTableHeuristic::splitColumns(boxes, 15.0).size(), 1);
    EXPECT_EQ(WirelessTableHeuristic::splitColumns(boxes, 5.0).size(), 2);
}

// 空输入返回空。
TEST(ut_WirelessTableHeuristic, splitColumnsEmpty)
{
    QList<OcrTextBox> boxes;
    EXPECT_TRUE(WirelessTableHeuristic::splitColumns(boxes, 5.0).isEmpty());
}

// ===== build：由文本框构建稀疏网格 =====

// 2 行 × 2 列的无线表：4 个文本框分属 4 个网格位，build 产出 4 个单元格。
TEST(ut_WirelessTableHeuristic, buildTwoByTwoGrid)
{
    QList<OcrTextBox> boxes;
    OcrTextBox a;   a.bbox = QRectF(10, 10, 30, 10);   // (0,0)
    OcrTextBox b;   b.bbox = QRectF(120, 12, 30, 10);  // (0,1)
    OcrTextBox c;   c.bbox = QRectF(10, 60, 30, 10);   // (1,0)
    OcrTextBox d;   d.bbox = QRectF(120, 62, 30, 10);  // (1,1)
    boxes << a << b << c << d;

    QList<DetectedCell> cells;
    QString error;
    WirelessTableHeuristic heuristic;
    ASSERT_TRUE(heuristic.build({}, boxes, cells, error)) << error.toStdString();
    ASSERT_EQ(cells.size(), 4);
    // 行列索引正确。
    auto findCell = [&](int r, int c) -> const DetectedCell * {
        for (const DetectedCell &cc : cells)
            if (cc.row == r && cc.col == c)
                return &cc;
        return nullptr;
    };
    ASSERT_NE(findCell(0, 0), nullptr);
    ASSERT_NE(findCell(1, 1), nullptr);
    EXPECT_EQ(findCell(0, 0)->colSpan, 1);
    EXPECT_EQ(findCell(0, 0)->rowSpan, 1);
    // 文字留空，交由 CellTextMapper 映射。
    EXPECT_TRUE(findCell(0, 0)->text.isEmpty());
}

// 空 OCR 输入返回失败。
TEST(ut_WirelessTableHeuristic, buildFailsOnEmptyBoxes)
{
    QList<DetectedCell> cells;
    QString error;
    WirelessTableHeuristic heuristic;
    EXPECT_FALSE(heuristic.build({}, {}, cells, error));
    EXPECT_FALSE(error.isEmpty());
}

// 稀疏网格：缺位不产出单元格，但存在的网格位行列索引正确。
TEST(ut_WirelessTableHeuristic, buildSparseGrid)
{
    QList<OcrTextBox> boxes;
    OcrTextBox a;   a.bbox = QRectF(10, 10, 30, 10);   // (0,0)
    OcrTextBox b;   b.bbox = QRectF(120, 60, 30, 10); // (1,1) — (0,1) 与 (1,0) 缺失
    boxes << a << b;

    QList<DetectedCell> cells;
    QString error;
    WirelessTableHeuristic heuristic;
    ASSERT_TRUE(heuristic.build({}, boxes, cells, error)) << error.toStdString();
    ASSERT_EQ(cells.size(), 2);
    bool has00 = false, has11 = false;
    for (const DetectedCell &cc : cells) {
        if (cc.row == 0 && cc.col == 0) has00 = true;
        if (cc.row == 1 && cc.col == 1) has11 = true;
    }
    EXPECT_TRUE(has00);
    EXPECT_TRUE(has11);
}

// ===== lineDensity：线密度估算 =====

// 空白图无线，线密度接近 0。
TEST(ut_WirelessTableHeuristic, lineDensityLowOnBlankImage)
{
    QImage image(300, 300, QImage::Format_RGB32);
    image.fill(Qt::white);
    const float d = WirelessTableHeuristic::lineDensity(image);
    EXPECT_LT(d, 0.01f);   // 远低于弱线阈值
}

// 线框网格图线密度明显高于弱线阈值。
TEST(ut_WirelessTableHeuristic, lineDensityHighOnWiredGrid)
{
    QImage image(300, 300, QImage::Format_RGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    QPen pen(Qt::black);
    pen.setWidth(2);
    painter.setPen(pen);
    for (int r = 0; r <= 3; ++r) {   // 3 行网格线
        const int y = 300 * r / 3;
        painter.drawLine(0, y, 300, y);
    }
    for (int c = 0; c <= 3; ++c) {   // 3 列网格线
        const int x = 300 * c / 3;
        painter.drawLine(x, 0, x, 300);
    }
    painter.end();
    const float d = WirelessTableHeuristic::lineDensity(image);
    EXPECT_GT(d, 0.01f);   // 高于弱线阈值，判定为有线表
}

// 空图返回 0。
TEST(ut_WirelessTableHeuristic, lineDensityZeroOnNullImage)
{
    EXPECT_FLOAT_EQ(WirelessTableHeuristic::lineDensity(QImage()), 0.0f);
}

// available() 恒可用（无外部依赖）。
TEST(ut_WirelessTableHeuristic, alwaysAvailable)
{
    WirelessTableHeuristic h;
    EXPECT_TRUE(h.available());
}
