// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daudioencoderinterface_p.h"

#include <QDir>
#include <QDebug>
#include <QLibrary>
#include <QLibraryInfo>
#ifdef LIBAVUTIL_VERSION_MAJOR
#define LIBAVUTIL_VER_AT_LEAST(major,minor)  (LIBAVUTIL_VERSION_MAJOR > major || \
                                              (LIBAVUTIL_VERSION_MAJOR == major && \
                                               LIBAVUTIL_VERSION_MINOR >= minor))
#else
#define LIBAVUTIL_VER_AT_LEAST(major,minor) 0
#endif
DMULTIMEDIA_USE_NAMESPACE

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

DAudioEncoderInterface::DAudioEncoderInterface(QObject *parent)
    : QObject(parent), d_ptr(new DAudioEncoderInterfacePrivate(this))
{
}

DAudioEncoderInterface::~DAudioEncoderInterface()
{
    Q_D(DAudioEncoderInterface);
    if (d->state != QMediaRecorder::StoppedState) {
        stopEncode();
    }

    if (d->isOpenInputAudioCtx) {
        d->d_avio_close(d->audioOutFormatCtx->pb);
        d->d_swr_free(&d->audioConverter);
        d->d_av_audio_fifo_free(d->audioFifo);
        d->d_avcodec_free_context(&d->audioInCodecCtx);
        d->d_avformat_close_input(&d->audioInFormatCtx);
    }

    if (d->isOpenOutputAudioCtx) {
        d->d_avcodec_free_context(&d->audioOutCodecCtx);
        d->d_avformat_free_context(d->audioOutFormatCtx);
    }

    if (d->isLoadFunction) {
        d->libavformat.unload();
        d->libavcodec.unload();
        d->libavdevice.unload();
        d->libavutil.unload();
        d->libswresample.unload();
    }
}

int DAudioEncoderInterface::bitRate() const
{
    Q_D(const DAudioEncoderInterface);
    return d->bitRate;
}

void DAudioEncoderInterface::setBitRate(int bitrate)
{
    Q_D(DAudioEncoderInterface);
    d->bitRate = bitrate;
}

DAudioRecorder::AChannelsID DAudioEncoderInterface::channelCount() const
{
    Q_D(const DAudioEncoderInterface);
    return d->channels;
}

void DAudioEncoderInterface::setChannelCount(DAudioRecorder::AChannelsID channels)
{
    Q_D(DAudioEncoderInterface);
    d->channels = channels;
}

int DAudioEncoderInterface::sampleRate() const
{
    Q_D(const DAudioEncoderInterface);
    return d->sampleRate;
}

void DAudioEncoderInterface::setSampleRate(int rate)
{
    Q_D(DAudioEncoderInterface);
    d->sampleRate = rate;
}

QString DAudioEncoderInterface::audioInput() const
{
    Q_D(const DAudioEncoderInterface);
    return d->deviceName;
}

void DAudioEncoderInterface::setAudioInput(const QString &name)
{
    Q_D(DAudioEncoderInterface);
    d->deviceName = name;
}

QUrl DAudioEncoderInterface::outputLocation() const
{
    Q_D(const DAudioEncoderInterface);
    return d->outFilePath;
}

bool DAudioEncoderInterface::setOutputLocation(const QUrl &location)
{
    Q_D(DAudioEncoderInterface);
    d->outFilePath = location;
    return true;
}

DAudioRecorder::ACodecID DAudioEncoderInterface::codec() const
{
    Q_D(const DAudioEncoderInterface);

    DAudioRecorder::ACodecID aCodec;
    switch (d->codec) {
    case AV_CODEC_ID_AAC: {
        aCodec = DAudioRecorder::CODEC_ID_AAC;
        break;
    }
    case AV_CODEC_ID_MP3: {
        aCodec = DAudioRecorder::CODEC_ID_MP3;
        break;
    }
    case AV_CODEC_ID_AC3: {
        aCodec = DAudioRecorder::CODEC_ID_AC3;
        break;
    }
    default: {
        aCodec = DAudioRecorder::CODEC_ID_NO;
    }
    }

    return aCodec;
}

