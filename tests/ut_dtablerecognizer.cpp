// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "../include/dtktablerecognizer/dtablerecognizer.h"
#include "../src/tablerecognizer/OrtInferenceEngine.h"
#include "../src/tablerecognizer/TableStructureDetector.h"
#include "../src/tablerecognizer/DtkOcrWrapper.h"
#include "../src/tablerecognizer/Img2TableFallback.h"
#include "../src/tablerecognizer/dtablerecognizer_p.h"

#include <gtest/gtest.h>
#include <QImage>
#include <QSignalSpy>
#include <QFileInfo>
#include <QPainter>
#include <QPen>
#include <QColor>
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

// 诊断信息（errorMessage）必须为纯 ASCII：库内不再出现中文/本地化文案。
static bool isAsciiOnly(const QString &text)
{
    for (const QChar &c : text) {
        if (c.unicode() > 0x7F)
            return false;
    }
    return true;
}

// 生成带清晰网格线的有线表图（高线密度），供需要「非弱线」场景的用例使用。
static QImage makeWiredGridImage(int w, int h)
{
    QImage img(w, h, QImage::Format_RGB32);
    img.fill(Qt::white);
    QPainter painter(&img);
    QPen pen(Qt::black);
    pen.setWidth(2);
    painter.setPen(pen);
    for (int r = 0; r <= 3; ++r) {   // 3 行网格线
        const int y = h * r / 3;
        painter.drawLine(0, y, w, y);
    }
    for (int c = 0; c <= 3; ++c) {   // 3 列网格线
        const int x = w * c / 3;
        painter.drawLine(x, 0, x, h);
    }
    return img;
}

// 用例1：无效图片直接返回失败（同步路径）。
TEST(ut_DTableRecognizer, invalidImageEmitsFailure)
{
    DTableRecognizer recognizer;
    QSignalSpy spy(&recognizer, &DTableRecognizer::recognitionDone);
    recognizer.recognizeAsync(QImage());
    const DTableResult result = waitForDone(spy, 2000);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, TableError::InvalidImage);
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
                   [](Img2TableFallback *, const QImage &, QList<DetectedCell> &, QString &) {
                       std::this_thread::sleep_for(std::chrono::milliseconds(300));
                       return false;
                   });

    QImage image(100, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    recognizer.recognizeAsync(image, std::chrono::milliseconds(10));
    const DTableResult result = waitForDone(spy, 3000);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, TableError::Timeout);
    // 诊断信息必须带上下文（阶段 + 超时预算），而不是一句笼统文案。
    EXPECT_TRUE(result.errorMessage.startsWith(QStringLiteral("timeout: ")));
    EXPECT_TRUE(result.errorMessage.contains(QStringLiteral("budget_ms=")));
}

// 用例3a：竞态修复——超时后旧工作线程未退出前，busy 锁不释放，新任务被单飞
// 守卫拒绝；旧线程退出后不投递过期结果。修复前超时直接释放 busy，新任务进入并
// 重置 emitted，旧线程投递过期结果导致状态混乱。
TEST(ut_DTableRecognizer, timeoutHoldsBusyUntilWorkerExits)
{
    DTableRecognizer recognizer;
    QSignalSpy spy(&recognizer, &DTableRecognizer::recognitionDone);

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(TableStructureDetector, available), []() { return false; });
    // 降级路径阻塞 500ms，确保 10ms 超时先触发，且旧工作线程仍在运行时发起第二次调用。
    stub.set_lamda(ADDR(Img2TableFallback, detect),
                   [](Img2TableFallback *, const QImage &, QList<DetectedCell> &, QString &) {
                       std::this_thread::sleep_for(std::chrono::milliseconds(500));
                       return false;
                   });

    QImage image(100, 100, QImage::Format_RGB32);
    image.fill(Qt::white);

    // 第一次：10ms 超时，应收到超时失败结果。超时仅置标志不释放 busy 锁。
    recognizer.recognizeAsync(image, std::chrono::milliseconds(10));
    ASSERT_TRUE(spy.wait(2000));
    ASSERT_EQ(spy.count(), 1);
    const DTableResult first = qvariant_cast<DTableResult>(spy.takeFirst().at(0));
    EXPECT_FALSE(first.success);
    EXPECT_EQ(first.error, TableError::Timeout);

    // 超时后立即发起第二次调用——旧工作线程仍在运行（阻塞 500ms），
    // busy 锁未释放，单飞守卫应拒绝而非启动新任务。
    recognizer.recognizeAsync(image, std::chrono::milliseconds(10));
    ASSERT_TRUE(spy.wait(2000));
    ASSERT_EQ(spy.count(), 1);
    const DTableResult second = qvariant_cast<DTableResult>(spy.takeFirst().at(0));
    EXPECT_FALSE(second.success);
    EXPECT_EQ(second.error, TableError::Busy);

    // 等待旧工作线程退出（~500ms），确认不再产生过期结果信号。
    EXPECT_FALSE(spy.wait(1000));
    EXPECT_EQ(spy.count(), 0);
}

