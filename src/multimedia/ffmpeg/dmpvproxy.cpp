// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "config.h"
#include "dcompositemanager.h"
#include "dmpvproxy_p.h"
#include <mpv/client.h>

#include <QtCore>
#include <QtGlobal>
#include <random>

#include <QLibrary>
#include <QX11Info>

DMULTIMEDIA_BEGIN_NAMESPACE

enum AsyncReplyTag { SEEK, CHANNEL, SPEED };
typedef enum {
    UN_KNOW = 0, //初始值
    MPEG1,       //下面为各种视频格式
    MPEG2,
    MPEG4,
    H264,
    VC1,
    DIVX4,
    DIVX5,
    HEVC,
    _MAXNULL       //超限处理
} decoder_profile; //视频格式解码请求值
typedef int VdpBool;
typedef enum {
    decoder_profiles_MPEG1 = 0,                 //     {"MPEG1", VDP_DECODER_PROFILE_MPEG1},
    decoder_profiles_MPEG2_SIMPLE,              //     {"MPEG2_SIMPLE", VDP_DECODER_PROFILE_MPEG2_SIMPLE},
    decoder_profiles_MPEG2_MAIN,                //     {"MPEG2_MAIN", VDP_DECODER_PROFILE_MPEG2_MAIN},
    decoder_profiles_H264_BASELINE,             //     {"H264_BASELINE", VDP_DECODER_PROFILE_H264_BASELINE},
    decoder_profiles_H264_MAIN,                 //     {"H264_MAIN", VDP_DECODER_PROFILE_H264_MAIN},
    decoder_profiles_H264_HIGH,                 //     {"H264_HIGH", VDP_DECODER_PROFILE_H264_HIGH},
    decoder_profiles_VC1_SIMPLE,                //     {"VC1_SIMPLE", VDP_DECODER_PROFILE_VC1_SIMPLE},
    decoder_profiles_VC1_MAIN,                  //     {"VC1_MAIN", VDP_DECODER_PROFILE_VC1_MAIN},
    decoder_profiles_VC1_ADVANCED,              //     {"VC1_ADVANCED", VDP_DECODER_PROFILE_VC1_ADVANCED},
    decoder_profiles_MPEG4_PART2_SP,            //     {"MPEG4_PART2_SP", VDP_DECODER_PROFILE_MPEG4_PART2_SP},
    decoder_profiles_MPEG4_PART2_ASP,           //     {"MPEG4_PART2_ASP",
                                                //     VDP_DECODER_PROFILE_MPEG4_PART2_ASP},
    decoder_profiles_DIVX4_QMOBILE,             //     {"DIVX4_QMOBILE", VDP_DECODER_PROFILE_DIVX4_QMOBILE},
    decoder_profiles_DIVX4_MOBILE,              //     {"DIVX4_MOBILE", VDP_DECODER_PROFILE_DIVX4_MOBILE},
    decoder_profiles_DIVX4_HOME_THEATER,        //     {"DIVX4_HOME_THEATER",
                                                //     VDP_DECODER_PROFILE_DIVX4_HOME_THEATER},
    decoder_profiles_DIVX4_HD_1080P,            //     {"DIVX4_HD_1080P", VDP_DECODER_PROFILE_DIVX4_HD_1080P},
    decoder_profiles_DIVX5_QMOBILE,             //     {"DIVX5_QMOBILE", VDP_DECODER_PROFILE_DIVX5_QMOBILE},
    decoder_profiles_DIVX5_MOBILE,              //     {"DIVX5_MOBILE", VDP_DECODER_PROFILE_DIVX5_MOBILE},
    decoder_profiles_DIVX5_HOME_THEATER,        //     {"DIVX5_HOME_THEATER",
                                                //     VDP_DECODER_PROFILE_DIVX5_HOME_THEATER},
    decoder_profiles_DIVX5_HD_1080P,            //     {"DIVX5_HD_1080P", VDP_DECODER_PROFILE_DIVX5_HD_1080P},
    decoder_profiles_H264_CONSTRAINED_BASELINE, //     {"H264_CONSTRAINED_BASELINE",
                                                //     VDP_DECODER_PROFILE_H264_CONSTRAINED_BASELINE},
    decoder_profiles_H264_EXTENDED,             //     {"H264_EXTENDED", VDP_DECODER_PROFILE_H264_EXTENDED},
    decoder_profiles_H264_PROGRESSIVE_HIGH,     //     {"H264_PROGRESSIVE_HIGH",
                                                //     VDP_DECODER_PROFILE_H264_PROGRESSIVE_HIGH},
    decoder_profiles_H264_CONSTRAINED_HIGH,     //     {"H264_CONSTRAINED_HIGH",
                                                //     VDP_DECODER_PROFILE_H264_CONSTRAINED_HIGH},
    decoder_profiles_H264_HIGH_444_PREDICTIVE,  //     {"H264_HIGH_444_PREDICTIVE",
                                                //     VDP_DECODER_PROFILE_H264_HIGH_444_PREDICTIVE},
    decoder_profiles_HEVC_MAIN,                 //     {"HEVC_MAIN", VDP_DECODER_PROFILE_HEVC_MAIN},
    decoder_profiles_HEVC_MAIN_10,              //     {"HEVC_MAIN_10", VDP_DECODER_PROFILE_HEVC_MAIN_10},
    decoder_profiles_HEVC_MAIN_STILL,           //     {"HEVC_MAIN_STILL",
                                                //     VDP_DECODER_PROFILE_HEVC_MAIN_STILL},
    decoder_profiles_HEVC_MAIN_12,              //     {"HEVC_MAIN_12", VDP_DECODER_PROFILE_HEVC_MAIN_12},
    decoder_profiles_HEVC_MAIN_444,             //     {"HEVC_MAIN_444", VDP_DECODER_PROFILE_HEVC_MAIN_444},
    _decoder_maxnull
} VDP_Decoder_e;
#define RET_INFO_LENTH_MAX (512)
typedef struct {
    VDP_Decoder_e func;                //具体值的功能查询
    VdpBool is_supported;              //是否支持具体值硬解码
    uint32_t max_width;                //最大支持视频宽度
    uint32_t max_height;               //最大支持视频高度
    uint32_t max_level;                //最大支持等级
    uint32_t max_macroblocks;          //最大宏块大小
    char ret_info[RET_INFO_LENTH_MAX]; //支持的列表
} VDP_Decoder_t;
struct nodeAutofree {
    mpv_node *pNode;
    explicit nodeAutofree(mpv_node *pValue) : pNode(pValue)
    {
    }
    ~nodeAutofree()
    {
        mpv_freeNode_contents(pNode);
    }
};
DMULTIMEDIA_END_NAMESPACE // end of namespace

    DMULTIMEDIA_USE_NAMESPACE
    //返回值大于0表示支持硬解， index 视频格式解码请求值， result 返回解码支持信息
    typedef unsigned int (*gpu_decoderInfo)(decoder_profile index, VDP_Decoder_t *result);

static QString libPath(const QString &sLib)
{
    QDir dir;
    QString path = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    dir.setPath(path);
    QStringList list = dir.entryList(QStringList() << (sLib + "*"),
        QDir::NoDotAndDotDot | QDir::Files); // filter name with strlib
    if(list.contains(sLib)) {
        return sLib;
    }
    else {
        list.sort();
    }

    if(list.size() > 0)
        return list.last();
    else
        return QString();
}


