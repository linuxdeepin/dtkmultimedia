// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DMEDIAFORMAT_H
#define DMEDIAFORMAT_H

#ifdef BUILD_Qt6
#include <QMediaCaptureSession>
#else
#include <QMediaObject>
#endif
#include "dtkmultimedia.h"

DMULTIMEDIA_BEGIN_NAMESPACE

class DMediaFormatPrivate;

class Q_MULTIMEDIA_EXPORT DMediaFormat : public QObject
{
    Q_GADGET
    Q_PROPERTY(FileFormat fileFormat READ fileFormat WRITE setFileFormat)
    Q_PROPERTY(AudioCodec audioCodec READ audioCodec WRITE setAudioCodec)
    Q_PROPERTY(VideoCodec videoCodec READ videoCodec WRITE setVideoCodec)
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    Q_DECLARE_PRIVATE(DMediaFormat)

public:
    enum FileFormat {
        UnspecifiedFormat = -1,
        // Video Formats
        WMV,
        AVI,
        Matroska,
        MPEG4,
        Ogg,
        QuickTime,
        WebM,
        // Audio Only Formats
        Mpeg4Audio,
        AAC,
        WMA,
        MP3,
        FLAC,
        Wave,
        LastFileFormat = Wave
    };
    Q_ENUM(FileFormat)

    enum class AudioCodec {
        Unspecified = -1,
        MP3,
        AAC,
        AC3,
        EAC3,
        FLAC,
        DolbyTrueHD,
        Opus,
        Vorbis,
        Wave,
        WMA,
        ALAC,
        LastAudioCodec = ALAC
    };
    Q_ENUM(AudioCodec)

    enum class VideoCodec {
        Unspecified = -1,
        MPEG1,
        MPEG2,
        MPEG4,
        H264,
        H265,
        VP8,
        VP9,
        AV1,
        Theora,
        WMV,
        MotionJPEG,
        LastVideoCodec = MotionJPEG
    };
    Q_ENUM(VideoCodec)

    enum ConversionMode {
        Encode,
        Decode
    };
    Q_ENUM(ConversionMode)

    enum ResolveFlags {
        NoFlags,
        RequiresVideo
    };

    DMediaFormat(FileFormat format = UnspecifiedFormat);
    DMediaFormat(const DMediaFormat &other);
    DMediaFormat &operator=(const DMediaFormat &other);
    ~DMediaFormat();

    FileFormat fileFormat() const;
    void setFileFormat(FileFormat f);

    void setVideoCodec(VideoCodec codec);
    VideoCodec videoCodec() const;

    void setAudioCodec(AudioCodec codec);
    AudioCodec audioCodec() const;

    Q_INVOKABLE bool isSupported(ConversionMode mode) const;

    Q_INVOKABLE QList<FileFormat> supportedFileFormats(ConversionMode m);
    Q_INVOKABLE QList<VideoCodec> supportedVideoCodecs(ConversionMode m);
    Q_INVOKABLE QList<AudioCodec> supportedAudioCodecs(ConversionMode m);

    Q_INVOKABLE static QString fileFormatName(FileFormat fileFormat);
    Q_INVOKABLE static QString audioCodecName(AudioCodec codec);
    Q_INVOKABLE static QString videoCodecName(VideoCodec codec);

    Q_INVOKABLE static QString fileFormatDescription(DMediaFormat::FileFormat fileFormat);
    Q_INVOKABLE static QString audioCodecDescription(DMediaFormat::AudioCodec codec);
    Q_INVOKABLE static QString videoCodecDescription(DMediaFormat::VideoCodec codec);

    bool operator==(const DMediaFormat &other) const;
    bool operator!=(const DMediaFormat &other) const
    {
        return !operator==(other);
    }

    void resolveForEncoding(ResolveFlags flags);

protected:
    QScopedPointer<DMediaFormatPrivate> d_ptr;
};

DMULTIMEDIA_END_NAMESPACE

#endif
