// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "videostreamffmpeg.h"
#include <QLibraryInfo>
#include <QImage>
#include <QDir>

static QString libPath(const QString &sLib)
{
    QDir dir;
    QString path = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    dir.setPath(path);
    QStringList list = dir.entryList(QStringList() << (sLib + "*"),
                                     QDir::NoDotAndDotDot | QDir::Files);   // filter name with strlib
    if (list.contains(sLib)) {
        return sLib;
    } else {
        list.sort();
    }

    if (list.size() > 0) {
        return list.last();
    } else {
        return QString();
    }
}

VideoStreamFfmpeg::VideoStreamFfmpeg(QObject *parent)
    : VideoStreamInterface(parent)
{
}

VideoStreamFfmpeg::~VideoStreamFfmpeg()
{
    if (isLoadFunction) {
        libavformat.unload();
        libavcodec.unload();
        libavdevice.unload();
        libavutil.unload();
        libswscale.unload();
    }
}

void VideoStreamFfmpeg::record()
{
    if (stateValue == QMediaRecorder::RecordingState) {
        return;
    }

    propertyRevise();

    if (!isLoadFunction) {
        if (!loadFunction()) {
            return;
        }
    }

    if (!isOpenInputVideoCtx) {
        if (!openInputVideoCtx()) {
            return;
        }
    }

    if (recStyle == Encoding && !isOpenOutputVideoCtx) {
        if (!openOutputVideoCtx()) {
            return;
        }
    }

    videoRecorderThread = new std::thread(&VideoStreamFfmpeg::encodeWork, this);
    stateValue = QMediaRecorder::RecordingState;
}

void VideoStreamFfmpeg::stop()
{
    if (stateValue != QMediaRecorder::RecordingState) {
        return;
    }
    isRecording.store(false);
    videoRecorderThread->join();
    stateValue = QMediaRecorder::StoppedState;

    if (recStyle != Encoding) return;
    if (d_av_write_trailer(videoOutFormatCtx) < 0) {
        qCritical("write trailer failed");
    }
}

void VideoStreamFfmpeg::encodeWork()
{
    int ret = 0;
    AVPacket *inputPacket = d_av_packet_alloc();
    AVFrame *inputFrame = d_av_frame_alloc();
    AVFrame *outFrame = d_av_frame_alloc();

    int nbytes = d_avpicture_get_size(pixFormat, resolutionSize.width(), resolutionSize.height());
    uint8_t *video_outbuf = static_cast<uint8_t *>(d_av_malloc(nbytes));

    ret = d_av_image_fill_arrays(outFrame->data, outFrame->linesize, video_outbuf,
                                 pixFormat, resolutionSize.width(), resolutionSize.height(), 1);
    if (ret < 0) {
        qCritical("error in filling image array");
    }
    SwsContext *swsCtx = d_sws_getContext(videoInCodecCtx->width, videoInCodecCtx->height, videoInCodecCtx->pix_fmt,
                                          resolutionSize.width(), resolutionSize.height(), pixFormat,
                                          SWS_BICUBIC, nullptr, nullptr, nullptr);
    int frameFinished = 0;
    int frameCout = 0;
    int got_picture = 0;
    isRecording.store(true);

    while (isRecording.load()) {
        if (d_av_read_frame(videoInFormatCtx, inputPacket) < 0) {
            qCritical("can not read frame");
        }
        ret = d_avcodec_decode_video2(videoInCodecCtx, inputFrame, &frameFinished, inputPacket);
        if (ret < 0) {
            qCritical("unable to decode video");
        }

        if (frameFinished) {
            d_sws_scale(swsCtx, inputFrame->data, inputFrame->linesize, 0, videoInCodecCtx->height, outFrame->data, outFrame->linesize);
            if (recStyle == CallbackReceive) {
                sendDataFunc(sendDataObj, outFrame->linesize[0] * resolutionSize.height(), outFrame->data[0]);
                continue;
            } else if (recStyle == SignalReceive) {
                QImage image((uchar *)(outFrame->data[0]), resolutionSize.width(), resolutionSize.height(), QImage::Format::Format_RGBA8888);
                emit screenStreamData(image);
                continue;
            }

            outFrame->width = resolutionSize.width();
            outFrame->height = resolutionSize.height();
            outFrame->format = pixFormat;
            AVPacket outPacket;
            d_av_init_packet(&outPacket);
            outPacket.data = nullptr;
            outPacket.size = 0;
            d_avcodec_encode_video2(videoOutCodecCtx, &outPacket, outFrame, &got_picture);
            if (1 == got_picture) {
                frameCout++;
                if (outPacket.pts != AV_NOPTS_VALUE)
                    outPacket.pts = d_av_rescale_q(outPacket.pts, videoOutStream->codec->time_base, videoOutStream->time_base);
                if (outPacket.dts != AV_NOPTS_VALUE)
                    outPacket.dts = d_av_rescale_q(outPacket.dts, videoOutStream->codec->time_base, videoOutStream->time_base);

                if (d_av_write_frame(videoOutFormatCtx, &outPacket) != 0) {
                    qCritical("error in writing video frame");
                }
                d_av_packet_unref(&outPacket);
            }
        }
    }
}