// 用例3b：竞态修复——工作线程提前完成后旧超时定时器不误投递。任务 A 工作线程
// 提前完成并释放 busy，任务 B 进入重置 emitted；任务 A 的超时定时器仍可能触发，
// 通过 taskId 比对丢弃过期定时器，避免向任务 B 错误投递超时结果。
TEST(ut_DTableRecognizer, staleTimeoutTimerFilteredByTaskId)
{
    DTableRecognizer recognizer;
    QSignalSpy spy(&recognizer, &DTableRecognizer::recognitionDone);

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(TableStructureDetector, available), []() { return false; });
    // 降级路径立即返回失败，使任务 A 工作线程提前完成。
    stub.set_lamda(ADDR(Img2TableFallback, detect),
                   [](Img2TableFallback *, const QImage &, QList<DetectedCell> &, QString &) {
                       return false;   // 无结构，立即返回
                   });

    QImage image(100, 100, QImage::Format_RGB32);
    image.fill(Qt::white);

    // 任务 A：长超时（2000ms），工作线程立即完成并投递 NoTableDetected 失败。
    recognizer.recognizeAsync(image, std::chrono::milliseconds(2000));
    ASSERT_TRUE(spy.wait(3000));
    ASSERT_EQ(spy.count(), 1);
    const DTableResult first = qvariant_cast<DTableResult>(spy.takeFirst().at(0));
    EXPECT_FALSE(first.success);
    EXPECT_EQ(first.error, TableError::NoTableDetected);

    // 任务 A 完成后 busy 已释放，任务 B 进入（重置 emitted/timedOut/taskId）。
    // 任务 A 的 2000ms 超时定时器仍在 pending，届时应被 taskId 过滤。
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
                       box.text = QStringLiteral("B");
                       boxes.append(box);
                       return true;
                   });

    // 任务 B：短超时（100ms），工作线程很快完成。
    recognizer.recognizeAsync(image, std::chrono::milliseconds(100));
    ASSERT_TRUE(spy.wait(3000));
    ASSERT_EQ(spy.count(), 1);
    const DTableResult second = qvariant_cast<DTableResult>(spy.takeFirst().at(0));
    // 任务 B 应成功（img2table + OCR），不应被任务 A 的过期超时定时器误投递为超时。
    EXPECT_TRUE(second.success);
    EXPECT_EQ(second.source.toStdString(), "img2table");
    ASSERT_EQ(second.cells.size(), 1);
    EXPECT_EQ(second.cells[0].text.toStdString(), "B");
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
    EXPECT_EQ(result.error, TableError::None);
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
    EXPECT_EQ(reject.error, TableError::Busy);
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
    EXPECT_EQ(second.error, TableError::InvalidImage);
}

// 用例8：OCR 失败分支（success=false、error 为 OcrFailed）。
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
                       err = QStringLiteral("ocr: analyze failed (engine=PPOCR_V5; boxes=0)");
                       return false;
                   });
    stub.set_lamda(ADDR(DtkOcrWrapper, lastFailure),
                   []() { return DtkOcrWrapper::Failure::EngineFailed; });

    QImage image(100, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    recognizer.recognizeAsync(image, std::chrono::seconds(10));
    const DTableResult result = waitForDone(spy, 5000);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, TableError::OcrFailed);
}

// 用例9：主路径与降级均无结构（error 为 NoTableDetected），且诊断信息带各路径尝试记录。
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
    EXPECT_EQ(result.error, TableError::NoTableDetected);
    EXPECT_TRUE(result.errorMessage.startsWith(QStringLiteral("structure: ")));
    // 审查意见 6：不能只断言存在 attempts= 键（空 attempts 也能通过），
    // 必须断言各路径的真实失败原因被记录进 attempts。
    EXPECT_TRUE(result.errorMessage.contains(QStringLiteral("attempts=slanet: ")));
    EXPECT_TRUE(result.errorMessage.contains(QStringLiteral("unavailable: model not loaded")));
}

