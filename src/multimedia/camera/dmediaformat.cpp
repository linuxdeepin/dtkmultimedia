// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dmediaformat_p.h"
#include <QtCore/qmimedatabase.h>

DMULTIMEDIA_USE_NAMESPACE

const char *mimeTypeForFormat[DMediaFormat::LastFileFormat + 2] = {
    "",
    "video/x-ms-wmv",
    "video/x-msvideo",
    "video/x-matroska",
    "video/mp4",
    "video/ogg",
    "video/quicktime",
    "video/webm",
    // Audio Formats
    "audio/mp4",
    "audio/aac",
    "audio/x-ms-wma",
    "audio/mpeg",
    "audio/flac",
    "audio/wav"
};

constexpr DMediaFormat::FileFormat videoFormatPriorityList[] = {
    DMediaFormat::MPEG4,
    DMediaFormat::QuickTime,
    DMediaFormat::AVI,
    DMediaFormat::WebM,
    DMediaFormat::WMV,
    DMediaFormat::Matroska,
    DMediaFormat::Ogg,
    DMediaFormat::UnspecifiedFormat
};

constexpr DMediaFormat::FileFormat audioFormatPriorityList[] = {
    DMediaFormat::Mpeg4Audio,
    DMediaFormat::MP3,
    DMediaFormat::WMA,
    DMediaFormat::FLAC,
    DMediaFormat::Wave,
    DMediaFormat::UnspecifiedFormat
};

constexpr DMediaFormat::AudioCodec audioPriorityList[] = {
    DMediaFormat::AudioCodec::AAC,
    DMediaFormat::AudioCodec::MP3,
    DMediaFormat::AudioCodec::AC3,
    DMediaFormat::AudioCodec::Opus,
    DMediaFormat::AudioCodec::EAC3,
    DMediaFormat::AudioCodec::DolbyTrueHD,
    DMediaFormat::AudioCodec::WMA,
    DMediaFormat::AudioCodec::FLAC,
    DMediaFormat::AudioCodec::Vorbis,
    DMediaFormat::AudioCodec::Wave,
    DMediaFormat::AudioCodec::Unspecified
};

constexpr DMediaFormat::VideoCodec videoPriorityList[] = {
    DMediaFormat::VideoCodec::H265,
    DMediaFormat::VideoCodec::VP9,
    DMediaFormat::VideoCodec::H264,
    DMediaFormat::VideoCodec::AV1,
    DMediaFormat::VideoCodec::VP8,
    DMediaFormat::VideoCodec::WMV,
    DMediaFormat::VideoCodec::Theora,
    DMediaFormat::VideoCodec::MPEG4,
    DMediaFormat::VideoCodec::MPEG2,
    DMediaFormat::VideoCodec::MPEG1,
    DMediaFormat::VideoCodec::MotionJPEG,
};

class DMediaFormatPrivate : public QSharedData
{
};

DMediaFormat::DMediaFormat(FileFormat format)
    : d_ptr(new DMediaFormatPrivate(this))
{
    Q_D(DMediaFormat);
    d->fmt = format;
}

DMediaFormat::DMediaFormat(const DMediaFormat &other)
    : d_ptr(new DMediaFormatPrivate(this))
{
    setFileFormat(other.fileFormat());
    setVideoCodec(other.videoCodec());
    setAudioCodec(other.audioCodec());
}

DMediaFormat &DMediaFormat::operator=(const DMediaFormat &other)
{
    d_ptr.reset(new DMediaFormatPrivate(this));
    setFileFormat(other.fileFormat());
    setVideoCodec(other.videoCodec());
    setAudioCodec(other.audioCodec());
    return *this;
}

DMediaFormat::~DMediaFormat()
{
}

DMediaFormat::FileFormat DMediaFormat::fileFormat() const
{
    Q_D(const DMediaFormat);
    return d->fmt;
}

void DMediaFormat::setFileFormat(DMediaFormat::FileFormat f)
{
    Q_D(DMediaFormat);
    d->fmt = f;
}

void DMediaFormat::setVideoCodec(DMediaFormat::VideoCodec codec)
{
    Q_D(DMediaFormat);
    d->video = codec;
}

DMediaFormat::VideoCodec DMediaFormat::videoCodec() const
{
    Q_D(const DMediaFormat);
    return d->video;
}

