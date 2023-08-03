// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DMEDIARECORDER_H
#define DMEDIARECORDER_H

#include <QMediaRecorder>
#include <QUrl>
#include "dmediametadata.h"
#include "dtkmultimedia.h"
#include "dmediaformat.h"

DMULTIMEDIA_BEGIN_NAMESPACE
class DCamera;
class DMediaCaptureSession;
class DMediaRecorderPrivate;

class Q_MULTIMEDIA_EXPORT DMediaRecorder : public QMediaRecorder
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DMediaRecorder)

public:
    enum Quality {
        VeryLowQuality,
        LowQuality,
        NormalQuality,
        HighQuality,
        VeryHighQuality
    };
    Q_ENUM(Quality)
    enum EncodingMode {
        ConstantQualityEncoding,
        ConstantBitRateEncoding,
        AverageBitRateEncoding,
        TwoPassEncoding
    };
    Q_ENUM(EncodingMode)
public:
#ifdef BUILD_Qt6
    explicit DMediaRecorder(QObject *parent = nullptr);
#else
    explicit DMediaRecorder(QMediaObject *parent = nullptr);
#endif
    ~DMediaRecorder();

    bool isAvailable() const;

    QUrl outputLocation() const;
    void setOutputLocation(const QUrl &location);

    QUrl actualLocation() const;

#ifdef BUILD_Qt6
    RecorderState recorderState() const;
#else
    State recorderState() const;
#endif

    Error error() const;
    QString errorString() const;

    qint64 duration() const;

    DMediaFormat mediaFormat() const;
    void setMediaFormat(const DMediaFormat &format);

    EncodingMode encodingMode() const;
    void setEncodingMode(EncodingMode);

    Quality quality() const;
    void setQuality(Quality quality);

    QSize videoResolution() const;
    void setVideoResolution(const QSize &);
    void setVideoResolution(int width, int height) { setVideoResolution(QSize(width, height)); }

    qreal videoFrameRate() const;
    void setVideoFrameRate(qreal frameRate);

    int videoBitRate() const;
    void setVideoBitRate(int bitRate);

    int audioBitRate() const;
    void setAudioBitRate(int bitRate);

    int audioChannelCount() const;
    void setAudioChannelCount(int channels);

    int audioSampleRate() const;
    void setAudioSampleRate(int sampleRate);

    DMediaMetaData metaData() const;
    void setMetaData(const DMediaMetaData &metaData);
#ifdef BUILD_Qt6
    /// qt6 removed void setMetaData(const QString &key, const QVariant &value);
#else
    void setMetaData(const QString &key, const QVariant &value);
#endif
    void addMetaData(const DMediaMetaData &metaData);

    DMediaCaptureSession *captureSession() const;
    QMediaRecorder *platformRecoder() const;

Q_SIGNALS:
#ifdef BUILD_Qt6
    void recorderStateChanged(RecorderState state);
#else
    void recorderStateChanged(State state);
#endif
    void actualLocationChanged(const QUrl &location);
    void encoderSettingsChanged();

    void errorOccurred(Error error, const QString &errorString);
    void errorChanged();

    void metaDataChanged();

    void mediaFormatChanged();
    void encodingModeChanged();
    void qualityChanged();
    void videoResolutionChanged();
    void videoFrameRateChanged();
    void videoBitRateChanged();
    void audioBitRateChanged();
    void audioChannelCountChanged();
    void audioSampleRateChanged();

public Q_SLOTS:
    void record();
    void pause();
    void stop();

protected:
    QScopedPointer<DMediaRecorderPrivate> d_ptr;
};
DMULTIMEDIA_END_NAMESPACE

#endif