MpvHandle::container::container(mpv_handle *pHandle) : m_pHandle(pHandle)
{
}

MpvHandle::container::~container()
{
    mpv_terminateDestroy func =
        (mpv_terminateDestroy) QLibrary::resolve(libPath("libmpv.so.1"), "mpv_terminate_destroy");
    func(m_pHandle);
}

MpvHandle MpvHandle::fromRawHandle(mpv_handle *pHandle)
{
    MpvHandle mpvHandle;
    mpvHandle.sptr = QSharedPointer<container>(new container(pHandle));
    return mpvHandle;
}

MpvHandle::operator mpv_handle *() const
{
    return sptr ? (*sptr).m_pHandle : 0;
}


static void mpv_callback(void *d)
{
    DMpvProxy *pMpv = static_cast<DMpvProxy *>(d);
    QMetaObject::invokeMethod(pMpv, "has_mpv_events", Qt::QueuedConnection);
}

DMpvProxy::DMpvProxy(QObject *parent) : DPlayerBackend(parent), d_ptr(new DMpvProxyPrivate(this))
{
    qRegisterMetaType<MpvHandle>("MpvHandle");
    initMember();

#if defined(__mips__) || defined(__aarch64__)
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
#endif
}

DMpvProxy::~DMpvProxy()
{
    Q_D(DMpvProxy);
    disconnect(this, &DMpvProxy::has_mpv_events, this, &DMpvProxy::handle_mpv_events);
    if(d->m_handle) {
        d->m_setWakeupCallback(d->m_handle, mpv_callback, nullptr);
    }
    d->m_bConnectStateChange = false;
    if(DCompositeManager::get().composited()) {
        disconnect(this, &DMpvProxy::stateChanged, nullptr, nullptr);
    }
}

MpvHandle DMpvProxy::getMpvHandle()
{
    Q_D(const DMpvProxy);
    return d->m_handle;
}

void DMpvProxy::setDecodeModel(const QVariant &value)
{
    Q_D(DMpvProxy);
    d->m_decodeMode = static_cast<DecodeMode>(value.toInt());
}

void DMpvProxy::initMpvFuns()
{
    Q_D(DMpvProxy);
    QLibrary mpvLibrary(libPath("libmpv.so.1"));

    d->m_waitEvent         = reinterpret_cast<mpv_waitEvent>(mpvLibrary.resolve("mpv_wait_event"));
    d->m_setOptionString   = reinterpret_cast<mpv_set_optionString>(mpvLibrary.resolve("mpv_set_option_string"));
    d->m_setProperty       = reinterpret_cast<mpv_setProperty>(mpvLibrary.resolve("mpv_set_property"));
    d->m_setPropertyAsync  = reinterpret_cast<mpv_setProperty_async>(mpvLibrary.resolve("mpv_set_property_async"));
    d->m_commandNode       = reinterpret_cast<mpv_commandNode>(mpvLibrary.resolve("mpv_command_node"));
    d->m_commandNodeAsync  = reinterpret_cast<mpv_commandNode_async>(mpvLibrary.resolve("mpv_command_node_async"));
    d->m_getProperty       = reinterpret_cast<mpv_getProperty>(mpvLibrary.resolve("mpv_get_property"));
    d->m_observeProperty   = reinterpret_cast<mpv_observeProperty>(mpvLibrary.resolve("mpv_observe_property"));
    d->m_eventName         = reinterpret_cast<mpv_eventName>(mpvLibrary.resolve("mpv_event_name"));
    d->m_creat             = reinterpret_cast<mpvCreate>(mpvLibrary.resolve("mpv_create"));
    d->m_requestLogMessage = reinterpret_cast<mpv_requestLog_messages>(mpvLibrary.resolve("mpv_request_log_messages"));
    d->m_setWakeupCallback = reinterpret_cast<mpv_setWakeup_callback>(mpvLibrary.resolve("mpv_set_wakeup_callback"));
    d->m_initialize        = reinterpret_cast<mpvinitialize>(mpvLibrary.resolve("mpv_initialize"));
    d->m_freeNodecontents  = reinterpret_cast<mpv_freeNode_contents>(mpvLibrary.resolve("mpv_free_node_contents"));
}

void DMpvProxy::initGpuInfoFuns()
{
    Q_D(DMpvProxy);
    QString path = QLibraryInfo::location(QLibraryInfo::LibrariesPath) + QDir::separator() + "libgpuinfo.so";
    if(!QFileInfo(path).exists()) {
        d->m_gpuInfo = NULL;
        return;
    }
    QLibrary mpvLibrary(libPath("libgpuinfo.so"));
    d->m_gpuInfo = reinterpret_cast<void *>(mpvLibrary.resolve("vdp_Iter_decoderInfo"));
}

void DMpvProxy::firstInit()
{
    Q_D(DMpvProxy);
    initMpvFuns();
    initGpuInfoFuns();
    if(d->m_creat) {
        d->m_handle = MpvHandle::fromRawHandle(d_ptr->mpv_init());
        if(DCompositeManager::get().composited()) {
            emit notifyCreateOpenGL(d->m_handle);
        }
    }

    d->m_bInited = true;
    initSetting();
}

void DMpvProxy::initSetting()
{
    Q_D(DMpvProxy);
    QMapIterator<QString, QVariant> mapItor(d->m_mapWaitSet);
    while(mapItor.hasNext()) {
        mapItor.next();
        d->my_set_property(d->m_handle, mapItor.key(), mapItor.value());
    }

    QVectorIterator<QVariant> vecItor(d->m_vecWaitCommand);
    while(vecItor.hasNext()) {
        d->my_command(d->m_handle, vecItor.peekNext());
        vecItor.next();
    }
}

