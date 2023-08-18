// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLAYLISTMODEL_H
#define DPLAYLISTMODEL_H

#include <QtWidgets>
//#include <QtConcurrent>
#ifdef BUILD_Qt6
#include <DGuiApplicationHelper>
#else
#include <DApplicationHelper>
#endif
#include <libffmpegthumbnailer/videothumbnailerc.h>

#include "utils.h"
#include <QMutex>
#include <QNetworkReply>
#include <dtkmultimedia.h>

#include <libffmpegthumbnailer/videothumbnailerc.h>

#define THUMBNAIL_SIZE 500
#define SEEK_TIME "00:00:01"

typedef video_thumbnailer *(*mvideo_thumbnailer)();
typedef void (*mvideo_thumbnailer_destroy)(video_thumbnailer *thumbnailer);
/* create image_data structure */
typedef image_data *(*mvideo_thumbnailer_create_image_data)(void);
/* destroy image_data structure */
typedef void (*mvideo_thumbnailer_destroy_image_data)(image_data *data);
typedef int (*mvideo_thumbnailer_generate_thumbnail_to_buffer)(
        video_thumbnailer *thumbnailer, const char *movie_filename, image_data *generated_image_data);

DGUI_USE_NAMESPACE
DMULTIMEDIA_BEGIN_NAMESPACE
class PlayerEngine;
class LoadThread;
class GetThumanbil;

struct ModeMovieInfo
{
    bool valid;
    QString title;
    QString fileType;
    QString resolution;
    QString filePath;
    QString creation;

    // rotation in metadata, this affects width/height
    int rawRotate;
    qint64 fileSize;
    qint64 duration;
    int width = -1;
    int height = -1;
    int vCodecID;
    qint64 vCodeRate;
    int fps;
    float proportion;
    int aCodeID;
    qint64 aCodeRate;
    int aDigit;
    int channels;
    int sampling;
#ifdef _MOVIE_USE_
    QString strFmtName;
#endif
    ModeMovieInfo()
    {
        valid = false;
        rawRotate = -1;
        fileSize = -1;
        duration = -1;
        width = -1;
        height = -1;
        vCodecID = -1;
        aCodeRate = -1;
        fps = -1;
        proportion = -1.0;
        aCodeID = -1;
        aCodeRate = -1;
        aDigit = -1;
        channels = -1;
        sampling = -1;
    }

    static struct ModeMovieInfo parseFromFile(const QFileInfo &fi, bool *ok = nullptr);
    QString durationStr() const
    {
        return utils::Time2str(duration);
    }

    QString videoCodec() const
    {
        return utils::videoIndex2str(vCodecID);
    }

    QString audioCodec() const
    {
        return utils::audioIndex2str(aCodeID);
    }

    QString sizeStr() const
    {
        auto K = 1024;
        auto M = 1024 * K;
        auto G = 1024 * M;
        if (fileSize > G) {
            return QString(QT_TR_NOOP("%1G")).arg((double)fileSize / G, 0, 'f', 1);
        } else if (fileSize > M) {
            return QString(QT_TR_NOOP("%1M")).arg((double)fileSize / M, 0, 'f', 1);
        } else if (fileSize > K) {
            return QString(QT_TR_NOOP("%1K")).arg((double)fileSize / K, 0, 'f', 1);
        }
        return QString(QT_TR_NOOP("%1")).arg(fileSize);
    }
    bool isRawFormat() const
    {
        bool bFlag = false;
#ifdef _MOVIE_USE_
        if (strFmtName.contains("raw", Qt::CaseInsensitive)) bFlag = true;
#endif

        return bFlag;
    }
};

struct PlayItemInfo
{
    bool valid;
    bool loaded;
    QUrl url;
    QFileInfo info;
    QPixmap thumbnail;
    QPixmap thumbnail_dark;
    struct ModeMovieInfo mi;

    bool refresh();
};

using AppendJob = QPair<QUrl, QFileInfo>;
using PlayItemInfoList = QList<PlayItemInfo>;
using UrlList = QList<QUrl>;

class DPlaylistModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int current READ current WRITE changeCurrent NOTIFY currentChanged)

public:
    friend class PlayerEngine;
    enum PlayMode {
        OrderPlay,
        ShufflePlay,
        SinglePlay,
        SingleLoop,
        ListLoop,
    };

    void stop();

    PlayMode playMode() const;
    void setPlayMode(PlayMode pm);

    explicit DPlaylistModel(PlayerEngine *engine);
    ~DPlaylistModel();

    qint64 getUrlFileTotalSize(QUrl url, int tryTimes) const;

    void clear();
    void remove(int pos);
    void append(const QUrl &);

    void appendAsync(const QList<QUrl> &);
    void collectionJob(const QList<QUrl> &, QList<QUrl> &);

    void playNext(bool fromUser);
    void playPrev(bool fromUser);

    int count() const;
    const QList<PlayItemInfo> &items() const
    {
        return m_infos;
    }
    QList<PlayItemInfo> &items()
    {
        return m_infos;
    }

    int current() const;
    const PlayItemInfo &currentInfo() const;
    PlayItemInfo &currentInfo();
    int size() const;
    int indexOf(const QUrl &url);
    void switchPosition(int p1, int p2);
    void handleAsyncAppendResults(QList<PlayItemInfo> &pil);
    struct PlayItemInfo calculatePlayInfo(const QUrl &, const QFileInfo &fi, bool isDvd = false);
    bool getthreadstate();
    void savePlaylist();
    void clearPlaylist();
    QList<QUrl> getLoadList()
    {
        return m_loadFile;
    };
    void loadPlaylist();
    bool getThumanbilRunning();

    ModeMovieInfo getMovieInfo(const QUrl &url, bool *is);

    QImage getMovieCover(const QUrl &url);

