// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DMPVPROXY_P_H
#define DMPVPROXY_P_H

#include "dmpvproxy.h"
#include <qobject.h>

#include "dmpvhelper.hpp"

typedef mpv_event *(*mpv_waitEvent)(mpv_handle *ctx, double timeout);
typedef int (*mpv_set_optionString)(mpv_handle *ctx, const char *name, const char *data);
typedef int (*mpv_setProperty)(mpv_handle *ctx, const char *name, mpv_format format, void *data);
typedef int (*mpv_setProperty_async)(
    mpv_handle *ctx, uint64_t reply_userdata, const char *name, mpv_format format, void *data);
typedef int (*mpv_commandNode)(mpv_handle *ctx, mpv_node *args, mpv_node *result);
typedef int (*mpv_commandNode_async)(mpv_handle *ctx, uint64_t reply_userdata, mpv_node *args);
typedef int (*mpv_getProperty)(mpv_handle *ctx, const char *name, mpv_format format, void *data);
typedef int (*mpv_observeProperty)(mpv_handle *mpv, uint64_t reply_userdata, const char *name, mpv_format format);
typedef const char *(*mpv_eventName)(mpv_event_id event);
typedef mpv_handle *(*mpvCreate)(void);
typedef int (*mpv_requestLog_messages)(mpv_handle *ctx, const char *min_level);
typedef int (*mpv_observeProperty)(mpv_handle *mpv, uint64_t reply_userdata, const char *name, mpv_format format);
typedef void (*mpv_setWakeup_callback)(mpv_handle *ctx, void (*cb)(void *d), void *d);
typedef int (*mpvinitialize)(mpv_handle *ctx);
typedef void (*mpv_freeNode_contents)(mpv_node *node);
typedef void (*mpv_terminateDestroy)(mpv_handle *ctx);

DMULTIMEDIA_BEGIN_NAMESPACE

class DMpvProxyPrivate : public QObject {
    Q_OBJECT
    Q_DECLARE_PUBLIC(DMpvProxy)
  public:
    explicit DMpvProxyPrivate(DMpvProxy *parent = nullptr) : QObject(parent), q_ptr(parent)
    {
    }

  public:
    mpv_handle *mpv_init();
    void processPropertyChange(mpv_event_property *pEvent);
    void processLogMessage(mpv_event_log_message *pEvent);
    QImage takeOneScreenshot() const;
    void updatePlayingMovieInfo();
    void setState(DPlayerBackend::PlayState state);
    qint64 nextBurstShootPoint();
    int volumeCorrection(int);
    QVariant my_get_property(mpv_handle *pHandle, const QString &sName) const;
    int my_set_property(mpv_handle *pHandle, const QString &sName, const QVariant &value);
    bool my_command_async(mpv_handle *pHandle, const QVariant &args, uint64_t tag);
    int my_set_property_async(mpv_handle *pHandle, const QString &sName, const QVariant &v, uint64_t tag);
    QVariant my_get_property_variant(mpv_handle *pHandle, const QString &sName);
    QVariant my_command(mpv_handle *pHandle, const QVariant &args);

  private:
    DMpvProxy *q_ptr;
    mpv_waitEvent m_waitEvent;
    mpv_set_optionString m_setOptionString;
    mpv_setProperty m_setProperty;
    mpv_setProperty_async m_setPropertyAsync;
    mpv_commandNode m_commandNode;
    mpv_commandNode_async m_commandNodeAsync;
    mpv_getProperty m_getProperty;
    mpv_observeProperty m_observeProperty;
    mpv_eventName m_eventName;
    mpvCreate m_creat;
    mpv_requestLog_messages m_requestLogMessage;
    mpv_setWakeup_callback m_setWakeupCallback;
    mpvinitialize m_initialize;
    mpv_freeNode_contents m_freeNodecontents;
    void *m_gpuInfo;
    MpvHandle m_handle;
    PlayingMovieInfo m_movieInfo;
    QString m_sInitVo;
    QVariant m_posBeforeBurst;
    QList<qint64> m_listBurstPoints;
    qint64 m_nBurstStart;
    bool m_bPendingSeek;
    bool m_bInBurstShotting;
    bool m_bPolling;
    bool m_bConnectStateChange;
    bool m_bPauseOnStart;
    bool m_bIsJingJia;
    bool m_bInited;
    bool m_bHwaccelAuto;
    bool m_bLastIsSpecficFormat;
    QMap<QString, QVariant> m_mapWaitSet;
    QVector<QVariant> m_vecWaitCommand;
    QMap<QString, QString> *m_pConfig;
    DecodeMode m_decodeMode{DecodeMode::AUTO};
};


DMULTIMEDIA_END_NAMESPACE

#endif
