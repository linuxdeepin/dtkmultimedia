// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

//#include "config.h"
#include "dcompositemanager.h"
#include <iostream>
#include <unistd.h>
#include <QtCore>
#include <QtGui>
#include <QX11Info>

#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <GL/glxext.h>
#undef Bool
#include "dmpvhelper.hpp"

#define BUFFERSIZE 255

typedef const char *glXGetScreenDriver_t (Display *dpy, int scrNum);

static glXGetScreenDriver_t *GetScreenDriver;

//TODO: override by user setting

DMULTIMEDIA_BEGIN_NAMESPACE
using namespace std;

static DCompositeManager *_compManager = nullptr;
bool DCompositeManager::m_bCanHwdec = true;

#define C2Q(cs) (QString::fromUtf8((cs).c_str()))

class PlatformChecker
{
public:
    PlatformChecker() {}
    Platform check()
    {
        QProcess uname;
        uname.start("uname -m");
        if (uname.waitForStarted()) {
            if (uname.waitForFinished()) {
                auto data = uname.readAllStandardOutput();
                string machine(data.trimmed().constData());
                qInfo() << QString("machine: %1").arg(machine.c_str());

                QRegExp re("x86.*|i?86|ia64", Qt::CaseInsensitive);
                if (re.indexIn(C2Q(machine)) != -1) {
                    qInfo() << "match x86";
                    _pf = Platform::X86;

                } else if (machine.find("alpha") != string::npos
                           || machine.find("sw_64") != string::npos) {
                    // shenwei
                    qInfo() << "match shenwei";
                    _pf = Platform::Alpha;

                } else if (machine.find("mips") != string::npos
                           || machine.find("loongarch64") != string::npos) { // loongson
                    qInfo() << "match loongson";
                    _pf = Platform::Mips;
                } else if (machine.find("aarch64") != string::npos) { // ARM64
                    qInfo() << "match arm";
                    _pf = Platform::Arm64;
                }
            }
        }

        return _pf;
    }

private:
    Platform _pf {Platform::Unknown};
};


DCompositeManager &DCompositeManager::get()
{
    if (!_compManager) {
        _compManager = new DCompositeManager();
    }

    return *_compManager;
}

//void compositingChanged(bool);

DCompositeManager::DCompositeManager()
{
    initMember();
    bool isDriverLoaded = isDriverLoadedCorrectly();
    softDecodeCheck();   //检测是否是kunpeng920（是否走软解码）

    bool isI915 = false;
    for (int id = 0; id <= 10; id++) {
        if (!QFile::exists(QString("/sys/class/drm/card%1").arg(id))) break;
        if (is_device_viable(id)) {
            vector<string> drivers = {"i915"};
            isI915 = is_card_exists(id, drivers);
            break;
        }
    }
    if (isI915) qInfo() << "is i915!";
    m_bZXIntgraphics = isI915 ? isI915 : m_bZXIntgraphics;

    if (check_wayland_env()) {
        m_composited = true;
        if (m_platform == Platform::Arm64 && isDriverLoaded)
            m_bHasCard = true;
        qInfo() << __func__ << "Composited is " << m_composited;
        return;
    }

    if (m_platform == Platform::X86) {
        if (m_bZXIntgraphics) {
            m_composited = false;
        } else {
            m_composited = true;
        }
    } else {
        if (m_platform == Platform::Arm64 && isDriverLoaded)
            m_bHasCard = true;
        m_composited = false;
    }
    if(!isMpvExists())
    {
        m_composited = true;
    }
    qInfo() << __func__ << "Composited is " << m_composited;
}

DCompositeManager::~DCompositeManager()
{
    delete m_pMpvConfig;
    m_pMpvConfig = nullptr;
}

#if !defined (__x86_64__)
bool DCompositeManager::hascard()
{
    return m_bHasCard;
}
#endif

static OpenGLInteropKind _interopKind = OpenGLInteropKind::InteropNone;

