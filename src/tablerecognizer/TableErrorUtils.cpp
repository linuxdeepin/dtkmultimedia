// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "TableErrorUtils.h"

#include <QStringList>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

namespace {

// 把任意文本收敛成纯 ASCII：非可打印 ASCII（含中文、控制字符）一律替换为 '?'。
// 字段值里可能夹带模型路径、第三方引擎报错等不可控文本，直接拼接会破坏
// errorMessage 的 ASCII-only 契约，顺带也能挡住换行注入。
QString toAsciiSafe(const QString &text)
{
    QString out;
    out.reserve(text.size());
    for (const QChar ch : text) {
        const ushort code = ch.unicode();
        out.append(code >= 0x20 && code <= 0x7E ? ch : QLatin1Char('?'));
    }
    return out;
}

} // namespace

namespace TableErrorUtils {

QString detail(const QString &stage, const QString &what, const QList<Field> &fields)
{
    const QString message = toAsciiSafe(stage) + QLatin1String(": ") + toAsciiSafe(what);
    if (fields.isEmpty())
        return message;

    QStringList parts;
    parts.reserve(fields.size());
    for (const Field &field : fields)
        parts << toAsciiSafe(field.first) + QLatin1Char('=') + toAsciiSafe(field.second);
    return message + QLatin1String(" (") + parts.join(QLatin1String("; ")) + QLatin1Char(')');
}

} // namespace TableErrorUtils

D_TABLERECOGNIZER_END_NAMESPACE
