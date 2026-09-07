// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "../include/dtktablerecognizer/dtablecell.h"
#include "../include/dtktablerecognizer/dtablerecognizer_global.h"
#include "../src/tablerecognizer/HtmlTableBuilder.h"

#include <gtest/gtest.h>

#include <QRegularExpression>
#include <QString>

D_TABLERECOGNIZER_USE_NAMESPACE

TEST(ut_HtmlTableBuilder, emptyCellsProducesEmptyTable)
{
    EXPECT_EQ(HtmlTableBuilder::build({}).toStdString(), "<table></table>");
}

TEST(ut_HtmlTableBuilder, simpleGrid)
{
    QList<DTableCell> cells;
    DTableCell a;
    a.row = 0;
    a.col = 0;
    a.text = QStringLiteral("A");
    DTableCell b;
    b.row = 0;
    b.col = 1;
    b.text = QStringLiteral("B");
    cells << a << b;

    const QString html = HtmlTableBuilder::build(cells);
    EXPECT_TRUE(html.contains(QStringLiteral("<table>")));
    EXPECT_TRUE(html.contains(QStringLiteral("<tr>")));
    EXPECT_TRUE(html.contains(QStringLiteral(">A</td>")));
    EXPECT_TRUE(html.contains(QStringLiteral(">B</td>")));
}

TEST(ut_HtmlTableBuilder, rowspanAndColspanAttributes)
{
    QList<DTableCell> cells;
    DTableCell span;
    span.row = 0;
    span.col = 0;
    span.rowSpan = 2;
    span.colSpan = 2;
    span.text = QStringLiteral("S");
    cells << span;

    const QString html = HtmlTableBuilder::build(cells);
    EXPECT_TRUE(html.contains(QStringLiteral("rowspan=\"2\"")));
    EXPECT_TRUE(html.contains(QStringLiteral("colspan=\"2\"")));
}

TEST(ut_HtmlTableBuilder, htmlEscapesSpecialChars)
{
    QList<DTableCell> cells;
    DTableCell c;
    c.row = 0;
    c.col = 0;
    c.text = QStringLiteral("<b>&\"'</b>");
    cells << c;

    const QString html = HtmlTableBuilder::build(cells);
    EXPECT_TRUE(html.contains(QStringLiteral("&lt;b&gt;&amp;&quot;&#39;&lt;/b&gt;")));
    EXPECT_FALSE(html.contains(QStringLiteral("<b>")));
}

TEST(ut_HtmlTableBuilder, multiRow)
{
    QList<DTableCell> cells;
    DTableCell a;
    a.row = 0;
    a.col = 0;
    a.text = QStringLiteral("A");
    DTableCell b;
    b.row = 1;
    b.col = 0;
    b.text = QStringLiteral("B");
    cells << a << b;

    const QString html = HtmlTableBuilder::build(cells);
    const int trCount = html.count(QStringLiteral("<tr>"));
    EXPECT_EQ(trCount, 2);
}

// 通用：结构检测对行数高估会产生全占位（-1）的空行，渲染应跳过这类空行。
// H2 v2：判定从「全 -1」放宽为「整行无任何有文本映射的单元格」。
TEST(ut_HtmlTableBuilder, phantomEmptyRowIsFiltered)
{
    // 仅在第 1 行有单元格，第 0 行无任何单元格也无跨格覆盖 -> 全 -1 空行。
    QList<DTableCell> cells;
    DTableCell a;
    a.row = 1;
    a.col = 0;
    a.text = QStringLiteral("A");
    cells << a;

    const QString html = HtmlTableBuilder::build(cells);
    const int trCount = html.count(QStringLiteral("<tr>"));
    EXPECT_EQ(trCount, 1);
    EXPECT_TRUE(html.contains(QStringLiteral(">A</td>")));
    // 被过滤的空行不应再产生空单元格。
    EXPECT_FALSE(html.contains(QStringLiteral("<td></td>")));
}

// H2 v2 专有：行有单元格但所有单元格均无文本（OCR 未映射到任何文本）-> 过滤。
// 这是 SLANet 「伪空首行」的典型形态：含占位 cell 但无文字内容，v1 不过滤、v2 过滤。
TEST(ut_HtmlTableBuilder, rowWithCellsButNoTextIsFiltered)
{
    // 第 0 行有单元格但无文本（空 text），第 1 行有文本内容。
    QList<DTableCell> cells;
    DTableCell phantom;
    phantom.row = 0;
    phantom.col = 0;
    phantom.text = QString();   // 占位 cell，无 OCR 文本
    DTableCell real;
    real.row = 1;
    real.col = 0;
    real.text = QStringLiteral("A");
    cells << phantom << real;

    const QString html = HtmlTableBuilder::build(cells);
    // 伪空首行被过滤，仅第 1 行保留。
    EXPECT_EQ(html.count(QStringLiteral("<tr>")), 1);
    EXPECT_TRUE(html.contains(QStringLiteral(">A</td>")));
    EXPECT_FALSE(html.contains(QStringLiteral("<td></td>")));
}

// 通用：有内容的行中若某列为空（占位 -1），该空单元格应保留为 <td></td>；
// 只有整行无文本时才过滤。确保"空单元格"与"空行"两个概念不被混淆。
TEST(ut_HtmlTableBuilder, emptyCellWithinContentRowIsPreserved)
{
    // 第 0 行第 0 列有内容，第 0 行第 1 列无单元格 -> 行有文本，保留空单元格。
    QList<DTableCell> cells;
    DTableCell a;
    a.row = 0;
    a.col = 0;
    a.text = QStringLiteral("A");
    DTableCell b;
    b.row = 1;
    b.col = 0;
    b.text = QStringLiteral("B");
    DTableCell d;
    d.row = 1;
    d.col = 1;
    d.text = QStringLiteral("D");
    cells << a << b << d;

    const QString html = HtmlTableBuilder::build(cells);
    // 两行均有文本，均保留。
    EXPECT_EQ(html.count(QStringLiteral("<tr>")), 2);
    // 第 0 行第 1 列为空单元格，应保留为 <td></td>。
    EXPECT_TRUE(html.contains(QStringLiteral("<td></td>")));
    EXPECT_TRUE(html.contains(QStringLiteral(">A</td>")));
}
