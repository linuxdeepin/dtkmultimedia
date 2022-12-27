// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOCR_H
#define DOCR_H

#include "dtkocr.h"

#include <QList>
#include <QObject>
#include <QPointF>

DOCR_BEGIN_NAMESPACE

enum HardwareID {
    Network = -1,
    CPU_Any = 0,
    CPU_X86,
    CPU_AArch64,
    CPU_MIPS,
    CPU_LoongArch,
    CPU_SW,
    CPU_RISC_V,

    GPU_Any = 100,
    GPU_Vulkan,
    GPU_NVIDIA,
    GPU_AMD,
    GPU_Intel,
    GPU_MT,
    GPU_JM,
    GPU_Loongson,
    GPU_Innosilicon,
    GPU_LM,
    GPU_BR
};

struct TextBox
{
    QList<QPointF> points;
    qreal angle;
};

class DOcrPrivate;
class DOcr : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DOcr)

public:
    explicit DOcr(QObject *parent = nullptr);
    ~DOcr();
    QStringList installedPluginNames();
    bool loadDefaultPlugin();
    bool loadPlugin(const QString &pluginName);
    bool pluginReady() const;
    QList<HardwareID> hardwareSupportList() const;
    bool setUseHardware(const QList<QPair<HardwareID, int>> &hardwareUsed);
    bool setUseMaxThreadsCount(int n);
    QStringList authKeys() const;
    bool setAuth(const QStringList &params);
    QStringList imageFileSupportFormats() const;
    bool setImageFile(const QString &filePath);
    bool setImage(const QImage &image);
    QImage imageCached() const;
    QString pluginExpandParam(const QString &key) const;
    bool setPluginExpandParam(const QString &key, const QString &value);
    bool analyze();
    bool breakAnalyze();
    bool isRunning() const;
    QStringList languageSupport() const;
    bool setLanguage(const QString &language);
    QList<TextBox> textBoxes() const;
    QList<TextBox> charBoxes(int index) const;
    QString simpleResult() const;
    QString resultFromBox(int index) const;

protected:
    QScopedPointer<DOcrPrivate> d_ptr;
};

DOCR_END_NAMESPACE

#endif
