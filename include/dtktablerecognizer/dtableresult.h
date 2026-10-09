// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTABLERESULT_H
#define DTABLERESULT_H

#include "dtablecell.h"

#include <QList>
#include <QObject>
#include <QString>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

Q_NAMESPACE

/**
 * @brief 识别失败原因（机器可读错误码）
 *
 * 调用方**只应依据本枚举**做分支判断，禁止解析 errorMessage。
 * 枚举值一经发布即固定，后续只允许在末尾追加新值。
 */
enum class TableError {
    None = 0,                 // 识别成功
    InvalidImage = 1,         // 传入图片无效（空图或无法解码）
    NoTableDetected = 2,      // 所有结构路径都没有产出表格结构
    NoTextDetected = 3,       // 已识别到表格结构，但没有任何文字（cells/html 仍尽力保留）
    Timeout = 4,              // 识别超时
    Busy = 5,                 // 已有识别任务在执行（单飞拒绝）
    OcrEngineUnavailable = 6, // OCR 插件/模型不可用（环境问题，重试无意义）
    OcrFailed = 7,            // OCR 引擎执行报错（可重试）
    InternalError = 8,        // 推理/输出解析等内部异常
};
Q_ENUM_NS(TableError)

struct DTableResult
{
    bool success = false;          // 识别是否成功
    QString html;                  // HTML 表格文本
    QList<DTableCell> cells;       // 结构化单元格列表
    QString source;                // 识别来源："SLANet_plus" / "img2table" / "wireless"
    // 失败诊断信息：纯 ASCII，格式 "<stage>: <what> (key=value; key=value)"，
    // 供日志/排查使用（含阶段、阈值、计数、耗时等上下文）。
    // 不保证稳定，调用方禁止解析。
    QString errorMessage;

    // 计时字段（Stage 2 速度测试）：ABI 友好，仅尾部追加。
    qint64 structureMs = 0;        // 结构检测耗时（SLANet_plus ONNX 推理 + 降级路径）
    qint64 ocrMs = 0;             // OCR 耗时（dtk6ocr PP-OCRv5）
    qint64 totalMs = 0;           // 端到端总耗时

    // 机器可读的失败原因（尾部追加：ABI 友好）。成功时必须为 TableError::None，
    // 失败时必须为非 None，调用方据此选择提示文案。
    // 例外：NoTextDetected / OcrEngineUnavailable / OcrFailed 表示结构已识别出来但
    // 文字缺失，此时 cells/html 仍尽力保留（可能为空），供调用方降级使用。
    TableError error = TableError::None;
};

D_TABLERECOGNIZER_END_NAMESPACE

Q_DECLARE_METATYPE(Dtk::TableRecognizer::DTableResult)
#endif // DTABLERESULT_H
