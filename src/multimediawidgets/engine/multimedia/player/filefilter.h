// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FILEFILTER_H
#define FILEFILTER_H

#include <QDir>
#include <QFileInfo>
#include <QLibrary>
#include <QLibraryInfo>
#include <QMap>
#include <QMimeDatabase>
#include <QMimeType>
#include <QObject>
#include <QUrl>
#include <string.h>

extern "C" {
#include <libavformat/avformat.h>
}

extern "C" {
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>
}

typedef int (*mvideo_avformat_open_input)(
        AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options);
typedef int (*mvideo_avformat_find_stream_info)(AVFormatContext *ic, AVDictionary **options);
typedef void (*mvideo_avformat_close_input)(AVFormatContext **s);

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
typedef GList *(*mvideo_gst_discoverer_info_get_subtitle_streams)(GstDiscovererInfo *info);

class FileFilter : public QObject
{
    Q_OBJECT

    enum MediaType { Audio = 0,
                     Video,
                     Subtitle,
                     Other };

public:
    ~FileFilter();

    static FileFilter *instance();
    bool isMediaFile(QUrl url);
    QList<QUrl> filterDir(QDir dir);
    QUrl fileTransfer(QString strFile);
    bool isAudio(QUrl url);
    bool isSubtitle(QUrl url);
    bool isVideo(QUrl url);
    MediaType typeJudgeByFFmpeg(const QUrl &url);
    MediaType typeJudgeByGst(const QUrl &url);

    static void discovered(GstDiscoverer *discoverer, GstDiscovererInfo *info, GError *err, MediaType *miType);

    static void finished(GstDiscoverer *discoverer, GMainLoop *loop);

    void stopThread();

private:
    FileFilter();

    QString libPath(const QString &strlib);

private:
    static FileFilter *m_pFileFilter;
    QMap<QUrl, bool> m_mapCheckAudio;
    mvideo_avformat_open_input g_mvideo_avformat_open_input = nullptr;
    mvideo_avformat_find_stream_info g_mvideo_avformat_find_stream_info = nullptr;
    mvideo_avformat_close_input g_mvideo_avformat_close_input = nullptr;

    mvideo_gst_init g_mvideo_gst_init = nullptr;
    mvideo_gst_discoverer_new g_mvideo_gst_discoverer_new = nullptr;
    mvideo_gst_discoverer_start g_mvideo_gst_discoverer_start = nullptr;
    mvideo_gst_discoverer_stop g_mvideo_gst_discoverer_stop = nullptr;
    mvideo_gst_discoverer_discover_uri_async g_mvideo_gst_discoverer_discover_uri_async = nullptr;

    QMimeDatabase m_mimeDB;
    bool m_bMpvExists;
    bool m_stopRunningThread;
    GstDiscoverer *m_pDiscoverer;
    GMainLoop *m_pLoop;
    MediaType m_miType;
};

#endif   // FILEFILTER_H