mpv_handle *DMpvProxyPrivate::mpv_init()
{
    Q_Q(DMpvProxy);
    mpv_handle *pHandle = static_cast<mpv_handle *>(m_creat());
    bool composited     = DCompositeManager::get().composited();
    switch(q->debugLevel()) {
    case DMpvProxy::DebugLevel::Info:
        m_requestLogMessage(pHandle, "info");
        break;

    case DMpvProxy::DebugLevel::Debug:
    case DMpvProxy::DebugLevel::Verbose:
        my_set_property(pHandle, "terminal", "yes");
        if(q->debugLevel() == DMpvProxy::DebugLevel::Verbose) {
            my_set_property(pHandle, "msg-level", "all=status");
            m_requestLogMessage(pHandle, "info");
        }
        else {
            my_set_property(pHandle, "msg-level", "all=v");
            m_requestLogMessage(pHandle, "v");
        }
        break;
    }
    if(composited) {
        auto interop = QString::fromUtf8("vaapi-glx");
        if(!qEnvironmentVariableIsEmpty("QT_XCB_GL_INTERGRATION")) {
            auto gl_int = qgetenv("QT_XCB_GL_INTERGRATION");
            if(gl_int == "xcb_egl") {
                interop = "vaapi-egl";
            }
            else if(gl_int == "xcb_glx") {
                interop = "vaapi-glx";
            }
            else {
                interop = "auto";
            }
        }
        my_set_property(pHandle, "gpu-hwdec-interop", interop.toUtf8().constData());
        qInfo() << "set gpu-hwdec-interop = " << interop;
    }
    my_set_property(pHandle, "hwdec", "auto");

#ifdef __aarch64__
    if(DCompositeManager::get().isOnlySoftDecode()) {
        my_set_property(pHandle, "hwdec", "no");
    }
    else {
        my_set_property(pHandle, "hwdec", "auto");
    }
    qInfo() << "modify HWDEC auto";
#endif

    my_set_property(pHandle, "panscan", 1.0);
    if(DecodeMode::SOFTWARE == m_decodeMode) {
        my_set_property(pHandle, "hwdec", "no");
    }
    else if(DecodeMode::AUTO == m_decodeMode) {
        QFileInfo fi("/dev/mwv206_0");
        QFileInfo jmfi("/dev/jmgpu");
        if(fi.exists() || jmfi.exists()) {
            QDir sdir(QLibraryInfo::location(QLibraryInfo::LibrariesPath) + QDir::separator() + "mwv206");
            QDir jmdir(QLibraryInfo::location(QLibraryInfo::LibrariesPath) + QDir::separator() + "mwv207");
            if(sdir.exists()) {
                my_set_property(pHandle, "hwdec", "vdpau");
            }
            else {
                my_set_property(pHandle, "hwdec", "auto");
            }

            if(!sdir.exists() && jmdir.exists()) {
                my_set_property(pHandle, "hwdec", "vaapi");
                my_set_property(pHandle, "vo", "vaapi");
                m_sInitVo = "vaapi";
            }
            else {
                my_set_property(pHandle, "vo", "vdpau,xv,x11");
                m_sInitVo = "vdpau,xv,x11";
            }
        }
        else if(QFile::exists("/dev/csmcore")) {
            my_set_property(pHandle, "vo", "xv,x11");
            my_set_property(pHandle, "hwdec", "auto");
            if(DCompositeManager::get().check_wayland_env()) {
                my_set_property(pHandle, "wid", q->winID());
            }
            m_sInitVo = "xv,x11";
        }
        else if(DCompositeManager::get().isOnlySoftDecode()) {
            my_set_property(pHandle, "hwdec", "no");
        }
        else {
            my_set_property(pHandle, "hwdec", "auto");
        }

#if defined(__mips__)
        if(!DCompositeManager::get().hascard()) {
            qInfo() << "修改音视频同步模式";
            my_set_property(pHandle, "video-sync", "desync");
        }
        my_set_property(pHandle, "vo", "vdpau,gpu,x11");
        my_set_property(pHandle, "ao", "alsa");
        m_sInitVo = "vdpau,gpu,x11";
#elif defined(_loongarch) || defined(__loongarch__) || defined(__loongarch64)
        if(!DCompositeManager::get().hascard()) {
            qInfo() << "修改音视频同步模式";
            my_set_property(pHandle, "video-sync", "desync");
        }
        if(!fi.exists() && !jmfi.exists()) {
            my_set_property(pHandle, "vo", "gpu,x11");
            m_sInitVo = "gpu,x11";
        }
#elif defined(__sw_64__)
        // Synchronously modify the video output of the SW platform vdpau(powered by
                zhangfl) my_set_property(pHandle, "vo", "vdpau,gpu,x11");
                m_sInitVo = "vdpau,gpu,x11";
#elif defined(__aarch64__)
        if(!fi.exists() && !jmfi.exists()) {
            my_set_property(pHandle, "vo", "gpu,xv,x11");
            m_sInitVo = "gpu,xv,x11";
        }
#else
        QFileInfo sjmfi("/dev/jmgpu");
        bool jmflag = false;
        if(sjmfi.exists()) {
            QDir jmdir(QLibraryInfo::location(QLibraryInfo::LibrariesPath) + QDir::separator() + "mwv207");
            if(jmdir.exists()) {
                jmflag = true;
            }
        }
        if(DCompositeManager::get().isZXIntgraphics() && !jmflag) {
            my_set_property(pHandle, "vo", "gpu");
        }
#endif
    }
    else {
        QFileInfo fi("/dev/mwv206_0");
        QFileInfo jmfi("/dev/jmgpu");
        if(fi.exists() || jmfi.exists()) {
            QDir sdir(QLibraryInfo::location(QLibraryInfo::LibrariesPath) + QDir::separator() + "mwv206");
            QDir jmdir(QLibraryInfo::location(QLibraryInfo::LibrariesPath) + QDir::separator() + "mwv207");
            if(sdir.exists()) {
                my_set_property(pHandle, "hwdec", "vdpau");
            }
            else {
                my_set_property(pHandle, "hwdec", "auto");
            }
            if(!sdir.exists() && jmdir.exists()) {
                my_set_property(pHandle, "hwdec", "vaapi");
                my_set_property(pHandle, "vo", "vaapi");
                m_sInitVo = "vaapi";
            }
            else {
                my_set_property(pHandle, "vo", "vdpau,xv,x11");
                m_sInitVo = "vdpau,xv,x11";
            }
        }
        else {
            my_set_property(pHandle, "hwdec", "auto");
        }
    }

    if(composited) {
#ifdef __mips__
        m_setOptionString(pHandle, "vo", "opengl-cb");
        m_setOptionString(pHandle, "hwdec-preload", "auto");
        m_setOptionString(pHandle, "opengl-hwdec-interop", "auto");
        m_setOptionString(pHandle, "hwdec", "auto");
        qInfo() << "-------- __mips__hwdec____________";
        m_sInitVo = "opengl-cb";
#else
        my_set_property(pHandle, "vo", "libmpv,opengl-cb");
        my_set_property(pHandle, "vd-lavc-dr", "no");
        my_set_property(pHandle, "gpu-sw", "on");
        m_sInitVo = "libmpv,opengl-cb";
#endif
    }
    else {
        my_set_property(pHandle, "wid", q->winID());
    }

    qInfo() << __func__ << "vo:" << my_get_property(pHandle, "vo").toString();
    qInfo() << __func__ << "hwdec:" << my_get_property(pHandle, "hwdec").toString();

    my_set_property(pHandle, "panscan", 0);
    my_set_property(pHandle, "volume-max", 200.0);
    my_set_property(pHandle, "input-cursor", "no");
    my_set_property(pHandle, "cursor-autohide", "no");
    my_set_property(pHandle, "sub-auto", "fuzzy");
    my_set_property(pHandle, "sub-visibility", "true");
    my_set_property(pHandle, "sub-pos", 100);
    my_set_property(pHandle, "sub-margin-y", 36);
    my_set_property(pHandle, "sub-border-size", 0);
    my_set_property(pHandle, "screenshot-template", "deepin-movie-shot%n");
    my_set_property(pHandle, "screenshot-directory", "/tmp");

    // only to get notification without data
    m_observeProperty(pHandle, 0, "time-pos", MPV_FORMAT_NONE); // playback-time ?
    m_observeProperty(pHandle, 0, "pause", MPV_FORMAT_NONE);
    m_observeProperty(pHandle, 0, "mute", MPV_FORMAT_NONE);
    m_observeProperty(pHandle, 0, "volume", MPV_FORMAT_NONE); // ao-volume ?
    m_observeProperty(pHandle, 0, "sid", MPV_FORMAT_NONE);
    m_observeProperty(pHandle, 0, "aid", MPV_FORMAT_NODE);
    m_observeProperty(pHandle, 0, "dwidth", MPV_FORMAT_NODE);
    m_observeProperty(pHandle, 0, "dheight", MPV_FORMAT_NODE);
    m_observeProperty(pHandle, 0, "core-idle", MPV_FORMAT_NODE);
    m_observeProperty(pHandle, 0, "paused-for-cache", MPV_FORMAT_NODE);

    m_setWakeupCallback(pHandle, mpv_callback, q);
    connect(q, &DMpvProxy::has_mpv_events, q, &DMpvProxy::handle_mpv_events, Qt::DirectConnection);
    if(m_initialize(pHandle) < 0) {
        std::runtime_error("mpv init failed");
    }

    // load profile
    auto ol = DCompositeManager::get().getBestProfile();
    auto p  = ol.begin();
    while(p != ol.end()) {
        if(!p->first.startsWith("#")) {
#if !defined(__mips__) && !defined(__aarch64__) && !defined(__sw_64__)
#ifdef MWV206_0
            QFileInfo fi("/dev/mwv206_0");
            QFileInfo jmfi("/dev/jmgpu");
            if(!fi.exists() && !jmfi.exists()) {
                my_set_property(pHandle, p->first.toUtf8().constData(), p->second.toUtf8().constData());
                qInfo() << "apply" << p->first << "=" << p->second;
            }
#else
            my_set_property(pHandle, p->first.toUtf8().constData(), p->second.toUtf8().constData());
            qInfo() << "apply" << p->first << "=" << p->second;
#endif
#endif
        }
        else {
            qInfo() << "ignore(commented out)" << p->first << "=" << p->second;
        }
        ++p;
    }

    return pHandle;
}

