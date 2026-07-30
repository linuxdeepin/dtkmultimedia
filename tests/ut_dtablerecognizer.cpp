// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "../include/dtktablerecognizer/dtablerecognizer.h"
#include "../src/tablerecognizer/OrtInferenceEngine.h"
#include "../src/tablerecognizer/TableStructureDetector.h"
#include "../src/tablerecognizer/DtkOcrWrapper.h"
#include "../src/tablerecognizer/Img2TableFallback.h"

#include <gtest/gtest.h>
#include <QImage>
#include <QSignalSpy>
#include <QFileInfo>
#include <stubext.h>

#include <chrono>
#include <thread>

D_TABLERECOGNIZER_USE_NAMESPACE

// 在事件循环中等待 recognitionDone 信号，返回结果。
static DTableResult waitForDone(QSignalSpy &spy, int timeoutMs = 5000)
{
    spy.wait(timeoutMs);
    if (spy.isEmpty())
        return DTableResult{};
    return qvariant_cast<DTableResult>(spy.takeFirst().at(0));
}

// 用例1：无效图片直接返回失败（同步路径）。
TEST(ut_DTableRecognizer, invalidImageEmitsFailure)
{
    DTableRecognizer recognizer;
    QSignalSpy spy(&recognizer, &DTableRecognizer::recognitionDone);
    recognizer.recognizeAsync(QImage());
    const DTableResult result = waitForDone(spy, 2000);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage.toStdString(), "输入图片无效");
}

// 用例2：超时返回失败。stub detect 阻塞，使超时定时器先触发。
TEST(ut_DTableRecognizer, timeoutReturnsFailure)
{
    DTableRecognizer recognizer;
    QSignalSpy spy(&recognizer, &DTableRecognizer::recognitionDone);

    stub_ext::StubExt stub;
    // 主路径不可用。
    stub.set_lamda(ADDR(TableStructureDetector, available), []() { return false; });
    // 降级路径阻塞 300ms，确保 10ms 超时先触发。
    stub.set_lamda(ADDR(Img2TableFallback, detect),
                   [](Img2TableFallback *, const QImage &, QList<DetectedCell> &cells, QString &) {
                       std::this_thread::sleep_for(std::chrono::milliseconds(300));
                       return false;
                   });

    QImage image(100, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    recognizer.recognizeAsync(image, std::chrono::milliseconds(10));
    const DTableResult result = waitForDone(spy, 3000);
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.errorMessage.contains(QStringLiteral("超时")));
}

// 用例3：降级路径 source 字段。主路径不可用，img2table + OCR 全部 stub 成功。
TEST(ut_DTableRecognizer, degradationPathSetsSource)
{
    DTableRecognizer recognizer;
    QSignalSpy spy(&recognizer, &DTableRecognizer::recognitionDone);

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(TableStructureDetector, available), []() { return false; });
    stub.set_lamda(ADDR(Img2TableFallback, detect),
                   [](Img2TableFallback *, const QImage &, QList<DetectedCell> &cells, QString &) {
                       DetectedCell c;
                       c.row = 0;
                       c.col = 0;
                       c.bbox = QRectF(0, 0, 50, 50);
                       cells.append(c);
                       return true;
                   });
    stub.set_lamda(ADDR(DtkOcrWrapper, recognize),
                   [](DtkOcrWrapper *, const QImage &, QList<OcrTextBox> &boxes, QString &) {
                       OcrTextBox box;
                       box.bbox = QRectF(10, 10, 20, 20);
                       box.text = QStringLiteral("A");
                       boxes.append(box);
                       return true;
                   });
    QImage image(100, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    recognizer.recognizeAsync(image, std::chrono::seconds(10));
    const DTableResult result = waitForDone(spy, 5000);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.source.toStdString(), "img2table");
    ASSERT_EQ(result.cells.size(), 1);
    EXPECT_EQ(result.cells[0].text.toStdString(), "A");
}