// ===== 错误码细分：OCR 语义 =====

// 「有结构、无文字」：不再混成 OcrFailed，且结构（cells/html）必须保留。
TEST(ut_DTableRecognizer, noTextDetectedKeepsStructure)
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
                       err = QStringLiteral("ocr: no text detected (engine=PPOCR_V5; boxes=0)");
                       return false;
                   });
    stub.set_lamda(ADDR(DtkOcrWrapper, lastFailure),
                   []() { return DtkOcrWrapper::Failure::NoTextDetected; });

    QImage image(100, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    recognizer.recognizeAsync(image, std::chrono::seconds(10));
    const DTableResult result = waitForDone(spy, 5000);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, TableError::NoTextDetected);
    EXPECT_EQ(result.source.toStdString(), "img2table");
    // 结构不能因为文字缺失被丢掉。
    ASSERT_EQ(result.cells.size(), 1);
    EXPECT_FALSE(result.html.isEmpty());
    EXPECT_TRUE(result.errorMessage.startsWith(QStringLiteral("ocr: ")));
    EXPECT_TRUE(result.errorMessage.contains(QStringLiteral("no text recognized")));
    EXPECT_TRUE(isAsciiOnly(result.errorMessage)) << result.errorMessage.toStdString();
}

// OCR 插件不可用：环境问题，独立错误码（与「引擎执行报错」区分）。
TEST(ut_DTableRecognizer, ocrEngineUnavailableReported)
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
                       err = QStringLiteral("ocr: engine unavailable (plugin=PPOCR_V5; installed=none)");
                       return false;
                   });
    stub.set_lamda(ADDR(DtkOcrWrapper, lastFailure),
                   []() { return DtkOcrWrapper::Failure::PluginUnavailable; });

    QImage image(100, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    recognizer.recognizeAsync(image, std::chrono::seconds(10));
    const DTableResult result = waitForDone(spy, 5000);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, TableError::OcrEngineUnavailable);
    EXPECT_TRUE(result.errorMessage.contains(QStringLiteral("engine unavailable")));
    EXPECT_TRUE(isAsciiOnly(result.errorMessage)) << result.errorMessage.toStdString();
}

// ===== 错误码细分：主路径内部异常 =====

// 主路径 ORT 推理内部失败 + 降级路径无结构 → InternalError（不能断言「图里没有表格」）。
TEST(ut_DTableRecognizer, internalErrorWhenMainPathFailsInternally)
{
    DTableRecognizer recognizer;
    QSignalSpy spy(&recognizer, &DTableRecognizer::recognitionDone);

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(TableStructureDetector, available), []() { return true; });
    stub.set_lamda(ADDR(TableStructureDetector, detect),
                   [](TableStructureDetector *, const QImage &, QList<DetectedCell> &,
                      QString &err, float *confidence) {
                       err = QStringLiteral("slanet: ORT inference produced no output "
                                            "(engine_error=ORT inference failed: boom)");
                       if (confidence)
                           *confidence = 0.0f;
                       return false;
                   });
    stub.set_lamda(ADDR(TableStructureDetector, lastFailure),
                   []() { return TableStructureDetector::Failure::InferenceFailed; });
    // 提前 OCR 也失败，且降级路径无结构。
    stub.set_lamda(ADDR(DtkOcrWrapper, recognize),
                   [](DtkOcrWrapper *, const QImage &, QList<OcrTextBox> &, QString &err) {
                       err = QStringLiteral("ocr: analyze failed (engine=PPOCR_V5; boxes=0)");
                       return false;
                   });
    stub.set_lamda(ADDR(DtkOcrWrapper, lastFailure),
                   []() { return DtkOcrWrapper::Failure::EngineFailed; });
    stub.set_lamda(ADDR(Img2TableFallback, detect),
                   [](Img2TableFallback *, const QImage &, QList<DetectedCell> &, QString &err) {
                       err = QStringLiteral("img2table: no table lines detected (h_lines=0; v_lines=0)");
                       return false;
                   });

    QImage image(100, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    recognizer.recognizeAsync(image, std::chrono::seconds(10));
    const DTableResult result = waitForDone(spy, 5000);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, TableError::InternalError);
    EXPECT_TRUE(result.errorMessage.contains(QStringLiteral("failed internally")));
    // 各路径的尝试记录都要保留，便于定位。
    EXPECT_TRUE(result.errorMessage.contains(QStringLiteral("slanet: ")));
    EXPECT_TRUE(result.errorMessage.contains(QStringLiteral("img2table: ")));
    EXPECT_TRUE(isAsciiOnly(result.errorMessage)) << result.errorMessage.toStdString();
}

