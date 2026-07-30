// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "../src/tablerecognizer/OrtInferenceEngine.h"

#include <gtest/gtest.h>

D_TABLERECOGNIZER_USE_NAMESPACE

// 验证：空路径加载失败且记录错误。
TEST(ut_OrtInferenceEngine, loadEmptyPathFails)
{
    OrtInferenceEngine engine;
    EXPECT_FALSE(engine.loadModel(QString()));
    EXPECT_FALSE(engine.lastError().isEmpty());
    EXPECT_FALSE(engine.isLoaded());
}

// 验证：不存在的模型文件加载失败且记录错误。
TEST(ut_OrtInferenceEngine, loadNonexistentFails)
{
    OrtInferenceEngine engine;
    EXPECT_FALSE(engine.loadModel(QStringLiteral("/nonexistent/SLANet_plus.onnx")));
    EXPECT_FALSE(engine.lastError().isEmpty());
    EXPECT_FALSE(engine.isLoaded());
}

// 验证：未加载模型时 run 返回空且记录错误。
TEST(ut_OrtInferenceEngine, runBeforeLoadReturnsEmpty)
{
    OrtInferenceEngine engine;
    const std::vector<float> input(3, 1.0f);
    const std::vector<int64_t> shape = {1, 3, 1, 1};
    auto out = engine.run(input, shape);
    EXPECT_TRUE(out.empty());
    EXPECT_FALSE(engine.lastError().isEmpty());
}
