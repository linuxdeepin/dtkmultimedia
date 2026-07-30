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
