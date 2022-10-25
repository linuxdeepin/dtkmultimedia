// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dmediametadata.h"
#include <QtCore/qcoreapplication.h>
#include <qvariant.h>
#include <qobject.h>
#include <qdatetime.h>
#include <QtCore>
//#include <qmediaformat.h>
#include <qsize.h>

DMULTIMEDIA_USE_NAMESPACE

QString DMediaMetaData::stringValue(DMediaMetaData::Key key) const
{
    QVariant value = data.value(key);
    if (value.isNull())
        return QString();

    switch (key) {
    // string based or convertible to string
    case Title:
    case Author:
    case Comment:
    case Description:
    case Genre:
    case Publisher:
    case Copyright:
    case Date:
    case Url:
    case MediaType:
    case AudioBitRate:
    case VideoBitRate:
    case VideoFrameRate:
    case AlbumTitle:
    case AlbumArtist:
    case ContributingArtist:
    case TrackNumber:
    case Composer:
    case Orientation:
    case LeadPerformer:
        return value.toString();
    case Language: {
        return "";
//        auto l = value.value<QLocale::Language>();
//        return QLocale::languageToString(l);
    }
    case Duration: {
        QTime time = QTime::fromMSecsSinceStartOfDay(value.toInt());
        return time.toString();
    }
    case FileFormat:
        return "";//QMediaFormat::fileFormatName(value.value<QMediaFormat::FileFormat>());
    case AudioCodec:
        return "";//QMediaFormat::audioCodecName(value.value<QMediaFormat::AudioCodec>());
    case VideoCodec:
        return "";//QMediaFormat::videoCodecName(value.value<QMediaFormat::VideoCodec>());
    case Resolution: {
        QSize size = value.toSize();
        return QString::fromUtf8("%1 x %2").arg(size.width()).arg(size.height());
    }
    case ThumbnailImage:
    case CoverArtImage:
        break;
    }
    return QString();
}

/*!
    returns a string representation of \a key that can be used when presenting
    meta data to users.
*/
QString DMediaMetaData::metaDataKeyToString(DMediaMetaData::Key key)
{
    switch (key) {
        case DMediaMetaData::Title:
            return (QCoreApplication::translate("QMediaMetaData", "Title"));
        case DMediaMetaData::Author:
            return (QCoreApplication::translate("QMediaMetaData", "Author"));
        case DMediaMetaData::Comment:
            return (QCoreApplication::translate("QMediaMetaData", "Comment"));
        case DMediaMetaData::Description:
            return (QCoreApplication::translate("QMediaMetaData", "Description"));
        case DMediaMetaData::Genre:
            return (QCoreApplication::translate("QMediaMetaData", "Genre"));
        case DMediaMetaData::Date:
            return (QCoreApplication::translate("QMediaMetaData", "Date"));
        case DMediaMetaData::Language:
            return (QCoreApplication::translate("QMediaMetaData", "Language"));
        case DMediaMetaData::Publisher:
            return (QCoreApplication::translate("QMediaMetaData", "Publisher"));
        case DMediaMetaData::Copyright:
            return (QCoreApplication::translate("QMediaMetaData", "Copyright"));
        case DMediaMetaData::Url:
            return (QCoreApplication::translate("QMediaMetaData", "Url"));
        case DMediaMetaData::Duration:
            return (QCoreApplication::translate("QMediaMetaData", "Duration"));
        case DMediaMetaData::MediaType:
            return (QCoreApplication::translate("QMediaMetaData", "Media type"));
        case DMediaMetaData::FileFormat:
            return (QCoreApplication::translate("QMediaMetaData", "Container Format"));
        case DMediaMetaData::AudioBitRate:
            return (QCoreApplication::translate("QMediaMetaData", "Audio bit rate"));
        case DMediaMetaData::AudioCodec:
            return (QCoreApplication::translate("QMediaMetaData", "Audio codec"));
        case DMediaMetaData::VideoBitRate:
            return (QCoreApplication::translate("QMediaMetaData", "Video bit rate"));
        case DMediaMetaData::VideoCodec:
            return (QCoreApplication::translate("QMediaMetaData", "Video codec"));
        case DMediaMetaData::VideoFrameRate:
            return (QCoreApplication::translate("QMediaMetaData", "Video frame rate"));
        case DMediaMetaData::AlbumTitle:
            return (QCoreApplication::translate("QMediaMetaData", "Album title"));
        case DMediaMetaData::AlbumArtist:
            return (QCoreApplication::translate("QMediaMetaData", "Album artist"));
        case DMediaMetaData::ContributingArtist:
            return (QCoreApplication::translate("QMediaMetaData", "Contributing artist"));
        case DMediaMetaData::TrackNumber:
            return (QCoreApplication::translate("QMediaMetaData", "Track number"));
        case DMediaMetaData::Composer:
            return (QCoreApplication::translate("QMediaMetaData", "Composer"));
        case DMediaMetaData::ThumbnailImage:
            return (QCoreApplication::translate("QMediaMetaData", "Thumbnail image"));
        case DMediaMetaData::CoverArtImage:
            return (QCoreApplication::translate("QMediaMetaData", "Cover art image"));
        case DMediaMetaData::Orientation:
            return (QCoreApplication::translate("QMediaMetaData", "Orientation"));
        case DMediaMetaData::Resolution:
            return (QCoreApplication::translate("QMediaMetaData", "Resolution"));
        case DMediaMetaData::LeadPerformer:
            return (QCoreApplication::translate("QMediaMetaData", "Lead performer"));
    }
    return QString();
}

