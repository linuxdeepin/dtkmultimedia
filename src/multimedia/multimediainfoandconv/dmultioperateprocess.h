// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DMULTIOPERATEPROCESS_H
#define DMULTIOPERATEPROCESS_H

#include "dmultioperate.h"
DMULTIMEDIA_BEGIN_NAMESPACE
class DMultiOperateProcess : public DMultiOperate
{
    Q_OBJECT

public:
    explicit DMultiOperateProcess();

public:
    virtual QString showFileInfo(const QString &fileName);
    virtual QString showVideoInfo(const QString &fileName);
    virtual QString showAudioInfo(const QString &fileName);
    virtual QString showSubtitleInfo(const QString &fileName);
    virtual bool videoConvert(const QString &fileName, const DMultiOperateInterface::videoCodecName &destCodecName,
                              const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc);
    virtual bool audioConvert(const QString &fileName, const DMultiOperateInterface::audioCodecName &destCodecName,
                              const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc);
    virtual bool muxerConvert(const QString &fileName, const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc);
    virtual bool adjustRate(const QString &fileName, const qint64 &minSize, const qint64 &maxSize, const qint64 &cacheSize,
                            const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc);
    virtual bool changeResolution(const QString &fileName, const QSize &Resolution, const DMultiOperateInterface::muxerName &outMuxerFormat,
                                  const QString &sOutFileName, QString &strErrDesc);
    virtual bool changeResolution(const QString &fileName, const int &nWidth, const int &nHeight, const DMultiOperateInterface::muxerName &outMuxerFormat,
                                  const QString &sOutFileName, QString &strErrDesc);
    virtual bool audioStreamFile(const QString &fileName, const DMultiOperateInterface::muxerName &outMuxerFormat,
                                 const QString &sOutFileName, QString &strErrDesc, const int audioId = -1);
    virtual bool videoStreamFile(const QString &fileName, const DMultiOperateInterface::muxerName &outMuxerFormat,
                                 const QString &sOutFileName, QString &strErrDesc);
    virtual bool addAudioforVideo(const QString &videoName, const QString &audioName, const DMultiOperateInterface::muxerName &outMuxerFormat,
                                  const QString &sOutFileName, QString &strErrDesc);
    virtual bool cutVideoToDuration(const QString &fileName, const QString &startTime, const QString &durationTime,
                                    const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc);
    virtual bool cutVideoToEnd(const QString &fileName, const QString &startTime, const QString &endTime,
                               const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc);
};
DMULTIMEDIA_END_NAMESPACE
#endif   // DMULTIOPERATEPROCESS_H
