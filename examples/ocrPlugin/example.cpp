// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "example.h"

#include <QFile>
#include <QImage>
#include <QProcessEnvironment>
#include <QThread>
#include <QtDebug>

ExampleDtkOcrPlugin::ExampleDtkOcrPlugin()
{
    inRunning = false;
    needBreak = false;

    QString fullPath;
    auto xdgDirs = QProcessEnvironment::systemEnvironment().value("XDG_DATA_DIRS").split(':');
    for (auto &xdgDir : xdgDirs) {
        fullPath = xdgDir + '/' + OCR_MODEL_DIR;
        if (QFile::exists(fullPath)) {
            modelPath = fullPath;
            break;
        }
    }

    if (modelPath.isEmpty()) {
        fullPath = "/usr/share/" OCR_MODEL_DIR;
        if (QFile::exists(fullPath)) {
            modelPath = fullPath;
        } else {
            qWarning() << "cannot find default model";
        }
    }

    if (!modelPath.isEmpty()) {
        modelPath += "model.pt";

        if (QFile::exists(modelPath)) {
            QFile modelFile(modelPath);
            modelFile.open(QIODevice::ReadOnly);
            qWarning() << "The first line of the model file is" << modelFile.readLine();
        }
    }
}

ExampleDtkOcrPlugin::~ExampleDtkOcrPlugin()
{
    ;
}

QList<Dtk::Ocr::HardwareID> ExampleDtkOcrPlugin::hardwareSupportList()
{
    return supportHardwares;
}

bool ExampleDtkOcrPlugin::setUseHardware(const QList<QPair<Dtk::Ocr::HardwareID, int>> &hardwareUsed)
{
    for (auto &hardwarePair : hardwareUsed) {
        if (!supportHardwares.contains(hardwarePair.first)) {
            return false;
        }
    }
    hardwareUses = hardwareUsed;
    return true;
}

bool ExampleDtkOcrPlugin::setUseMaxThreadsCount(int n)
{
    if (n > 0) {
        maxThreadsUse = n;
        return true;
    } else {
        return false;
    }
}

QStringList ExampleDtkOcrPlugin::authKeys()
{
    return {"ID", "CDKEY"};
}

bool ExampleDtkOcrPlugin::setAuth(const QStringList &params)
{
    if (params.size() < 2) {
        return false;
    }

    auto id = params[0].toInt();
    auto cdkey = params[1].toInt();

    if (id % cdkey == 7) {
        return true;
    } else {
        return false;
    }
}

QStringList ExampleDtkOcrPlugin::imageFileSupportFormats() const
{
    return {"jpg", "png"};
}

bool ExampleDtkOcrPlugin::setImageFile(const QString &filePath)
{
    QImage image(filePath);
    if (image.isNull()) {
        return false;
    }
    imageCache = image;
    return true;
}

bool ExampleDtkOcrPlugin::setImage(const QImage &image)
{
    imageCache = image;
    return true;
}

QImage ExampleDtkOcrPlugin::imageCached() const
{
    return imageCache;
}

QString ExampleDtkOcrPlugin::pluginExpandParam(const QString &key) const
{
    if (key == "GPU memory usage") {
        return "200MB";
    } else {
        return "";
    }
}

bool ExampleDtkOcrPlugin::setPluginExpandParam(const QString &key, const QString &value)
{
    if (key == "GPU memory usage") {
        int realValue = value.toInt();
        if (realValue > 0) {
            gpuMaxUsage = realValue;
            return true;
        }
    }
    return false;
}

bool ExampleDtkOcrPlugin::analyze()
{
    inRunning = true;
    needBreak = false;
    bool dataValid = true;

    for (int i = 0; i != 5; ++i) {
        if (needBreak) {
            dataValid = false;
            break;
        }
        QThread::sleep(1);
    }

    needBreak = false;
    inRunning = false;
    return dataValid;
}

bool ExampleDtkOcrPlugin::breakAnalyze()
{
    if (!needBreak && isRunning()) {
        needBreak = true;
        return true;
    } else {
        return false;
    }
}

bool ExampleDtkOcrPlugin::isRunning() const
{
    return inRunning;
}

QStringList ExampleDtkOcrPlugin::languageSupport() const
{
    return supportLanguages;
}

bool ExampleDtkOcrPlugin::setLanguage(const QString &language)
{
    if (supportLanguages.contains(language)) {
        useLanguage = language;
        return true;
    } else {
        return false;
    }
}

QString ExampleDtkOcrPlugin::simpleResult() const
{
    return "1233211234567";
}
