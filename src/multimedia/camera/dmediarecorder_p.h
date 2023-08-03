// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DMEDIARECORDER_P_H
#define DMEDIARECORDER_P_H
#include <dmediarecorder.h>

class QTimer;
DMULTIMEDIA_BEGIN_NAMESPACE
class DMediaRecorderPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DMediaRecorder)
public:
    explicit DMediaRecorderPrivate(DMediaRecorder *parent = nullptr)
        : QObject(parent), q_ptr(parent)
    {
    }

private:
    DMediaRecorder *q_ptr;

private:
    DCamera *camera;
#ifdef BUILD_Qt6
    QMediaRecorder::RecorderState state = QMediaRecorder::StoppedState;
#else
    QMediaRecorder::State state = QMediaRecorder::StoppedState;
#endif
    qint64 nDuration = 0;
    QTimer *pTimer = nullptr;
    bool bAvailable = true;
    QUrl location;
    DMediaFormat mediaFormat;
    DMediaRecorder::EncodingMode enMode;
    DMediaRecorder::Quality quality;
    QSize vResolution;
    qreal vFrameRate;
    qreal vBitRate;
    qreal aBitRate;
    int aChannelCount;
    int aSampleRate;
    DMediaMetaData metaData;
    DMediaCaptureSession *mediaCapSession = nullptr;
};

DMULTIMEDIA_END_NAMESPACE

#endif   // DMEDIARECORDER_P_H
