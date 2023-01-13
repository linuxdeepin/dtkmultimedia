// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dmultioperateinterface_p.h"
#include "dmultioperateprocess.h"

DMULTIMEDIA_USE_NAMESPACE

DMultiOperateInterface::DMultiOperateInterface(QObject *parent)
    : QObject(parent), d_ptr(new DMultiOperateInterfacePrivate(this))
{
    Q_D(DMultiOperateInterface);
    d->videoInterface = new DMultiOperateProcess;
}

DMultiOperateInterface::~DMultiOperateInterface()
{
    Q_D(DMultiOperateInterface);
    if(d->videoInterface) {
        delete d->videoInterface;
        d->videoInterface = nullptr;
    }
}

QStringList DMultiOperateInterface::showVideoMuxList(const DMultiOperateInterface::videoCodecName &name)
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->showVideoMuxList(name);
}

QStringList DMultiOperateInterface::showAudioMuxList(const DMultiOperateInterface::audioCodecName &name)
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->showAudioMuxList(name);
}

QString DMultiOperateInterface::nameOfMuxer(const DMultiOperateInterface::muxerName &name)
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->nameOfMuxer(name);
}

QString DMultiOperateInterface::nameOfVideoCodec(const DMultiOperateInterface::videoCodecName &name)
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->nameOfVideoCodec(name);
}

QString DMultiOperateInterface::nameOfAudioCodec(const DMultiOperateInterface::audioCodecName &name)
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->nameOfAudioCodec(name);
}

void DMultiOperateInterface::setOutFilePath(const QString &outFilePath)
{
    Q_D(DMultiOperateInterface);
    d->videoInterface->setOutFilePath(outFilePath);
}

QString DMultiOperateInterface::outFilePath()
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->outFilePath();
}

QString DMultiOperateInterface::showFileInfo(const QString &fileName)
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->showFileInfo(fileName);
}

QString DMultiOperateInterface::showVideoInfo(const QString &fileName)
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->showVideoInfo(fileName);
}

QString DMultiOperateInterface::showAudioInfo(const QString &fileName)
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->showAudioInfo(fileName);
}

QString DMultiOperateInterface::showSubtitleInfo(const QString &fileName)
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->showSubtitleInfo(fileName);
}

bool DMultiOperateInterface::videoConvert(const QString &fileName, const DMultiOperateInterface::videoCodecName &destCodecName,
                                          const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->videoConvert(fileName, destCodecName, outMuxerFormat, sOutFileName, strErrDesc);
}

bool DMultiOperateInterface::audioConvert(const QString &fileName, const DMultiOperateInterface::audioCodecName &destCodecName,
                                          const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->audioConvert(fileName, destCodecName, outMuxerFormat, sOutFileName, strErrDesc);
}

bool DMultiOperateInterface::muxerConvert(const QString &fileName, const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->muxerConvert(fileName, outMuxerFormat, sOutFileName, strErrDesc);
}

bool DMultiOperateInterface::adjustRate(const QString &fileName, const qint64 &minSize, const qint64 &maxSize, const qint64 &cacheSize,
                                        const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->adjustRate(fileName, minSize, maxSize, cacheSize, outMuxerFormat, sOutFileName, strErrDesc);
}

bool DMultiOperateInterface::changeResolution(const QString &fileName, const QSize &Resolution,
                                              const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->changeResolution(fileName, Resolution, outMuxerFormat, sOutFileName, strErrDesc);
}

bool DMultiOperateInterface::changeResolution(const QString &fileName, const int &nWidth, const int &nHeight,
                                              const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->changeResolution(fileName, nWidth, nHeight, outMuxerFormat, sOutFileName, strErrDesc);
}

bool DMultiOperateInterface::audioStreamFile(const QString &fileName, const DMultiOperateInterface::muxerName &outMuxerFormat,
                                             const QString &sOutFileName, QString &strErrDesc, const int audioId)
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->audioStreamFile(fileName, outMuxerFormat, sOutFileName, strErrDesc, audioId);;
}

bool DMultiOperateInterface::videoStreamFile(const QString &fileName, const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->videoStreamFile(fileName, outMuxerFormat, sOutFileName, strErrDesc);;
}

bool DMultiOperateInterface::addAudioforVideo(const QString &videoName, const QString &audioName, const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->addAudioforVideo(videoName, audioName, outMuxerFormat, sOutFileName, strErrDesc);
}

bool DMultiOperateInterface::cutVideoToDuration(const QString &fileName, const QString &startTime, const QString &durationTime, const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->cutVideoToDuration(fileName, startTime, durationTime, outMuxerFormat, sOutFileName, strErrDesc);;
}

bool DMultiOperateInterface::cutVideoToEnd(const QString &fileName, const QString &startTime, const QString &endTime,
                                           const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    Q_D(DMultiOperateInterface);
    return d->videoInterface->cutVideoToEnd(fileName, startTime, endTime, outMuxerFormat, sOutFileName, strErrDesc);;
}
