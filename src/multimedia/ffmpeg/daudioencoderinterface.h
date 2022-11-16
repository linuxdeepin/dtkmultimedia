// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIOENCODERINTERFACE_H
#define DAUDIOENCODERINTERFACE_H

#include <dtkmultimedia.h>
#include <QMediaRecorder>
#include <QObject>

DMULTIMEDIA_BEGIN_NAMESPACE

class DAudioEncoderInterfacePrivate;

class DAudioEncoderInterface : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DAudioEncoderInterface)
public:
    explicit DAudioEncoderInterface(QObject *parent = nullptr);
    ~DAudioEncoderInterface();

public:
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

public:
    void startEncode();
    void stopEncode();
    void pauseEncode();

    QMediaRecorder::State state() const;

private:
    bool loadFunction();
    bool openInputAudioCtx();
    bool openOutputAudioCtx();
    void encodeWork();

public slots:
protected:
    QScopedPointer<DAudioEncoderInterfacePrivate> d_ptr;
};
DMULTIMEDIA_END_NAMESPACE
#endif   // DAUDIOENCODERINTERFACE_H
