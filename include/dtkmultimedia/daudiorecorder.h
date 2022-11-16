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
    DAudioRecorder(QObject *parent = nullptr);
    ~DAudioRecorder();

    QString codec() const;
    void setCodec(const QString &codec);

    int bitRate() const;
    void setBitRate(int bitrate);

    int channelCount() const;
    void setChannelCount(int channels);

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
