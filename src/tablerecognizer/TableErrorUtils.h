// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TABLEERRORUTILS_H
#define TABLEERRORUTILS_H

#include "dtablerecognizer_global.h"

#include <QList>
#include <QPair>
#include <QString>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

// 诊断信息格式化工具（模块内部使用，不属于对外 API）。
//
// 统一格式： "<stage>: <what> (key=value; key=value; ...)"
//  - stage  固定词表：image / pipeline / structure / slanet / img2table /
//                      wireless / ocr / timeout
//  - fields 固定小写 key，值为实际上下文（阈值、计数、耗时、路径等）
// 目的是让 DTableResult::errorMessage 在日志里可检索、可对比，而不是一句笼统的
// "识别失败"。信息量与稳定性无关——调用方仍只允许依赖 DTableResult::error。
namespace TableErrorUtils {

using Field = QPair<QString, QString>;

D_TABLERECOGNIZER_EXPORT QString detail(const QString &stage, const QString &what,
                                        const QList<Field> &fields = {});

} // namespace TableErrorUtils

D_TABLERECOGNIZER_END_NAMESPACE

#endif // TABLEERRORUTILS_H
