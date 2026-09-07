// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "../src/tablerecognizer/Img2TableFallback.h"
#include "../src/tablerecognizer/tabletypes.h"

#include <gtest/gtest.h>

#include <QColor>
#include <QImage>
#include <QPainter>
#include <QPen>

D_TABLERECOGNIZER_USE_NAMESPACE

static QImage makeWiredTableImage(int w, int h)
{
    QImage img(w, h, QImage::Format_RGB32);
    img.fill(Qt::white);
    QPainter painter(&img);
    QPen pen(Qt::black);
    pen.setWidth(2);
    painter.setPen(pen);

    // 3 行 x 3 列网格线。
    const int rows = 3;
    const int cols = 3;
    for (int r = 0; r <= rows; ++r) {
        const int y = h * r / rows;
        painter.drawLine(0, y, w, y);
    }
    for (int c = 0; c <= cols; ++c) {
        const int x = w * c / cols;
        painter.drawLine(x, 0, x, h);
    }
    return img;
}

TEST(ut_Img2TableFallback, detectsWiredGrid)
{
    Img2TableFallback fallback;
    QImage image = makeWiredTableImage(300, 300);
    QList<DetectedCell> cells;
    QString error;
    ASSERT_TRUE(fallback.detect(image, cells, error)) << error.toStdString();
    EXPECT_FALSE(cells.isEmpty());
}

TEST(ut_Img2TableFallback, failsOnBlankImage)
{
    Img2TableFallback fallback;
    QImage image(200, 200, QImage::Format_RGB32);
    image.fill(Qt::white);
    QList<DetectedCell> cells;
    QString error;
    EXPECT_FALSE(fallback.detect(image, cells, error));
    EXPECT_FALSE(error.isEmpty());
}