// 降级路径内部处理失败（QImage->cv::Mat 转换失败）：不能断言「图里没有表格」，
// 必须上报 InternalError——否则会把内部错误伪装成「确实没有表格」。
TEST(ut_DTableRecognizer, internalErrorWhenFallbackConversionFails)
{
    DTableRecognizer recognizer;
    QSignalSpy spy(&recognizer, &DTableRecognizer::recognitionDone);

    stub_ext::StubExt stub;
    // 主路径不可用（模型未加载），只剩降级路径。
    stub.set_lamda(ADDR(TableStructureDetector, available), []() { return false; });
    // 降级路径失败，且原因是内部处理失败（转换失败）而非「确实没有表格」。
    stub.set_lamda(ADDR(Img2TableFallback, detect),
                   [](Img2TableFallback *, const QImage &, QList<DetectedCell> &, QString &err) {
                       err = QStringLiteral("img2table: failed to convert image (image=100x100)");
                       return false;
                   });
    stub.set_lamda(ADDR(Img2TableFallback, lastFailure),
                   []() { return Img2TableFallback::Failure::ConversionFailed; });

    QImage image(100, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    recognizer.recognizeAsync(image, std::chrono::seconds(10));
    const DTableResult result = waitForDone(spy, 5000);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, TableError::InternalError);
    EXPECT_TRUE(result.errorMessage.contains(QStringLiteral("failed internally")));
    EXPECT_TRUE(isAsciiOnly(result.errorMessage)) << result.errorMessage.toStdString();
}

// SLANet 预处理失败 + 降级无结构：属于内部异常，必须 InternalError（此前漏判 PreprocessFailed）。
TEST(ut_DTableRecognizer, internalErrorWhenSlanetPreprocessFails)
{
    DTableRecognizer recognizer;
    QSignalSpy spy(&recognizer, &DTableRecognizer::recognitionDone);

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(TableStructureDetector, available), []() { return true; });
    stub.set_lamda(ADDR(TableStructureDetector, detect),
                   [](TableStructureDetector *, const QImage &, QList<DetectedCell> &,
                      QString &err, float *confidence) {
                       err = QStringLiteral("slanet: preprocess failed (resize)");
                       if (confidence)
                           *confidence = 0.0f;
                       return false;
                   });
    stub.set_lamda(ADDR(TableStructureDetector, lastFailure),
                   []() { return TableStructureDetector::Failure::PreprocessFailed; });
    stub.set_lamda(ADDR(DtkOcrWrapper, recognize),
                   [](DtkOcrWrapper *, const QImage &, QList<OcrTextBox> &, QString &err) {
                       err = QStringLiteral("ocr: analyze failed");
                       return false;
                   });
    stub.set_lamda(ADDR(DtkOcrWrapper, lastFailure),
                   []() { return DtkOcrWrapper::Failure::EngineFailed; });
    stub.set_lamda(ADDR(Img2TableFallback, detect),
                   [](Img2TableFallback *, const QImage &, QList<DetectedCell> &, QString &err) {
                       err = QStringLiteral("img2table: no table lines detected");
                       return false;
                   });

    QImage image(100, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    recognizer.recognizeAsync(image, std::chrono::seconds(10));
    const DTableResult result = waitForDone(spy, 5000);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, TableError::InternalError);
    EXPECT_TRUE(result.errorMessage.contains(QStringLiteral("failed internally")));
    EXPECT_TRUE(isAsciiOnly(result.errorMessage)) << result.errorMessage.toStdString();
}

// ===== 超时（deadline）语义：审查意见 1 / 5 =====
// 白盒：直接驱动 runPipeline，用「已越过 deadline」稳定复现工作线程侧的三个 no-structure /
// 已识别分支。不能说「靠公共信号」触发——外层超时定时器会先投递 stage=pipeline 的超时结果。

