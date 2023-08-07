// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIOINPUTDEVICE_H
#define DAUDIOINPUTDEVICE_H
#if BUILD_Qt6
#include <QMediaRecorder>
#include <QMediaCaptureSession>
#include <QAudioInput>
#else
#include <QMediaObject>
#endif

#include <dtkmultimedia.h>

DMULTIMEDIA_BEGIN_NAMESPACE
class DAudioInputPrivate;

#ifdef BUILD_Qt6
class Q_MULTIMEDIA_EXPORT DAudioInput : public QAudioInput
#else
class Q_MULTIMEDIA_EXPORT DAudioInput : public QObject
#endif
{
    Q_OBJECT
    Q_PROPERTY(QString device READ device WRITE setDevice NOTIFY deviceChanged)
    Q_PROPERTY(float volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)
    Q_DECLARE_PRIVATE(DAudioInput)

public:
#ifdef BUILD_Qt6
    explicit DAudioInput(QObject *parent = nullptr);
//    explicit DAudioInput(const QAudioDevice &deviceInfo, QObject *parent = nullptr);
#endif
    explicit DAudioInput(const QString &device, QObject *parent = nullptr);
    ~DAudioInput();

    QString device() const;
    float volume() const;
    bool isMuted() const;

public Q_SLOTS:
    void setDevice(const QString &device);
    void setVolume(float volume);
    void setMuted(bool muted);

Q_SIGNALS:
    void deviceChanged();
    void volumeChanged(float volume);
    void mutedChanged(bool muted);

protected:
    QScopedPointer<DAudioInputPrivate> d_ptr;
};

DMULTIMEDIA_END_NAMESPACE

#endif   // QAUDIOINPUTDEVICE_H