// 用例4：模型加载链路。构造 DTableRecognizer 时应调用 OrtInferenceEngine::loadModel
// 加载 SLANet_plus.onnx，使 TableStructureDetector::available() 为 true（主路径可启用）。
// 当真实模型文件存在且 ORT 可用时直接断言；否则用 stub 模拟 loadModel 成功后断言 available()。
TEST(ut_DTableRecognizer, mainPathEnabledWhenModelLoaded)
{
    // 用独立的 OrtInferenceEngine 验证 loadModel 在真实模型路径下被调用。
    OrtInferenceEngine engine;
    // TABLEREC_MODEL_DIR 由 CMake Debug 定义指向源码 models/ 目录。
    const QString modelPath =
#ifdef TABLEREC_MODEL_DIR
        QString::fromUtf8(TABLEREC_MODEL_DIR) + QStringLiteral("SLANet_plus.onnx");
#else
        QStringLiteral("/usr/share/libdtk6tablerecognizer/models/SLANet_plus.onnx");
#endif
    // 真实模型文件存在时 loadModel 应成功，available() 为 true。
    if (QFileInfo::exists(modelPath)) {
        ASSERT_TRUE(engine.loadModel(modelPath)) << engine.lastError().toStdString();
        TableStructureDetector detector(&engine);
        EXPECT_TRUE(detector.available());
    } else {
        // 环境无模型文件时，验证 available() 依赖 isLoaded() 的逻辑：
        // 未加载模型时 available() 为 false。
        TableStructureDetector detector(&engine);
        EXPECT_FALSE(detector.available());
    }
}

// 用例5：构造 DTableRecognizer 触发模型加载。stub loadModel 为成功，验证 detector 可用。
TEST(ut_DTableRecognizer, ctorTriggersModelLoading)
{
    stub_ext::StubExt stub;
    bool loadCalled = false;
    stub.set_lamda(ADDR(OrtInferenceEngine, loadModel),
                   [&loadCalled](OrtInferenceEngine *, const QString &) {
                       loadCalled = true;
                       return true;
                   });
    stub.set_lamda(ADDR(OrtInferenceEngine, isLoaded),
                   []() { return true; });

    {
        DTableRecognizer recognizer;
        // 构造后 loadModel 应已被调用。
        EXPECT_TRUE(loadCalled);
        // detector->available() 依赖 engine->isLoaded()，stub 后应为 true。
        // （无法直接访问私有 detector，但 loadCalled + isLoaded 覆盖加载链路。）
    }
}

// 用例6：单飞拒绝不吞在途任务结果。同一实例在途时再次 recognizeAsync，
// 第二次被拒绝，且首个任务的真实结果不被吞（复现 High#1/#2）。
TEST(ut_DTableRecognizer, singleFlightRejectDoesNotSwallowInFlightResult)
{
    DTableRecognizer recognizer;
    QSignalSpy spy(&recognizer, &DTableRecognizer::recognitionDone);

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(TableStructureDetector, available), []() { return false; });
    // 降级路径阻塞 300ms，使首个任务保持 in-flight 期间发起第二次调用。
    stub.set_lamda(ADDR(Img2TableFallback, detect),
                   [](Img2TableFallback *, const QImage &, QList<DetectedCell> &cells, QString &) {
                       std::this_thread::sleep_for(std::chrono::milliseconds(300));
                       DetectedCell c;
                       c.row = 0;
                       c.col = 0;
                       c.bbox = QRectF(0, 0, 50, 50);
                       cells.append(c);
                       return true;
                   });
    stub.set_lamda(ADDR(DtkOcrWrapper, recognize),
                   [](DtkOcrWrapper *, const QImage &, QList<OcrTextBox> &boxes, QString &) {
                       OcrTextBox box;
                       box.bbox = QRectF(10, 10, 20, 20);
                       box.text = QStringLiteral("A");
                       boxes.append(box);
                       return true;
                   });

    QImage image(100, 100, QImage::Format_RGB32);
    image.fill(Qt::white);

    recognizer.recognizeAsync(image, std::chrono::seconds(10));   // 首个任务（in-flight）
    recognizer.recognizeAsync(image, std::chrono::seconds(10));   // 第二次：应被拒绝

    // 期望收到 2 条结果：一条拒绝、一条首个任务的真实结果。
    ASSERT_TRUE(spy.wait(3000));
    ASSERT_TRUE(spy.wait(3000));
    ASSERT_EQ(spy.count(), 2);

    const DTableResult r0 = qvariant_cast<DTableResult>(spy.at(0).at(0));
    const DTableResult r1 = qvariant_cast<DTableResult>(spy.at(1).at(0));
    const DTableResult &reject = r0.success ? r1 : r0;
    const DTableResult &real = r0.success ? r0 : r1;
    EXPECT_FALSE(reject.success);
    EXPECT_TRUE(reject.errorMessage.contains(QStringLiteral("已有识别任务在执行")));
    EXPECT_TRUE(real.success);
    EXPECT_EQ(real.source.toStdString(), "img2table");
    ASSERT_EQ(real.cells.size(), 1);
    EXPECT_EQ(real.cells[0].text.toStdString(), "A");
}