void DMpvProxyPrivate::setState(DMpvProxy::PlayState state)
{
    Q_Q(DMpvProxy);
    bool bRawFormat = false;
    if(q->state() != state) {
        q->setState(state);
        emit q->stateChanged();
    }
}

void DMpvProxy::pollingEndOfPlayback()
{
    Q_D(DMpvProxy);
    if(state() != DPlayerBackend::Stopped) {
        d->m_bPolling = true;
        blockSignals(true);
        stop();
        bool bIdel = d->my_get_property(d->m_handle, "idle-active").toBool();
        if(bIdel) {
            blockSignals(false);
            d->setState(DPlayerBackend::Stopped);
            d->m_bPolling = false;
            return;
        }

        while(state() != DPlayerBackend::Stopped) {
            mpv_event *pEvent = d->m_waitEvent(d->m_handle, 0.005);
            if(pEvent->event_id == MPV_EVENT_NONE) continue;

            if(pEvent->event_id == MPV_EVENT_END_FILE) {
                blockSignals(false);
                d->setState(DPlayerBackend::Stopped);
                break;
            }
        }
        d->m_bPolling = false;
    }
}

const PlayingMovieInfo &DMpvProxy::playingMovieInfo()
{
    Q_D(DMpvProxy);
    return d->m_movieInfo;
}

bool DMpvProxy::isPlayable() const
{
    return true;
}

bool DMpvProxy::isSurportHardWareDecode(const QString sDecodeName, const int &nVideoWidth, const int &nVideoHeight)
{
    Q_D(DMpvProxy);
    bool isHardWare              = true;
    decoder_profile decoderValue = decoder_profile::UN_KNOW;
    decoderValue                 = (decoder_profile) getDecodeProbeValue(sDecodeName);
    if(decoderValue != decoder_profile::UN_KNOW) {
        VDP_Decoder_t *probeDecode = new VDP_Decoder_t;
        if(d->m_gpuInfo) {
            int nSurport = ((gpu_decoderInfo) d->m_gpuInfo)(decoderValue, probeDecode);
            isHardWare =
                (nSurport > 0 && probeDecode->max_width >= nVideoWidth && probeDecode->max_height >= nVideoHeight);
        }
        delete probeDecode;
    }
    return isHardWare;
}

int DMpvProxy::getDecodeProbeValue(const QString sDecodeName)
{
    QStringList sNameList;
    sNameList << "MPEG1"
              << "MPEG2"
              << "MPEG4"
              << "H264"
              << "VC1"
              << "DIVX4"
              << "DIVX5"
              << "HEVC";
    int nCount = sNameList.count();
    for(int i = 0; i < nCount; i++) {
        QString sValue = sNameList.at(i);
        if(sDecodeName.toUpper().contains(sValue)) {
            return (int) decoder_profile(decoder_profile::UN_KNOW + 1 + i);
        }
    }
    return (int) decoder_profile::UN_KNOW;
}

void DMpvProxy::setWinID(const qint64 &winID)
{
    DPlayerBackend::setWinID(winID);
}

void DMpvProxy::handle_mpv_events()
{
    Q_D(DMpvProxy);
    if(DCompositeManager::get().check_wayland_env() && DCompositeManager::get().isTestFlag()) {
        qInfo() << "not handle mpv events!";
        return;
    }
    while(1) {
        mpv_event *pEvent = d->m_waitEvent(d->m_handle, 0.0005);
        if(pEvent->event_id == MPV_EVENT_NONE) break;

        switch(pEvent->event_id) {
        case MPV_EVENT_LOG_MESSAGE:
            d->processLogMessage(reinterpret_cast<mpv_event_log_message *>(pEvent->data));
            break;

        case MPV_EVENT_PROPERTY_CHANGE:
            d->processPropertyChange(reinterpret_cast<mpv_event_property *>(pEvent->data));
            break;

        case MPV_EVENT_COMMAND_REPLY:
            if(pEvent->error < 0) {
                qInfo() << "command error";
            }

            if(pEvent->reply_userdata == AsyncReplyTag::SEEK) {
                d->m_bPendingSeek = false;
            }
            break;

        case MPV_EVENT_PLAYBACK_RESTART:
            break;

#if MPV_CLIENT_API_VERSION < MPV_MAKE_VERSION(2, 0)
        case MPV_EVENT_TRACKS_CHANGED:
            qInfo() << d->m_eventName(pEvent->event_id);
            d->updatePlayingMovieInfo();
            emit tracksChanged();
            break;
#endif

        case MPV_EVENT_FILE_LOADED: {
            qInfo() << d->m_eventName(pEvent->event_id);

            if(winID() != -1) {
                qInfo() << "hwdec-interop" << d->my_get_property(d->m_handle, "gpu-hwdec-interop")
                        << "codec: " << d->my_get_property(d->m_handle, "video-codec")
                        << "format: " << d->my_get_property(d->m_handle, "video-format");
            }

            d->setState(PlayState::Playing); // might paused immediately
            emit fileLoaded();
            qInfo() << QString("rotate metadata: dec %1, out %2")
                           .arg(d->my_get_property(d->m_handle, "video-dec-params/rotate").toInt())
                           .arg(d->my_get_property(d->m_handle, "video-params/rotate").toInt());
            break;
        }
        case MPV_EVENT_VIDEO_RECONFIG: {
            QSize size = videoSize();
            if(!size.isEmpty()) emit videoSizeChanged();
            break;
        }

        case MPV_EVENT_END_FILE: {
            mpv_event_end_file *ev_ef = reinterpret_cast<mpv_event_end_file *>(pEvent->data);
            qInfo() << d->m_eventName(pEvent->event_id) << "reason " << ev_ef->reason;

            d->setState(PlayState::Stopped);
            break;
        }

        case MPV_EVENT_IDLE:
            qInfo() << d->m_eventName(pEvent->event_id);
            d->setState(PlayState::Stopped);
            emit elapsedChanged();
            break;

        default:
            qInfo() << d->m_eventName(pEvent->event_id);
            break;
        }
    }
}

