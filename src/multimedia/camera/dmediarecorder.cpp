// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dmediarecorder_p.h"
#include <QTimer>
#include "dcamera.h"
extern "C" {
#include "camview.h"
}

DMULTIMEDIA_USE_NAMESPACE

#ifdef BUILD_Qt6
DMediaRecorder::DMediaRecorder(QObject *parent)
#else
DMediaRecorder::DMediaRecorder(QMediaObject *parent)
#endif
    : QMediaRecorder(parent), d_ptr(new DMediaRecorderPrivate(this))
{
    Q_D(DMediaRecorder);
    d->camera = qobject_cast<DCamera *>(parent);
    if (d->camera->isFfmpegEnv()) {
        d->pTimer = new QTimer;
        connect(d->pTimer, &QTimer::timeout, [=] {
            if (d->state == QMediaRecorder::RecordingState) {
                int duration = round(get_video_time_capture() * 1000);
                if (d->nDuration != duration) {
                    d->nDuration = duration;
                    emit durationChanged(d->nDuration);
                }
            }
        });
        d->pTimer->start(1000);
    }
}

DMediaRecorder::~DMediaRecorder()
{
}

bool DMediaRecorder::isAvailable() const
{
    Q_D(const DMediaRecorder);
    return d->bAvailable;
}

QUrl DMediaRecorder::outputLocation() const
{
    Q_D(const DMediaRecorder);
    return d->location;
}

void DMediaRecorder::setOutputLocation(const QUrl &location)
{
    Q_D(DMediaRecorder);
    if (location.toLocalFile() != d->location.toLocalFile()) {
        d->location = location;
        emit actualLocationChanged(d->location);
    }
}

QUrl DMediaRecorder::actualLocation() const
{
    Q_D(const DMediaRecorder);
    return d->location;
}

#ifdef BUILD_Qt6
QMediaRecorder::RecorderState DMediaRecorder::recorderState() const
{
    Q_D(const DMediaRecorder);
    return d->state;
}
#else
QMediaRecorder::State DMediaRecorder::recorderState() const
{
    Q_D(const DMediaRecorder);
    return d->state;
}
#endif

QMediaRecorder::Error DMediaRecorder::error() const
{
    return QMediaRecorder::error();
}

QString DMediaRecorder::errorString() const
{
    return QMediaRecorder::errorString();
}

qint64 DMediaRecorder::duration() const
{
    Q_D(const DMediaRecorder);
    if (d->camera->isFfmpegEnv()) {
        if (d->state == QMediaRecorder::RecordingState) {
            return d->nDuration;
        } else {
            return 0;
        }
    } else {
        return QMediaRecorder::duration();
    }
}

DMediaFormat DMediaRecorder::mediaFormat() const
{
    Q_D(const DMediaRecorder);
    return d->mediaFormat;
}

void DMediaRecorder::setMediaFormat(const DMediaFormat &format)
{
    Q_D(DMediaRecorder);
    if (d->mediaFormat != format) {
        d->mediaFormat = format;
        emit mediaFormatChanged();
    }
}

DMediaRecorder::EncodingMode DMediaRecorder::encodingMode() const
{
    Q_D(const DMediaRecorder);
    return d->enMode;
}

void DMediaRecorder::setEncodingMode(DMediaRecorder::EncodingMode mode)
{
    Q_D(DMediaRecorder);
    if (d->enMode != mode) {
        d->enMode = mode;
        emit encodingModeChanged();
    }
}

DMediaRecorder::Quality DMediaRecorder::quality() const
{
    Q_D(const DMediaRecorder);
    return d->quality;
}

void DMediaRecorder::setQuality(DMediaRecorder::Quality quality)
{
    Q_D(DMediaRecorder);
    if (d->quality != quality) {
        d->quality = quality;
        emit qualityChanged();
    }
}

QSize DMediaRecorder::videoResolution() const
{
    Q_D(const DMediaRecorder);
    return d->vResolution;
}

void DMediaRecorder::setVideoResolution(const QSize &size)
{
    Q_D(DMediaRecorder);
    if (d->vResolution != size) {
        d->vResolution = size;
        emit videoResolutionChanged();
    }
}

qreal DMediaRecorder::videoFrameRate() const
{
    Q_D(const DMediaRecorder);
    return d->vFrameRate;
}

