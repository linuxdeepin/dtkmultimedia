// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DMULTIOPERATEINTER_H
#define DMULTIOPERATEINTER_H

#if BUILD_Qt6
#include <QtMultimedia/QMediaRecorder>
#else
#include <QMediaObject>
#endif

#include <dtkmultimedia.h>

DMULTIMEDIA_BEGIN_NAMESPACE
class DMultiOperateInterfacePrivate;

class Q_MULTIMEDIA_EXPORT DMultiOperateInterface : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DMultiOperateInterface)
public:
    enum videoCodecName {
        h264 = 0,
        Hevc,
        mpeg4,
        vp8,
        vp9,
        avs2
    };
    enum audioCodecName {
        aac = 0,
        mp3,
        ac_3,
        flac,
        vorbis,
        opus,
    };
    enum muxerName {
        mp4 = 0,
        mkv,
        webm,
    };

public:
    explicit DMultiOperateInterface(QObject *parent = nullptr);
    ~DMultiOperateInterface();

public:
    QStringList showVideoMuxList(const videoCodecName &name);
    QStringList showAudioMuxList(const audioCodecName &name);
    QString nameOfMuxer(const muxerName &name);
    QString nameOfVideoCodec(const videoCodecName &name);
    QString nameOfAudioCodec(const audioCodecName &name);
    void setOutFilePath(const QString &outFileName);
    QString outFilePath();
    QString showFileInfo(const QString &fileName);
    QString showVideoInfo(const QString &fileName);
    QString showAudioInfo(const QString &fileName);
    QString showSubtitleInfo(const QString &fileName);
    bool videoConvert(const QString &fileName, const videoCodecName &destCodecName,
                      const muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc);
    bool audioConvert(const QString &fileName, const audioCodecName &destCodecName,
                      const muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc);
    bool muxerConvert(const QString &fileName, const muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc);
    bool adjustRate(const QString &fileName, const qint64 &minSize, const qint64 &maxSize, const qint64 &cacheSize,
                    const muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc);
    bool changeResolution(const QString &fileName, const QSize &Resolution, const muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc);
    bool changeResolution(const QString &fileName, const int &nWidth, const int &nHeight,
                          const muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc);
    bool audioStreamFile(const QString &fileName, const muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc, const int audioId = -1);
    bool videoStreamFile(const QString &fileName, const muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc);
    bool addAudioforVideo(const QString &videoName, const QString &audioName, const muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc);

    bool cutVideoToDuration(const QString &fileName, const QString &startTime, const QString &durationTime,
                            const muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc);
    bool cutVideoToEnd(const QString &fileName, const QString &startTime, const QString &endTime,
                       const muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc);

protected:
    QScopedPointer<DMultiOperateInterfacePrivate> d_ptr;
};

DMULTIMEDIA_END_NAMESPACE

#endif   // DMULTIOPERATEINTER_H