bool DCompositeManager::runningOnVmwgfx()
{
    static bool s_runningOnVmwgfx = false;
    for (int id = 0; id <= 10; id++) {
        if (!QFile::exists(QString("/sys/class/drm/card%1").arg(id))) break;
        if (is_device_viable(id)) {
            vector<string> drivers = {"vmwgfx"};
            s_runningOnVmwgfx = is_card_exists(id, drivers);
            break;
        }
    }

    return s_runningOnVmwgfx;
}

bool DCompositeManager::isPadSystem()
{
    return false;
}

bool DCompositeManager::isCanHwdec()
{
    return m_bCanHwdec;
}

void DCompositeManager::setCanHwdec(bool bCanHwdec)
{
    m_bCanHwdec = bCanHwdec;
}

bool DCompositeManager::isMpvExists()
{
    QDir dir;
    QString path  = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    dir.setPath(path);
    QStringList list = dir.entryList(QStringList() << (QString("libmpv.so.1") + "*"), QDir::NoDotAndDotDot | QDir::Files);
    if (list.contains("libmpv.so.1")) {
        return true;
    }
    return false;
}

bool DCompositeManager::isZXIntgraphics() const
{
    return m_bZXIntgraphics;
}

bool DCompositeManager::runningOnNvidia()
{
    static bool s_runningOnNvidia = false;

    for (int id = 0; id <= 10; id++) {
        if (!QFile::exists(QString("/sys/class/drm/card%1").arg(id))) break;
        if (is_device_viable(id)) {
            vector<string> drivers = {"nvidia"};
            s_runningOnNvidia = is_card_exists(id, drivers);
            break;
        }
    }

    return s_runningOnNvidia;
}

void DCompositeManager::softDecodeCheck()
{
    //获取cpu型号
    QFile cpuInfo("/proc/cpuinfo");
    if (cpuInfo.open(QIODevice::ReadOnly)) {
        QString line = cpuInfo.readLine();
        while (!cpuInfo.atEnd()) {
            line = cpuInfo.readLine();
            QStringList listPara = line.split(":");
            qInfo() << listPara;
            if (listPara.size() < 2) {
                continue;
            }
            if (listPara.at(0).contains("model name")) {
                m_cpuModelName = listPara.at(1);
                break;
            }
        }
        cpuInfo.close();
    }

    //获取设备名
    QFile board("/sys/class/dmi/id/board_vendor");
    if (board.open(QIODevice::ReadOnly)) {
        QString line = board.readLine();
        while (!board.atEnd()) {
            m_boardVendor = line;
            break;
        }
        board.close();
    }

    if ((runningOnNvidia() && m_boardVendor.contains("Sugon"))
            || m_cpuModelName.contains("Kunpeng 920")) {
        m_bOnlySoftDecode = true;
    }
    if(m_boardVendor.toLower().contains("huawei")) {
        m_bHasCard = true;
    }

    m_setSpecialControls = m_boardVendor.contains("Ruijie");

    //判断N卡驱动版本
    QFile nvidiaVersion("/proc/driver/nvidia/version");
    if (nvidiaVersion.open(QIODevice::ReadOnly)) {
        QString str = nvidiaVersion.readLine();
        int start = str.indexOf("Module");
        start += 6;
        QString version = str.mid(start, 6);
        while (version.left(1) == " ") {
            start++;
            version = str.mid(start, 6);
        }
        qInfo() << "nvidia version :" << version;
        if (version.toFloat() >= 460.39) {
            m_bOnlySoftDecode = true;
        }
        nvidiaVersion.close();
    }
}

bool DCompositeManager::isOnlySoftDecode()
{
    return m_bOnlySoftDecode;
}

bool DCompositeManager::isSpecialControls()
{
    return m_setSpecialControls;
}

