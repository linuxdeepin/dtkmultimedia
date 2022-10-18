// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QABSTRACTVIDEOSINK_H
#define QABSTRACTVIDEOSINK_H

#include <QtMultimedia/qtmultimediaglobal.h>
#include <QtCore/qobject.h>
#include <QtGui/qwindowdefs.h>

QT_BEGIN_NAMESPACE

class QRectF;
class QVideoFrameFormat;
class QVideoFrame;

class DVideoSinkPrivate;
class QPlatformVideoSink;
class QRhi;

class Q_MULTIMEDIA_EXPORT DVideoSink : public QObject
{
    Q_OBJECT
public:
    DVideoSink(QObject *parent = nullptr);
    ~DVideoSink();

    QRhi *rhi() const;
    void setRhi(QRhi *rhi);

    QSize videoSize() const;

    QString subtitleText() const;
    void setSubtitleText(const QString &subtitle);

    void setVideoFrame(const QVideoFrame &frame);
    QVideoFrame videoFrame() const;

    QPlatformVideoSink *platformVideoSink() const;
Q_SIGNALS:
    void videoFrameChanged(const QVideoFrame &frame) const;
    void subtitleTextChanged(const QString &subtitleText) const;

    void videoSizeChanged();

private:
    friend class QMediaPlayerPrivate;
    friend class QMediaCaptureSessionPrivate;
    void setSource(QObject *source);

    DVideoSinkPrivate *d = nullptr;
};

QT_END_NAMESPACE

#endif
