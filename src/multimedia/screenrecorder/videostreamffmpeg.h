// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VIDEOSTREAMFFMPEG_H
#define VIDEOSTREAMFFMPEG_H

#include "videostreaminterface.h"
#include <QLibrary>
#include <QObject>

#ifdef LIBAVUTIL_VERSION_MAJOR
#define LIBAVUTIL_VER_AT_LEAST(major,minor)  (LIBAVUTIL_VERSION_MAJOR > major || \
                                              (LIBAVUTIL_VERSION_MAJOR == major && \
                                               LIBAVUTIL_VERSION_MINOR >= minor))
#else
#define LIBAVUTIL_VER_AT_LEAST(major,minor) 0
#endif

DMULTIMEDIA_USE_NAMESPACE

class VideoStreamFfmpeg : public VideoStreamInterface
{
    Q_OBJECT
public:
    VideoStreamFfmpeg(QObject *parent = nullptr);
    ~VideoStreamFfmpeg() override;

public:
    void record() override;
    void stop() override;

private:
    void encodeWork();
    bool openInputVideoCtx();
    bool openOutputVideoCtx();
    bool loadFunction();

private:
    QLibrary libavutil;
    QLibrary libavcodec;
    QLibrary libavformat;
    QLibrary libavfilter;
    QLibrary libswscale;
    QLibrary libavdevice;

    decltype(avdevice_register_all) *d_avdevice_register_all { nullptr };

    decltype(av_find_input_format) *d_av_find_input_format { nullptr };
    decltype(avformat_open_input) *d_avformat_open_input { nullptr };
    decltype(av_read_frame) *d_av_read_frame { nullptr };
    decltype(avformat_alloc_output_context2) *d_avformat_alloc_output_context2 { nullptr };
    decltype(av_guess_format) *d_av_guess_format { nullptr };
    decltype(avformat_new_stream) *d_avformat_new_stream { nullptr };
    decltype(avformat_write_header) *d_avformat_write_header { nullptr };
    decltype(av_write_trailer) *d_av_write_trailer { nullptr };
    decltype(avio_open2) *d_avio_open2 { nullptr };
    decltype(av_write_frame) *d_av_write_frame { nullptr };

    decltype(avcodec_find_decoder) *d_avcodec_find_decoder { nullptr };
    decltype(avcodec_open2) *d_avcodec_open2 { nullptr };
    decltype(av_packet_alloc) *d_av_packet_alloc { nullptr };
#if LIBAVUTIL_VER_AT_LEAST(57,6)
    void *d_avcodec_decode_video2 { nullptr };
#else
    decltype(avcodec_decode_video2) *d_avcodec_decode_video2 { nullptr };
#endif
    decltype(avcodec_find_encoder) *d_avcodec_find_encoder { nullptr };
    decltype(avcodec_alloc_context3) *d_avcodec_alloc_context3 { nullptr };
#if LIBAVUTIL_VER_AT_LEAST(57,6)
    void *d_avcodec_encode_video2 { nullptr };
#else
     decltype(avcodec_encode_video2) *d_avcodec_encode_video2 { nullptr };
#endif
    decltype(av_init_packet) *d_av_init_packet { nullptr };
    decltype(av_packet_unref) *d_av_packet_unref { nullptr };
#if LIBAVUTIL_VER_AT_LEAST(57,6)
    void *d_avpicture_get_size { nullptr };
#else
    decltype(avpicture_get_size) *d_avpicture_get_size { nullptr };
#endif
    decltype(av_dict_set) *d_av_dict_set { nullptr };
    decltype(av_frame_alloc) *d_av_frame_alloc { nullptr };
    decltype(av_malloc) *d_av_malloc { nullptr };
    decltype(av_image_fill_arrays) *d_av_image_fill_arrays { nullptr };
    decltype(av_opt_set) *d_av_opt_set { nullptr };
    decltype(av_rescale_q) *d_av_rescale_q { nullptr };
    decltype(av_gettime) *d_av_gettime { nullptr };

    decltype(sws_getContext) *d_sws_getContext { nullptr };
    decltype(sws_scale) *d_sws_scale { nullptr };

    AVFormatContext *videoInFormatCtx { nullptr };
    AVStream *videoInStream { nullptr };
    AVCodecContext *videoInCodecCtx { nullptr };

    AVFormatContext *videoOutFormatCtx { nullptr };
    AVStream *videoOutStream { nullptr };
    AVCodecContext *videoOutCodecCtx { nullptr };

    AVDictionary *options { nullptr };
};

#endif   // VIDEOSTREAMFFMPEG_H