void DCompositeManager::detectOpenGLEarly()
{
    static bool detect_run = false;

    if (detect_run) return;

#ifndef USE_DXCB
    /*
     * see mpv/render_gl.h for more details, below is copied verbatim:
     *
     * - Intel/Linux: EGL is required, and also the native display resource needs
     *                to be provided (e.g. MPV_RENDER_PARAM_X11_DISPLAY for X11 and
     *                MPV_RENDER_PARAM_WL_DISPLAY for Wayland)
     * - nVidia/Linux: Both GLX and EGL should work (GLX is required if vdpau is
     *                 used, e.g. due to old drivers.)
     *
     * mpv hwdec is broken with vmwgfx and should use glx
     */
    if (DCompositeManager::runningOnNvidia()) {
        qputenv("QT_XCB_GL_INTEGRATION", "xcb_glx");
    } else if (!DCompositeManager::runningOnVmwgfx()) {
        auto e = QProcessEnvironment::systemEnvironment();
        QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
        QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

        if (XDG_SESSION_TYPE != QLatin1String("wayland") &&
                !WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
            qputenv("QT_XCB_GL_INTEGRATION", "xcb_egl");
        }
    }
#else
    if (_interopKind == INTEROP_VAAPI_EGL) {
        _interopKind = INTEROP_VAAPI_GLX;
    }

#endif

    detect_run = true;
}

void DCompositeManager::detectPciID()
{
    QProcess pcicheck;
    pcicheck.start("lspci -vn");
    if (pcicheck.waitForStarted() && pcicheck.waitForFinished()) {

        auto data = pcicheck.readAllStandardOutput();

        QString output(data.trimmed().constData());
        qInfo() << "DCompositeManager::detectPciID()" << output.split(QChar('\n')).count();

        QStringList outlist = output.split(QChar('\n'));
        foreach (QString line, outlist) {
            if (line.contains(QString("00:02.0"))) {
                if (line.contains(QString("8086")) && line.contains(QString("1912"))) {
                    qInfo() << "DCompositeManager::detectPciID():need to change to iHD";
                    qputenv("LIBVA_DRIVER_NAME", "iHD");
                    break;
                }
            }
        }
    }
}

void DCompositeManager::getMpvConfig(QMap<QString, QString> *&aimMap)
{
    aimMap = nullptr;
    if (nullptr != m_pMpvConfig) {
        aimMap = m_pMpvConfig;
    }
}

OpenGLInteropKind DCompositeManager::interopKind()
{
    return _interopKind;
}

bool DCompositeManager::isDriverLoadedCorrectly()
{
    static QRegExp aiglx_err("\\(EE\\)\\s+AIGLX error");
    static QRegExp dri_ok("direct rendering: DRI\\d+ enabled");
    static QRegExp swrast("GLX: Initialized DRISWRAST");
    static QRegExp regZX("loading driver: zx");

    QString xorglog = QString("/var/log/Xorg.%1.log").arg(QX11Info::appScreen());
    qInfo() << "check " << xorglog;
    QFile f(xorglog);
    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "can not open " << xorglog;
        return false;
    }

    QTextStream ts(&f);
    while (!ts.atEnd()) {
        QString ln = ts.readLine();
        if (aiglx_err.indexIn(ln) != -1) {
            qInfo() << "found aiglx error";
            return false;
        }

        if (dri_ok.indexIn(ln) != -1) {
            qInfo() << "dri enabled successfully";
            return true;
        }

        if (swrast.indexIn(ln) != -1) {
            qInfo() << "swrast driver used";
            return false;
        }

        if (regZX.indexIn(ln) != -1) {
            m_bZXIntgraphics = true;
        }
    }
    f.close();
    return true;
}

void DCompositeManager::overrideCompositeMode(bool useCompositing)
{
    if (m_composited != useCompositing) {
        qInfo() << "override composited = " << useCompositing;
        m_composited = useCompositing;
    }
}

bool DCompositeManager::composited() const
{
    return m_composited;
}

Platform DCompositeManager::platform() const
{
    return m_platform;
}

bool DCompositeManager::isTestFlag() const
{
    return m_isCoreFlag;
}
void DCompositeManager::setTestFlag(bool flag)
{
    m_isCoreFlag = flag;
}

using namespace std;

bool DCompositeManager::is_card_exists(int id, const vector<string> &drivers)
{
    char buf[1024] = {0};
    snprintf(buf, sizeof buf, "/sys/class/drm/card%d/device/driver", id);

    char buf2[1024] = {0};
    if (readlink(buf, buf2, sizeof buf2) < 0) {
        return false;
    }

    string driver = basename(buf2);
    qInfo() << "drm driver " << driver.c_str();
    if (std::any_of(drivers.cbegin(), drivers.cend(), [ = ](string s) {return s == driver;})) {
        return true;
    }

    return false;
}

