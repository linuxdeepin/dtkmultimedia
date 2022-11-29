// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIMAGECAPTURE_H
#define DIMAGECAPTURE_H

#include <QCameraImageCapture>
#include "dtkmultimedia.h"
#include "dmediametadata.h"

DMULTIMEDIA_BEGIN_NAMESPACE
class DCamera;
class DImageCapturePrivate;
class DMediaCaptureSession;

class Q_MULTIMEDIA_EXPORT DImageCapture : public QCameraImageCapture
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DImageCapture)

public:
    enum Quality {
        VeryLowQuality,
        LowQuality,
        NormalQuality,
        HighQuality,
        VeryHighQuality
    };
    Q_ENUM(Quality)

    enum FileFormat {
        UnspecifiedFormat,
        JPEG,
        PNG,
        WebP,
        Tiff,
        LastFileFormat = Tiff
    };
    Q_ENUM(FileFormat)
public:
    explicit DImageCapture(QMediaObject *parent = nullptr);
    ~DImageCapture();

    bool isAvailable() const;

    DMediaCaptureSession *captureSession() const;

    Error error() const;
    QString errorString() const;

    bool isReadyForCapture() const;

    FileFormat fileFormat() const;
    void setFileFormat(FileFormat format);

    static QList<FileFormat> supportedFormats();
    static QString fileFormatName(FileFormat f);
    static QString fileFormatDescription(FileFormat f);

    QSize resolution() const;
    void setResolution(const QSize &);
    void setResolution(int width, int height) { setResolution(QSize(width, height)); }

    Quality quality() const;
    void setQuality(Quality quality);

    DMediaMetaData metaData() const;
    void setMetaData(const DMediaMetaData &metaData);
    void addMetaData(const DMediaMetaData &metaData);

public Q_SLOTS:
    int captureToFile(const QString &location = QString());
    int capture(const QString &location = QString());

Q_SIGNALS:
    void errorChanged();
    void errorOccurred(int id, DImageCapture::Error error, const QString &errorString);

    void readyForCaptureChanged(bool ready);
    void metaDataChanged();

    void fileFormatChanged();
    void qualityChanged();
    void resolutionChanged();

protected:
    QScopedPointer<DImageCapturePrivate> d_ptr;
};
DMULTIMEDIA_END_NAMESPACE

#endif
