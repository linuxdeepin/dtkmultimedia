// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "docr.h"
#include "docr_p.h"
#include "docrplugininterface.h"
#include "ppocr.h"

#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QtDebug>

#include <dlfcn.h>

DOCR_BEGIN_NAMESPACE

DOcrPrivate::DOcrPrivate(DOcr *parent)
    : q_ptr(parent)
{
    ;
}

bool DOcrPrivate::isCompatible()
{
    return false;
}

void DOcrPrivate::resetPlugin()
{
    ;
}

QString DOcrPrivate::currentModuleDir()
{
    Dl_info dlInfo;
    int rc = 1;
    rc = dladdr(this, &dlInfo);
    if (rc == 0) {
        qWarning() << "Unable to locate dtkocr library path";
        return "";
    }

    const QString &modelFullPath = dlInfo.dli_fname;
    QFileInfo info(modelFullPath);
    return info.absoluteDir().path();
}

DOcr::DOcr(QObject *parent)
    : QObject(parent)
    , d_ptr(new DOcrPrivate(this))
{
}

DOcr::~DOcr() {}

QStringList DOcr::installedPluginNames()
{
    return {};
}

bool DOcr::loadDefaultPlugin()
{
    Q_D(DOcr);
    if (pluginReady()) {
        d->resetPlugin();
    }
    d->plugin = new PaddleOCRApp;
    d->pluginIsLoaded = true;
    return true;
}

bool DOcr::loadPlugin(const QString &pluginName)
{
    return false;
}

bool DOcr::pluginReady() const
{
    Q_D(const DOcr);
    return d->pluginIsLoaded;
}

QList<HardwareID> DOcr::hardwareSupportList() const
{
    if (!pluginReady()) {
        qWarning() << "you need load a plugin first: " << __FUNCTION__;
        return QList<HardwareID>();
    }

    Q_D(const DOcr);
    return d->plugin->hardwareSupportList();
}

bool DOcr::setUseHardware(const QList<QPair<HardwareID, int>> &hardwareUsed)
{
    if (!pluginReady()) {
        qWarning() << "you need load a plugin first: " << __FUNCTION__;
        return false;
    }

    Q_D(DOcr);
    return d->plugin->setUseHardware(hardwareUsed);
}

bool DOcr::setUseMaxThreadsCount(int n)
{
    if (!pluginReady()) {
        qWarning() << "you need load a plugin first: " << __FUNCTION__;
        return false;
    }

    Q_D(DOcr);
    return d->plugin->setUseMaxThreadsCount(n);
}

QStringList DOcr::authKeys() const
{
    if (!pluginReady()) {
        qWarning() << "you need load a plugin first: " << __FUNCTION__;
        return QStringList();
    }

    Q_D(const DOcr);
    return d->plugin->authKeys();
}

bool DOcr::setAuth(const QStringList &params)
{
    if (!pluginReady()) {
        qWarning() << "you need load a plugin first: " << __FUNCTION__;
        return false;
    }

    Q_D(DOcr);
    return d->plugin->setAuth(params);
}

QStringList DOcr::imageFileSupportFormats() const
{
    if (!pluginReady()) {
        qWarning() << "you need load a plugin first: " << __FUNCTION__;
        return QStringList();
    }

    Q_D(const DOcr);
    return d->plugin->imageFileSupportFormats();
}

bool DOcr::setImageFile(const QString &filePath)
{
    if (!pluginReady()) {
        qWarning() << "you need load a plugin first: " << __FUNCTION__;
        return false;
    }

    Q_D(DOcr);
    return d->plugin->setImageFile(filePath);
}

bool DOcr::setImage(const QImage &image)
{
    if (!pluginReady()) {
        qWarning() << "you need load a plugin first: " << __FUNCTION__;
        return false;
    }

    Q_D(DOcr);
    return d->plugin->setImage(image);
}

QImage DOcr::imageCached() const
{
    if (!pluginReady()) {
        qWarning() << "you need load a plugin first: " << __FUNCTION__;
        return QImage();
    }

    Q_D(const DOcr);
    return d->plugin->imageCached();
}

QString DOcr::pluginExpandParam(const QString &key) const
{
    if (!pluginReady()) {
        qWarning() << "you need load a plugin first: " << __FUNCTION__;
        return "";
    }

    Q_D(const DOcr);
    return d->plugin->pluginExpandParam(key);
}

bool DOcr::setPluginExpandParam(const QString &key, const QString &value)
{
    if (!pluginReady()) {
        qWarning() << "you need load a plugin first: " << __FUNCTION__;
        return false;
    }

    Q_D(DOcr);
    return d->plugin->setPluginExpandParam(key, value);
}

bool DOcr::analyze()
{
    if (!pluginReady()) {
        qWarning() << "you need load a plugin first: " << __FUNCTION__;
        return false;
    }

    Q_D(DOcr);
    return d->plugin->analyze();
}

bool DOcr::breakAnalyze()
{
    if (!pluginReady()) {
        qWarning() << "you need load a plugin first: " << __FUNCTION__;
        return false;
    }

    Q_D(DOcr);
    return d->plugin->breakAnalyze();
}

bool DOcr::isRunning() const
{
    if (!pluginReady()) {
        qWarning() << "you need load a plugin first: " << __FUNCTION__;
        return false;
    }

    Q_D(const DOcr);
    return d->plugin->isRunning();
}

QStringList DOcr::languageSupport() const
{
    if (!pluginReady()) {
        qWarning() << "you need load a plugin first: " << __FUNCTION__;
        return QStringList();
    }

    Q_D(const DOcr);
    return d->plugin->languageSupport();
}

bool DOcr::setLanguage(const QString &language)
{
    if (!pluginReady()) {
        qWarning() << "you need load a plugin first: " << __FUNCTION__;
        return false;
    }

    Q_D(DOcr);
    return d->plugin->setLanguage(language);
}

QList<TextBox> DOcr::textBoxes() const
{
    if (!pluginReady()) {
        qWarning() << "you need load a plugin first: " << __FUNCTION__;
        return QList<TextBox>();
    }

    Q_D(const DOcr);
    return d->plugin->textBoxes();
}

QList<TextBox> DOcr::charBoxes(int index) const
{
    if (!pluginReady()) {
        qWarning() << "you need load a plugin first: " << __FUNCTION__;
        return QList<TextBox>();
    }

    Q_D(const DOcr);
    return d->plugin->charBoxes(index);
}

QString DOcr::simpleResult() const
{
    if (!pluginReady()) {
        qWarning() << "you need load a plugin first: " << __FUNCTION__;
        return QString();
    }

    Q_D(const DOcr);
    return d->plugin->simpleResult();
}

QString DOcr::resultFromBox(int index) const
{
    if (!pluginReady()) {
        qWarning() << "you need load a plugin first: " << __FUNCTION__;
        return QString();
    }

    Q_D(const DOcr);
    return d->plugin->resultFromBox(index);
}

DOCR_END_NAMESPACE