void DMpvProxyPrivate::processLogMessage(mpv_event_log_message *pEvent)
{
    Q_Q(DMpvProxy);
    switch(pEvent->log_level) {
    case MPV_LOG_LEVEL_WARN:
        qWarning() << QString("%1: %2").arg(pEvent->prefix).arg(pEvent->text);
        emit q->mpvWarningLogsChanged(QString(pEvent->prefix), QString(pEvent->text));
        break;

    case MPV_LOG_LEVEL_ERROR:
    case MPV_LOG_LEVEL_FATAL: {
        QString strError = pEvent->text;
        if(strError.contains("Failed setup for format vdpau")) {
            m_bLastIsSpecficFormat = true;
        }
        qCritical() << QString("%1: %2").arg(pEvent->prefix).arg(strError);
        emit q->mpvErrorLogsChanged(QString(pEvent->prefix), strError);
    } break;

    case MPV_LOG_LEVEL_INFO:
        qInfo() << QString("%1: %2").arg(pEvent->prefix).arg(pEvent->text);
        break;

    default:
        qInfo() << QString("%1: %2").arg(pEvent->prefix).arg(pEvent->text);
        break;
    }
}

void DMpvProxyPrivate::processPropertyChange(mpv_event_property *pEvent)
{
    Q_Q(DMpvProxy);
    QString sName = QString::fromUtf8(pEvent->name);
    if(sName != "time-pos") qInfo() << sName;

    if(sName == "time-pos") {
        emit q->elapsedChanged();
    }
    else if(sName == "volume") {
        emit q->volumeChanged();
    }
    else if(sName == "dwidth" || sName == "dheight") {
        auto sz = q->videoSize();
        if(!sz.isEmpty()) emit q->videoSizeChanged();
        qInfo() << "update videoSize " << sz;
    }
    else if(sName == "aid") {
        emit q->aidChanged();
    }
    else if(sName == "sid") {
        emit q->sidChanged();
    }
    else if(sName == "mute") {
        emit q->muteChanged();
    }
    else if(sName == "pause") {
        auto idle = my_get_property(m_handle, "idle-active").toBool();
        if(my_get_property(m_handle, "pause").toBool()) {
            if(!idle)
                setState(DMpvProxy::PlayState::Paused);
            else
                my_set_property(m_handle, "pause", false);
        }
        else {
            if(q->state() != DMpvProxy::PlayState::Stopped) {
                setState(DMpvProxy::PlayState::Playing);
            }
        }
    }
    else if(sName == "core-idle") {
    }
    else if(sName == "paused-for-cache") {
        qInfo() << "paused-for-cache" << my_get_property_variant(m_handle, "paused-for-cache");
        emit q->urlpause(my_get_property_variant(m_handle, "paused-for-cache").toBool());
    }
}

bool DMpvProxy::loadSubtitle(const QFileInfo &fileInfo)
{
    Q_D(DMpvProxy);
    if(!fileInfo.exists()) return false;

    QList<QVariant> args = {"sub-add", fileInfo.absoluteFilePath(), "select"};
    qInfo() << args;
    QVariant id = d->my_command(d->m_handle, args);
    if(id.canConvert<ErrorReturn>()) {
        return false;
    }

    d->updatePlayingMovieInfo();

    // by settings this flag, we can match the corresponding sid change and save it
    // in the movie database
    return true;
}

bool DMpvProxy::isSubVisible()
{
    Q_D(DMpvProxy);
    return d->my_get_property(d->m_handle, "sub-visibility").toBool();
}

void DMpvProxy::setSubDelay(double dSecs)
{
    Q_D(DMpvProxy);
    d->my_set_property(d->m_handle, "sub-delay", dSecs);
}

double DMpvProxy::subDelay() const
{
    Q_D(const DMpvProxy);
    return d->my_get_property(d->m_handle, "sub-delay").toDouble();
}

QString DMpvProxy::subCodepage()
{
    Q_D(const DMpvProxy);
    auto cp = d->my_get_property(d->m_handle, "sub-codepage").toString();
    if(cp.startsWith("+")) {
        cp.remove(0, 1);
    }

    return cp;
}

void DMpvProxy::addSubSearchPath(const QString &sPath)
{
    Q_D(DMpvProxy);
    d->my_set_property(d->m_handle, "sub-paths", sPath);
    d->my_set_property(d->m_handle, "sub-file-paths", sPath);
}

void DMpvProxy::setSubCodepage(const QString &sCodePage)
{
    Q_D(DMpvProxy);
    QString strTmp = sCodePage;
    if(!sCodePage.startsWith("+") && sCodePage != "auto") strTmp.prepend('+');

    d->my_set_property(d->m_handle, "sub-codepage", strTmp);
    d->my_command(d->m_handle, {"sub-reload"});
}

void DMpvProxy::updateSubStyle(const QString &sFont, int nSize)
{
    Q_D(DMpvProxy);
    d->my_set_property(d->m_handle, "sub-font", sFont);
    d->my_set_property(d->m_handle, "sub-font-size", nSize);
    d->my_set_property(d->m_handle, "sub-color", "#FFFFFF");
    d->my_set_property(d->m_handle, "sub-border-size", 1);
    d->my_set_property(d->m_handle, "sub-border-color", "0.0/0.0/0.0/0.50");
    d->my_set_property(d->m_handle, "sub-shadow-offset", 1);
    d->my_set_property(d->m_handle, "sub-shadow-color", "0.0/0.0/0.0/0.50");
}

void DMpvProxy::savePlaybackPosition()
{
    if(state() == PlayState::Stopped) {
        return;
    }
}

void DMpvProxy::setPlaySpeed(double dTimes)
{
    Q_D(DMpvProxy);
    if(d->m_handle) d->my_set_property_async(d->m_handle, "speed", dTimes, AsyncReplyTag::SPEED);
}

void DMpvProxy::selectSubtitle(int nId)
{
    Q_D(DMpvProxy);
    if(nId > d->m_movieInfo.subs.size()) {
        nId = d->m_movieInfo.subs.size() == 0 ? -1 : d->m_movieInfo.subs[0]["id"].toInt();
    }

    d->my_set_property(d->m_handle, "sid", nId);
}

void DMpvProxy::toggleSubtitle()
{
    Q_D(DMpvProxy);
    if(state() == PlayState::Stopped) {
        return;
    }

    d->my_set_property(d->m_handle, "sub-visibility", !isSubVisible());
}

int DMpvProxy::aid() const
{
    Q_D(const DMpvProxy);
    return d->my_get_property(d->m_handle, "aid").toInt();
}

int DMpvProxy::sid() const
{
    Q_D(const DMpvProxy);
    return d->my_get_property(d->m_handle, "sid").toInt();
}

void DMpvProxy::selectTrack(int nId)
{
    Q_D(DMpvProxy);
    if(nId >= d->m_movieInfo.audios.size()) return;
    QVariant aid = d->m_movieInfo.audios[nId]["id"];
    d->my_set_property(d->m_handle, "aid", aid);
}