// 意见 1：结构路径越过 deadline 且最终无结构时，必须报 Timeout，而不是 NoTableDetected/InternalError。
TEST(ut_DTableRecognizer, expiredNoStructureReportsTimeout)
{
    DTableRecognizer recognizer;
    DTableRecognizerPrivate *priv = recognizer.findChild<DTableRecognizerPrivate *>();
    ASSERT_NE(priv, nullptr);

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(TableStructureDetector, available), []() { return true; });
    stub.set_lamda(ADDR(TableStructureDetector, detect),
                   [](TableStructureDetector *, const QImage &, QList<DetectedCell> &cells,
                      QString &err, float *confidence) {
                       std::this_thread::sleep_for(std::chrono::milliseconds(120));   // 越过 deadline
                       cells.clear();
                       err = QStringLiteral("slanet: no cells built");
                       if (confidence)
                           *confidence = 0.0f;
                       return false;
                   });
    stub.set_lamda(ADDR(TableStructureDetector, lastFailure),
                   []() { return TableStructureDetector::Failure::BadOutput; });
    stub.set_lamda(ADDR(DtkOcrWrapper, recognize),
                   [](DtkOcrWrapper *, const QImage &, QList<OcrTextBox> &, QString &err) {
                       err = QStringLiteral("ocr: analyze failed");
                       return false;
                   });
    stub.set_lamda(ADDR(DtkOcrWrapper, lastFailure),
                   []() { return DtkOcrWrapper::Failure::EngineFailed; });
    stub.set_lamda(ADDR(Img2TableFallback, detect),
                   [](Img2TableFallback *, const QImage &, QList<DetectedCell> &, QString &err) {
                       err = QStringLiteral("img2table: no table lines detected");
                       return false;
                   });

    QImage image(100, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(30);
    const DTableResult result = priv->runPipeline(image, deadline, 30);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, TableError::Timeout);   // 不是 NoTableDetected/InternalError
    EXPECT_TRUE(result.errorMessage.startsWith(QStringLiteral("timeout: ")));
    EXPECT_TRUE(result.errorMessage.contains(QStringLiteral("stage=structure")));
    EXPECT_TRUE(isAsciiOnly(result.errorMessage)) << result.errorMessage.toStdString();
}

// 意见 5：超时诊断必须保留当时的 source（此前先 clear 再取值，source 恒为空）。
TEST(ut_DTableRecognizer, timeoutDiagnosticKeepsSource)
{
    DTableRecognizer recognizer;
    DTableRecognizerPrivate *priv = recognizer.findChild<DTableRecognizerPrivate *>();
    ASSERT_NE(priv, nullptr);

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(TableStructureDetector, available), []() { return true; });
    stub.set_lamda(ADDR(TableStructureDetector, detect),
                   [](TableStructureDetector *, const QImage &, QList<DetectedCell> &cells,
                      QString &, float *confidence) {
                       std::this_thread::sleep_for(std::chrono::milliseconds(120));   // 越过 deadline
                       DetectedCell c0;
                       c0.row = 0;
                       c0.col = 0;
                       c0.bbox = QRectF(0, 0, 50, 100);
                       DetectedCell c1;
                       c1.row = 0;
                       c1.col = 1;
                       c1.bbox = QRectF(50, 0, 50, 100);
                       cells << c0 << c1;
                       if (confidence)
                           *confidence = 0.9f;
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

    QImage image(300, 300, QImage::Format_RGB32);
    image.fill(Qt::white);
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(30);
    const DTableResult result = priv->runPipeline(image, deadline, 30);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, TableError::Timeout);
    EXPECT_TRUE(result.errorMessage.contains(QStringLiteral("source=SLANet_plus")))
        << result.errorMessage.toStdString();
    EXPECT_TRUE(isAsciiOnly(result.errorMessage)) << result.errorMessage.toStdString();
}

// ===== M1：SLANet 置信度门集成 =====
// detect 返回带置信度的非空输出，runPipeline 依据置信度/cell 数决定是否信任主路径。

