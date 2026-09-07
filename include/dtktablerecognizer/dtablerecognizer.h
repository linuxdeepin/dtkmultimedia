// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTABLERECOGNIZER_H
#define DTABLERECOGNIZER_H

#include "dtablerecognizer_global.h"
#include "dtablecell.h"
#include "dtableresult.h"

#include <QImage>
#include <QObject>

#include <chrono>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

class DTableRecognizerPrivate;

// 公开门面：QImage -> 结构化表格（HTML + 单元格列表）
class D_TABLERECOGNIZER_EXPORT DTableRecognizer : public QObject
{
    Q_OBJECT
public:
    explicit DTableRecognizer(QObject *parent = nullptr);
    ~DTableRecognizer();

    // 异步识别；结果通过 recognitionDone 信号返回。
    void recognizeAsync(const QImage &image,
                        std::chrono::milliseconds timeout = std::chrono::seconds(25));

signals:
    void recognitionDone(const DTableResult &result);

private:
    QScopedPointer<DTableRecognizerPrivate> d_ptr;
    Q_DISABLE_COPY(DTableRecognizer)
    Q_DECLARE_PRIVATE(DTableRecognizer)
};

D_TABLERECOGNIZER_END_NAMESPACE

#endif // DTABLERECOGNIZER_H
