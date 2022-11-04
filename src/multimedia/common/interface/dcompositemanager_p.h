// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DCOMPOSITEMANAGER_P_H
#define DCOMPOSITEMANAGER_P_H
#include "dcompositemanager.h"

DMULTIMEDIA_BEGIN_NAMESPACE

class DCompositeManagerPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DCompositeManager)

public:
    explicit DCompositeManagerPrivate(DCompositeManager *parent = nullptr)
        : QObject(parent), q_ptr(parent)
    {
    }

private:
    bool isDriverLoadedCorrectly();
    bool isProprietaryDriver();
    void initMember();
    static bool is_device_viable(int id);
    static bool is_card_exists(int id, const std::vector<std::string> &drivers);

private:
    static bool bCanHwdec;
    bool composited { false };
    Platform platform { Platform::Unknown };
    bool isCoreFlag { false };
    bool bHasCard { false };
    bool bOnlySoftDecode { false };
    bool setSpecialControls { false };
    bool bZXIntgraphics;
    QMap<QString, QString> *pMpvConfig = nullptr;
    QString cpuModelName;
    QString boardVendor;
    bool m_isWayland { false };

private:
    DCompositeManager *q_ptr;
};
DMULTIMEDIA_END_NAMESPACE
#endif