// M1-A：主路径输出「自信但低置信」（confidence < 阈值）仍触发降级到 img2table。
// 修复前仅在 !structOk || cells.isEmpty() 降级，此类非空低置信输出永不降级。
TEST(ut_DTableRecognizer, lowConfidenceNonEmptyOutputDegradesToImg2Table)
{
    DTableRecognizer recognizer;
    QSignalSpy spy(&recognizer, &DTableRecognizer::recognitionDone);

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(TableStructureDetector, available), []() { return true; });
    // 主路径「自信但错误」：返回 4 个非空单元格，但置信度 0.2 < 0.5 阈值。
    stub.set_lamda(ADDR(TableStructureDetector, detect),
                   [](TableStructureDetector *, const QImage &, QList<DetectedCell> &cells,
                      QString &, float *conf) {
                       for (int i = 0; i < 4; ++i) {
                           DetectedCell c;
                           c.row = 0;
                           c.col = i;
                           c.bbox = QRectF(i * 25, 0, 25, 100);
                           cells.append(c);
                       }
                       if (conf)
                           *conf = 0.2f;
                       return true;
                   });
    // 降级目标：img2table 返回 1 个单元格。
    stub.set_lamda(ADDR(Img2TableFallback, detect),
                   [](Img2TableFallback *, const QImage &, QList<DetectedCell> &cells, QString &) {
                       DetectedCell c;
                       c.row = 0;
                       c.col = 0;
                       c.bbox = QRectF(0, 0, 100, 100);
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

    const QImage image = makeWiredGridImage(300, 300);   // 有线表：隔离 M1 置信度门，避免 H3 weakLine 路由
    recognizer.recognizeAsync(image, std::chrono::seconds(10));
    const DTableResult result = waitForDone(spy, 5000);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.source.toStdString(), "img2table");
    ASSERT_EQ(result.cells.size(), 1);
    EXPECT_EQ(result.cells[0].text.toStdString(), "A");
}

// M1-B：高置信 + 合理 cell 数 → 信任主路径，source=SLANet_plus。
TEST(ut_DTableRecognizer, highConfidenceKeepsMainPath)
{
    DTableRecognizer recognizer;
    QSignalSpy spy(&recognizer, &DTableRecognizer::recognitionDone);

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(TableStructureDetector, available), []() { return true; });
    stub.set_lamda(ADDR(TableStructureDetector, detect),
                   [](TableStructureDetector *, const QImage &, QList<DetectedCell> &cells,
                      QString &, float *conf) {
                       DetectedCell c0;
                       c0.row = 0;
                       c0.col = 0;
                       c0.bbox = QRectF(0, 0, 50, 100);
                       DetectedCell c1;
                       c1.row = 0;
                       c1.col = 1;
                       c1.bbox = QRectF(50, 0, 50, 100);
                       cells << c0 << c1;
                       if (conf)
                           *conf = 0.9f;
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

    const QImage image = makeWiredGridImage(300, 300);   // 有线表：隔离 M1 置信度门，避免 H3 weakLine 路由
    recognizer.recognizeAsync(image, std::chrono::seconds(10));
    const DTableResult result = waitForDone(spy, 5000);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.source.toStdString(), "SLANet_plus");
    ASSERT_EQ(result.cells.size(), 2);
    EXPECT_EQ(result.cells[0].text.toStdString(), "A");
}

// M1-C：cell 数不合理（< 2）即使高置信也降级。
TEST(ut_DTableRecognizer, tooFewCellsDegradesEvenIfConfident)
{
    DTableRecognizer recognizer;
    QSignalSpy spy(&recognizer, &DTableRecognizer::recognitionDone);

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(TableStructureDetector, available), []() { return true; });
    stub.set_lamda(ADDR(TableStructureDetector, detect),
                   [](TableStructureDetector *, const QImage &, QList<DetectedCell> &cells,
                      QString &, float *conf) {
                       DetectedCell c;
                       c.row = 0;
                       c.col = 0;
                       c.bbox = QRectF(0, 0, 100, 100);
                       cells.append(c);
                       if (conf)
                           *conf = 0.9f;
                       return true;
                   });
    stub.set_lamda(ADDR(Img2TableFallback, detect),
                   [](Img2TableFallback *, const QImage &, QList<DetectedCell> &cells, QString &) {
                       DetectedCell c;
                       c.row = 0;
                       c.col = 0;
                       c.bbox = QRectF(0, 0, 100, 100);
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

    const QImage image = makeWiredGridImage(300, 300);   // 有线表：隔离 M1 cell 数门，避免 H3 weakLine 路由
    recognizer.recognizeAsync(image, std::chrono::seconds(10));
    const DTableResult result = waitForDone(spy, 5000);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.source.toStdString(), "img2table");
}

