// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIOENCODERINTERFACE_P_H
#define DAUDIOENCODERINTERFACE_P_H

#include "daudioencoderinterface.h"
#include <QLibrary>
#include <QUrl>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libavdevice/avdevice.h>
#include <libavutil/audio_fifo.h>
}
#include <thread>

DMULTIMEDIA_BEGIN_NAMESPACE

class DAudioEncoderInterfacePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DAudioEncoderInterface)
public:
    explicit DAudioEncoderInterfacePrivate(DAudioEncoderInterface *parent = nullptr)
        : QObject(parent), q_ptr(parent)
    {
    }

private:
    int bitRateAdaptation();

    AVCodecID codecAdaptation();

private:
    DAudioEncoderInterface *q_ptr;

    QLibrary libavutil;
    QLibrary libavcodec;
    QLibrary libavformat;
    QLibrary libavfilter;
    QLibrary libswresample;
    QLibrary libavdevice;

    decltype(av_find_input_format) *d_av_find_input_format { nullptr };
    decltype(avformat_open_input) *d_avformat_open_input { nullptr };
    decltype(avformat_find_stream_info) *d_avformat_find_stream_info { nullptr };
    decltype(avformat_alloc_output_context2) *d_avformat_alloc_output_context2 { nullptr };
    decltype(avio_open) *d_avio_open { nullptr };
    decltype(avio_close) *d_avio_close { nullptr };
    decltype(avformat_new_stream) *d_avformat_new_stream { nullptr };
    decltype(avformat_write_header) *d_avformat_write_header { nullptr };
    decltype(av_write_trailer) *d_av_write_trailer { nullptr };
    decltype(av_read_frame) *d_av_read_frame { nullptr };
    decltype(av_write_frame) *d_av_write_frame { nullptr };
    decltype(avformat_close_input) *d_avformat_close_input { nullptr };
    decltype(avformat_free_context) *d_avformat_free_context { nullptr };

    decltype(avcodec_find_decoder) *d_avcodec_find_decoder { nullptr };
    decltype(avcodec_alloc_context3) *d_avcodec_alloc_context3 { nullptr };
    decltype(avcodec_parameters_to_context) *d_avcodec_parameters_to_context { nullptr };
    decltype(avcodec_open2) *d_avcodec_open2 { nullptr };
    decltype(avcodec_find_encoder) *d_avcodec_find_encoder { nullptr };
    decltype(avcodec_parameters_from_context) *d_avcodec_parameters_from_context { nullptr };
    decltype(av_packet_alloc) *d_av_packet_alloc { nullptr };
    decltype(avcodec_send_packet) *d_avcodec_send_packet { nullptr };
    decltype(avcodec_receive_frame) *d_avcodec_receive_frame { nullptr };
    decltype(avcodec_send_frame) *d_avcodec_send_frame { nullptr };
    decltype(avcodec_receive_packet) *d_avcodec_receive_packet { nullptr };
    decltype(av_packet_free) *d_av_packet_free { nullptr };
    decltype(av_packet_unref) *d_av_packet_unref { nullptr };
    decltype(avcodec_free_context) *d_avcodec_free_context { nullptr };

    decltype(swr_alloc_set_opts) *d_swr_alloc_set_opts { nullptr };
    decltype(swr_init) *d_swr_init { nullptr };
    decltype(swr_convert) *d_swr_convert { nullptr };
    decltype(swr_free) *d_swr_free { nullptr };

    decltype(av_audio_fifo_alloc) *d_av_audio_fifo_alloc { nullptr };
    decltype(av_frame_alloc) *d_av_frame_alloc { nullptr };
    decltype(av_get_default_channel_layout) *d_av_get_default_channel_layout { nullptr };
    decltype(av_samples_alloc_array_and_samples) *d_av_samples_alloc_array_and_samples { nullptr };
    decltype(av_audio_fifo_space) *d_av_audio_fifo_space { nullptr };
    decltype(av_audio_fifo_write) *d_av_audio_fifo_write { nullptr };
    decltype(av_freep) *d_av_freep { nullptr };
    decltype(av_frame_unref) *d_av_frame_unref { nullptr };

    decltype(av_frame_get_buffer) *d_av_frame_get_buffer { nullptr };
    decltype(av_audio_fifo_read) *d_av_audio_fifo_read { nullptr };
    decltype(av_frame_free) *d_av_frame_free { nullptr };
    decltype(av_audio_fifo_size) *d_av_audio_fifo_size { nullptr };
    decltype(av_audio_fifo_free) *d_av_audio_fifo_free { nullptr };

    decltype(avdevice_register_all) *d_avdevice_register_all { nullptr };

    AVFormatContext *audioInFormatCtx { nullptr };
    AVStream *audioInStream { nullptr };
    AVCodecContext *audioInCodecCtx { nullptr };
    SwrContext *audioConverter { nullptr };
    AVAudioFifo *audioFifo { nullptr };
    AVFormatContext *audioOutFormatCtx { nullptr };
    AVStream *audioOutStream { nullptr };
    AVCodecContext *audioOutCodecCtx { nullptr };

    QString deviceName { "default" };
    QUrl outFilePath { "outAudioFile.aac" };
    int sampleRate { 48000 };
    int bitRate { 0 };
    DAudioRecorder::AChannelsID channels { DAudioRecorder::CHANNELS_ID_STEREO };
    AVCodecID codec { AV_CODEC_ID_NONE };

#if BUILD_Qt6
    QMediaRecorder::RecorderState state { QMediaRecorder::StoppedState };
#else
    QMediaRecorder::State state { QMediaRecorder::StoppedState };
#endif
    std::thread *audioRecorderThread { nullptr };
    std::atomic_bool isRecording { false };
    std::atomic_bool isPause { false };

    bool isLoadFunction { false };
    bool isOpenInputAudioCtx { false };
    bool isOpenOutputAudioCtx { false };
};

DMULTIMEDIA_END_NAMESPACE
#endif   // DAUDIOENCODERINTERFACE_P_H