bool VideoStreamFfmpeg::openInputVideoCtx()
{
    if (isOpenInputVideoCtx) {
        return true;
    }

    int ret = 0;

    AVInputFormat *inputFormat = nullptr;
    inputFormat = d_av_find_input_format("x11grab");
    if (inputFormat == nullptr) {
        qCritical("unable to find the input format x11grab");
        return false;
    }

    ret = d_av_dict_set(&options, "framerate", QString("%1").arg(framerate).toLatin1(), 0);
    if (ret < 0) {
        qCritical("error in setting dictionary value");
        return false;
    }
    ret = d_av_dict_set(&options, "video_size",
                        QString("%1x%2").arg(resolutionSize.width()).arg(resolutionSize.height()).toLatin1(), 0);
    if (ret < 0) {
        qCritical("error in setting dictionary value");
        return false;
    }
    ret = d_av_dict_set(&options, "preset", "medium", 0);
    if (ret < 0) {
        qCritical("error in setting dictionary value");
        return false;
    }

    d_avformat_open_input(&videoInFormatCtx, QString(":0+%1,%2").arg(topLeftP.x()).arg(topLeftP.y()).toLatin1(),
                          inputFormat, &options);

    for (unsigned int i = 0; i < videoInFormatCtx->nb_streams; ++i) {
        if (videoInFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoInCodecCtx = videoInFormatCtx->streams[i]->codec;
            break;
        }
    }
    if (videoInCodecCtx == nullptr) {
        qCritical("unable to find the video stream index");
        return false;
    }

    AVCodec *inputAVCodec = nullptr;
    inputAVCodec = d_avcodec_find_decoder(videoInCodecCtx->codec_id);
    if (inputAVCodec == nullptr) {
        qCritical("unable to find the decoder");
        return false;
    }

    ret = d_avcodec_open2(videoInCodecCtx, inputAVCodec, nullptr);
    if (ret < 0) {
        qCritical("unable to open the av codec");
        return false;
    }

    isOpenInputVideoCtx = true;
    return true;
}

