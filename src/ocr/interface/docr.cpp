// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "docr.h"
#include "docr_p.h"

#include <QImage>

DOCR_BEGIN_NAMESPACE

DOcrPrivate::DOcrPrivate(DOcr *parent)
    : q_ptr(parent)
{
    ;
}

DOcr::DOcr(QObject *parent)
    : QObject(parent), d_ptr(new DOcrPrivate(this))
{
}

DOcr::~DOcr()
{
}

QStringList DOcr::installedPluginNames()
{
    ;
}

bool DOcr::loadDefaultPlugin()
{
    ;
}

bool DOcr::loadPlugin(const QString &pluginName)
{
    ;
}

bool DOcr::pluginReady() const
{
    ;
}

QList<HardwareID> DOcr::hardwareSupportList() const
{
    ;
}

bool DOcr::setUseHardware(const QList<QPair<HardwareID, int>> &hardwareUsed)
{
    ;
}

bool DOcr::setUseMaxThreadsCount(int n)
{
    ;
}

QStringList DOcr::authKeys() const
{
    ;
}

bool DOcr::setAuth(const QStringList &params)
{
    ;
}

QStringList DOcr::imageFileSupportFormats() const
{
    ;
}

bool DOcr::setImageFile(const QString &filePath)
{
    ;
}

bool DOcr::setImage(const QImage &image)
{
    ;
}

QImage DOcr::imageCached() const
{
    ;
}

bool DOcr::pluginExpandParam(const QString &key) const
{
    ;
}

bool DOcr::setPluginExpandParam(const QString &key, const QString &value)
{
    ;
}

bool DOcr::analyze()
{
    ;
}

bool DOcr::breakAnalyze()
{
    ;
}

bool DOcr::isRunning() const
{
    ;
}

QStringList DOcr::languageSupport() const
{
    ;
}

bool DOcr::setLanguage(const QString &language)
{
    ;
}

QList<TextBox> DOcr::textBoxes() const
{
    ;
}

QList<TextBox> DOcr::charBoxes() const
{
    ;
}

QString DOcr::simpleResult() const
{
    ;
}

QString DOcr::resultFromBox(int index) const
{
    ;
}

DOCR_END_NAMESPACE
