// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dmultioperateinterface_p.h"

DMULTIMEDIA_USE_NAMESPACE

DMultiOperateInterface::DMultiOperateInterface(QObject *parent)
    : QObject(parent), d_ptr(new DMultiOperateInterfacePrivate(this))
{
}

DMultiOperateInterface::~DMultiOperateInterface()
{
}

QStringList DMultiOperateInterface::showVideoMuxList(const DMultiOperateInterface::videoCodecName &name)
{
    //TODO
    return QStringList();
}

QStringList DMultiOperateInterface::showAideoMuxList(const DMultiOperateInterface::audioCodecName &name)
{
    //TODO
    return QStringList();
}

QString DMultiOperateInterface::nameOfMuxer(const DMultiOperateInterface::muxerName &name)
{
    //TODO
    return QString();
}

QString DMultiOperateInterface::nameOfVideoCodec(const DMultiOperateInterface::videoCodecName &name)
{
    //TODO
    return QString();
}

QString DMultiOperateInterface::nameOfAudioCodec(const DMultiOperateInterface::audioCodecName &name)
{
    //TODO
    return QString();
}

void DMultiOperateInterface::setOutFilePath(const QString &outFilePath)
{
    //TODO
}

QString DMultiOperateInterface::outFilePath()
{
    //TODO
    return QString();
}

QString DMultiOperateInterface::showFileInfo(const QString &fileName)
{
    //TODO
    return QString();
}

QString DMultiOperateInterface::showVideoInfo(const QString &fileName)
{
    //TODO
    return QString();
}

QString DMultiOperateInterface::showAudioInfo(const QString &fileName)
{
    //TODO
    return QString();
}

QString DMultiOperateInterface::showSubtitleInfo(const QString &fileName)
{
    //TODO
    return QString();
}

bool DMultiOperateInterface::videoConvert(const QString &fileName, const DMultiOperateInterface::videoCodecName &destCodecName,
                                          const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    //TODO
    return true;
}

bool DMultiOperateInterface::audioConvert(const QString &fileName, const DMultiOperateInterface::audioCodecName &destCodecName,
                                          const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    //TODO
    return true;
}

bool DMultiOperateInterface::muxerConvert(const QString &fileName, const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    //TODO
    return true;
}

bool DMultiOperateInterface::adjustRate(const QString &fileName, const qint64 &minSize, const qint64 &maxSize, const qint64 &cacheSize,
                                        const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    //TODO
    return true;
}

bool DMultiOperateInterface::changeResolution(const QString &fileName, const QSize &Resolution,
                                              const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    //TODO
    return true;
}

bool DMultiOperateInterface::changeResolution(const QString &fileName, const int &nWidth, const int &nHeight,
                                              const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    //TODO
    return true;
}

bool DMultiOperateInterface::audioStreamFile(const QString &fileName, const DMultiOperateInterface::muxerName &outMuxerFormat,
                                             const QString &sOutFileName, QString &strErrDesc, const int audioId)
{
    //TODO
    return true;
}

bool DMultiOperateInterface::videoStreamFile(const QString &fileName, const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    //TODO
    return true;
}

bool DMultiOperateInterface::addAudioforVideo(const QString &videoName, const QString &audioName, const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    //TODO
    return true;
}

bool DMultiOperateInterface::cutVideoToDuration(const QString &fileName, const QString &startTime, const QString &durationTime, const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    //TODO
    return true;
}

bool DMultiOperateInterface::cutVideoToEnd(const QString &fileName, const QString &startTime, const QString &endTime,
                                           const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    //TODO
    return true;
}
