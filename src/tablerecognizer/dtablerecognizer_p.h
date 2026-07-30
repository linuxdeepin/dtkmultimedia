// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTABLERECOGNIZER_P_H
#define DTABLERECOGNIZER_P_H

#include "dtablerecognizer.h"
#include "dtablerecognizer_global.h"

#include <QAtomicInt>
#include <QFuture>
#include <QFutureWatcher>
#include <QImage>
#include <QObject>
#include <QScopedPointer>
#include <QString>

#include <chrono>

#include "OrtInferenceEngine.h"
#include "TableStructureDetector.h"
#include "DtkOcrWrapper.h"
#include "CellTextMapper.h"
#include "HtmlTableBuilder.h"
#include "Img2TableFallback.h"

D_TABLERECOGNIZER_BEGIN_NAMESPACE

class DTableRecognizerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DTableRecognizerPrivate(DTableRecognizer *q);
    ~DTableRecognizerPrivate();

    void start(const QImage &image, std::chrono::milliseconds timeout);
    DTableResult runPipeline(QImage image, std::chrono::steady_clock::time_point deadline);
    // 早返回投递：不参与单飞 bookkeeping（busy/emitted），
    // 用于空图片 / 单飞拒绝等未启动管线的路径。
    void emitImmediate(const DTableResult &result);

    DTableRecognizer *q_ptr = nullptr;

    QScopedPointer<OrtInferenceEngine> ortEngine;
    QScopedPointer<TableStructureDetector> detector;
    DtkOcrWrapper ocr;
    CellTextMapper mapper;
    Img2TableFallback fallback;

    QAtomicInt timedOut{0};
    QAtomicInt emitted{0};
    QAtomicInt busy{0};             // 单飞守卫（0=Idle, 1=Running）
    QFutureWatcher<void> watcher;   // 析构时等待在途任务完成

    Q_DISABLE_COPY(DTableRecognizerPrivate)
    Q_DECLARE_PUBLIC(DTableRecognizer)
};

D_TABLERECOGNIZER_END_NAMESPACE

#endif // DTABLERECOGNIZER_P_H
