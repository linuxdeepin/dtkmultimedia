// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtablerecognizer.h"
#include "dtablerecognizer_p.h"

#include "OrtInferenceEngine.h"
#include "TableStructureDetector.h"
#include "DtkOcrWrapper.h"
#include "CellTextMapper.h"
#include "HtmlTableBuilder.h"
#include "Img2TableFallback.h"

#include <QDir>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QTimer>
#include <QThreadPool>
#include <QLoggingCategory>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

namespace {
Q_LOGGING_CATEGORY(lcTableRecognizer, "dtk.tablerecognizer")

// SLANet_plus 模型文件名。
static constexpr const char *kModelFileName = "SLANet_plus.onnx";

QString defaultModelPath()
{
#ifdef TABLEREC_MODEL_DIR
    return QString::fromUtf8(TABLEREC_MODEL_DIR) + QLatin1String(kModelFileName);
#else
    return QStringLiteral("/usr/share/libdtk6tablerecognizer/models/%1").arg(QLatin1String(kModelFileName));
#endif
}
} // namespace

DTableRecognizer::DTableRecognizer(QObject *parent)
    : QObject(parent)
    , d_ptr(new DTableRecognizerPrivate(this))
{
    Q_D(DTableRecognizer);
    d->ortEngine.reset(new OrtInferenceEngine);
    d->detector.reset(new TableStructureDetector(d->ortEngine.data()));
    // 初始化时加载主模型 SLANet_plus.onnx，使主路径（ORT 推理）可用。
    // 加载失败时记录错误，自动降级到 img2table（现有降级逻辑保留）。
    const QString modelPath = defaultModelPath();
    if (!d->ortEngine->loadModel(modelPath)) {
        qCWarning(lcTableRecognizer) << "Failed to load SLANet_plus model at" << modelPath
                                     << ":" << d->ortEngine->lastError()
                                     << "(will fall back to img2table)";
    }
}

DTableRecognizer::~DTableRecognizer()
{
    Q_D(DTableRecognizer);
    // 析构期保护：等待在途识别任务完成，避免工作线程访问已释放的 this。
    if (d->watcher.isRunning())
        d->watcher.waitForFinished();
}

void DTableRecognizer::recognizeAsync(const QImage &image, std::chrono::milliseconds timeout)
{
    Q_D(DTableRecognizer);
    d->start(image, timeout);
}

// ===== Private 实现 =====

DTableRecognizerPrivate::DTableRecognizerPrivate(DTableRecognizer *q)
    : QObject(q)
    , q_ptr(q)
{
}

DTableRecognizerPrivate::~DTableRecognizerPrivate() = default;

void DTableRecognizerPrivate::start(const QImage &image, std::chrono::milliseconds timeout)
{
    if (image.isNull()) {
        DTableResult result;
        result.success = false;
        result.errorMessage = QStringLiteral("输入图片无效");
        emitImmediate(result);
        return;
    }

    // 单飞守卫：Idle→Running，并发调用直接拒绝。
    if (!busy.testAndSetAcquire(0, 1)) {
        DTableResult result;
        result.success = false;
        result.errorMessage = QStringLiteral("已有识别任务在执行");
        emitImmediate(result);
        return;
    }

    timedOut.storeRelease(0);
    emitted.storeRelease(0);

    QImage imageCopy = image.copy();
    auto deadline = std::chrono::steady_clock::now() + timeout;

    // 超时定时器：到点置标志，若工作尚未完成则下发超时失败结果。
    QTimer::singleShot(std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count(), this, [this]() {
        timedOut.storeRelease(1);
        if (emitted.testAndSetAcquire(0, 1)) {
            DTableResult result;
            result.success = false;
            result.errorMessage = QStringLiteral("识别超时");
            Q_Q(DTableRecognizer);
            QMetaObject::invokeMethod(q, [this, result]() {
                busy.storeRelease(0);   // 回到 Idle。
                emit q_ptr->recognitionDone(result);
            }, Qt::QueuedConnection);
        }
    });

    QFuture<void> future = QtConcurrent::run(QThreadPool::globalInstance(), [this, imageCopy, deadline]() {
        DTableResult result = runPipeline(std::move(imageCopy), deadline);
        Q_Q(DTableRecognizer);
        QMetaObject::invokeMethod(q, [this, result]() {
            busy.storeRelease(0);   // 回到 Idle。
            if (emitted.testAndSetAcquire(0, 1))
                emit q_ptr->recognitionDone(result);
        }, Qt::QueuedConnection);
    });
    watcher.setFuture(future);
}