bool DCompositeManager::is_device_viable(int id)
{
    char path[128];
    snprintf(path, sizeof path, "/sys/class/drm/card%d", id);
    if (access(path, F_OK) != 0) {
        return false;
    }

    //OK, on shenwei, this file may have no read permission for group/other.
    char buf[512];
    snprintf(buf, sizeof buf, "%s/device/enable", path);
    if (access(buf, R_OK) == 0) {
        FILE *fp = fopen(buf, "r");
        if (!fp) {
            return false;
        }

        int enabled = 0;
        int error = fscanf(fp, "%d", &enabled);
        if (error < 0) {
            qInfo() << "someting error";
        }
        fclose(fp);

        // nouveau may write 2, others 1
        return enabled > 0;
    }

    return false;
}

bool DCompositeManager::isProprietaryDriver()
{
    for (int id = 0; id <= 10; id++) {
        if (!QFile::exists(QString("/sys/class/drm/card%1").arg(id))) break;
        if (is_device_viable(id)) {
            vector<string> drivers = {"nvidia", "fglrx", "vmwgfx", "hibmc-drm", "radeon", "i915", "amdgpu", "phytium_display"};
            return is_card_exists(id, drivers);
        }
    }

    return false;
}

void DCompositeManager::initMember()
{
    m_pMpvConfig = nullptr;
    m_platform = PlatformChecker().check();

    m_bZXIntgraphics = false;
    m_bHasCard = false;
}

//this is not accurate when proprietary driver used
bool DCompositeManager::isDirectRendered()
{
//避免klu 上产生xdriinfo的coredump
//    QProcess xdriinfo;
//    xdriinfo.start("xdriinfo driver 0");
//    if (xdriinfo.waitForStarted() && xdriinfo.waitForFinished()) {
//        QString drv = QString::fromUtf8(xdriinfo.readAllStandardOutput().trimmed().constData());
//        qInfo() << "xdriinfo: " << drv;
//        return !drv.contains("not direct rendering capable");
//    }

    return true;
}

//FIXME: what about merge options from both config
PlayerOptionList DCompositeManager::getProfile(const QString &name)
{
    auto localPath = QString("%1/%2/%3/%4.profile")
                     .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                     .arg(qApp->organizationName())
                     .arg(qApp->applicationName())
                     .arg(name);
    auto defaultPath = QString(":/resources/profiles/%1.profile").arg(name);
    QString oc;

    PlayerOptionList ol;

    QList<QString> files = {oc, localPath, defaultPath};
    auto p = files.begin();
    while (p != files.end()) {
        QFileInfo fi(*p);
        if (fi.exists()) {
            qInfo() << "load" << fi.absoluteFilePath();
            QFile f(fi.absoluteFilePath());
            f.open(QIODevice::ReadOnly);
            QTextStream ts(&f);
            while (!ts.atEnd()) {
                auto l = ts.readLine().trimmed();
                if (l.isEmpty()) continue;

                auto kv = l.split("=");
                qInfo() << l << kv;
                if (kv.size() == 1) {
                    ol.push_back(qMakePair(kv[0], QString::fromUtf8("")));
                } else {
                    ol.push_back(qMakePair(kv[0], kv[1]));
                }
            }
            f.close();

            return ol;
        }
        ++p;
    }

    return ol;
}

PlayerOptionList DCompositeManager::getBestProfile()
{
    QString profile_name = "default";
    switch (m_platform) {
    case Platform::Alpha:
    case Platform::Mips:
    case Platform::Arm64:
        profile_name = m_composited ? "composited" : "failsafe";
        break;
    case Platform::X86:
        profile_name = m_composited ? "composited" : "default";
        break;
    case Platform::Unknown:
        break;
    }

    return getProfile(profile_name);
}

bool DCompositeManager::first_check_wayland_env()
{
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        m_isWayland = true;
        return true;
    } else {
        m_isWayland = false;
        return false;
    }
}

bool DCompositeManager::check_wayland_env()
{
    return m_isWayland;
}

#undef C2Q
DMULTIMEDIA_END_NAMESPACE