bool VideoStreamFfmpeg::openOutputVideoCtx()
{
    if (isOpenOutputVideoCtx) {
        return true;
    }

    if (d_avformat_alloc_output_context2(&videoOutFormatCtx, nullptr, nullptr, outFilePath.toString().toLatin1()) < 0) {
        qCritical("unable alloc_output_context2");
        return false;
    }

    videoOutStream = d_avformat_new_stream(videoOutFormatCtx, nullptr);
    AVCodec *outAVCodec = d_avcodec_find_encoder(codecId);
    videoOutCodecCtx = videoOutStream->codec;
    videoOutCodecCtx->codec_id = codecId;   // AV_CODEC_ID_MPEG4; // AV_CODEC_ID_H264 // AV_CODEC_ID_MPEG1VIDEO
    videoOutCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    videoOutCodecCtx->pix_fmt = pixFormat;
    videoOutCodecCtx->bit_rate = bitrate;
    videoOutCodecCtx->width = resolutionSize.width();
    videoOutCodecCtx->height = resolutionSize.height();
    videoOutCodecCtx->gop_size = 0;
    videoOutCodecCtx->max_b_frames = 2;
    videoOutCodecCtx->time_base.num = 1;
    videoOutCodecCtx->time_base.den = framerate;

    AVDictionary *param = nullptr;
    if (codecId == AV_CODEC_ID_H264) {
        videoOutCodecCtx->qmin = 10;
        videoOutCodecCtx->qmax = 51;
        videoOutCodecCtx->max_b_frames = 0;
        d_av_dict_set(&param, "preset", "ultrafast", 0);
    }

    if (videoOutFormatCtx->oformat->flags & AVFMT_GLOBALHEADER) {
        videoOutFormatCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (d_avcodec_open2(videoOutCodecCtx, outAVCodec, &param) < 0) {
        qCritical("error in opening the avcodec");
        return true;
    }

    if (!(videoOutFormatCtx->flags & AVFMT_NOFILE)) {
        if (d_avio_open2(&videoOutFormatCtx->pb, outFilePath.toString().toLatin1(), AVIO_FLAG_WRITE, nullptr, nullptr) < 0) {
            qCritical("error in creating the video file");
            return false;
        }
    }

    if (!videoOutFormatCtx->nb_streams) {
        qCritical("output file dose not contain any stream");
        return false;
    }

    if (d_avformat_write_header(videoOutFormatCtx, &options) < 0) {
        qCritical("error in writing the header context");
        return false;
    }

    isOpenOutputVideoCtx = true;
    return true;
}

bool VideoStreamFfmpeg::loadFunction()
{
    if (isLoadFunction) {
        return true;
    }
    libavformat.setFileName(libPath("libavformat.so"));
    libavcodec.setFileName(libPath("libavcodec.so"));
    libavdevice.setFileName(libPath("libavdevice.so"));
    libavutil.setFileName(libPath("libavutil.so"));
    libswscale.setFileName(libPath("libswscale.so"));
    if (!(libavformat.load() && libavcodec.load() && libavdevice.load()
          && libavutil.load() && libswscale.load())) {
        qCritical("Couldn't load so.");
        return false;
    }

    d_avdevice_register_all = reinterpret_cast<decltype(avdevice_register_all) *>(libavdevice.resolve("avdevice_register_all"));

    d_av_find_input_format = reinterpret_cast<decltype(av_find_input_format) *>(libavformat.resolve("av_find_input_format"));
    d_avformat_open_input = reinterpret_cast<decltype(avformat_open_input) *>(libavformat.resolve("avformat_open_input"));
    d_av_read_frame = reinterpret_cast<decltype(av_read_frame) *>(libavformat.resolve("av_read_frame"));
    d_avformat_alloc_output_context2 = reinterpret_cast<decltype(avformat_alloc_output_context2) *>(libavformat.resolve("avformat_alloc_output_context2"));
    d_av_guess_format = reinterpret_cast<decltype(av_guess_format) *>(libavformat.resolve("av_guess_format"));
    d_avformat_new_stream = reinterpret_cast<decltype(avformat_new_stream) *>(libavformat.resolve("avformat_new_stream"));
    d_avformat_write_header = reinterpret_cast<decltype(avformat_write_header) *>(libavformat.resolve("avformat_write_header"));
    d_avio_open2 = reinterpret_cast<decltype(avio_open2) *>(libavformat.resolve("avio_open2"));
    d_av_write_trailer = reinterpret_cast<decltype(av_write_trailer) *>(libavformat.resolve("av_write_trailer"));
    d_av_write_frame = reinterpret_cast<decltype(av_write_frame) *>(libavformat.resolve("av_write_frame"));
    d_av_init_packet = reinterpret_cast<decltype(av_init_packet) *>(libavformat.resolve("av_init_packet"));

    d_avcodec_find_decoder = reinterpret_cast<decltype(avcodec_find_decoder) *>(libavcodec.resolve("avcodec_find_decoder"));
    d_avcodec_open2 = reinterpret_cast<decltype(avcodec_open2) *>(libavcodec.resolve("avcodec_open2"));
    d_av_packet_alloc = reinterpret_cast<decltype(av_packet_alloc) *>(libavcodec.resolve("av_packet_alloc"));
    d_avcodec_decode_video2 = reinterpret_cast<decltype(avcodec_decode_video2) *>(libavcodec.resolve("avcodec_decode_video2"));
    d_avcodec_find_encoder = reinterpret_cast<decltype(avcodec_find_encoder) *>(libavcodec.resolve("avcodec_find_encoder"));
    d_avcodec_alloc_context3 = reinterpret_cast<decltype(avcodec_alloc_context3) *>(libavcodec.resolve("avcodec_alloc_context3"));
    d_avcodec_encode_video2 = reinterpret_cast<decltype(avcodec_encode_video2) *>(libavcodec.resolve("avcodec_encode_video2"));
    d_av_packet_unref = reinterpret_cast<decltype(av_packet_unref) *>(libavcodec.resolve("av_packet_unref"));
    d_avpicture_get_size = reinterpret_cast<decltype(avpicture_get_size) *>(libavcodec.resolve("avpicture_get_size"));

    d_av_dict_set = reinterpret_cast<decltype(av_dict_set) *>(libavutil.resolve("av_dict_set"));
    d_av_frame_alloc = reinterpret_cast<decltype(av_frame_alloc) *>(libavutil.resolve("av_frame_alloc"));
    d_av_malloc = reinterpret_cast<decltype(av_malloc) *>(libavutil.resolve("av_malloc"));
    d_av_image_fill_arrays = reinterpret_cast<decltype(av_image_fill_arrays) *>(libavutil.resolve("av_image_fill_arrays"));
    d_av_opt_set = reinterpret_cast<decltype(av_opt_set) *>(libavutil.resolve("av_opt_set"));
    d_av_rescale_q = reinterpret_cast<decltype(av_rescale_q) *>(libavutil.resolve("av_rescale_q"));

    d_sws_getContext = reinterpret_cast<decltype(sws_getContext) *>(libswscale.resolve("sws_getContext"));
    d_sws_scale = reinterpret_cast<decltype(sws_scale) *>(libswscale.resolve("sws_scale"));

    d_avdevice_register_all();

    isLoadFunction = true;
    return true;
}