void DAudioEncoderInterface::setCodec(const DAudioRecorder::ACodecID &codec)
{
    Q_D(DAudioEncoderInterface);

    switch (codec) {
    case DAudioRecorder::CODEC_ID_AAC: {
        d->codec = AV_CODEC_ID_AAC;
        break;
    }
    case DAudioRecorder::CODEC_ID_MP3: {
        d->codec = AV_CODEC_ID_MP3;
        break;
    }
    case DAudioRecorder::CODEC_ID_AC3: {
        d->codec = AV_CODEC_ID_AC3;
        break;
    }
    default: {
        d->codec = AV_CODEC_ID_NONE;
    }
    }
}

bool DAudioEncoderInterface::openInputAudioCtx()
{
    Q_D(DAudioEncoderInterface);
    if (d->isOpenInputAudioCtx) {
        return true;
    }

    AVDictionary *options = nullptr;
    int ret = 0;
#if LIBAVUTIL_VER_AT_LEAST(57,6)
    const AVInputFormat *inputFormat = d->d_av_find_input_format("pulse");
#else
    AVInputFormat *inputFormat = d->d_av_find_input_format("pulse");
#endif
    ret = d->d_avformat_open_input(&(d->audioInFormatCtx), d->deviceName.toLatin1(), inputFormat, &options);
    if (ret != 0) {
        qCritical("Couldn't open input audio stream.");
        return false;
    }

    if (d->d_avformat_find_stream_info(d->audioInFormatCtx, nullptr) < 0) {
        qCritical("Couldn't find audio stream information.");
        return false;
    }

    for (unsigned int i = 0; i < d->audioInFormatCtx->nb_streams; i++) {
        if (d->audioInFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            d->audioInStream = d->audioInFormatCtx->streams[i];
            break;
        }
    }
    if (!d->audioInStream) {
        qCritical("Couldn't find audio stream.");
        return false;
    }
#if LIBAVUTIL_VER_AT_LEAST(57,6)
    const AVCodec *audioInCodec = d->d_avcodec_find_decoder(d->audioInStream->codecpar->codec_id);
#else
    AVCodec *audioInCodec = d->d_avcodec_find_decoder(d->audioInStream->codecpar->codec_id);
#endif
    d->audioInCodecCtx = d->d_avcodec_alloc_context3(audioInCodec);
    d->d_avcodec_parameters_to_context(d->audioInCodecCtx, d->audioInStream->codecpar);
    if (d->d_avcodec_open2(d->audioInCodecCtx, audioInCodec, nullptr) < 0) {
        qCritical("Could not open video codec.");
        return false;
    }

    // audio converter, convert other fmt to requireAudioFmt
    d->audioConverter = d->d_swr_alloc_set_opts(nullptr,
                                                d->d_av_get_default_channel_layout(d->channels),
                                                AV_SAMPLE_FMT_FLTP,   // aac encoder only receive this format
                                                d->sampleRate,
                                                d->d_av_get_default_channel_layout(d->audioInCodecCtx->channels),
                                                (AVSampleFormat)d->audioInStream->codecpar->format,
                                                d->audioInStream->codecpar->sample_rate,
                                                0, nullptr);
    d->d_swr_init(d->audioConverter);
    //FIFO buffer
    d->audioFifo = d->d_av_audio_fifo_alloc(AV_SAMPLE_FMT_FLTP,
                                            d->channels,
                                            d->audioInCodecCtx->sample_rate * 2);

    d->isOpenInputAudioCtx = true;
    return true;
}