// ===== H3 v3：结构-内容一致性信号接入路由 =====

// H3 v3-A：一致（SLANet列==OCR列簇数）→ 信任 SLANet。
// SLANet 返回 2 列，OCR 文本框分属 2 个列簇（x 间距 > gapTolerance）→ 一致 → 留 SLANet。
TEST(ut_DTableRecognizer, consistentColumnsKeepsSlanet)
{
    DTableRecognizer recognizer;
    QSignalSpy spy(&recognizer, &DTableRecognizer::recognitionDone);

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(TableStructureDetector, available), []() { return true; });
    // SLANet 返回 2 列（col 0, col 1），高置信。
    stub.set_lamda(ADDR(TableStructureDetector, detect),
                   [](TableStructureDetector *, const QImage &, QList<DetectedCell> &cells,
                      QString &, float *conf) {
                       DetectedCell c0; c0.row = 0; c0.col = 0; c0.bbox = QRectF(0, 0, 50, 100);
                       DetectedCell c1; c1.row = 0; c1.col = 1; c1.bbox = QRectF(50, 0, 50, 100);
                       cells << c0 << c1;
                       if (conf) *conf = 0.95f;
                       return true;
                   });
    // OCR 返回 2 个文本框，x 间距大 → 2 个列簇（与 SLANet 一致）。
    stub.set_lamda(ADDR(DtkOcrWrapper, recognize),
                   [](DtkOcrWrapper *, const QImage &, QList<OcrTextBox> &boxes, QString &) {
                       OcrTextBox b0; b0.bbox = QRectF(5, 10, 30, 20);  b0.text = QStringLiteral("A");
                       OcrTextBox b1; b1.bbox = QRectF(60, 10, 30, 20);  b1.text = QStringLiteral("B");
                       boxes << b0 << b1;
                       return true;
                   });

    const QImage image = makeWiredGridImage(300, 300);
    recognizer.recognizeAsync(image, std::chrono::seconds(10));
    const DTableResult result = waitForDone(spy, 5000);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.source.toStdString(), "SLANet_plus");
    ASSERT_EQ(result.cells.size(), 2);
}

// H3 v3-B：粘列（SLANet列 < OCR列簇数）→ 降级 wireless 启发式。
// SLANet 返回 1 列（把 2 列粘成 1 列），OCR 文本框分属 2 个列簇 → 粘列 → wireless。
TEST(ut_DTableRecognizer, mergedColumnsRouteToWireless)
{
    DTableRecognizer recognizer;
    QSignalSpy spy(&recognizer, &DTableRecognizer::recognitionDone);

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(TableStructureDetector, available), []() { return true; });
    // SLANet 返回 1 列（col 0），高置信——但实际是 2 列被粘成 1 列。
    stub.set_lamda(ADDR(TableStructureDetector, detect),
                   [](TableStructureDetector *, const QImage &, QList<DetectedCell> &cells,
                      QString &, float *conf) {
                       DetectedCell c0; c0.row = 0; c0.col = 0; c0.bbox = QRectF(0, 0, 100, 100);
                       cells << c0;
                       if (conf) *conf = 0.95f;
                       return true;
                   });
    // OCR 返回 2 个文本框，x 间距大 → 2 个列簇（SLANet 1 < OCR 2 → 粘列）。
    stub.set_lamda(ADDR(DtkOcrWrapper, recognize),
                   [](DtkOcrWrapper *, const QImage &, QList<OcrTextBox> &boxes, QString &) {
                       OcrTextBox b0; b0.bbox = QRectF(5, 10, 30, 20);  b0.text = QStringLiteral("A");
                       OcrTextBox b1; b1.bbox = QRectF(60, 10, 30, 20);  b1.text = QStringLiteral("B");
                       boxes << b0 << b1;
                       return true;
                   });

    QImage image(300, 300, QImage::Format_RGB32);   // 空白图（无线表场景）
    image.fill(Qt::white);
    recognizer.recognizeAsync(image, std::chrono::seconds(10));
    const DTableResult result = waitForDone(spy, 5000);
    EXPECT_TRUE(result.success);
    // 粘列 → 降级 wireless（splitColumns(boxes, 5.0) = 2 列 → build 1 行 2 列）
    EXPECT_EQ(result.source.toStdString(), "wireless");
    ASSERT_EQ(result.cells.size(), 2);
    EXPECT_EQ(result.cells[0].text.toStdString(), "A");
    EXPECT_EQ(result.cells[1].text.toStdString(), "B");
}