void DMediaFormat::setAudioCodec(DMediaFormat::AudioCodec codec)
{
    Q_D(DMediaFormat);
    d->audio = codec;
}

DMediaFormat::AudioCodec DMediaFormat::audioCodec() const
{
    Q_D(const DMediaFormat);
    return d->audio;
}

bool DMediaFormat::isSupported(ConversionMode mode) const
{
    return false;
}

QList<DMediaFormat::FileFormat> DMediaFormat::supportedFileFormats(DMediaFormat::ConversionMode m)
{
    return QList<DMediaFormat::FileFormat>();
}

QList<DMediaFormat::VideoCodec> DMediaFormat::supportedVideoCodecs(DMediaFormat::ConversionMode m)
{
    return QList<DMediaFormat::VideoCodec>();
}

QList<DMediaFormat::AudioCodec> DMediaFormat::supportedAudioCodecs(DMediaFormat::ConversionMode m)
{
    return QList<DMediaFormat::AudioCodec>();
}

QString DMediaFormat::fileFormatName(DMediaFormat::FileFormat fileFormat)
{
    constexpr const char *descriptions[DMediaFormat::LastFileFormat + 2] = {
        "Unspecified",
        "WMV",
        "AVI",
        "Matroska",
        "MPEG-4",
        "Ogg",
        "QuickTime",
        "WebM",
        // Audio Formats
        "MPEG-4 Audio",
        "AAC",
        "WMA",
        "MP3",
        "FLAC",
        "Wave"
    };
    return QString::fromUtf8(descriptions[int(fileFormat) + 1]);
}

QString DMediaFormat::audioCodecName(DMediaFormat::AudioCodec codec)
{
    constexpr const char *descriptions[] = {
        "Invalid",
        "MP3",
        "AAC",
        "AC3",
        "EAC3",
        "FLAC",
        "DolbyTrueHD",
        "Opus",
        "Vorbis",
        "Wave",
        "WMA",
        "ALAC",
    };
    return QString::fromUtf8(descriptions[int(codec) + 1]);
}

QString DMediaFormat::videoCodecName(DMediaFormat::VideoCodec codec)
{
    constexpr const char *descriptions[] = {
        "Invalid",
        "MPEG1",
        "MPEG2",
        "MPEG4",
        "H264",
        "H265",
        "VP8",
        "VP9",
        "AV1",
        "Theora",
        "WMV",
        "MotionJPEG"
    };
    return QString::fromUtf8(descriptions[int(codec) + 1]);
}

QString DMediaFormat::fileFormatDescription(DMediaFormat::FileFormat fileFormat)
{
    constexpr const char *descriptions[DMediaFormat::LastFileFormat + 2] = {
        "Unspecified File Format",
        "Windows Media Video",
        "Audio Video Interleave",
        "Matroska Multimedia Container",
        "MPEG-4 Video Container",
        "Ogg",
        "QuickTime Container",
        "WebM",
        // Audio Formats
        "MPEG-4 Audio",
        "AAC",
        "Windows Media Audio",
        "MP3",
        "Free Lossless Audio Codec (FLAC)",
        "Wave File"
    };
    return QString::fromUtf8(descriptions[int(fileFormat) + 1]);
}

QString DMediaFormat::audioCodecDescription(DMediaFormat::AudioCodec codec)
{
    constexpr const char *descriptions[] = {
        "Unspecified Audio Codec",
        "MP3",
        "Advanced Audio Codec (AAC)",
        "Dolby Digital (AC3)",
        "Dolby Digital Plus (E-AC3)",
        "Free Lossless Audio Codec (FLAC)",
        "Dolby True HD",
        "Opus",
        "Vorbis",
        "Wave",
        "Windows Media Audio",
        "Apple Lossless Audio Codec (ALAC)",
    };
    return QString::fromUtf8(descriptions[int(codec) + 1]);
}

QString DMediaFormat::videoCodecDescription(DMediaFormat::VideoCodec codec)
{
    constexpr const char *descriptions[] = {
        "Unspecified Video Codec",
        "MPEG-1 Video",
        "MPEG-2 Video",
        "MPEG-4 Video",
        "H.264",
        "H.265",
        "VP8",
        "VP9",
        "AV1",
        "Theora",
        "Windows Media Video",
        "MotionJPEG"
    };
    return QString::fromUtf8(descriptions[int(codec) + 1]);
}