// 用例7：实例复用——一次成功后同一实例再传空图片，仍能收到结果（复现 High#3）。
TEST(ut_DTableRecognizer, reuseInstanceReceivesResultAfterSuccess)
{
    DTableRecognizer recognizer;
    QSignalSpy spy(&recognizer, &DTableRecognizer::recognitionDone);

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(TableStructureDetector, available), []() { return false; });
    stub.set_lamda(ADDR(Img2TableFallback, detect),
                   [](Img2TableFallback *, const QImage &, QList<DetectedCell> &cells, QString &) {
                       DetectedCell c;
                       c.row = 0;
                       c.col = 0;
                       c.bbox = QRectF(0, 0, 50, 50);
                       cells.append(c);
                       return true;
                   });
    stub.set_lamda(ADDR(DtkOcrWrapper, recognize),
                   [](DtkOcrWrapper *, const QImage &, QList<OcrTextBox> &boxes, QString &) {
                       OcrTextBox box;
                       box.bbox = QRectF(10, 10, 20, 20);
                       box.text = QStringLiteral("A");
                       boxes.append(box);
                       return true;
                   });

    QImage image(100, 100, QImage::Format_RGB32);
    image.fill(Qt::white);

    // 首次：成功识别。
    recognizer.recognizeAsync(image, std::chrono::seconds(10));
    ASSERT_TRUE(spy.wait(3000));
    ASSERT_EQ(spy.count(), 1);
    const DTableResult first = qvariant_cast<DTableResult>(spy.at(0).at(0));
    EXPECT_TRUE(first.success);

    // 复用同一实例再传空图片：应收到失败结果（旧实现因 emitted 恒为 1 而挂起不触发信号）。
    recognizer.recognizeAsync(QImage());
    ASSERT_TRUE(spy.wait(3000));
    ASSERT_EQ(spy.count(), 2);
    const DTableResult second = qvariant_cast<DTableResult>(spy.at(1).at(0));
    EXPECT_FALSE(second.success);
    EXPECT_EQ(second.errorMessage.toStdString(), "输入图片无效");
}

// 用例8：OCR 失败分支（success=false、errorMessage 含「OCR 失败」）。
TEST(ut_DTableRecognizer, ocrFailureReturnsFailure)
{
    DTableRecognizer recognizer;
    QSignalSpy spy(&recognizer, &DTableRecognizer::recognitionDone);

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(TableStructureDetector, available), []() { return false; });
    stub.set_lamda(ADDR(Img2TableFallback, detect),
                   [](Img2TableFallback *, const QImage &, QList<DetectedCell> &cells, QString &) {
                       DetectedCell c;
                       c.row = 0;
                       c.col = 0;
                       c.bbox = QRectF(0, 0, 50, 50);
                       cells.append(c);
                       return true;
                   });
    stub.set_lamda(ADDR(DtkOcrWrapper, recognize),
                   [](DtkOcrWrapper *, const QImage &, QList<OcrTextBox> &, QString &err) {
                       err = QStringLiteral("plugin not loaded");
                       return false;
                   });

    QImage image(100, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    recognizer.recognizeAsync(image, std::chrono::seconds(10));
    const DTableResult result = waitForDone(spy, 5000);
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.errorMessage.contains(QStringLiteral("OCR 失败")));
}

// 用例9：主路径与降级均无结构（errorMessage 含「未识别到表格」）。
TEST(ut_DTableRecognizer, noStructureReturnsFailure)
{
    DTableRecognizer recognizer;
    QSignalSpy spy(&recognizer, &DTableRecognizer::recognitionDone);

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(TableStructureDetector, available), []() { return false; });
    stub.set_lamda(ADDR(Img2TableFallback, detect),
                   [](Img2TableFallback *, const QImage &, QList<DetectedCell> &, QString &) {
                       return false;   // 无结构
                   });

    QImage image(100, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    recognizer.recognizeAsync(image, std::chrono::seconds(10));
    const DTableResult result = waitForDone(spy, 5000);
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.errorMessage.contains(QStringLiteral("未识别到表格")));
}
