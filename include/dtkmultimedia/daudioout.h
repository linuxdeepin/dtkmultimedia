// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIOOUT_H
#define DAUDIOOUT_H
#if BUILD_Qt6
#include <QtMultimedia/QMediaCaptureSession>
#else
#include <QMediaObject>
#endif
#include <dtkmultimedia.h>

DMULTIMEDIA_BEGIN_NAMESPACE

class DAudioDevice
{
public:
    QString device()
    {
        return m_device;
    }
    inline void setDevice(QString device)
    {
        m_device = device;
    }

private:
    QString m_device;
};
class DAudioOutputPrivate;

#if BUILD_Qt6
class Q_MULTIMEDIA_EXPORT DAudioOutput : public QMediaCaptureSession
#else
class Q_MULTIMEDIA_EXPORT DAudioOutput : public QMediaObject
#endif
{
    Q_OBJECT
    Q_PROPERTY(DAudioDevice device READ device WRITE setDevice NOTIFY deviceChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)
    Q_DECLARE_PRIVATE(DAudioOutput)

public:
    DAudioOutput(QObject *parent = nullptr);
    ~DAudioOutput();
    void setDevice(const DAudioDevice &device);
    void setVolume(float volume);
    void setMuted(bool muted);
    DAudioDevice device() const;
    float volume() const;
    bool isMuted() const;

signals:
    void deviceChanged();
    void volumeChanged(float volume);
    void mutedChanged(bool muted);

protected:
    QScopedPointer<DAudioOutputPrivate> d_ptr;
};

DMULTIMEDIA_END_NAMESPACE
#endif
