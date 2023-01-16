// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DOCR_P_H
#define DOCR_P_H

#include "docr.h"
#include "dtkocr.h"

#include <QAtomicInt>
#include <QPluginLoader>

DOCR_BEGIN_NAMESPACE

class DOcrPluginInterface;
class DOcrPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DOcr)

public:
    explicit DOcrPrivate(DOcr *parent = nullptr);

    enum PluginType { Plugin_Default, Plugin_Qt, Plugin_Cpp, Plugin_None };

private:
    void resetPlugin();
    QString currentModuleDir();

    QString pluginInstallDir;
    QString pluginName;
    int pluginVersion = 0;
    QAtomicInt isRunning = 0;
    DOcrPluginInterface *plugin = nullptr;
    QPluginLoader *qtPluginLoader;
    PluginType currentPluginType = Plugin_None;

    DOcr *q_ptr;
};

DOCR_END_NAMESPACE
#endif
