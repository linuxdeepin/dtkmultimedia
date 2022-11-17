// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DOCR_P_H
#define DOCR_P_H

#include "docr.h"
#include "dtkocr.h"

#include <QAtomicInt>

DOCR_BEGIN_NAMESPACE

class DOcrPlugin;
class DOcrPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DOcr)

public:
    explicit DOcrPrivate(DOcr *parent = nullptr);

private:
    bool isCompatible();
    void resetPlugin();
    QString pluginInstallDir;
    QString pluginName;
    int pluginVersion;
    QAtomicInt isRunning;
    DOcrPlugin *plugin;

    DOcr *q_ptr;
};

DOCR_END_NAMESPACE
#endif