bool DMediaFormat::operator==(const DMediaFormat &other) const
{
    Q_D(const DMediaFormat);
    return d->fmt == other.fileFormat() && d->audio == other.audioCodec() && d->video == other.videoCodec();
}

void DMediaFormat::resolveForEncoding(ResolveFlags flags)
{
    Q_D(DMediaFormat);
    const bool requiresVideo = (flags & ResolveFlags::RequiresVideo) != 0;

    if (!requiresVideo)
        d->video = VideoCodec::Unspecified;

    // need to adjust the format. Priority is given first to file format, then video codec, then audio codec

    DMediaFormat nullFormat;
    auto supportedFormats = nullFormat.supportedFileFormats(DMediaFormat::Encode);
    auto supportedAudioCodecs = nullFormat.supportedAudioCodecs(DMediaFormat::Encode);
    auto supportedVideoCodecs = nullFormat.supportedVideoCodecs(DMediaFormat::Encode);

    auto bestSupportedFileFormat = [&](DMediaFormat::AudioCodec audio = DMediaFormat::AudioCodec::Unspecified,
                                       DMediaFormat::VideoCodec video = DMediaFormat::VideoCodec::Unspecified) {
        DMediaFormat f;
        f.setAudioCodec(audio);
        f.setVideoCodec(video);
        auto supportedFormats = f.supportedFileFormats(DMediaFormat::Encode);
        auto *list = (flags == NoFlags) ? audioFormatPriorityList : videoFormatPriorityList;
        while (*list != DMediaFormat::UnspecifiedFormat) {
            if (supportedFormats.contains(*list))
                break;
            ++list;
        }
        return *list;
    };

    // reset format if it does not support video when video is required
    if (requiresVideo && this->supportedVideoCodecs(DMediaFormat::Encode).isEmpty())
        d->fmt = DMediaFormat::UnspecifiedFormat;

    // reset non supported formats and codecs
    if (!supportedFormats.contains(d->fmt))
        d->fmt = DMediaFormat::UnspecifiedFormat;
    if (!supportedAudioCodecs.contains(d->audio))
        d->audio = DMediaFormat::AudioCodec::Unspecified;
    if (!requiresVideo || !supportedVideoCodecs.contains(d->video))
        d->video = DMediaFormat::VideoCodec::Unspecified;

    if (requiresVideo) {
        // try finding a file format that is supported
        if (d->fmt == DMediaFormat::UnspecifiedFormat)
            d->fmt = bestSupportedFileFormat(d->audio, d->video);
        // try without the audio codec
        if (d->fmt == DMediaFormat::UnspecifiedFormat)
            d->fmt = bestSupportedFileFormat(DMediaFormat::AudioCodec::Unspecified, d->video);
    }
    // try without the video codec
    if (d->fmt == DMediaFormat::UnspecifiedFormat)
        d->fmt = bestSupportedFileFormat(d->audio);
    // give me a format that's supported
    if (d->fmt == DMediaFormat::UnspecifiedFormat)
        d->fmt = bestSupportedFileFormat();
    // still nothing? Give up
    if (d->fmt == DMediaFormat::UnspecifiedFormat) {
        *this = {};
        return;
    }

    // find a working video codec
    if (requiresVideo) {
        // reset the audio codec, so that we won't throw away the video codec
        // if it is supported (choosing the specified video codec has higher
        // priority than the specified audio codec)
        auto a = d->audio;
        d->audio = DMediaFormat::AudioCodec::Unspecified;
        auto videoCodecs = this->supportedVideoCodecs(DMediaFormat::Encode);
        if (!videoCodecs.contains(d->video)) {
            // not supported, try to find a replacement
            auto *list = videoPriorityList;
            while (*list != DMediaFormat::VideoCodec::Unspecified) {
                if (videoCodecs.contains(*list))
                    break;
                ++list;
            }
            d->video = *list;
        }
        d->audio = a;
    } else {
        d->video = DMediaFormat::VideoCodec::Unspecified;
    }

    // and a working audio codec
    auto audioCodecs = this->supportedAudioCodecs(DMediaFormat::Encode);
    if (!audioCodecs.contains(d->audio)) {
        auto *list = audioPriorityList;
        while (*list != DMediaFormat::AudioCodec::Unspecified) {
            if (audioCodecs.contains(*list))
                break;
            ++list;
        }
        d->audio = *list;
    }
}