void DMediaRecorder::setVideoFrameRate(qreal frameRate)
{
    Q_D(DMediaRecorder);
    if (d->vFrameRate != frameRate) {
        d->vFrameRate = frameRate;
        emit videoFrameRateChanged();
    }
}

int DMediaRecorder::videoBitRate() const
{
    Q_D(const DMediaRecorder);
    return d->vBitRate;
}

void DMediaRecorder::setVideoBitRate(int bitRate)
{
    Q_D(DMediaRecorder);
    if (d->vBitRate != bitRate) {
        d->vBitRate = bitRate;
        emit videoBitRateChanged();
    }
}

int DMediaRecorder::audioBitRate() const
{
    Q_D(const DMediaRecorder);
    return d->aBitRate;
}

void DMediaRecorder::setAudioBitRate(int bitRate)
{
    Q_D(DMediaRecorder);
    if (d->aBitRate != bitRate) {
        d->aBitRate = bitRate;
        emit audioBitRateChanged();
    }
}

int DMediaRecorder::audioChannelCount() const
{
    Q_D(const DMediaRecorder);
    return d->aChannelCount;
}

void DMediaRecorder::setAudioChannelCount(int channels)
{
    Q_D(DMediaRecorder);
    if (d->aChannelCount != channels) {
        d->aChannelCount = channels;
        emit audioChannelCountChanged();
    }
}

int DMediaRecorder::audioSampleRate() const
{
    Q_D(const DMediaRecorder);
    return d->aSampleRate;
}

void DMediaRecorder::setAudioSampleRate(int sampleRate)
{
    Q_D(DMediaRecorder);
    if (d->aSampleRate != sampleRate) {
        d->aSampleRate = sampleRate;
        emit audioSampleRateChanged();
    }
}

DMediaMetaData DMediaRecorder::metaData() const
{
    Q_D(const DMediaRecorder);
    return d->metaData;
}

void DMediaRecorder::setMetaData(const DMediaMetaData &metaData)
{
    Q_D(DMediaRecorder);
    if (d->metaData != metaData) {
        d->metaData = metaData;
        emit metaDataChanged();
    }
}

#ifdef BUILD_Qt6

#else
void DMediaRecorder::setMetaData(const QString &key, const QVariant &value)
{
    QMediaRecorder::setMetaData(key, value);
}
#endif

void DMediaRecorder::addMetaData(const DMediaMetaData &metaData)
{
    Q_D(DMediaRecorder);
    for (DMediaMetaData::Key key : metaData.keys()) {
        d->metaData[key] = metaData.value(key);
    }
    if (!metaData.isEmpty()) {
        emit metaDataChanged();
    }
}

DMediaCaptureSession *DMediaRecorder::captureSession() const
{
    Q_D(const DMediaRecorder);
    return d->camera->captureSession();
}

QMediaRecorder *DMediaRecorder::platformRecoder() const
{
    //todo
    return nullptr;
}

void DMediaRecorder::record()
{
    Q_D(DMediaRecorder);
    if (d->camera->isFfmpegEnv()) {
        if (d->state == QMediaRecorder::RecordingState) return;
        d->camera->takeVideo(d->location.toLocalFile());
        d->state = QMediaRecorder::RecordingState;
#ifdef BUILD_Qt6
        emit recorderStateChanged(d->state);
#else
        emit stateChanged(d->state);
#endif
        d->pTimer->start();
    } else {
        QMediaRecorder::record();
    }
}

void DMediaRecorder::pause()
{
    Q_D(DMediaRecorder);
    if (d->camera->isFfmpegEnv()) {

    } else {
        QMediaRecorder::pause();
    }
}

void DMediaRecorder::stop()
{
    Q_D(DMediaRecorder);
    if (d->camera->isFfmpegEnv()) {
        if (d->state != QMediaRecorder::RecordingState) return;
        d->camera->takeVideo(d->location.toLocalFile());
        d->state = QMediaRecorder::StoppedState;
#ifdef BUILD_Qt6
        emit recorderStateChanged(d->state);
#else
        emit stateChanged(d->state);
#endif
        d->nDuration = 0;
        d->pTimer->stop();
    } else {
        QMediaRecorder::stop();
    }
}
