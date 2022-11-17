// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIORECORDER_H
#define DAUDIORECORDER_H

#include <QUrl>
#include <QAudioRecorder>
#include <dtkmultimedia.h>
DMULTIMEDIA_BEGIN_NAMESPACE

class DAudioRecorderPrivate;

class Q_MULTIMEDIA_EXPORT DAudioRecorder : public QAudioRecorder
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DAudioRecorder)

public:
    enum ACodecID {
        CODEC_ID_NO,
        CODEC_ID_AAC,
        CODEC_ID_MP3,
        CODEC_ID_AC3,
    };

    enum AChannelsID {
        CHANNELS_ID_MONO = 1,
        CHANNELS_ID_STEREO,
    };

public:
    DAudioRecorder(QObject *parent = nullptr);
    ~DAudioRecorder();

    ACodecID codec() const;
    void setCodec(const ACodecID &codec);

    int bitRate() const;
    void setBitRate(int bitrate);

    AChannelsID channelCount() const;
    void setChannelCount(AChannelsID channels);

    int sampleRate() const;
    void setSampleRate(int rate);

    QString audioInput() const;
    void setAudioInput(const QString &name);

    QUrl outputLocation() const;
    bool setOutputLocation(const QUrl &location);

    State state() const;

public Q_SLOTS:
    void record();
    void pause();
    void stop();

protected:
    QScopedPointer<DAudioRecorderPrivate> d_ptr;
};

DMULTIMEDIA_END_NAMESPACE

#endif   // DAUDIORECORDER_H
