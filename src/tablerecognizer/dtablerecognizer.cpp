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
#include "WirelessTableHeuristic.h"
#include "TableErrorUtils.h"

#include <QDir>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QTimer>
#include <QThreadPool>
#include <QLoggingCategory>
#include <algorithm>
#include <chrono>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

namespace {
Q_LOGGING_CATEGORY(lcTableRecognizer, "dtk.tablerecognizer")

// SLANet_plus 模型文件名。
static constexpr const char *kModelFileName = "SLANet_plus.onnx";

// M1：SLANet 结构 置信度门（概率直读）。低于此值认为模型对结构不自信，
// 触发降级/质量路径。值为启发式默认，可按实测样张校准。
static constexpr float kSlanetConfidenceThreshold = 0.9f;
// M1：合理表格的最小单元格数（少于 2 视为噪声/非表格）。
static constexpr int kMinReasonableCells = 2;
// H3：表格线密度门。线密度低于此值视为无线/弱线表，启用文本对齐启发式。
// 值为启发式默认，可按实测样张校准。
static constexpr float kWeakLineDensityThreshold = 0.01f;

QString imageSizeText(const QImage &image)
{
    return QStringLiteral("%1x%2").arg(image.width()).arg(image.height());
}

// DetectedCell（内部结构）-> DTableCell（公开结构）。
QList<DTableCell> toPublicCells(const QList<DetectedCell> &cells)
{
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
    return publicCells;
}

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
    // 注册元类型：跨线程 QueuedConnection 信号投递 DTableResult/DTableCell 需要。
    qRegisterMetaType<DTableResult>();
    qRegisterMetaType<DTableCell>();
    d->ortEngine.reset(new OrtInferenceEngine);
    d->detector.reset(new TableStructureDetector(d->ortEngine.data()));
    // 初始化时加载主模型 SLANet_plus.onnx，使主路径（ORT 推理）可用。
    // 加载失败时记录错误，自动降级到 img2table（现有降级逻辑保留）。
    d->modelPath = defaultModelPath();
    if (!d->ortEngine->loadModel(d->modelPath)) {
        qCWarning(lcTableRecognizer) << "Failed to load SLANet_plus model at" << d->modelPath
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
        result.error = TableError::InvalidImage;
        result.errorMessage = TableErrorUtils::detail(
            QStringLiteral("image"), QStringLiteral("invalid input image"),
            {{QStringLiteral("image"), QStringLiteral("null")}});
        emitImmediate(result);
        return;
    }

    // 单飞守卫：Idle→Running，并发调用直接拒绝。
    if (!busy.testAndSetAcquire(0, 1)) {
        DTableResult result;
        result.success = false;
        result.error = TableError::Busy;
        result.errorMessage = TableErrorUtils::detail(
            QStringLiteral("pipeline"), QStringLiteral("rejected: a recognition task is already running"),
            {{QStringLiteral("busy"), QStringLiteral("true")}});
        emitImmediate(result);
        return;
    }
    timedOut.storeRelease(0);
    emitted.storeRelease(0);
    const quint32 gen = static_cast<quint32>(generation.fetchAndAddRelease(1) + 1);

    QImage imageCopy = image.copy();
    auto deadline = std::chrono::steady_clock::now() + timeout;

    // 超时定时器：到点置标志，若工作尚未完成则下发超时失败结果。
    // 注意：超时路径不重置 busy——只有工作线程完成时才重置 busy，
    // 以确保旧任务的工作线程完全退出后新任务才能启动，避免并发访问共享状态。
    const qint64 timeoutMs = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
    QTimer::singleShot(timeoutMs, this, [this, gen, timeoutMs]() {
        timedOut.storeRelease(1);
        Q_Q(DTableRecognizer);
        QMetaObject::invokeMethod(q, [this, gen, timeoutMs]() {
            if (gen != static_cast<quint32>(generation.loadAcquire()))
                return;  // 过期回调：新任务已启动。
            if (emitted.testAndSetAcquire(0, 1)) {
                DTableResult result;
                result.success = false;
                result.error = TableError::Timeout;
                result.errorMessage = TableErrorUtils::detail(
                    QStringLiteral("timeout"), QStringLiteral("recognition timed out"),
                    {{QStringLiteral("stage"), QStringLiteral("pipeline")},
                     {QStringLiteral("budget_ms"), QString::number(timeoutMs)}});
                emit q_ptr->recognitionDone(result);
            }
        }, Qt::QueuedConnection);
    });