void DMpvProxy::changeSoundMode(SoundMode soundMode)
{
    Q_D(DMpvProxy);
    QList<QVariant> listArgs;

    switch(soundMode) {
    case SoundMode::Stereo:
        listArgs << "af"
                 << "set"
                 << "stereotools=muter=false";
        break;
    case SoundMode::Left:
        listArgs << "af"
                 << "set"
                 << "stereotools=muter=true";
        break;
    case SoundMode::Right:
        listArgs << "af"
                 << "set"
                 << "stereotools=mutel=true";
        break;
    }

    d->my_command(d->m_handle, listArgs);
}

void DMpvProxy::volumeUp()
{
    if(volume() >= 200) return;

    changeVolume(volume() + 10);
}

void DMpvProxy::changeVolume(int nVol)
{
    Q_D(DMpvProxy);
    d->my_set_property(d->m_handle, "volume", d->volumeCorrection(nVol));
}

void DMpvProxy::volumeDown()
{
    if(volume() <= 0) return;

    changeVolume(volume() - 10);
}

int DMpvProxy::volume() const
{
    Q_D(const DMpvProxy);
    int nActualVol  = d->my_get_property(d->m_handle, "volume").toInt();
    int nDispalyVol = static_cast<int>((nActualVol - 40) / 60.0 * 100.0);
    return nDispalyVol > 100 ? nActualVol : nDispalyVol;
}

int DMpvProxy::videoRotation() const
{
    Q_D(const DMpvProxy);
    int nRotate = d->my_get_property(d->m_handle, "video-rotate").toInt();
    return (nRotate + 360) % 360;
}

void DMpvProxy::setVideoRotation(int nDegree)
{
    Q_D(DMpvProxy);
    d->my_set_property(d->m_handle, "video-rotate", nDegree);
}

void DMpvProxy::setVideoAspect(double dValue)
{
    Q_D(DMpvProxy);
    d->my_set_property(d->m_handle, "video-aspect", dValue);
}

double DMpvProxy::videoAspect() const
{
    Q_D(const DMpvProxy);
    return d->my_get_property(d->m_handle, "video-aspect").toDouble();
}

bool DMpvProxy::muted() const
{
    Q_D(const DMpvProxy);
    return d->my_get_property(d->m_handle, "mute").toBool();
}

void DMpvProxy::toggleMute()
{
    Q_D(DMpvProxy);
    QList<QVariant> listArgs = {"cycle", "mute"};
    qInfo() << listArgs;
    d->my_command(d->m_handle, listArgs);
}

void DMpvProxy::setMute(bool bMute)
{
    Q_D(DMpvProxy);
    d->my_set_property(d->m_handle, "mute", bMute);
}

void DMpvProxy::refreshDecode()
{
    Q_D(DMpvProxy);
    QList<QString> canHwTypes;

    if(DecodeMode::SOFTWARE == d->m_decodeMode) {
        d->my_set_property(d->m_handle, "hwdec", "no");
    }
    else if(DecodeMode::AUTO == d->m_decodeMode) {
        bool isSoftCodec = false;
        if(isSoftCodec) {
            qInfo() << "my_set_property hwdec no";
            d->my_set_property(d->m_handle, "hwdec", "no");
        }
        else {
            QFileInfo fi("/dev/mwv206_0");
            QFileInfo jmfi("/dev/jmgpu");
            if(fi.exists() || jmfi.exists()) {
                QDir sdir(QLibraryInfo::location(QLibraryInfo::LibrariesPath) + QDir::separator() + "mwv206");
                QDir jmdir(QLibraryInfo::location(QLibraryInfo::LibrariesPath) + QDir::separator() + "mwv207");
                if(sdir.exists()) {
                    d->my_set_property(d->m_handle, "hwdec", "vdpau");
                }
                else {
                    d->my_set_property(d->m_handle, "hwdec", "auto");
                }
                if(!sdir.exists() && jmdir.exists()) {
                    d->my_set_property(d->m_handle, "hwdec", "vaapi");
                }
            }
            else if(DCompositeManager::get().isOnlySoftDecode()) {
                d->my_set_property(d->m_handle, "hwdec", "no");
            }
            else {
                d->my_set_property(d->m_handle, "hwdec", "auto");
            }
        }
    }
    else {
        if(DCompositeManager::get().isOnlySoftDecode()) {
            d->my_set_property(d->m_handle, "hwdec", "no");
        }
        else {
            d->my_set_property(d->m_handle, "hwdec", "auto");
        }
        QFileInfo fi("/dev/mwv206_0");
        QFileInfo jmfi("/dev/jmgpu");
        if(fi.exists() || jmfi.exists()) {
            QDir sdir(QLibraryInfo::location(QLibraryInfo::LibrariesPath) + QDir::separator() + "mwv206");
            QDir jmdir(QLibraryInfo::location(QLibraryInfo::LibrariesPath) + QDir::separator() + "mwv207");
            if(sdir.exists()) {
                d->my_set_property(d->m_handle, "hwdec", "vdpau");
            }
            else {
                d->my_set_property(d->m_handle, "hwdec", "auto");
            }
            if(!sdir.exists() && jmdir.exists()) {
                d->my_set_property(d->m_handle, "hwdec", "vaapi");
            }
        }
    }
}

void DMpvProxy::initMember()
{
    Q_D(DMpvProxy);
    d->m_nBurstStart          = 0;
    d->m_bInBurstShotting     = false;
    d->m_posBeforeBurst       = false;
    d->m_bPendingSeek         = false;
    d->m_bPolling             = false;
    d->m_bConnectStateChange  = false;
    d->m_bPauseOnStart        = false;
    d->m_bIsJingJia           = false;
    d->m_bInited              = false;
    d->m_bHwaccelAuto         = false;
    d->m_bLastIsSpecficFormat = false;

    d->m_sInitVo = "gpu,xv,x11";
    d->m_listBurstPoints.clear();
    d->m_mapWaitSet.clear();
    d->m_vecWaitCommand.clear();

    d->m_waitEvent         = nullptr;
    d->m_setOptionString   = nullptr;
    d->m_setProperty       = nullptr;
    d->m_setPropertyAsync  = nullptr;
    d->m_commandNode       = nullptr;
    d->m_commandNodeAsync  = nullptr;
    d->m_getProperty       = nullptr;
    d->m_observeProperty   = nullptr;
    d->m_eventName         = nullptr;
    d->m_creat             = nullptr;
    d->m_requestLogMessage = nullptr;
    d->m_setWakeupCallback = nullptr;
    d->m_initialize        = nullptr;
    d->m_freeNodecontents  = nullptr;
    d->m_pConfig           = nullptr;
    d->m_gpuInfo           = nullptr;
}

void DMpvProxy::play()
{
    Q_D(DMpvProxy);
    bool bRawFormat          = false;
    QList<QVariant> listArgs = {"loadfile"};
    QStringList listOpts     = {};
    bool bAudio              = false;

    if(!d->m_bInited) {
        firstInit();
    }
    if(bAudio) {
        d->my_set_property(d->m_handle, "vo", "null");
    }
    else {
        d->my_set_property(d->m_handle, "vo", d->m_sInitVo);
    }

    if(urlFile().isLocalFile()) {
        listArgs << QFileInfo(urlFile().toLocalFile()).absoluteFilePath();
    }
    else {
        listArgs << urlFile().url();
    }

    refreshDecode();

    QFileInfo fi("/dev/mwv206_0");
    if(fi.exists()) {
        QDir sdir(QLibraryInfo::location(QLibraryInfo::LibrariesPath) + QDir::separator() + "mwv206");
    }

    if(listOpts.size()) {
        listArgs << "replace" << listOpts.join(',');
    }

    qInfo() << listArgs;

    d->my_command(d->m_handle, listArgs);
    d->my_set_property(d->m_handle, "pause", d->m_bPauseOnStart);
}