DTableResult DTableRecognizerPrivate::runPipeline(QImage image,
                                                   std::chrono::steady_clock::time_point deadline)
{
    DTableResult result;
    result.success = false;

    const auto now = std::chrono::steady_clock::now();
    if (now > deadline) {
        result.errorMessage = QStringLiteral("识别超时");
        return result;
    }

    // 阶段1：表格结构检测（主路径 SLANet_plus via ORT）。
    QList<DetectedCell> cells;
    QString error;
    bool structOk = false;
    if (detector && detector->available()) {
        structOk = detector->detect(image, cells, error);
        if (!structOk)
            qCWarning(lcTableRecognizer) << "Main path (SLANet_plus) detect failed:" << error
                                          << "— falling back to img2table";
    } else {
        qCWarning(lcTableRecognizer) << "Main path unavailable (model not loaded)"
                                      << "— falling back to img2table";
    }
    if (!structOk || cells.isEmpty()) {
        // 阶段1降级：img2table（OpenCV 有线表格）。
        cells.clear();
        if (fallback.detect(image, cells, error)) {
            if (cells.isEmpty()) {
                // 降级实现返回成功但未产出结构，视为未识别到表格，避免空表格 + success=true。
                result.success = false;
                result.errorMessage = QStringLiteral("未识别到表格");
                return result;
            }
            result.source = QStringLiteral("img2table");
        } else {
            result.errorMessage = error.isEmpty() ? QStringLiteral("未识别到表格") : error;
            return result;
        }
    } else {
        result.source = QStringLiteral("SLANet_plus");
    }

    if (timedOut.loadAcquire() || std::chrono::steady_clock::now() > deadline) {
        result.cells.clear();
        result.html.clear();
        result.source.clear();
        result.errorMessage = QStringLiteral("识别超时");
        result.success = false;
        return result;
    }

    // 阶段2：OCR 文字识别。按详细设计 §7.1，OCR 失败应置 success=false 并记录错误。
    QList<OcrTextBox> ocrBoxes;
    if (!ocr.recognize(image, ocrBoxes, error)) {
        result.errorMessage = QStringLiteral("OCR 失败：%1").arg(error);
        return result;
    }
    // 阶段3：将 OCR 文本框映射到单元格。
    mapper.map(cells, ocrBoxes);

    // 转 public 数据结构。
    QList<DTableCell> publicCells;
    publicCells.reserve(cells.size());
    for (const DetectedCell &dc : cells) {
        DTableCell pc;
        pc.row = dc.row;
        pc.col = dc.col;
        pc.rowSpan = dc.rowSpan;
        pc.colSpan = dc.colSpan;
        pc.bbox = dc.bbox;
        pc.text = dc.text;
        publicCells.append(pc);
    }

    // 阶段4：构建 HTML。
    result.html = HtmlTableBuilder::build(publicCells);

    result.cells = publicCells;
    result.success = true;
    return result;
}

void DTableRecognizerPrivate::emitImmediate(const DTableResult &result)
{
    // 早返回投递：空图片 / 单飞拒绝等路径未启动管线，不应触碰在途任务的
    // busy 锁或抢占其 emitted 去重槽——否则会误释 busy 导致后续调用绕过单飞
    // 守卫、或抢占 emitted 吞掉在途任务的最终结果。故此处仅经 QueuedConnection
    // 投递结果，不动 busy/emitted。一次完整投递后 emitted 在下次有效调用 start()
    // 时复位，早返回不依赖 emitted。
    Q_Q(DTableRecognizer);
    QMetaObject::invokeMethod(q, [this, result]() { emit q_ptr->recognitionDone(result); },
                              Qt::QueuedConnection);
}

D_TABLERECOGNIZER_END_NAMESPACE
