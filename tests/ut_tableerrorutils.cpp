// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "../src/tablerecognizer/TableErrorUtils.h"

#include <gtest/gtest.h>

D_TABLERECOGNIZER_USE_NAMESPACE

// 诊断信息格式契约： "<stage>: <what> (key=value; key=value)"。
// 该格式是日志检索的约定（stage/key 固定，value 任意），不属于对外 API。

TEST(ut_TableErrorUtils, formatsStageAndWhatWithoutFields)
{
    EXPECT_EQ(TableErrorUtils::detail(QStringLiteral("pipeline"),
                                      QStringLiteral("rejected: a recognition task is already running")),
              QStringLiteral("pipeline: rejected: a recognition task is already running"));
}

TEST(ut_TableErrorUtils, formatsFieldsInOrder)
{
    const QString message = TableErrorUtils::detail(
        QStringLiteral("img2table"), QStringLiteral("no table lines detected"),
        {{QStringLiteral("h_lines"), QStringLiteral("1")},
         {QStringLiteral("v_lines"), QStringLiteral("0")},
         {QStringLiteral("image"), QStringLiteral("800x400")}});
    EXPECT_EQ(message,
              QStringLiteral("img2table: no table lines detected (h_lines=1; v_lines=0; image=800x400)"));
}

TEST(ut_TableErrorUtils, emptyFieldListKeepsMessageCompact)
{
    const QString message = TableErrorUtils::detail(QStringLiteral("ocr"), QStringLiteral("no text detected"),
                                                    {});
    EXPECT_EQ(message, QStringLiteral("ocr: no text detected"));
    EXPECT_EQ(message.count(QLatin1Char('(')), 0);
}

TEST(ut_TableErrorUtils, messageIsAsciiOnly)
{
    const QString message = TableErrorUtils::detail(
        QStringLiteral("slanet"), QStringLiteral("no table detected"),
        {{QStringLiteral("model"), QStringLiteral("/usr/share/libdtk6tablerecognizer/models/SLANet_plus.onnx")}});
    for (const QChar &c : message)
        EXPECT_LE(c.unicode(), 0x7F) << "non-ASCII char in diagnostic message";
}

// 字段值里可能夹带模型路径/第三方引擎报错等不可控文本（含中文），
// 直接拼接会破坏 ASCII-only 契约：必须在 detail() 内收敛为纯 ASCII。
TEST(ut_TableErrorUtils, sanitizesNonAsciiFieldValues)
{
    // 用码点显式构造非 ASCII 文本（源码本身保持纯 ASCII）。
    const QString nonAsciiPath = QStringLiteral("/home/") + QChar(0x7528) + QChar(0x6237)
                                 + QStringLiteral("/models/SLANet_plus.onnx");
    const QString message = TableErrorUtils::detail(
        QStringLiteral("slanet"), QStringLiteral("unavailable: model not loaded"),
        {{QStringLiteral("model"), nonAsciiPath}});
    for (const QChar &c : message)
        EXPECT_LE(c.unicode(), 0x7F) << "non-ASCII char in diagnostic message";
    EXPECT_TRUE(message.contains(QStringLiteral("model=/home/")));
}

// 控制字符（换行/制表）同样要收敛，避免诊断文本被注入换行破坏日志结构。
TEST(ut_TableErrorUtils, sanitizesControlCharacters)
{
    const QString message = TableErrorUtils::detail(
        QStringLiteral("ocr"), QStringLiteral("failed"),
        {{QStringLiteral("engine_error"), QStringLiteral("line1\nline2\tend")}});
    EXPECT_FALSE(message.contains(QLatin1Char('\n')));
    EXPECT_FALSE(message.contains(QLatin1Char('\t')));
}