public slots:
    void changeCurrent(int);
    void delayedAppendAsync(const QList<QUrl> &);
    void clearLoad();

private slots:
    void onAsyncFinished();
    void onAsyncUpdate(PlayItemInfo);
    void slotStateChanged();

signals:
    void countChanged();
    void currentChanged();
    void itemRemoved(int);
    void itemsAppended();
    void emptied();
    void playModeChanged(PlayMode);
    void asyncAppendFinished(const QList<PlayItemInfo> &);
    void itemInfoUpdated(int id);
    void updateDuration();

private:
    void initThumb();
    void initFFmpeg();
    bool getMusicPix(const QFileInfo &fi, QPixmap &rImg);
    struct ModeMovieInfo parseFromFile(const QFileInfo &fi, bool *ok = nullptr);
    struct ModeMovieInfo parseFromFileByQt(const QFileInfo &fi, bool *ok = nullptr);
    QString libPath(const QString &strlib);
    bool m_firstLoad { true };
    int m_count { 0 };
    int m_current { -1 };
    int m_last { -1 };
    bool m_hasNormalVideo { false };
    PlayMode m_playMode { PlayMode::OrderPlay };
    QList<PlayItemInfo> m_infos;

    QList<int> m_playOrder;
    int m_shufflePlayed { 0 };
    int m_loopCount { 0 };

    QList<AppendJob> m_pendingJob;
    QSet<QString> m_urlsInJob;

    QQueue<UrlList> m_pendingAppendReq;

    bool m_userRequestingItem { false };

    video_thumbnailer *m_video_thumbnailer = nullptr;
    image_data *m_image_data = nullptr;

    mvideo_thumbnailer m_mvideo_thumbnailer = nullptr;
    mvideo_thumbnailer_destroy m_mvideo_thumbnailer_destroy = nullptr;
    mvideo_thumbnailer_create_image_data m_mvideo_thumbnailer_create_image_data = nullptr;
    mvideo_thumbnailer_destroy_image_data m_mvideo_thumbnailer_destroy_image_data = nullptr;
    mvideo_thumbnailer_generate_thumbnail_to_buffer m_mvideo_thumbnailer_generate_thumbnail_to_buffer = nullptr;

    PlayerEngine *m_engine { nullptr };

    QString m_playlistFile;

    LoadThread *m_ploadThread;
    GetThumanbil *m_getThumanbil { nullptr };
    QMutex *m_pdataMutex;
    bool m_brunning;
    QList<QUrl> m_tempList;
    QList<QUrl> m_loadFile;
    bool m_initFFmpeg { false };
    bool m_bInitThumb { false };
    PlayItemInfo m_currentInfo;

    void reshuffle();
    void appendSingle(const QUrl &);
    void tryPlayCurrent(bool next);
};

class LoadThread : public QThread
{
    Q_OBJECT

public:
    LoadThread(DPlaylistModel *model, const QList<QUrl> &urls);
    ~LoadThread();

public:
    void run();

private:
    DPlaylistModel *m_pModel;
    QList<QUrl> m_urls;
    QList<AppendJob> m_pendingJob;
    QSet<QString> m_urlsInJob;
};

class GetThumanbil : public QThread
{
    Q_OBJECT
public:
    GetThumanbil(DPlaylistModel *model, const QList<QUrl> &urls)
        : m_model(model), m_urls(urls)
    {
        //        m_model = model;
        //        m_urls = urls;
        m_mutex = new QMutex;
    };
    ~GetThumanbil()
    {
        m_stop = true;
        delete m_mutex;
        m_mutex = nullptr;
    };
    void stop()
    {
        m_stop = true;
    };
    void setUrls(QList<QUrl> urls)
    {
        m_mutex->lock();
        m_urls = urls;
        m_mutex->unlock();
    };
    void clearItem()
    {
        m_mutex->lock();
        m_urls.clear();
        m_mutex->unlock();
    };

    void run()
    {
        m_mutex->lock();
        QList<QUrl> urls = m_urls;
        m_mutex->unlock();
        foreach (QUrl url, urls) {
            QFileInfo info(url.path());
            emit updateItem(m_model->calculatePlayInfo(url, info, false));
            if (m_stop) break;
        }
    }

signals:
    void updateItem(PlayItemInfo);

private:
    DPlaylistModel *m_model;
    QList<QUrl> m_urls;
    QMutex *m_mutex;
    bool m_stop { false };
};

DMULTIMEDIA_END_NAMESPACE

#endif /* ifndef DPLAYLISTMODEL_H */
