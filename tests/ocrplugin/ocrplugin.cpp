// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

// This is a test resource for DOcrPluginInterface to load a plugin

#include "docrplugininterface.h"

#include <QFile>
#include <QImage>
#include <QProcessEnvironment>
#include <QThread>
#include <QtDebug>

class OcrPlugin : public QObject, public Dtk::Ocr::DOcrPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(Dtk::Ocr::DOcrPluginInterface)
    Q_PLUGIN_METADATA(IID DOcrPluginInterface_iid FILE "qtplugin.json")

public:
    OcrPlugin();
    ~OcrPlugin() override;
    QList<Dtk::Ocr::HardwareID> hardwareSupportList() override;
    bool setUseHardware(const QList<QPair<Dtk::Ocr::HardwareID, int>> &hardwareUsed) override;
    bool setUseMaxThreadsCount(int n) override;
    QStringList authKeys() override;
    bool setAuth(const QStringList &params) override;
    QStringList imageFileSupportFormats() const override;
    bool setImageFile(const QString &filePath) override;
    bool setImage(const QImage &image) override;
    QImage imageCached() const override;
    QString pluginExpandParam(const QString &key) const override;
    bool setPluginExpandParam(const QString &key, const QString &value) override;
    bool analyze() override;
    bool breakAnalyze() override;
    bool isRunning() const override;
    QStringList languageSupport() const override;
    bool setLanguage(const QString &language) override;
    QString simpleResult() const override;
    QList<Dtk::Ocr::TextBox> textBoxes() const override;
    QList<Dtk::Ocr::TextBox> charBoxes(int index) const override;
    QString resultFromBox(int index) const override;

private:
    QString modelPath;
    QStringList supportLanguages = {"zh_Ancient", "zh_Modern"};
    QString useLanguage = "zh_Modern";
    QList<Dtk::Ocr::HardwareID> supportHardwares = {Dtk::Ocr::CPU_X86, Dtk::Ocr::GPU_NVIDIA};
    QList<QPair<Dtk::Ocr::HardwareID, int>> hardwareUses = {{Dtk::Ocr::CPU_X86, 0}};
    int maxThreadsUse = 1;
    int gpuMaxUsage = 200;
    std::atomic_bool inRunning;
    std::atomic_bool needBreak;
    QImage imageCache;
};

OcrPlugin::OcrPlugin()
{
    inRunning = false;
    needBreak = false;
}

OcrPlugin::~OcrPlugin()
{
    ;
}

QList<Dtk::Ocr::HardwareID> OcrPlugin::hardwareSupportList()
{
    return supportHardwares;
}

bool OcrPlugin::setUseHardware(const QList<QPair<Dtk::Ocr::HardwareID, int>> &hardwareUsed)
{
    for (auto &hardwarePair : hardwareUsed) {
        if (!supportHardwares.contains(hardwarePair.first)) {
            return false;
        }
    }
    hardwareUses = hardwareUsed;
    return true;
}

bool OcrPlugin::setUseMaxThreadsCount(int n)
{
    if (n > 0) {
        maxThreadsUse = n;
        return true;
    } else {
        return false;
    }
}

QStringList OcrPlugin::authKeys()
{
    return {"Product ID", "CDKEY"};
}

bool OcrPlugin::setAuth(const QStringList &params)
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

QStringList OcrPlugin::imageFileSupportFormats() const
{
    return {"jpg", "png"};
}

bool OcrPlugin::setImageFile(const QString &filePath)
{
    QImage image(filePath);
    if (image.isNull()) {
        return false;
    }
    imageCache = image;
    return true;
}

bool OcrPlugin::setImage(const QImage &image)
{
    imageCache = image;
    return true;
}

QImage OcrPlugin::imageCached() const
{
    return imageCache;
}

QString OcrPlugin::pluginExpandParam(const QString &key) const
{
    if (key == "GPU memory usage") {
        return QString("%1MB").arg(gpuMaxUsage);
    } else {
        return "";
    }
}

bool OcrPlugin::setPluginExpandParam(const QString &key, const QString &value)
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

bool OcrPlugin::analyze()
{
    inRunning = true;
    needBreak = false;
    bool dataValid = true;

    for (int i = 0; i != 5; ++i) {
        if (needBreak) {
            dataValid = false;
            break;
        }
        QThread::msleep(10);
    }

    needBreak = false;
    inRunning = false;
    return dataValid;
}

bool OcrPlugin::breakAnalyze()
{
    if (!needBreak && isRunning()) {
        needBreak = true;
        return true;
    } else {
        return false;
    }
}

bool OcrPlugin::isRunning() const
{
    return inRunning;
}

QStringList OcrPlugin::languageSupport() const
{
    return supportLanguages;
}

bool OcrPlugin::setLanguage(const QString &language)
{
    if (supportLanguages.contains(language)) {
        useLanguage = language;
        return true;
    } else {
        return false;
    }
}

QString OcrPlugin::simpleResult() const
{
    return "1233211234567";
}

QList<Dtk::Ocr::TextBox> OcrPlugin::textBoxes() const
{
    Dtk::Ocr::TextBox box;
    box.points = {{1.1, 1.1}, {2.2, 2.2}, {3.3, 3.3}, {4.4, 4.4}};
    box.angle = 10.1;
    return {box};
}

QList<Dtk::Ocr::TextBox> OcrPlugin::charBoxes(int index) const
{
    if (index != 0) {
        return {};
    }

    Dtk::Ocr::TextBox box;
    box.points = {{4.4, 4.4}, {3.3, 3.3}, {2.2, 2.2}, {1.1, 1.1}};
    box.angle = 1.01;
    return {box};
}

QString OcrPlugin::resultFromBox(int index) const
{
    if (index != 0) {
        return {};
    }

    return "1234567321123";
}

#include "ocrplugin.moc"
