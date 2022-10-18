// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DAudioOutput_H
#define DAudioOutput_H
#include <QMediaObject>

class DAudioDevice {
public:
    QString device() {return m_device;}
    void setDevice(QString device) {m_device = device;}
private:
    QString m_device;
};

class Q_MULTIMEDIA_EXPORT DAudioOutput : public QMediaObject
{
    Q_OBJECT
    Q_PROPERTY(DAudioDevice device READ device WRITE setDevice NOTIFY deviceChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)

public:
    DAudioOutput(QObject *parent = nullptr);
    ~DAudioOutput();
public:
    void  setDevice (const DAudioDevice &device);
    void  setVolume (float volume);
    void  setMuted (bool muted);
    DAudioDevice  device () const;
    float  volume () const;
    bool  isMuted () const;

signals:
    void  deviceChanged ();
    void  volumeChanged (float volume);
    void  mutedChanged (bool muted);

private:
    DAudioDevice m_audioDevice;
    bool m_bMuted = false;
    float m_fVolume = 0.f;
};


#endif
