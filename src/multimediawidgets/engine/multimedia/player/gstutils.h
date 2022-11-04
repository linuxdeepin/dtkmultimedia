// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GSTUTILS_H
#define GSTUTILS_H

#include "dplaylistmodel.h"
#include <QObject>
#include <QString>
#include <dtkmultimedia.h>
#include <string.h>

extern "C" {
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>
}

DMULTIMEDIA_BEGIN_NAMESPACE
typedef void (*mvideo_gst_init)(int *argc, char **argv[]);
typedef GstDiscoverer *(*mvideo_gst_discoverer_new)(GstClockTime timeout, GError **err);
typedef void (*mvideo_gst_discoverer_start)(GstDiscoverer *discoverer);
typedef void (*mvideo_gst_discoverer_stop)(GstDiscoverer *discoverer);
typedef gboolean (*mvideo_gst_discoverer_discover_uri_async)(GstDiscoverer *discoverer, const gchar *uri);

typedef gchar *(*mvideo_gst_discoverer_info_get_uri)(const GstDiscovererInfo *info);
typedef GstDiscovererResult (*mvideo_gst_discoverer_info_get_result)(const GstDiscovererInfo *info);
typedef const GstStructure *(*mvideo_gst_discoverer_info_get_misc)(const GstDiscovererInfo *info);
typedef gchar *(*mvideo_gst_structure_to_string)(const GstStructure *structure);
typedef GList *(*mvideo_gst_discoverer_info_get_video_streams)(GstDiscovererInfo *info);
typedef GList *(*mvideo_gst_discoverer_info_get_audio_streams)(GstDiscovererInfo *info);
typedef guint (*mvideo_gst_discoverer_video_info_get_width)(const GstDiscovererVideoInfo *info);
typedef guint (*mvideo_gst_discoverer_video_info_get_height)(const GstDiscovererVideoInfo *info);
typedef guint (*mvideo_gst_discoverer_video_info_get_framerate_num)(const GstDiscovererVideoInfo *info);
typedef guint (*mvideo_gst_discoverer_video_info_get_framerate_denom)(const GstDiscovererVideoInfo *info);
typedef guint (*mvideo_gst_discoverer_video_info_get_bitrate)(const GstDiscovererVideoInfo *info);
typedef GstClockTime (*mvideo_gst_discoverer_info_get_duration)(const GstDiscovererInfo *info);
typedef guint (*mvideo_gst_discoverer_audio_info_get_sample_rate)(const GstDiscovererAudioInfo *info);
typedef guint (*mvideo_gst_discoverer_audio_info_get_bitrate)(const GstDiscovererAudioInfo *info);
typedef guint (*mvideo_gst_discoverer_audio_info_get_channels)(const GstDiscovererAudioInfo *info);
typedef guint (*mvideo_gst_discoverer_audio_info_get_depth)(const GstDiscovererAudioInfo *info);

typedef struct CustomData
{
    GstDiscoverer *discoverer;
    GMainLoop *loop;
} CustomData;

class GstUtils
{

public:
    ~GstUtils();
    static GstUtils *get();
    static void discovered(GstDiscoverer *discoverer, GstDiscovererInfo *info, GError *err, CustomData *data);
    static void finished(GstDiscoverer *discoverer, CustomData *data);
    ModeMovieInfo parseFileByGst(const QFileInfo &fi);

private:
    GstUtils();
    QString libPath(const QString &strlib);

private:
    static ModeMovieInfo m_movieInfo;
    static GstUtils *m_pGstUtils;
    CustomData m_gstData;

    mvideo_gst_init g_mvideo_gst_init = nullptr;
    mvideo_gst_discoverer_new g_mvideo_gst_discoverer_new = nullptr;
    mvideo_gst_discoverer_start g_mvideo_gst_discoverer_start = nullptr;
    mvideo_gst_discoverer_stop g_mvideo_gst_discoverer_stop = nullptr;
    mvideo_gst_discoverer_discover_uri_async g_mvideo_gst_discoverer_discover_uri_async = nullptr;
};
DMULTIMEDIA_END_NAMESPACE

#endif   // GSTUTILS_H
