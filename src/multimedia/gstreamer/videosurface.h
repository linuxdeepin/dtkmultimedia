// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VIDEOSURFACE_H
#define VIDEOSURFACE_H

#include <QAbstractVideoSurface>
#include <dtkmultimedia.h>

DMULTIMEDIA_BEGIN_NAMESPACE

class VideoSurface : public QAbstractVideoSurface
{
        Q_OBJECT

public:
        explicit VideoSurface(QObject *parent = Q_NULLPTR);
        ~VideoSurface();

        QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;
        bool present(const QVideoFrame &frame);

signals:
        void frameAvailable(QVideoFrame &frame);

};
DMULTIMEDIA_END_NAMESPACE
#endif // VIDEOSURFACE_H