void DMpvProxy::pauseResume()
{
    Q_D(DMpvProxy);
    if(state() == PlayState::Stopped) return;

    d->my_set_property(d->m_handle, "pause", !paused());
}

void DMpvProxy::stop()
{
    Q_D(DMpvProxy);
    QList<QVariant> args = {"stop"};
    qInfo() << args;
    d->my_command(d->m_handle, args);
}

QImage DMpvProxy::takeScreenshot() const
{
    Q_D(const DMpvProxy);

    return d->takeOneScreenshot();
}

void DMpvProxy::burstScreenshot()
{
    Q_D(DMpvProxy);
    if(d->m_bInBurstShotting) {
        qWarning() << "already in burst screenshotting mode";
        return;
    }

    if(state() == PlayState::Stopped) return;

    d->m_posBeforeBurst = d->my_get_property(d->m_handle, "time-pos");

    int nDuration = static_cast<int>(duration() / 15);

    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<int> uniform_dist(0, nDuration);
    d->m_listBurstPoints.clear();
    for(int i = 0; i < 15; i++) {
        d->m_listBurstPoints.append(nDuration * i + uniform_dist(g));
    }
    d->m_nBurstStart = 0;

    if(duration() < 35) {
        emit notifyScreenshot(QImage(), 0);
        stopBurstScreenshot();
        return;
    }
    qInfo() << "burst span " << d->m_nBurstStart;

    if(!paused()) pauseResume();
    d->m_bInBurstShotting = true;
    QTimer::singleShot(0, this, &DMpvProxy::stepBurstScreenshot);
}

qint64 DMpvProxyPrivate::nextBurstShootPoint()
{
    Q_Q(const DMpvProxy);
    auto next = m_listBurstPoints[static_cast<int>(m_nBurstStart++)];
    if(next >= q->duration()) {
        next = q->duration() - 5;
    }

    return next;
}

int DMpvProxyPrivate::volumeCorrection(int displayVol)
{
    int realVol = 0;
    if(displayVol > 100) return displayVol;
    realVol = static_cast<int>((displayVol / 100.0) * 60.0 + 40);
    return (realVol == 40 ? 0 : realVol);
}

QVariant DMpvProxyPrivate::my_get_property(mpv_handle *pHandle, const QString &sName) const
{
    Q_Q(const DMpvProxy);
    mpv_node node;
    if(!m_getProperty) return QVariant();
    int err = m_getProperty(pHandle, sName.toUtf8().data(), MPV_FORMAT_NODE, &node);
    if(err < 0) return QVariant::fromValue(ErrorReturn(err));
    auto variant = node_to_variant(&node);
    m_freeNodecontents(&node);
    return variant;
}

int DMpvProxyPrivate::my_set_property(mpv_handle *pHandle, const QString &sName, const QVariant &v)
{
    QVariant sValue = v;

    node_builder node(sValue);

    if(!m_bInited) {
        m_mapWaitSet.insert(sName, sValue);
        return 0;
    }

    if(!m_setProperty) return 0;
    int res = m_setProperty(pHandle, sName.toUtf8().data(), MPV_FORMAT_NODE, node.node());
    return res;
}

bool DMpvProxyPrivate::my_command_async(mpv_handle *pHandle, const QVariant &args, uint64_t tag)
{
    node_builder node(args);
    int nErr = m_commandNodeAsync(pHandle, tag, node.node());
    return nErr == 0;
}

int DMpvProxyPrivate::my_set_property_async(
    mpv_handle *pHandle, const QString &sName, const QVariant &value, uint64_t tag)
{
    node_builder node(value);
    return m_setPropertyAsync(pHandle, tag, sName.toUtf8().data(), MPV_FORMAT_NODE, node.node());
}

QVariant DMpvProxyPrivate::my_get_property_variant(mpv_handle *pHandle, const QString &sName)
{
    mpv_node node;
    if(m_getProperty(pHandle, sName.toUtf8().data(), MPV_FORMAT_NODE, &node) < 0) return QVariant();
    nodeAutofree f(&node);
    return node_to_variant(&node);
}

QVariant DMpvProxyPrivate::my_command(mpv_handle *pHandle, const QVariant &args)
{
    if(!m_bInited) {
        m_vecWaitCommand.append(args);
        return QVariant();
    }

    node_builder node(args);
    mpv_node res;
    int nErr = m_commandNode(pHandle, node.node(), &res);
    if(nErr < 0) return QVariant::fromValue(ErrorReturn(nErr));
    auto variant = node_to_variant(&res);
    m_freeNodecontents(&res);
    return variant;
}

QImage DMpvProxyPrivate::takeOneScreenshot() const
{
    Q_Q(const DMpvProxy);
    bool bNeedRotate = false;
    QString strVO    = q->getProperty("current-vo").toString();

    if(strVO.compare("vdpau", Qt::CaseInsensitive) == 0) {
        bNeedRotate = true;
    }

    if(q->state() == DMpvProxy::PlayState::Stopped) return QImage();

    QList<QVariant> args = {"screenshot-raw"};
    node_builder node(args);
    mpv_node res;
    int nErr = m_commandNode(m_handle, node.node(), &res);
    if(nErr < 0) {
        qWarning() << "screenshot raw failed";
        return QImage();
    }

    Q_ASSERT(res.format == MPV_FORMAT_NODE_MAP);

    int w = 0, h = 0, stride = 0;

    mpv_node_list *pNodeList = res.u.list;
    uchar *pData             = nullptr;

    for(int n = 0; n < pNodeList->num; n++) {
        auto key = QString::fromUtf8(pNodeList->keys[n]);
        if(key == "w") {
            w = static_cast<int>(pNodeList->values[n].u.int64);
        }
        else if(key == "h") {
            h = static_cast<int>(pNodeList->values[n].u.int64);
        }
        else if(key == "stride") {
            stride = static_cast<int>(pNodeList->values[n].u.int64);
        }
        else if(key == "format") {
            auto format = QString::fromUtf8(pNodeList->values[n].u.string);
            qInfo() << "format" << format;
        }
        else if(key == "data") {
            pData = static_cast<uchar *>(pNodeList->values[n].u.ba->data);
        }
    }

    if(pData) {
        // alpha should be ignored
        auto img = QImage(static_cast<const uchar *>(pData), w, h, stride, QImage::Format_RGB32);
        img.bits();
        int rotationdegree = q->videoRotation();
        if(rotationdegree && (DCompositeManager::get().composited() || bNeedRotate)) { //只有opengl窗口需要自己旋转
            QMatrix matrix;
            matrix.rotate(rotationdegree);
            img = QPixmap::fromImage(img).transformed(matrix, Qt::SmoothTransformation).toImage();
        }
        m_freeNodecontents(&res);
        return img;
    }

    m_freeNodecontents(&res);
    qInfo() << "failed";
    return QImage();
}

