// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PLAYLISTFILEPARSER_P_H
#define PLAYLISTFILEPARSER_P_H

#include "qtmultimediaglobal.h"
#include <QtCore/qobject.h>
#include "qmediaplaylist.h"

class QIODevice;
class QUrl;
class QNetworkRequest;

class QPlaylistFileParserPrivate;

class QPlaylistFileParser : public QObject
{
    Q_OBJECT
public:
    QPlaylistFileParser(QObject *parent = nullptr);
    ~QPlaylistFileParser();

    enum FileType
    {
        UNKNOWN,
        M3U,
        M3U8, // UTF-8 version of M3U
        PLS
    };

    void start(const QUrl &media, QIODevice *stream = nullptr, const QString &mimeType = QString());
    void start(const QUrl &request, const QString &mimeType = QString());
    void start(QIODevice *stream, const QString &mimeType = QString());
    void abort();

    QList<QUrl> playlist;

Q_SIGNALS:
    void newItem(const QVariant& content);
    void finished();
    void error(QMediaPlaylist::Error err, const QString& errorMsg);

private Q_SLOTS:
    void handleData();
    void handleError();

private:

    static FileType findByMimeType(const QString &mime);
    static FileType findBySuffixType(const QString &suffix);
    static FileType findByDataHeader(const char *data, quint32 size);
    static FileType findPlaylistType(QIODevice *device,
                                     const QString& mime);
    static FileType findPlaylistType(const QString &suffix,
                                     const QString& mime,
                                     const char *data = nullptr,
                                     quint32 size = 0);

    Q_DISABLE_COPY(QPlaylistFileParser)
    Q_DECLARE_PRIVATE(QPlaylistFileParser)
    QScopedPointer<QPlaylistFileParserPrivate> d_ptr;
};


#endif // PLAYLISTFILEPARSER_P_H
