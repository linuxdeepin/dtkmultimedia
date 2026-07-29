// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTABLERECOGNIZER_GLOBAL_H
#define DTABLERECOGNIZER_GLOBAL_H

#include <QtCore/qglobal.h>

#define D_TABLERECOGNIZER_BEGIN_NAMESPACE namespace Dtk { namespace TableRecognizer {
#define D_TABLERECOGNIZER_END_NAMESPACE }}
#define D_TABLERECOGNIZER_USE_NAMESPACE using namespace Dtk::TableRecognizer;

#if defined(DTK_BUILD_TABLERECOGNIZER_LIB)
#  define D_TABLERECOGNIZER_EXPORT Q_DECL_EXPORT
#else
#  define D_TABLERECOGNIZER_EXPORT Q_DECL_IMPORT
#endif

#endif // DTABLERECOGNIZER_GLOBAL_H
