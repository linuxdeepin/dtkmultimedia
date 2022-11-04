// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DCOMPOSITEMANAGER_P_H
#define DCOMPOSITEMANAGER_P_H
#include "dcompositemanager.h"

DMULTIMEDIA_BEGIN_NAMESPACE

class DCompositeManagerPrivate : public QObject {
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
    static bool m_bCanHwdec;
    bool m_composited{false};
    Platform m_platform{Platform::Unknown};
    bool m_isCoreFlag{false};
    bool m_bHasCard;
    bool m_bOnlySoftDecode{false};
    bool m_setSpecialControls{false};
    bool m_bZXIntgraphics;
    QMap<QString, QString> *m_pMpvConfig = nullptr;
    QString m_cpuModelName;
    QString m_boardVendor;
    bool m_isWayland;

  private:
    DCompositeManager *q_ptr;
};
DMULTIMEDIA_END_NAMESPACE
#endif