    QFuture<void> future = QtConcurrent::run(QThreadPool::globalInstance(), [this, imageCopy, deadline, gen, timeoutMs]() {
        DTableResult result = runPipeline(std::move(imageCopy), deadline, timeoutMs);
        QMetaObject::invokeMethod(q_ptr, [this, gen, result]() {
            busy.storeRelease(0);   // 工作线程完成，回到 Idle。
            if (gen != static_cast<quint32>(generation.loadAcquire()))
                return;  // 过期回调：新任务已启动，不 emit 陈旧结果。
            if (emitted.testAndSetAcquire(0, 1))
                emit q_ptr->recognitionDone(result);
        }, Qt::QueuedConnection);
    });
    watcher.setFuture(future);
}

DTableResult DTableRecognizerPrivate::runPipeline(QImage image,
                                                   std::chrono::steady_clock::time_point deadline,
                                                   qint64 budgetMs)
{
    DTableResult result;
    result.success = false;

    // Stage 2 速度测试：计时累加器（毫秒）。
    qint64 structureMsAccum = 0;
    qint64 ocrMsAccum = 0;

    const auto pipelineStart = std::chrono::steady_clock::now();
    const auto now = pipelineStart;
    if (now > deadline) {
        result.error = TableError::Timeout;
        result.errorMessage = TableErrorUtils::detail(
            QStringLiteral("timeout"), QStringLiteral("deadline exceeded before pipeline start"),
            {{QStringLiteral("overdue_ms"),
              QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(now - deadline).count())}});
        result.totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - pipelineStart).count();
        return result;
    }

    // 超时判定：定时器置位，或本地已越过 deadline。
    const auto isExpired = [&]() {
        return timedOut.loadAcquire() != 0 || std::chrono::steady_clock::now() > deadline;
    };
    // 统一的超时收尾：丢弃结构结果，但保留「当时走的哪条路」用于定位。
    const auto finishAsTimeout = [&]() {
        const QString activeRoute = result.source;
        result.cells.clear();
        result.html.clear();
        result.source.clear();
        result.success = false;
        result.error = TableError::Timeout;
        result.errorMessage = TableErrorUtils::detail(
            QStringLiteral("timeout"), QStringLiteral("recognition timed out"),
            {{QStringLiteral("stage"), QStringLiteral("structure")},
             {QStringLiteral("source"),
              activeRoute.isEmpty() ? QStringLiteral("<none>") : activeRoute},
             {QStringLiteral("budget_ms"), QString::number(budgetMs)},
             {QStringLiteral("image"), imageSizeText(image)},
             {QStringLiteral("elapsed_ms"),
              QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                                  std::chrono::steady_clock::now() - pipelineStart).count())},
             {QStringLiteral("overdue_ms"),
              QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                                  std::chrono::steady_clock::now() - deadline).count())}});
        result.structureMs = structureMsAccum;
        result.ocrMs = ocrMsAccum;
        result.totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - pipelineStart).count();
        return result;
    };

    // 阶段1：表格结构检测（主路径 SLANet_plus via ORT）。
    QList<DetectedCell> cells;
    QString error;
    QStringList attempts;   // 各结构路径的失败诊断，最终拼进 errorMessage
    float slanetConfidence = 1.0f;   // M1：结构置信度（detect 成功时由模型 logits 填充）
    bool structOk = false;
    const bool modelRan = detector && detector->available();
    if (modelRan) {
        const auto t0 = std::chrono::steady_clock::now();
        structOk = detector->detect(image, cells, error, &slanetConfidence);
        structureMsAccum += std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count();
        if (!structOk) {
            qCWarning(lcTableRecognizer) << "Main path (SLANet_plus) detect failed:" << error
                                          << "- will try quality path";
            attempts << TableErrorUtils::detail(
                QStringLiteral("slanet"),
                error.isEmpty() ? QStringLiteral("detect failed") : error,
                {{QStringLiteral("confidence"), QString::number(slanetConfidence, 'f', 3)},
                 {QStringLiteral("cells"), QString::number(cells.size())},
                 {QStringLiteral("failure"), QString::number(static_cast<int>(detector->lastFailure()))}});
        }
    } else {
        qCWarning(lcTableRecognizer) << "Main path unavailable (model not loaded)"
                                      << "- falling back to img2table";
        attempts << TableErrorUtils::detail(
            QStringLiteral("slanet"), QStringLiteral("unavailable: model not loaded"),
            {{QStringLiteral("model"), modelPath}});
    }

    // 阶段2（条件提前）：模型曾运行时提前做 OCR，供结构-内容一致性信号使用。
    // 模型缺失时不提前 OCR——走既有 img2table 流程后再 OCR（保留已验证行为）。
    QList<OcrTextBox> ocrBoxes;
    bool ocrDone = false;
    if (modelRan) {
        const auto t0 = std::chrono::steady_clock::now();
        if (ocr.recognize(image, ocrBoxes, error)) {
            ocrDone = true;
        } else {
            qCWarning(lcTableRecognizer) << "OCR for consistency check failed:" << error;
        }
        ocrMsAccum += std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count();
    }

    // H3 v3：结构-内容一致性信号——SLANet 预测列数 vs OCR 文本框列簇数。
    // 仪表化：逐张打印真实置信度、SLANet 列数、OCR 列簇数、线密度、路由结果（为 Step 2 采数据）。
    int slanetCols = 0;
    if (structOk && !cells.isEmpty()) {
        int maxCol = 0;
        for (const DetectedCell &c : cells)
            maxCol = std::max(maxCol, c.col + std::max(1, c.colSpan) - 1);
        slanetCols = maxCol + 1;
    }
    const int ocrColClusters = ocrDone
        ? WirelessTableHeuristic::splitColumns(ocrBoxes, 5.0).size() : 0;

    // M1 门：置信度 + cell 数合理性。
    const bool m1Gate = structOk && !cells.isEmpty()
                        && slanetConfidence >= kSlanetConfidenceThreshold
                        && cells.size() >= kMinReasonableCells;
    // H3 v3：结构-内容一致性——M1 门通过后，用 SLANet 列数 vs OCR 列簇数判定。
    // 一致 → 信任 SLANet；粘列(slanet<ocr) → wireless；多列(slanet>ocr) → 保留 SLANet。
    bool mainTrusted = false;
    if (!modelRan) {
        mainTrusted = false;   // 模型缺失 → img2table 降级（不变）。
    } else if (!m1Gate) {
        mainTrusted = false;   // M1 门未通过 → 降级。
    } else if (!ocrDone) {
        mainTrusted = true;    // M1 门通过但 OCR 失败 → 无法检查一致性，保留 SLANet。
    } else {
        const bool consistent = (slanetCols == ocrColClusters);
        if (consistent) {
            mainTrusted = true;   // 一致 → 信任 SLANet。
        } else if (slanetCols < ocrColClusters) {
            mainTrusted = false;  // 粘列 → 降级 wireless。
        } else {
            mainTrusted = true;   // 多列 → 保留 SLANet 不降级（AT 证回退更差）。
        }
    }

    if (!mainTrusted) {
        // 线密度：从属确认信号，仅在 !mainTrusted 时用于 wireless/img2table 二选一。
        const float lineDensityVal = modelRan ? WirelessTableHeuristic::lineDensity(image) : 1.0f;
        const bool weakLine = lineDensityVal < kWeakLineDensityThreshold;
        // 判定降级目标：粘列(slanet<ocr) → wireless；M1 门失败时按线密度二选一。
        const bool routeToWireless = modelRan && ocrDone && slanetCols > 0
            && slanetCols < ocrColClusters;
        const bool routeToWirelessByDensity = modelRan && !m1Gate && weakLine && ocrDone;
        const bool useWireless = routeToWireless || routeToWirelessByDensity;

        if (modelRan) {
            attempts << TableErrorUtils::detail(
                QStringLiteral("slanet"), QStringLiteral("structure rejected by quality gate"),
                {{QStringLiteral("confidence"), QString::number(slanetConfidence, 'f', 3)},
                 {QStringLiteral("cells"), QString::number(cells.size())},
                 {QStringLiteral("slanet_cols"), QString::number(slanetCols)},
                 {QStringLiteral("ocr_col_clusters"), QString::number(ocrColClusters)},
                 {QStringLiteral("line_density"), QString::number(lineDensityVal, 'f', 5)},
                 {QStringLiteral("route"), useWireless ? QStringLiteral("wireless") : QStringLiteral("img2table")}});
        }
        qCWarning(lcTableRecognizer) << "Main path untrusted (structOk=" << structOk
                                      << "confidence=" << slanetConfidence
                                      << "cells=" << cells.size()
                                      << "slanetCols=" << slanetCols
                                      << "ocrColClusters=" << ocrColClusters
                                      << "lineDensity=" << lineDensityVal
                                      << "route=" << (useWireless ? "wireless" : "img2table")
                                      << ") - entering quality path";
        cells.clear();
        const auto t0 = std::chrono::steady_clock::now();
        bool gotStructure = false;
        if (useWireless) {
            if (wireless.build(image.size(), ocrBoxes, cells, error) && !cells.isEmpty()) {
                result.source = QStringLiteral("wireless");
                gotStructure = true;
            } else {
                qCWarning(lcTableRecognizer) << "Wireless heuristic failed:" << error;
                attempts << (error.isEmpty()
                                 ? TableErrorUtils::detail(QStringLiteral("wireless"),
                                                           QStringLiteral("no cells built"))
                                 : error);
            }
        }
        if (!gotStructure) {
            // 有线降级：img2table（霍夫线检测）。
            if (fallback.detect(image, cells, error)) {
                if (cells.isEmpty()) {
                    structureMsAccum += std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - t0).count();
                    if (isExpired())
                        return finishAsTimeout();
                    result.success = false;
                    result.error = TableError::NoTableDetected;
                    result.errorMessage = TableErrorUtils::detail(
                        QStringLiteral("structure"), QStringLiteral("no table detected"),
                        {{QStringLiteral("route"), QStringLiteral("img2table")},
                         {QStringLiteral("cells"), QStringLiteral("0")},
                         {QStringLiteral("image"), imageSizeText(image)},
                         {QStringLiteral("attempts"), attempts.join(QStringLiteral(" | "))}});
                    result.structureMs = structureMsAccum;
                    result.ocrMs = ocrMsAccum;
                    result.totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - pipelineStart).count();
                    return result;
                }
                result.source = QStringLiteral("img2table");
                gotStructure = true;
            }
        }
        structureMsAccum += std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count();
        if (!gotStructure) {
            if (!error.isEmpty())
                attempts << error;
            // 已越过 deadline：各路径的结论都不再可信，统一按超时上报。
            if (isExpired())
                return finishAsTimeout();
            // 主路径内部异常（推理失败 / 输出不符约定 / 预处理失败），或降级路径内部处理失败时，
            // 不能断言「图里没有表格」，上报 InternalError 并带上原因与各路径尝试记录。
            const bool mainInternalFailure =
                modelRan && detector
                && (detector->lastFailure() == TableStructureDetector::Failure::InferenceFailed
                    || detector->lastFailure() == TableStructureDetector::Failure::BadOutput
                    || detector->lastFailure() == TableStructureDetector::Failure::PreprocessFailed);
            const bool fallbackInternalFailure =
                fallback.lastFailure() == Img2TableFallback::Failure::ConversionFailed;
            const bool internalFailure = mainInternalFailure || fallbackInternalFailure;
            result.error = internalFailure ? TableError::InternalError : TableError::NoTableDetected;
            result.errorMessage = TableErrorUtils::detail(
                QStringLiteral("structure"),
                internalFailure ? QStringLiteral("no table detected, main path failed internally")
                                : QStringLiteral("no table detected by any route"),
                {{QStringLiteral("image"), imageSizeText(image)},
                 {QStringLiteral("cells"), QString::number(cells.size())},
                 {QStringLiteral("attempts"), attempts.join(QStringLiteral(" | "))}});
            result.structureMs = structureMsAccum;
            result.ocrMs = ocrMsAccum;
            result.totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - pipelineStart).count();
            return result;
        }
    } else {
        result.source = QStringLiteral("SLANet_plus");
    }

    if (isExpired())
        return finishAsTimeout();

    qCDebug(lcTableRecognizer) << "Instrumentation: confidence=" << slanetConfidence
                               << "slanetCols=" << slanetCols
                               << "ocrColClusters=" << ocrColClusters
                               << "route=" << result.source;

    // 阶段3：OCR 文字识别（若模型缺失或早期 OCR 失败，此处补做；按详细设计 OCR 失败置 success=false）。
    if (!ocrDone) {
        const auto t0 = std::chrono::steady_clock::now();
        if (!ocr.recognize(image, ocrBoxes, error)) {
            ocrMsAccum += std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - t0).count();
            // 结构已识别出来，文字缺失不应把结构一起丢掉：cells/html 尽力保留，
            // 由调用方决定是否降级使用（success 仍为 false）。
            result.cells = toPublicCells(cells);
            result.html = HtmlTableBuilder::build(result.cells);

            QString what = QStringLiteral("table structure detected, OCR failed");
            switch (ocr.lastFailure()) {
            case DtkOcrWrapper::Failure::NoTextDetected:
                result.error = TableError::NoTextDetected;
                what = QStringLiteral("table structure detected, no text recognized");
                break;
            case DtkOcrWrapper::Failure::PluginUnavailable:
                result.error = TableError::OcrEngineUnavailable;
                what = QStringLiteral("table structure detected, OCR engine unavailable");
                break;
            case DtkOcrWrapper::Failure::InvalidImage:
                result.error = TableError::InvalidImage;
                what = QStringLiteral("table structure detected, invalid input image");
                break;
            case DtkOcrWrapper::Failure::EngineFailed:
            case DtkOcrWrapper::Failure::None:
                result.error = TableError::OcrFailed;
                break;
            }
            result.errorMessage = TableErrorUtils::detail(
                QStringLiteral("ocr"), what,
                {{QStringLiteral("ocr_failure"), QString::number(static_cast<int>(ocr.lastFailure()))},
                 {QStringLiteral("source"), result.source},
                 {QStringLiteral("cells"), QString::number(result.cells.size())},
                 {QStringLiteral("ocr_error"), error},
                 {QStringLiteral("image"), imageSizeText(image)}});
            result.structureMs = structureMsAccum;
            result.ocrMs = ocrMsAccum;
            result.totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - pipelineStart).count();
            return result;
        }
        ocrMsAccum += std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count();
        ocrDone = true;
    }
    // 将 OCR 文本框映射到单元格。
    mapper.map(cells, ocrBoxes);

    // 转 public 数据结构。
    const QList<DTableCell> publicCells = toPublicCells(cells);

    // 阶段4：构建 HTML。
    result.html = HtmlTableBuilder::build(publicCells);

    result.cells = publicCells;
    result.success = true;
    result.structureMs = structureMsAccum;
    result.ocrMs = ocrMsAccum;
    result.totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - pipelineStart).count();
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