void DMpvProxy::stepBurstScreenshot()
{
    Q_D(DMpvProxy);
    if(!d->m_bInBurstShotting) {
        return;
    }

    auto pos = d->nextBurstShootPoint();
    d->my_command(d->m_handle, QList<QVariant>{"seek", pos, "absolute"});
    while(true) {
        mpv_event *pEvent = d->m_waitEvent(d->m_handle, 0.005);
        if(pEvent->event_id == MPV_EVENT_NONE) continue;

        if(pEvent->event_id == MPV_EVENT_PLAYBACK_RESTART) {
            qInfo() << "seek finished" << elapsed();
            break;
        }

        if(pEvent->event_id == MPV_EVENT_END_FILE) {
            qInfo() << "seek finished (end of file)" << elapsed();
            break;
        }
    }

    QImage img = d->takeOneScreenshot();
    if(img.isNull()) {
        emit notifyScreenshot(img, elapsed());
        stopBurstScreenshot();
        return;
    }
    emit notifyScreenshot(img, elapsed());
    QTimer::singleShot(0, this, &DMpvProxy::stepBurstScreenshot);
}

void DMpvProxy::stopBurstScreenshot()
{
    Q_D(DMpvProxy);
    d->m_bInBurstShotting = false;
    d->my_set_property(d->m_handle, "time-pos", d->m_posBeforeBurst);
}

void DMpvProxy::seekForward(int nSecs)
{
    Q_D(DMpvProxy);
    if(state() == PlayState::Stopped) return;

    if(d->m_bPendingSeek) return;
    QList<QVariant> listArgs = {"seek", QVariant(nSecs), "relative+exact"};
    qInfo() << listArgs;
    d->my_command_async(d->m_handle, listArgs, AsyncReplyTag::SEEK);
    d->m_bPendingSeek = true;
}

void DMpvProxy::seekBackward(int nSecs)
{
    Q_D(DMpvProxy);
    if(state() == PlayState::Stopped) return;

    if(d->m_bPendingSeek) return;
    if(nSecs > 0) nSecs = -nSecs;
    QList<QVariant> listArgs = {"seek", QVariant(nSecs), "relative+exact"};
    qInfo() << listArgs;
    d->my_command_async(d->m_handle, listArgs, AsyncReplyTag::SEEK);
    d->m_bPendingSeek = true;
}

void DMpvProxy::seekAbsolute(int nPos)
{
    Q_D(DMpvProxy);
    if(state() == PlayState::Stopped) return;

    if(d->m_bPendingSeek) return;
    QList<QVariant> listArgs = {"seek", QVariant(nPos), "absolute"};
    qInfo() << listArgs;
    d->m_bPendingSeek = true;
    d->my_command_async(d->m_handle, listArgs, AsyncReplyTag::SEEK);
}

QSize DMpvProxy::videoSize() const
{
    Q_D(const DMpvProxy);
    if(state() == PlayState::Stopped) return QSize(-1, -1);
    QSize size =
        QSize(d->my_get_property(d->m_handle, "dwidth").toInt(), d->my_get_property(d->m_handle, "dheight").toInt());

    auto r = d->my_get_property(d->m_handle, "video-out-params/rotate").toInt();
    if(r == 90 || r == 270) {
        size.transpose();
    }

    return size;
}

qint64 DMpvProxy::duration() const
{
    Q_D(const DMpvProxy);
    bool bRawFormat = false;

    if(bRawFormat) {
        return 0;
    }
    else {
        return d->my_get_property(d->m_handle, "duration").value<qint64>();
    }
}


qint64 DMpvProxy::elapsed() const
{
    Q_D(const DMpvProxy);
    if(state() == PlayState::Stopped) return 0;
    return d->my_get_property(d->m_handle, "time-pos").value<qint64>();
}

void DMpvProxyPrivate::updatePlayingMovieInfo()
{
    m_movieInfo.subs.clear();
    m_movieInfo.audios.clear();

    QList<QVariant> listInfo = my_get_property(m_handle, "track-list").toList();
    auto p                   = listInfo.begin();
    while(p != listInfo.end()) {
        const auto &t = p->toMap();
        if(t["type"] == "audio") {
            AudioInfo audioInfo;
            audioInfo["type"]              = t["type"];
            audioInfo["id"]                = t["id"];
            audioInfo["lang"]              = t["lang"];
            audioInfo["external"]          = t["external"];
            audioInfo["external-filename"] = t["external-filename"];
            audioInfo["selected"]          = t["selected"];
            audioInfo["title"]             = t["title"];

            if(t["title"].toString().size() == 0) {
                if(t["lang"].isValid() && t["lang"].toString().size() && t["lang"].toString() != "und")
                    audioInfo["title"] = t["lang"];
                else if(!t["external"].toBool())
                    audioInfo["title"] = "[internal]";
            }


            m_movieInfo.audios.append(audioInfo);
        }
        else if(t["type"] == "sub") {
            SubtitleInfo titleInfo;
            titleInfo["type"]              = t["type"];
            titleInfo["id"]                = t["id"];
            titleInfo["lang"]              = t["lang"];
            titleInfo["external"]          = t["external"];
            titleInfo["external-filename"] = t["external-filename"];
            titleInfo["selected"]          = t["selected"];
            titleInfo["title"]             = t["title"];
            if(t["title"].toString().size() == 0) {
                if(t["lang"].isValid() && t["lang"].toString().size() && t["lang"].toString() != "und")
                    titleInfo["title"] = t["lang"];
                else if(!t["external"].toBool())
                    titleInfo["title"] = tr("Internal");
            }
            m_movieInfo.subs.append(titleInfo);
        }
        ++p;
    }

    qInfo() << m_movieInfo.subs;
    qInfo() << m_movieInfo.audios;
}

void DMpvProxy::nextFrame()
{
    Q_D(DMpvProxy);
    if(state() == PlayState::Stopped) return;

    QList<QVariant> listArgs = {"frame-step"};
    d->my_command(d->m_handle, listArgs);
}

void DMpvProxy::previousFrame()
{
    Q_D(DMpvProxy);
    if(state() == PlayState::Stopped) return;

    QList<QVariant> listArgs = {"frame-back-step"};
    d->my_command(d->m_handle, listArgs);
}

void DMpvProxy::changehwaccelMode(hwaccelMode hwaccelMode)
{
    Q_D(DMpvProxy);
    switch(hwaccelMode) {
    case hwaccelAuto:
        d->m_bHwaccelAuto = true;
        break;
    case hwaccelOpen:
        d->m_bHwaccelAuto = false;
        d->my_set_property(d->m_handle, "hwdec", "auto");
        break;
    case hwaccelClose:
        d->m_bHwaccelAuto = false;
        d->my_set_property(d->m_handle, "hwdec", "off");
        break;
    }
}

void DMpvProxy::makeCurrent()
{
    //    m_pMpvGLwidget->makeCurrent();
}

QVariant DMpvProxy::getProperty(const QString &sName) const
{
    Q_D(const DMpvProxy);
    return d->my_get_property(d->m_handle, sName.toUtf8().data());
}

void DMpvProxy::setProperty(const QString &sName, const QVariant &val)
{
    Q_D(DMpvProxy);
    if(sName == "pause-on-start") {
        d->m_bPauseOnStart = val.toBool();
    }
    else if(sName == "video-zoom") {
        d->my_set_property(d->m_handle, sName, val.toDouble());
    }
    else {
        d->my_set_property(d->m_handle, sName.toUtf8().data(), val);
    }
}
