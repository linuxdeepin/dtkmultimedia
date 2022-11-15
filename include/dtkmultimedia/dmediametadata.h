// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DMEDIAMETADATA_H
#define DMEDIAMETADATA_H

#include <QHash>
#include <QVariant>
#include <QtMultimedia/qtmultimediaglobal.h>
#include <dtkmultimedia.h>

class QString;
DMULTIMEDIA_BEGIN_NAMESPACE
class Q_MULTIMEDIA_EXPORT DMediaMetaData
{
    Q_GADGET
public:
    enum Key {
        Title,
        Author,
        Comment,
        Description,
        Genre,
        Date,

        Language,
        Publisher,
        Copyright,
        Url,

        Duration,
        MediaType,
        FileFormat,

        AudioBitRate,
        AudioCodec,
        VideoBitRate,
        VideoCodec,
        VideoFrameRate,

        AlbumTitle,
        AlbumArtist,
        ContributingArtist,
        TrackNumber,
        Composer,
        LeadPerformer,

        ThumbnailImage,
        CoverArtImage,

        Orientation,
        Resolution
    };
    Q_ENUM(Key)

    static constexpr int NumMetaData = Resolution + 1;

    Q_INVOKABLE QVariant value(Key k) const
    {
        return data.value(k);
    }
    Q_INVOKABLE void insert(Key k, const QVariant &value)
    {
        data.insert(k, value);
    }
    Q_INVOKABLE void remove(Key k)
    {
        data.remove(k);
    }
    Q_INVOKABLE QList<Key> keys() const
    {
        return data.keys();
    }
    QVariant &operator[](Key k)
    {
        return data[k];
    }
    Q_INVOKABLE void clear()
    {
        data.clear();
    }
    Q_INVOKABLE bool isEmpty() const
    {
        return data.isEmpty();
    }
    Q_INVOKABLE QString stringValue(Key k) const;
    Q_INVOKABLE static QString metaDataKeyToString(Key k);

protected:
    friend bool operator==(const DMediaMetaData &a, const DMediaMetaData &b)
    {
        return a.data == b.data;
    }
    friend bool operator!=(const DMediaMetaData &a, const DMediaMetaData &b)
    {
        return a.data != b.data;
    }

protected:
    QHash<Key, QVariant> data;
};

DMULTIMEDIA_END_NAMESPACE

Q_DECLARE_METATYPE(DMULTIMEDIA_NAMESPACE::DMediaMetaData)

#endif   //
