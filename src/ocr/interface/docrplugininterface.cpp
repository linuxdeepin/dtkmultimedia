// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "docrplugininterface.h"

#include <QtDebug>

DOCR_BEGIN_NAMESPACE

static const QString errorMessage = "current plugin do not implement this function: ";

bool DOcrPluginInterface::setUseHardware(const QList<QPair<HardwareID, int>> &hardwareUsed)
{
    Q_UNUSED(hardwareUsed)
    qWarning() << errorMessage << __FUNCTION__;
    return false;
}

bool DOcrPluginInterface::setUseMaxThreadsCount(int n)
{
    Q_UNUSED(n)
    qWarning() << errorMessage << __FUNCTION__;
    return false;
}

QStringList DOcrPluginInterface::authKeys()
{
    qWarning() << errorMessage << __FUNCTION__;
    return QStringList();
}

bool DOcrPluginInterface::setAuth(const QStringList &params)
{
    Q_UNUSED(params)
    qWarning() << errorMessage << __FUNCTION__;
    return false;
}

QStringList DOcrPluginInterface::imageFileSupportFormats() const
{
    qWarning() << errorMessage << __FUNCTION__;
    return QStringList();
}

bool DOcrPluginInterface::setImageFile(const QString &filePath)
{
    Q_UNUSED(filePath)
    qWarning() << errorMessage << __FUNCTION__;
    return false;
}

bool DOcrPluginInterface::setImage(const QImage &image)
{
    Q_UNUSED(image)
    qWarning() << errorMessage << __FUNCTION__;
    return false;
}

QString DOcrPluginInterface::pluginExpandParam(const QString &key) const
{
    Q_UNUSED(key)
    qWarning() << errorMessage << __FUNCTION__;
    return "";
}

bool DOcrPluginInterface::setPluginExpandParam(const QString &key, const QString &value)
{
    Q_UNUSED(key)
    Q_UNUSED(value)
    qWarning() << errorMessage << __FUNCTION__;
    return false;
}

bool DOcrPluginInterface::breakAnalyze()
{
    qWarning() << errorMessage << __FUNCTION__;
    return false;
}

QList<TextBox> DOcrPluginInterface::textBoxes() const
{
    qWarning() << errorMessage << __FUNCTION__;
    return QList<TextBox>();
}

QList<TextBox> DOcrPluginInterface::charBoxes(int index) const
{
    Q_UNUSED(index)
    qWarning() << errorMessage << __FUNCTION__;
    return QList<TextBox>();
}

QString DOcrPluginInterface::resultFromBox(int index) const
{
    Q_UNUSED(index)
    qWarning() << errorMessage << __FUNCTION__;
    return QString();
}

DOCR_END_NAMESPACE
