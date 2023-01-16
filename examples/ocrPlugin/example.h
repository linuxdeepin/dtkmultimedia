// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <DOcrPluginInterface>
#include <QImage>
#include <QObject>
#include <atomic>

class ExampleDtkOcrPlugin : public QObject, public Dtk::Ocr::DOcrPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DOcrPluginInterface_iid FILE "qtplugin.json")
    Q_INTERFACES(Dtk::Ocr::DOcrPluginInterface)

public:
    ExampleDtkOcrPlugin();
    ~ExampleDtkOcrPlugin() override;

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

private:
    QString modelPath;
    QStringList supportLanguages = {"zh_Ancient", "zh_Modern"};
    QString useLanguage = "zh_Modern";
    QList<Dtk::Ocr::HardwareID> supportHardwares = {Dtk::Ocr::CPU_X86, Dtk::Ocr::GPU_NVIDIA};
    QList<QPair<Dtk::Ocr::HardwareID, int>> hardwareUses = {{Dtk::Ocr::CPU_X86, 0}};
    int maxThreadsUse = 1;
    QImage imageCache;
    int gpuMaxUsage = 200;
    std::atomic_bool inRunning;
    std::atomic_bool needBreak;
};

#endif