// H3 v3-C：多列（SLANet列 > OCR列簇数）→ 保留 SLANet 不降级。
// SLANet 返回 3 列，OCR 文本框分属 2 个列簇 → 多列 → 保留 SLANet（AT 证回退更差）。
TEST(ut_DTableRecognizer, splitColumnsKeepsSlanet)
{
    DTableRecognizer recognizer;
    QSignalSpy spy(&recognizer, &DTableRecognizer::recognitionDone);

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(TableStructureDetector, available), []() { return true; });
    // SLANet 返回 3 列（多拆了列），高置信。
    stub.set_lamda(ADDR(TableStructureDetector, detect),
                   [](TableStructureDetector *, const QImage &, QList<DetectedCell> &cells,
                      QString &, float *conf) {
                       DetectedCell c0; c0.row = 0; c0.col = 0; c0.bbox = QRectF(0, 0, 33, 100);
                       DetectedCell c1; c1.row = 0; c1.col = 1; c1.bbox = QRectF(33, 0, 34, 100);
                       DetectedCell c2; c2.row = 0; c2.col = 2; c2.bbox = QRectF(67, 0, 33, 100);
                       cells << c0 << c1 << c2;
                       if (conf) *conf = 0.95f;
                       return true;
                   });
    // OCR 返回 2 个文本框 → 2 个列簇（SLANet 3 > OCR 2 → 多列）。
    stub.set_lamda(ADDR(DtkOcrWrapper, recognize),
                   [](DtkOcrWrapper *, const QImage &, QList<OcrTextBox> &boxes, QString &) {
                       OcrTextBox b0; b0.bbox = QRectF(5, 10, 30, 20);  b0.text = QStringLiteral("A");
                       OcrTextBox b1; b1.bbox = QRectF(60, 10, 30, 20);  b1.text = QStringLiteral("B");
                       boxes << b0 << b1;
                       return true;
                   });

    const QImage image = makeWiredGridImage(300, 300);
    recognizer.recognizeAsync(image, std::chrono::seconds(10));
    const DTableResult result = waitForDone(spy, 5000);
    EXPECT_TRUE(result.success);
    // 多列 → 保留 SLANet 不降级
    EXPECT_EQ(result.source.toStdString(), "SLANet_plus");
    ASSERT_EQ(result.cells.size(), 3);
}


// ===== 错误码契约 =====
// 库对外只承诺错误码（TableError）：成功必为 None、失败必为非 None；
// errorMessage 仅作诊断使用，必须为纯 ASCII（库内不再出现中文文案）。
TEST(ut_DTableRecognizer, errorCodeContract)
{
    DTableRecognizer recognizer;
    QSignalSpy spy(&recognizer, &DTableRecognizer::recognitionDone);

    // 空图片：InvalidImage，且诊断信息为纯 ASCII。
    recognizer.recognizeAsync(QImage());
    const DTableResult invalid = waitForDone(spy, 2000);
    EXPECT_FALSE(invalid.success);
    EXPECT_EQ(invalid.error, TableError::InvalidImage);
    EXPECT_TRUE(isAsciiOnly(invalid.errorMessage)) << invalid.errorMessage.toStdString();

    // 主路径与降级路径均无结构：NoTableDetected。
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(TableStructureDetector, available), []() { return false; });
    stub.set_lamda(ADDR(Img2TableFallback, detect),
                   [](Img2TableFallback *, const QImage &, QList<DetectedCell> &, QString &err) {
                       err = QStringLiteral("no table lines detected");
                       return false;
                   });

    QImage image(100, 100, QImage::Format_RGB32);
    image.fill(Qt::white);
    recognizer.recognizeAsync(image, std::chrono::seconds(10));
    const DTableResult noTable = waitForDone(spy, 5000);
    EXPECT_FALSE(noTable.success);
    EXPECT_EQ(noTable.error, TableError::NoTableDetected);
    EXPECT_TRUE(isAsciiOnly(noTable.errorMessage)) << noTable.errorMessage.toStdString();
}