bool DAudioEncoderInterface::openOutputAudioCtx()
{
    Q_D(DAudioEncoderInterface);
    if (d->isOpenOutputAudioCtx) {
        return true;
    }

    int ret = 0;
    ret = d->d_avformat_alloc_output_context2(&(d->audioOutFormatCtx),
                                              nullptr, nullptr, d->outFilePath.toString().toLatin1());
    if (ret < 0) {
        qCritical("Failed to alloc ouput context");
        return false;
    }

    ret = d->d_avio_open(&(d->audioOutFormatCtx->pb), d->outFilePath.toString().toLatin1(), AVIO_FLAG_WRITE);
    if (ret < 0) {
        qCritical("Fail to open output file.");
        return false;
    }
#if LIBAVUTIL_VER_AT_LEAST(57,6)
    const AVCodec *audioOutCodec = d->d_avcodec_find_encoder(d->codecAdaptation());
#else
    AVCodec *audioOutCodec = d->d_avcodec_find_encoder(d->codecAdaptation());
#endif
    if (!audioOutCodec) {
        qCritical("Fail to find aac encoder. Please check your DLL.");
        return false;
    }

    d->audioOutCodecCtx = d->d_avcodec_alloc_context3(audioOutCodec);
    d->audioOutCodecCtx->channels = d->channels;
    d->audioOutCodecCtx->channel_layout = d->d_av_get_default_channel_layout(d->channels);
    d->audioOutCodecCtx->sample_rate = d->sampleRate;
    d->audioOutCodecCtx->sample_fmt = audioOutCodec->sample_fmts[0];   //for aac , there is AV_SAMPLE_FMT_FLTP =8
    d->audioOutCodecCtx->bit_rate = d->bitRateAdaptation();
    d->audioOutCodecCtx->time_base.num = 1;
    d->audioOutCodecCtx->time_base.den = d->audioOutCodecCtx->sample_rate;
    qInfo() << d->audioOutCodecCtx->channels << d->audioOutCodecCtx->channel_layout << d->audioOutCodecCtx->sample_rate << d->audioOutCodecCtx->sample_fmt << d->audioOutCodecCtx->time_base.num << d->audioOutCodecCtx->time_base.den;
    if (d->audioOutFormatCtx->oformat->flags & AVFMT_GLOBALHEADER) {
        d->audioOutCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (d->d_avcodec_open2(d->audioOutCodecCtx, audioOutCodec, nullptr) < 0) {
        qCritical("Fail to open ouput audio encoder.");
        return false;
    }

    //Add a new stream to output,should be called by the user before avformat_write_header() for muxing
    d->audioOutStream = d->d_avformat_new_stream(d->audioOutFormatCtx, audioOutCodec);
    if (d->audioOutStream == nullptr) {
        qCritical("Fail to new a audio stream.");
        return false;
    }
    d->d_avcodec_parameters_from_context(d->audioOutStream->codecpar, d->audioOutCodecCtx);

    // write file header
    if (d->d_avformat_write_header(d->audioOutFormatCtx, nullptr) < 0) {
        qCritical("Fail to write header for audio.");
        return false;
    }

    d->isOpenOutputAudioCtx = true;
    return true;
}

void DAudioEncoderInterface::startEncode()
{
    // TODO 开始录音
    Q_D(DAudioEncoderInterface);
    if (d->state == QMediaRecorder::RecordingState) {
        return;
    }

    if (d->state == QMediaRecorder::PausedState) {
        d->isPause.store(false);
        d->state = QMediaRecorder::RecordingState;
        return;
    }

    if (!d->isLoadFunction) {
        if (!loadFunction()) {
            return;
        }
    }

    if (!d->isOpenInputAudioCtx) {
        if (!openInputAudioCtx()) {
            return;
        }
    }

    if (!d->isOpenOutputAudioCtx) {
        if (!openOutputAudioCtx()) {
            return;
        }
    }

    // new thread
    d->audioRecorderThread = new std::thread(&DAudioEncoderInterface::encodeWork, this);

    d->state = QMediaRecorder::RecordingState;
}

void DAudioEncoderInterface::stopEncode()
{
    Q_D(DAudioEncoderInterface);
    if (d->state == QMediaRecorder::StoppedState) {
        return;
    }

    d->isRecording.store(false);
    d->audioRecorderThread->join();

    if (d->d_av_write_trailer(d->audioOutFormatCtx) < 0) {
        qCritical("can not write file trailer.");
    }
    d->state = QMediaRecorder::StoppedState;
}

void DAudioEncoderInterface::pauseEncode()
{
    Q_D(DAudioEncoderInterface);
    if (d->state == QMediaRecorder::PausedState) {
        return;
    }

    if (d->state == QMediaRecorder::RecordingState) {
        d->isPause.store(true);
        d->state = QMediaRecorder::PausedState;
        return;
    }
}

#if BUILD_Qt6
QMediaRecorder::RecorderState DAudioEncoderInterface::recorderState() const
{
    Q_D(const DAudioEncoderInterface);
    return d->state;
}
#else
QMediaRecorder::State DAudioEncoderInterface::state() const
{
    Q_D(const DAudioEncoderInterface);
    return d->state;
}
#endif

void DAudioEncoderInterface::encodeWork()
{
    Q_D(DAudioEncoderInterface);
    d->isRecording.store(true);
    AVFrame *inputFrame = d->d_av_frame_alloc();
    AVPacket *inputPacket = d->d_av_packet_alloc();
    AVPacket *outputPacket = d->d_av_packet_alloc();

    uint64_t frameCount = 0;
    int ret;

    while (d->isRecording.load()) {

        if (d->d_av_read_frame(d->audioInFormatCtx, inputPacket) < 0) {
            qCritical("can not read frame");
        }
        if (d->d_avcodec_send_packet(d->audioInCodecCtx, inputPacket) < 0) {
            qCritical("can not send pkt in decoding");
        }
        if (d->d_avcodec_receive_frame(d->audioInCodecCtx, inputFrame) < 0) {
            qCritical("can not receive frame in decoding");
        }

        if (d->isPause.load()) {
            continue;
        }
        // encoding
        uint8_t **cSamples = nullptr;
        ret = d->d_av_samples_alloc_array_and_samples(&cSamples, nullptr, d->audioOutCodecCtx->channels,
                                                      inputFrame->nb_samples, AV_SAMPLE_FMT_FLTP, 0);
        if (ret < 0) {
            qCritical("Fail to alloc samples by av_samples_alloc_array_and_samples.");
        }

        int outCout = inputFrame->nb_samples * d->sampleRate / d->audioInStream->codecpar->sample_rate;

        ret = d->d_swr_convert(d->audioConverter, cSamples, inputFrame->nb_samples,
                               (const uint8_t **)inputFrame->extended_data, inputFrame->nb_samples);
        if (ret < 0) {
            qCritical("Fail to swr_convert.");
        }
        if (d->d_av_audio_fifo_space(d->audioFifo) < outCout) {
            qCritical("audio buffer is too small.");
        }
        ret = d->d_av_audio_fifo_write(d->audioFifo, (void **)cSamples, outCout);
        if (ret < 0) {
            qCritical("Fail to write fifo");
        }
        d->d_av_freep(&cSamples[0]);
        d->d_av_frame_unref(inputFrame);
        d->d_av_packet_unref(inputPacket);

        while (d->d_av_audio_fifo_size(d->audioFifo) >= d->audioOutCodecCtx->frame_size) {
            AVFrame *outputFrame = d->d_av_frame_alloc();
            outputFrame->nb_samples = d->audioOutCodecCtx->frame_size;
            outputFrame->channels = d->audioInCodecCtx->channels;
            outputFrame->channel_layout = d->d_av_get_default_channel_layout(d->audioInCodecCtx->channels);
            outputFrame->format = AV_SAMPLE_FMT_FLTP;
            outputFrame->sample_rate = d->audioOutCodecCtx->sample_rate;

            ret = d->d_av_frame_get_buffer(outputFrame, 0);
            assert(ret >= 0);
            ret = d->d_av_audio_fifo_read(d->audioFifo, (void **)outputFrame->data, d->audioOutCodecCtx->frame_size);
            assert(ret >= 0);

            outputFrame->pts = frameCount * d->audioOutStream->time_base.den * 1024 / d->audioOutCodecCtx->sample_rate;

            ret = d->d_avcodec_send_frame(d->audioOutCodecCtx, outputFrame);
            if (ret < 0) {
                qCritical("Fail to send frame in encoding");
            }
            d->d_av_frame_free(&outputFrame);
            ret = d->d_avcodec_receive_packet(d->audioOutCodecCtx, outputPacket);
            if (ret == AVERROR(EAGAIN)) {
                continue;
            } else if (ret < 0) {
                qCritical("Fail to receive packet in encoding");
            }

            outputPacket->stream_index = d->audioOutStream->index;
            outputPacket->duration = d->audioOutStream->time_base.den * 1024 / d->audioOutCodecCtx->sample_rate;
            outputPacket->dts = outputPacket->pts = frameCount * d->audioOutStream->time_base.den * 1024 / d->audioOutCodecCtx->sample_rate;
            frameCount++;
            ret = d->d_av_write_frame(d->audioOutFormatCtx, outputPacket);
            d->d_av_packet_unref(outputPacket);
        }
    }

    d->d_av_packet_free(&inputPacket);
    d->d_av_packet_free(&outputPacket);
    d->d_av_frame_free(&inputFrame);
}

bool DAudioEncoderInterface::loadFunction()
{
    Q_D(DAudioEncoderInterface);
    if (d->isLoadFunction) {
        return true;
    }

    d->libavformat.setFileName(libPath("libavformat.so"));
    d->libavcodec.setFileName(libPath("libavcodec.so"));
    d->libavdevice.setFileName(libPath("libavdevice.so"));
    d->libavutil.setFileName(libPath("libavutil.so"));
    d->libswresample.setFileName(libPath("libswresample.so"));

    if (!(d->libavformat.load() && d->libavcodec.load() && d->libavdevice.load()
          && d->libavutil.load() && d->libswresample.load())) {
        qCritical("Couldn't load so.");
        return false;
    }

    d->d_avdevice_register_all = reinterpret_cast<decltype(avdevice_register_all) *>(d->libavdevice.resolve("avdevice_register_all"));

    d->d_av_find_input_format = reinterpret_cast<decltype(av_find_input_format) *>(d->libavformat.resolve("av_find_input_format"));
    d->d_avformat_open_input = reinterpret_cast<decltype(avformat_open_input) *>(d->libavformat.resolve("avformat_open_input"));
    d->d_avformat_find_stream_info = reinterpret_cast<decltype(avformat_find_stream_info) *>(d->libavformat.resolve("avformat_find_stream_info"));
    d->d_avformat_alloc_output_context2 = reinterpret_cast<decltype(avformat_alloc_output_context2) *>(d->libavformat.resolve("avformat_alloc_output_context2"));
    d->d_avio_open = reinterpret_cast<decltype(avio_open) *>(d->libavformat.resolve("avio_open"));
    d->d_avio_close = reinterpret_cast<decltype(avio_close) *>(d->libavformat.resolve("avio_close"));
    d->d_avformat_new_stream = reinterpret_cast<decltype(avformat_new_stream) *>(d->libavformat.resolve("avformat_new_stream"));
    d->d_avformat_write_header = reinterpret_cast<decltype(avformat_write_header) *>(d->libavformat.resolve("avformat_write_header"));
    d->d_av_read_frame = reinterpret_cast<decltype(av_read_frame) *>(d->libavformat.resolve("av_read_frame"));
    d->d_av_write_frame = reinterpret_cast<decltype(av_write_frame) *>(d->libavformat.resolve("av_write_frame"));
    d->d_av_write_trailer = reinterpret_cast<decltype(av_write_trailer) *>(d->libavformat.resolve("av_write_trailer"));
    d->d_avformat_close_input = reinterpret_cast<decltype(avformat_close_input) *>(d->libavformat.resolve("avformat_close_input"));
    d->d_avformat_free_context = reinterpret_cast<decltype(avformat_free_context) *>(d->libavformat.resolve("avformat_free_context"));

    d->d_avcodec_find_decoder = reinterpret_cast<decltype(avcodec_find_decoder) *>(d->libavcodec.resolve("avcodec_find_decoder"));
    d->d_avcodec_alloc_context3 = reinterpret_cast<decltype(avcodec_alloc_context3) *>(d->libavcodec.resolve("avcodec_alloc_context3"));
    d->d_avcodec_parameters_to_context = reinterpret_cast<decltype(avcodec_parameters_to_context) *>(d->libavcodec.resolve("avcodec_parameters_to_context"));
    d->d_avcodec_open2 = reinterpret_cast<decltype(avcodec_open2) *>(d->libavcodec.resolve("avcodec_open2"));
    d->d_avcodec_find_encoder = reinterpret_cast<decltype(avcodec_find_encoder) *>(d->libavcodec.resolve("avcodec_find_encoder"));
    d->d_avcodec_parameters_from_context = reinterpret_cast<decltype(avcodec_parameters_from_context) *>(d->libavcodec.resolve("avcodec_parameters_from_context"));
    d->d_av_packet_alloc = reinterpret_cast<decltype(av_packet_alloc) *>(d->libavcodec.resolve("av_packet_alloc"));
    d->d_avcodec_send_packet = reinterpret_cast<decltype(avcodec_send_packet) *>(d->libavcodec.resolve("avcodec_send_packet"));
    d->d_avcodec_receive_frame = reinterpret_cast<decltype(avcodec_receive_frame) *>(d->libavcodec.resolve("avcodec_receive_frame"));
    d->d_av_packet_unref = reinterpret_cast<decltype(av_packet_unref) *>(d->libavcodec.resolve("av_packet_unref"));
    d->d_avcodec_send_frame = reinterpret_cast<decltype(avcodec_send_frame) *>(d->libavcodec.resolve("avcodec_send_frame"));
    d->d_av_packet_free = reinterpret_cast<decltype(av_packet_free) *>(d->libavcodec.resolve("av_packet_free"));
    d->d_avcodec_receive_packet = reinterpret_cast<decltype(avcodec_receive_packet) *>(d->libavcodec.resolve("avcodec_receive_packet"));
    d->d_avcodec_free_context = reinterpret_cast<decltype(avcodec_free_context) *>(d->libavcodec.resolve("avcodec_free_context"));

    d->d_swr_alloc_set_opts = reinterpret_cast<decltype(swr_alloc_set_opts) *>(d->libswresample.resolve("swr_alloc_set_opts"));
    d->d_swr_init = reinterpret_cast<decltype(swr_init) *>(d->libswresample.resolve("swr_init"));
    d->d_swr_convert = reinterpret_cast<decltype(swr_convert) *>(d->libswresample.resolve("swr_convert"));
    d->d_swr_free = reinterpret_cast<decltype(swr_free) *>(d->libswresample.resolve("swr_free"));

    d->d_av_get_default_channel_layout = reinterpret_cast<decltype(av_get_default_channel_layout) *>(d->libavutil.resolve("av_get_default_channel_layout"));
    d->d_av_audio_fifo_alloc = reinterpret_cast<decltype(av_audio_fifo_alloc) *>(d->libavutil.resolve("av_audio_fifo_alloc"));
    d->d_av_frame_alloc = reinterpret_cast<decltype(av_frame_alloc) *>(d->libavutil.resolve("av_frame_alloc"));
    d->d_av_samples_alloc_array_and_samples = reinterpret_cast<decltype(av_samples_alloc_array_and_samples) *>(d->libavutil.resolve("av_samples_alloc_array_and_samples"));
    d->d_av_audio_fifo_space = reinterpret_cast<decltype(av_audio_fifo_space) *>(d->libavutil.resolve("av_audio_fifo_space"));
    d->d_av_audio_fifo_write = reinterpret_cast<decltype(av_audio_fifo_write) *>(d->libavutil.resolve("av_audio_fifo_write"));
    d->d_av_freep = reinterpret_cast<decltype(av_freep) *>(d->libavutil.resolve("av_freep"));
    d->d_av_frame_unref = reinterpret_cast<decltype(av_frame_unref) *>(d->libavutil.resolve("av_frame_unref"));

    d->d_av_frame_get_buffer = reinterpret_cast<decltype(av_frame_get_buffer) *>(d->libavutil.resolve("av_frame_get_buffer"));
    d->d_av_audio_fifo_read = reinterpret_cast<decltype(av_audio_fifo_read) *>(d->libavutil.resolve("av_audio_fifo_read"));
    d->d_av_frame_free = reinterpret_cast<decltype(av_frame_free) *>(d->libavutil.resolve("av_frame_free"));
    d->d_av_audio_fifo_size = reinterpret_cast<decltype(av_audio_fifo_size) *>(d->libavutil.resolve("av_audio_fifo_size"));
    d->d_av_audio_fifo_free = reinterpret_cast<decltype(av_audio_fifo_free) *>(d->libavutil.resolve("av_audio_fifo_free"));

    d->d_avdevice_register_all();

    d->isLoadFunction = true;
    return true;
}

int DAudioEncoderInterfacePrivate::bitRateAdaptation()
{
    if (bitRate != 0) {
        return bitRate;
    }

    if (codec == AV_CODEC_ID_MP3) {
        bitRate = 128000;
    } else if (codec == AV_CODEC_ID_AC3) {
        bitRate = 190000;
    } else {
        bitRate = 48000;
    }

    return bitRate;
}

AVCodecID DAudioEncoderInterfacePrivate::codecAdaptation()
{
    if (codec != AV_CODEC_ID_NONE) {
        return codec;
    }

    QString fileName = outFilePath.toString();
    if (fileName.endsWith("mp3", Qt::CaseInsensitive)) {
        codec = AV_CODEC_ID_MP3;
    } else if (fileName.endsWith("ac3", Qt::CaseInsensitive)) {
        codec = AV_CODEC_ID_AC3;
    } else {
        codec = AV_CODEC_ID_AAC;
    }
    return codec;
}
