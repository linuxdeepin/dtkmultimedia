// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "docr.h"
#include "docr_p.h"
#include "docrplugininterface.h"
#include "ppocr.h"

#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QPluginLoader>
#include <QThread>
#include <QtDebug>

#include <dlfcn.h>

DOCR_BEGIN_NAMESPACE

DOcrPrivate::DOcrPrivate(DOcr *parent)
    : qtPluginLoader(new QPluginLoader(this))
    , q_ptr(parent)
{
    auto dlDir = currentModuleDir();
    if (!dlDir.isEmpty()) {
        pluginInstallDir = currentModuleDir() + "/libdtkocr/plugins";
    } else {
        qWarning() << "Unable to locate dtkocr library path";
        qWarning() << "Normal plugins will be disabled";
    }
}

static void dlAnchor() {}

void DOcrPrivate::resetPlugin()
{
    if (currentPluginType != Plugin_None && plugin->isRunning()) {
        plugin->breakAnalyze();
        while (plugin->isRunning()) {
            QThread::msleep(50);
        }
    }

    switch (currentPluginType) {
        default:
            delete plugin;
            break;
        case Plugin_Qt:
            qtPluginLoader->unload();
            break;
        case Plugin_Cpp:
            delete plugin;
            break;
        case Plugin_None:
            delete plugin;
            break;
    }

    plugin = nullptr;
    currentPluginType = Plugin_None;
}

QString DOcrPrivate::currentModuleDir()
{
    Dl_info dlInfo;
    int rc = 1;
    rc = dladdr(reinterpret_cast<const void *>(&dlAnchor), &dlInfo);
    if (rc == 0) {
        return "";
    }

    const QString &moduleFullPath = dlInfo.dli_fname;
    QFileInfo info(moduleFullPath);
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
    Q_D(DOcr);
    if (d->pluginInstallDir.isEmpty()) {
        qWarning() << "Normal plugin is disabled";
        return {};
    }

    QDir pluginDir(d->pluginInstallDir);
    if (!pluginDir.exists()) {
        return {};
    }

    return pluginDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
}

bool DOcr::loadDefaultPlugin()
{
    Q_D(DOcr);
    if (pluginReady()) {
        d->resetPlugin();
    }
    d->plugin = new PaddleOCRApp;
    d->currentPluginType = DOcrPrivate::Plugin_Default;
    return true;
}

bool DOcr::loadPlugin(const QString &pluginName)
{
    Q_D(DOcr);
    if (d->pluginInstallDir.isEmpty()) {
        qWarning() << "Normal plugin is disabled";
        return {};
    }

    auto pluginLibPath = d->pluginInstallDir + "/" + pluginName + "/libload.so";
    if (!QFile::exists(pluginLibPath)) {
        qWarning() << "Cannot find plugin entrance file:" << pluginLibPath;
        return false;
    }

    d->qtPluginLoader->setFileName(pluginLibPath);
    bool ret = d->qtPluginLoader->load();
    qDebug() << "load plugin: " << ret;
    if (!ret) {
        qWarning() << "Error message:" << d->qtPluginLoader->errorString();
    }
    auto pluginObject = d->qtPluginLoader->instance();
    if (pluginObject != nullptr) {
        auto pluginDetail = qobject_cast<DOcrPluginInterface *>(pluginObject);
        if (pluginDetail != nullptr) {
            if (pluginReady()) {
                d->resetPlugin();
            }
            d->plugin = pluginDetail;
            d->currentPluginType = DOcrPrivate::Plugin_Qt;
            return true;
        }
    }

    qWarning() << "Cannot load plugin from" << pluginLibPath;
    qWarning() << "Error message:" << d->qtPluginLoader->errorString();
    return false;
}

bool DOcr::pluginReady() const
{
    Q_D(const DOcr);
    return d->currentPluginType != DOcrPrivate::Plugin_None;
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
