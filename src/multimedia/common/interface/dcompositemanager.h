// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCOMPOSITEMANAGER_H
#define DCOMPOSITEMANAGER_H
#define MWV206_0  

#include <string>
#include <QtCore>
#include <vector>
#include <QGSettings/QGSettings>
#include <dtkmultimedia.h>

DMULTIMEDIA_BEGIN_NAMESPACE

enum Platform {
    Unknown,
    X86,  
    Mips, 
    Alpha, 
    Arm64
};

enum OpenGLInteropKind {
    InteropNone,
    InteropAuto,
    InteropVaapiEGL,
    InteropVaapiGLX,
    InteropVdpauGLX,
};

using PlayerOption = QPair<QString, QString>;
using PlayerOptionList = QList<PlayerOption>;

class DCompositeManager: public QObject
{
public:
    static DCompositeManager &get();
    virtual ~DCompositeManager();
    static void detectOpenGLEarly();
    static OpenGLInteropKind interopKind();
    static bool runningOnVmwgfx();
    static bool isPadSystem();  
    static bool isCanHwdec();
    static void setCanHwdec(bool bCanHwdec);
    static bool isMpvExists();
#if !defined (__x86_64__)
    bool hascard();
#endif
    void overrideCompositeMode(bool useCompositing);
    bool composited() const;
    Platform platform() const;
    bool isTestFlag() const;
    void setTestFlag(bool flag);
    bool isZXIntgraphics() const;
    PlayerOptionList getProfile(const QString &name);
    PlayerOptionList getBestProfile();
    static void detectPciID();
    static bool runningOnNvidia();
    void softDecodeCheck();
    bool isOnlySoftDecode();
    bool isSpecialControls();
    void getMpvConfig(QMap<QString, QString> *&aimMap);
    bool first_check_wayland_env();
    bool check_wayland_env();

signals:
    void compositingChanged(bool);

private:
    DCompositeManager();
    bool isDriverLoadedCorrectly();
    bool isDirectRendered();
    bool isProprietaryDriver();
    void initMember();
    static bool is_device_viable(int id);
    static bool is_card_exists(int id, const std::vector<std::string> &drivers);

public:
    static bool m_bCanHwdec;
    
private:
    bool m_composited {false};
    Platform m_platform {Platform::Unknown};
    bool m_isCoreFlag {false};
    bool m_bHasCard;
    bool m_bOnlySoftDecode {false};
    bool m_setSpecialControls {false};
    bool m_bZXIntgraphics;
    QMap<QString, QString> *m_pMpvConfig;
    QString m_cpuModelName;
    QString m_boardVendor;
    bool m_isWayland;
};
DMULTIMEDIA_END_NAMESPACE

#endif /* ifndef DCOMPOSITEMANAGER_H */

