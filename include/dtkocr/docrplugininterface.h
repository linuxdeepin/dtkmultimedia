// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DOCRPLUGININTERFACE_H
#define DOCRPLUGININTERFACE_H

#include "dtkocr.h"
#include "docr.h"

DOCR_BEGIN_NAMESPACE

class DOcrPluginInterface
{
public:
    DOcrPluginInterface() = default;
    virtual ~DOcrPluginInterface() = default;
    virtual QList<Dtk::Ocr::HardwareID> hardwareSupportList() = 0;
    virtual bool setUseHardware(const QList<QPair<Dtk::Ocr::HardwareID, int>> &hardwareUsed);
    virtual bool setUseMaxThreadsCount(int n);
    virtual QStringList authKeys();
    virtual bool setAuth(const QStringList &params);
    virtual QStringList imageFileSupportFormats() const;
    virtual bool setImageFile(const QString &filePath);
    virtual bool setImage(const QImage &image);
    virtual QImage imageCached() const = 0;
    virtual bool pluginExpandParam(const QString &key) const;
    virtual bool setPluginExpandParam(const QString &key, const QString &value);
    virtual bool analyze() = 0;
    virtual bool breakAnalyze();
    virtual bool isRunning() const = 0;
    virtual QStringList languageSupport() const = 0;
    virtual bool setLanguage(const QString &language) = 0;
    virtual QList<Dtk::Ocr::TextBox> textBoxes() const;
    virtual QList<Dtk::Ocr::TextBox> charBoxes() const;
    virtual QString simpleResult() const = 0;
    virtual QString resultFromBox(int index) const;
};

DOCR_END_NAMESPACE

Q_DECLARE_INTERFACE(Dtk::Ocr::DOcrPluginInterface, "Dtk.Plugin.DOcrPluginInterface/1.0")

#endif
