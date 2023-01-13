// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DMULTIOPERATE_H
#define DMULTIOPERATE_H

#include <QString>
#include <QSize>
#include <QObject>
#include <QMap>
#include "dmultioperateinterface.h"

DMULTIMEDIA_BEGIN_NAMESPACE
class DMultiOperate : public QObject
{
    Q_OBJECT

public:
    explicit DMultiOperate(QObject *parent = nullptr);

public:
    QStringList showVideoMuxList(const DMultiOperateInterface::videoCodecName &name);
    QStringList showAudioMuxList(const DMultiOperateInterface::audioCodecName &name);
    QString nameOfMuxer(const DMultiOperateInterface::muxerName &name);
    QString nameOfVideoCodec(const DMultiOperateInterface::videoCodecName &name);
    QString nameOfAudioCodec(const DMultiOperateInterface::audioCodecName &name);
    void setOutFilePath(const QString &outFileName);
    QString outFilePath();

public:
    virtual QString showFileInfo(const QString &fileName) = 0;
    virtual QString showVideoInfo(const QString &fileName) = 0;
    virtual QString showAudioInfo(const QString &fileName) = 0;
    virtual QString showSubtitleInfo(const QString &fileName) = 0;
    virtual bool videoConvert(const QString &fileName, const DMultiOperateInterface::videoCodecName &destCodecName,
                              const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc) = 0;
    virtual bool audioConvert(const QString &fileName, const DMultiOperateInterface::audioCodecName &destCodecName,
                              const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc) = 0;
    virtual bool muxerConvert(const QString &fileName, const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc) = 0;
    virtual bool adjustRate(const QString &fileName, const qint64 &minSize, const qint64 &maxSize, const qint64 &cacheSize,
                            const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc) = 0;
    virtual bool changeResolution(const QString &fileName, const QSize &Resolution,
                                  const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc) = 0;
    virtual bool changeResolution(const QString &fileName, const int &nWidth, const int &nHeight,
                                  const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc) = 0;
    virtual bool audioStreamFile(const QString &fileName, const DMultiOperateInterface::muxerName &outMuxerFormat,
                                 const QString &sOutFileName, QString &strErrDesc, const int audioId = -1) = 0;
    virtual bool videoStreamFile(const QString &fileName, const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc) = 0;
    virtual bool addAudioforVideo(const QString &videoName, const QString &audioName, const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc) = 0;
    virtual bool cutVideoToDuration(const QString &fileName, const QString &startTime, const QString &durationTime,
                                    const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc) = 0;
    virtual bool cutVideoToEnd(const QString &fileName, const QString &startTime, const QString &endTime,
                               const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc) = 0;
protected:
    QMap<DMultiOperateInterface::muxerName, QString> m_mapMuxer;
    QMap<DMultiOperateInterface::videoCodecName, QString> m_mapVideoName;
    QMap<DMultiOperateInterface::audioCodecName, QString> m_mapAudioName;
    QString m_strOutFileName;
    QString m_strOutFilePath;
    bool m_isFFmpgEnv = false;
};
DMULTIMEDIA_END_NAMESPACE
#endif   // DMULTIOPERATE_H
