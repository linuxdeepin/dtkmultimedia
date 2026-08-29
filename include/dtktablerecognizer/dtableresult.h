// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTABLERESULT_H
#define DTABLERESULT_H

#include "dtablecell.h"

#include <QList>
#include <QString>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

struct DTableResult
{
    bool success = false;          // 识别是否成功
    QString html;                  // HTML 表格文本
    QList<DTableCell> cells;       // 结构化单元格列表
    QString source;                // 识别来源："SLANet_plus" 或 "img2table"
    QString errorMessage;          // 失败时的错误说明

    // 计时字段（Stage 2 速度测试）：ABI 友好，仅尾部追加。
    qint64 structureMs = 0;        // 结构检测耗时（SLANet_plus ONNX 推理 + 降级路径）
    qint64 ocrMs = 0;             // OCR 耗时（dtk6ocr PP-OCRv5）
    qint64 totalMs = 0;           // 端到端总耗时
};

Q_DECLARE_METATYPE(DTableResult)

D_TABLERECOGNIZER_END_NAMESPACE
#endif // DTABLERESULT_H
