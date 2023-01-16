// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "dmultioperateprocess.h"
#include <QStringList>
#include <QProcess>
#include <QtDebug>
#include <QFileInfo>
#include <QDir>
#include <QEventLoop>
DMULTIMEDIA_USE_NAMESPACE

DMultiOperateProcess::DMultiOperateProcess()
{
}

QString DMultiOperateProcess::showFileInfo(const QString &fileName)
{
    if(!m_isFFmpgEnv) {
        qInfo() << "system has't install ffmpeg env!";
        return "";
    }
    QFileInfo fileinfo(fileName);
    if(!fileinfo.exists()) {
        qInfo() << "file" << fileName << " not exist!";
        return "";
    }
    QString strRst;
    QString program = "ffprobe";
    QStringList arguments;
    arguments << "-show_format"
              << "-of"
              << "json" << fileName;
    QProcess *myProcess = new QProcess(NULL);
    myProcess->start(program, arguments);
    connect(myProcess, &QProcess::readyReadStandardOutput, [=]() {
        QString strTemp = myProcess->property("rst").toString() + myProcess->readAll();
        myProcess->setProperty("rst", strTemp);
        qInfo() << myProcess->property("rst").toString();
    });
    myProcess->waitForFinished();
    myProcess->deleteLater();
    return strRst;
}

QString DMultiOperateProcess::showVideoInfo(const QString &fileName)
{
    if(!m_isFFmpgEnv) {
        qInfo() << "system has't install ffmpeg env!";
        return "";
    }
    QFileInfo fileinfo(fileName);
    if(!fileinfo.exists()) {
        qInfo() << "file" << fileName << " not exist!";
        return "";
    }
    QString strRst;
    QString program = "ffprobe";
    QStringList arguments;
    arguments << "-show_streams"
              << "-select_streams"
              << "v"
              << "-of"
              << "json" << fileName;
    QProcess *myProcess = new QProcess(NULL);
    myProcess->start(program, arguments);
    connect(myProcess, &QProcess::readyReadStandardOutput, [=]() {
        QString strTemp = myProcess->property("rst").toString() + myProcess->readAll();
        myProcess->setProperty("rst", strTemp);
        qInfo() << myProcess->property("rst").toString();
    });
    myProcess->waitForFinished();
    strRst = myProcess->property("rst").toString();
    myProcess->deleteLater();
    return strRst;
}

QString DMultiOperateProcess::showAudioInfo(const QString &fileName)
{
    if(!m_isFFmpgEnv) {
        qInfo() << "system has't install ffmpeg env!";
        return "";
    }
    QFileInfo fileinfo(fileName);
    if(!fileinfo.exists()) {
        qInfo() << "file" << fileName << " not exist!";
        return "";
    }
    QString strRst;
    QString program = "ffprobe";
    QStringList arguments;
    arguments << "-show_streams"
              << "-select_streams"
              << "a"
              << "-of"
              << "json" << fileName;
    QProcess *myProcess = new QProcess(NULL);
    myProcess->start(program, arguments);
    connect(myProcess, &QProcess::readyReadStandardOutput, [=]() {
        QString strTemp = myProcess->property("rst").toString() + myProcess->readAll();
        myProcess->setProperty("rst", strTemp);
        qInfo() << myProcess->property("rst").toString();
    });
    myProcess->waitForFinished();
    strRst = myProcess->property("rst").toString();
    myProcess->deleteLater();
    return strRst;
}

QString DMultiOperateProcess::showSubtitleInfo(const QString &fileName)
{
    if(!m_isFFmpgEnv) {
        qInfo() << "system has't install ffmpeg env!";
        return "";
    }
    QFileInfo fileinfo(fileName);
    if(!fileinfo.exists()) {
        qInfo() << "file" << fileName << " not exist!";
        return "";
    }
    QString strRst;
    QString program = "ffprobe";
    QStringList arguments;
    arguments << "-show_streams"
              << "-select_streams"
              << "s"
              << "-of"
              << "json" << fileName;
    QProcess *myProcess = new QProcess(NULL);
    myProcess->start(program, arguments);
    connect(myProcess, &QProcess::readyReadStandardOutput, [=]() {
        QString strTemp = myProcess->property("rst").toString() + myProcess->readAll();
        myProcess->setProperty("rst", strTemp);
        qInfo() << myProcess->property("rst").toString();
    });
    myProcess->waitForFinished();
    strRst = myProcess->property("rst").toString();
    myProcess->deleteLater();
    return strRst;
}

bool DMultiOperateProcess::videoConvert(const QString &fileName, const DMultiOperateInterface::videoCodecName &destCodecName, const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    if(!m_isFFmpgEnv) {
        qInfo() << "system has't install ffmpeg env!";
        return false;
    }
    QFileInfo fileinfo(fileName);
    if(!fileinfo.exists()) {
        qInfo() << "file" << fileName << " not exist!";
        return false;
    }
    bool bConvert = false, bFindMuxer = false;
    strErrDesc.clear();
    if (fileName.isEmpty() || fileName.isNull()) {
        strErrDesc = "Please enter the correct file name!";
        return bConvert;
    }
    QStringList sList = showVideoMuxList(destCodecName);
    for (QString sMuxerName : sList) {
        if (sMuxerName == nameOfMuxer(outMuxerFormat)) {
            bFindMuxer = true;
            break;
        }
    }
    if (!bFindMuxer) {
        strErrDesc = "Cannot adapt to the corresponding format " + outMuxerFormat;
        qInfo() << strErrDesc;
        return bConvert;
    }
    QFileInfo info;
    info.setFile(QDir(outFilePath()), sOutFileName + "." + nameOfMuxer(outMuxerFormat));
    QString sName = info.absoluteFilePath();
    QString program = "/usr/bin/ffmpeg";
    QStringList arguments;
    arguments << "-i"
              << fileName
              << "-c:v"
              << nameOfVideoCodec(destCodecName)
              << sName
              << "-y";
    QProcess *myProcess = new QProcess(NULL);
    myProcess->start(program, arguments);
    connect(myProcess, &QProcess::readyReadStandardError, [=]() {
        QString strTemp = myProcess->readAllStandardError();
        QString strDesc = myProcess->property("errdesc").toString();
        myProcess->setProperty("errdesc", strDesc + strTemp);
        qInfo() << strTemp;
    });
    myProcess->waitForFinished(-1);
    bConvert = (myProcess->exitCode() == 0);
    if (!bConvert) {
        QString strTemp = myProcess->property("errdesc").toString();
        int nstart = strTemp.indexOf("Invalid");
        strErrDesc = strTemp.mid(nstart);
        qInfo() << strErrDesc;
    }
    myProcess->deleteLater();
    return bConvert;
}

bool DMultiOperateProcess::audioConvert(const QString &fileName, const DMultiOperateInterface::audioCodecName &destCodecName, const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    if(!m_isFFmpgEnv) {
        qInfo() << "system has't install ffmpeg env!";
        return false;
    }
    QFileInfo fileinfo(fileName);
    if(!fileinfo.exists()) {
        qInfo() << "file" << fileName << " not exist!";
        return false;
    }
    bool bConvert = false, bFindMuxer = false;
    strErrDesc.clear();
    if (fileName.isEmpty() || fileName.isNull()) {
        strErrDesc = "Please enter the correct file name!";
        return bConvert;
    }
    QStringList sList = showAudioMuxList(destCodecName);
    for (QString sMuxerName : sList) {
        if (sMuxerName == nameOfMuxer(outMuxerFormat)) {
            bFindMuxer = true;
            break;
        }
    }
    if (!bFindMuxer) {
        strErrDesc = "Cannot adapt to the corresponding format " + outMuxerFormat;
        qInfo() << strErrDesc;
        return bConvert;
    }
    QFileInfo info;
    info.setFile(QDir(outFilePath()), sOutFileName + "." + nameOfMuxer(outMuxerFormat));
    QString sName = info.absoluteFilePath();
    QString program = "/usr/bin/ffmpeg";
    QStringList arguments;
    arguments << "-i"
              << fileName
              << "-c:a"
              << nameOfAudioCodec(destCodecName)
              << sName
              << "-y";
    QProcess *myProcess = new QProcess(NULL);
    myProcess->start(program, arguments);
    connect(myProcess, &QProcess::readyReadStandardError, [=]() {
        QString strTemp = myProcess->readAllStandardError();
        QString strDesc = myProcess->property("errdesc").toString();
        myProcess->setProperty("errdesc", strDesc + strTemp);
        qInfo() << strTemp;
    });
    myProcess->waitForFinished(-1);
    bConvert = (myProcess->exitCode() == 0);
    if (!bConvert) {
        QString strTemp = myProcess->property("errdesc").toString();
        int nstart = strTemp.indexOf("Invalid");
        strErrDesc = strTemp.mid(nstart);
        qInfo() << strErrDesc;
    }
    myProcess->deleteLater();
    return bConvert;
}

bool DMultiOperateProcess::muxerConvert(const QString &fileName, const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    if(!m_isFFmpgEnv) {
        qInfo() << "system has't install ffmpeg env!";
        return false;
    }
    QFileInfo fileinfo(fileName);
    if(!fileinfo.exists()) {
        qInfo() << "file" << fileName << " not exist!";
        return false;
    }
    bool bConvert = false;
    strErrDesc.clear();
    if (fileName.isEmpty() || fileName.isNull()) {
        strErrDesc = "Please enter the correct file name!";
        return bConvert;
    }
    QFileInfo info;
    info.setFile(QDir(outFilePath()), sOutFileName + "." + nameOfMuxer(outMuxerFormat));
    QString sName = info.absoluteFilePath();
    QString program = "/usr/bin/ffmpeg";
    QStringList arguments;
    arguments << "-i"
              << fileName
              << "-c"
              << "copy"
              << sName
              << "-y";
    QProcess *myProcess = new QProcess(NULL);
    myProcess->start(program, arguments);
    connect(myProcess, &QProcess::readyReadStandardError, [=]() {
        QString strTemp = myProcess->readAllStandardError();
        QString strDesc = myProcess->property("errdesc").toString();
        myProcess->setProperty("errdesc", strDesc + strTemp);
        qInfo() << strTemp;
    });
    myProcess->waitForFinished(-1);
    bConvert = (myProcess->exitCode() == 0);
    if (!bConvert) {
        QString strTemp = myProcess->property("errdesc").toString();
        int nstart = strTemp.indexOf("Invalid");
        strErrDesc = strTemp.mid(nstart);
        qInfo() << strErrDesc;
    }
    myProcess->deleteLater();
    return bConvert;
}

bool DMultiOperateProcess::adjustRate(const QString &fileName, const qint64 &minSize,
                                      const qint64 &maxSize, const qint64 &cacheSize,
                                      const DMultiOperateInterface::muxerName &outMuxerFormat,
                                      const QString &sOutFileName, QString &strErrDesc)
{
    if(!m_isFFmpgEnv) {
        qInfo() << "system has't install ffmpeg env!";
        return false;
    }
    QFileInfo fileinfo(fileName);
    if(!fileinfo.exists()) {
        qInfo() << "file" << fileName << " not exist!";
        return false;
    }
    bool bConvert = false;
    strErrDesc.clear();
    if (fileName.isEmpty() || fileName.isNull()) {
        strErrDesc = "Please enter the correct file name!";
        return bConvert;
    }
    QFileInfo info;
    info.setFile(QDir(outFilePath()), sOutFileName + "." + nameOfMuxer(outMuxerFormat));
    QString sName = info.absoluteFilePath();
    QString program = "/usr/bin/ffmpeg";
    QStringList arguments;
    arguments << "-i"
              << fileName
              << "-minrate"
              << QString::number(minSize) + "k"
              << "-maxrate"
              << QString::number(maxSize) + "k"
              << "-bufsize"
              << QString::number(cacheSize) + "k"
              << sName
              << "-y";
    QProcess *myProcess = new QProcess(NULL);
    myProcess->start(program, arguments);
    connect(myProcess, &QProcess::readyReadStandardError, [=]() {
        QString strTemp = myProcess->readAllStandardError();
        QString strDesc = myProcess->property("errdesc").toString();
        myProcess->setProperty("errdesc", strDesc + strTemp);
        qInfo() << strTemp;
    });
    myProcess->waitForFinished(-1);
    bConvert = (myProcess->exitCode() == 0);
    if (!bConvert) {
        QString strTemp = myProcess->property("errdesc").toString();
        int nstart = strTemp.indexOf("Invalid");
        strErrDesc = strTemp.mid(nstart);
        qInfo() << strErrDesc;
    }
    myProcess->deleteLater();
    return bConvert;
}

bool DMultiOperateProcess::changeResolution(const QString &fileName, const QSize &Resolution,
                                            const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    if(!m_isFFmpgEnv) {
        qInfo() << "system has't install ffmpeg env!";
        return false;
    }
    QFileInfo fileinfo(fileName);
    if(!fileinfo.exists()) {
        qInfo() << "file" << fileName << " not exist!";
        return false;
    }
    bool bConvert = false;
    strErrDesc.clear();
    if (fileName.isEmpty() || fileName.isNull()) {
        strErrDesc = "Please enter the correct file name!";
        return bConvert;
    }
    QFileInfo info;
    info.setFile(QDir(outFilePath()), sOutFileName + "." + nameOfMuxer(outMuxerFormat));
    QString sName = info.absoluteFilePath();
    QString program = "/usr/bin/ffmpeg";
    QStringList arguments;
    arguments << "-i"
              << fileName
              << "-vf"
              << "scale=" + QString::number(Resolution.width()) + ":" + QString::number(Resolution.height())
              << sName
              << "-y";
    QProcess *myProcess = new QProcess(NULL);
    myProcess->start(program, arguments);
    connect(myProcess, &QProcess::readyReadStandardError, [=]() {
        QString strTemp = myProcess->readAllStandardError();
        QString strDesc = myProcess->property("errdesc").toString();
        myProcess->setProperty("errdesc", strDesc + strTemp);
        qInfo() << strTemp;
    });
    myProcess->waitForFinished(-1);
    bConvert = (myProcess->exitCode() == 0);
    if (!bConvert) {
        QString strTemp = myProcess->property("errdesc").toString();
        int nstart = strTemp.indexOf("Invalid");
        strErrDesc = strTemp.mid(nstart);
        qInfo() << strErrDesc;
    }
    myProcess->deleteLater();
    return bConvert;
}

bool DMultiOperateProcess::changeResolution(const QString &fileName, const int &nWidth, const int &nHeight,
                                            const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    return changeResolution(fileName, QSize(nWidth, nHeight), outMuxerFormat, sOutFileName, strErrDesc);
}

bool DMultiOperateProcess::audioStreamFile(const QString &fileName, const DMultiOperateInterface::muxerName &outMuxerFormat,
                                           const QString &sOutFileName, QString &strErrDesc, const int audioId)
{
    if(!m_isFFmpgEnv) {
        qInfo() << "system has't install ffmpeg env!";
        return false;
    }
    QFileInfo fileinfo(fileName);
    if(!fileinfo.exists()) {
        qInfo() << "file" << fileName << " not exist!";
        return false;
    }
    bool bConvert = false;
    strErrDesc.clear();
    if (fileName.isEmpty() || fileName.isNull()) {
        strErrDesc = "Please enter the correct file name!";
        return bConvert;
    }
    QFileInfo info;
    info.setFile(QDir(outFilePath()), sOutFileName + "." + nameOfMuxer(outMuxerFormat));
    QString sName = info.absoluteFilePath();
    QString program = "/usr/bin/ffmpeg";
    QStringList arguments;
    if (audioId >= 0) {
        arguments << "-i"
                  << fileName
                  << "-vn"
                  << "-map"
                  << "0:a:" + QString::number(audioId)
                  << "-c"
                  << "copy"
                  << sName
                  << "-y";
    } else {
        arguments << "-i"
                  << fileName
                  << "-vn"
                  << "-c:a"
                  << "copy"
                  << sName
                  << "-y";
    }

    QProcess *myProcess = new QProcess(NULL);
    myProcess->start(program, arguments);
    connect(myProcess, &QProcess::readyReadStandardError, [=]() {
        QString strTemp = myProcess->readAllStandardError();
        QString strDesc = myProcess->property("errdesc").toString();
        myProcess->setProperty("errdesc", strDesc + strTemp);
        qInfo() << strTemp;
    });
    myProcess->waitForFinished(-1);
    bConvert = (myProcess->exitCode() == 0);
    if (!bConvert) {
        QString strTemp = myProcess->property("errdesc").toString();
        int nstart = strTemp.indexOf("Invalid");
        strErrDesc = strTemp.mid(nstart);
        qInfo() << strErrDesc;
    }
    myProcess->deleteLater();
    return bConvert;
}

bool DMultiOperateProcess::videoStreamFile(const QString &fileName, const DMultiOperateInterface::muxerName &outMuxerFormat,
                                           const QString &sOutFileName, QString &strErrDesc)
{
    if(!m_isFFmpgEnv) {
        qInfo() << "system has't install ffmpeg env!";
        return false;
    }
    QFileInfo fileinfo(fileName);
    if(!fileinfo.exists()) {
        qInfo() << "file" << fileName << " not exist!";
        return false;
    }
    bool bConvert = false;
    strErrDesc.clear();
    if (fileName.isEmpty() || fileName.isNull()) {
        strErrDesc = "Please enter the correct file name!";
        return bConvert;
    }
    QFileInfo info;
    info.setFile(QDir(outFilePath()), sOutFileName + "." + nameOfMuxer(outMuxerFormat));
    QString sName = info.absoluteFilePath();
    QString program = "/usr/bin/ffmpeg";
    QStringList arguments;

    arguments << "-i"
              << fileName
              << "-an"
              << "-c:v"
              << "copy"
              << sName
              << "-y";

    QProcess *myProcess = new QProcess(NULL);
    myProcess->start(program, arguments);
    connect(myProcess, &QProcess::readyReadStandardError, [=]() {
        QString strTemp = myProcess->readAllStandardError();
        QString strDesc = myProcess->property("errdesc").toString();
        myProcess->setProperty("errdesc", strDesc + strTemp);
        qInfo() << strTemp;
    });
    myProcess->waitForFinished(-1);
    bConvert = (myProcess->exitCode() == 0);
    if (!bConvert) {
        QString strTemp = myProcess->property("errdesc").toString();
        int nstart = strTemp.indexOf("Invalid");
        strErrDesc = strTemp.mid(nstart);
        qInfo() << strErrDesc;
    }
    myProcess->deleteLater();
    return bConvert;
}

bool DMultiOperateProcess::addAudioforVideo(const QString &videoName, const QString &audioName,
                                            const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    if(!m_isFFmpgEnv) {
        qInfo() << "system has't install ffmpeg env!";
        return false;
    }
    QFileInfo fileinfo(videoName), fileaudio(audioName);
    if(!fileinfo.exists()) {
        qInfo() << "file" << videoName << " not exist!";
        return false;
    }
    if(!fileaudio.exists()) {
        qInfo() << "file" << audioName << " not exist!";
        return false;
    }
    bool bConvert = false;
    strErrDesc.clear();
    if (videoName.isEmpty() || videoName.isNull() || audioName.isEmpty() || audioName.isNull()) {
        strErrDesc = "Please enter the correct file name!";
        return bConvert;
    }
    QFileInfo info;
    info.setFile(QDir(outFilePath()), sOutFileName + "." + nameOfMuxer(outMuxerFormat));
    QString sName = info.absoluteFilePath();
    QString program = "/usr/bin/ffmpeg";
    QStringList arguments;

    arguments << "-i"
              << videoName
              << "-i"
              << audioName
              << sName
              << "-y";

    QProcess *myProcess = new QProcess(NULL);
    myProcess->start(program, arguments);
    connect(myProcess, &QProcess::readyReadStandardError, [=]() {
        QString strTemp = myProcess->readAllStandardError();
        QString strDesc = myProcess->property("errdesc").toString();
        myProcess->setProperty("errdesc", strDesc + strTemp);
        qInfo() << strTemp;
    });
    myProcess->waitForFinished(-1);
    bConvert = (myProcess->exitCode() == 0);
    if (!bConvert) {
        QString strTemp = myProcess->property("errdesc").toString();
        int nstart = strTemp.indexOf("Invalid");
        strErrDesc = strTemp.mid(nstart);
        qInfo() << strErrDesc;
    }
    myProcess->deleteLater();
    return bConvert;
}

bool DMultiOperateProcess::cutVideoToDuration(const QString &fileName, const QString &startTime, const QString &durationTime,
                                              const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    if(!m_isFFmpgEnv) {
        qInfo() << "system has't install ffmpeg env!";
        return false;
    }
    QFileInfo fileinfo(fileName);
    if(!fileinfo.exists()) {
        qInfo() << "file" << fileName << " not exist!";
        return false;
    }
    bool bConvert = false;
    strErrDesc.clear();
    if (fileName.isEmpty() || fileName.isNull()) {
        strErrDesc = "Please enter the correct file name!";
        return bConvert;
    }
    QFileInfo info;
    info.setFile(QDir(outFilePath()), sOutFileName + "." + nameOfMuxer(outMuxerFormat));
    QString sName = info.absoluteFilePath();
    QString program = "/usr/bin/ffmpeg";
    QStringList arguments;

    arguments << "-i"
              << fileName
              << "-ss"
              << startTime
              << "-t"
              << durationTime
              << "-c"
              << "copy"
              << sName
              << "-y";

    QProcess *myProcess = new QProcess(NULL);
    myProcess->start(program, arguments);
    connect(myProcess, &QProcess::readyReadStandardError, [=]() {
        QString strTemp = myProcess->readAllStandardError();
        QString strDesc = myProcess->property("errdesc").toString();
        myProcess->setProperty("errdesc", strDesc + strTemp);
        qInfo() << strTemp;
    });
    myProcess->waitForFinished(-1);
    bConvert = (myProcess->exitCode() == 0);
    if (!bConvert) {
        QString strTemp = myProcess->property("errdesc").toString();
        int nstart = strTemp.indexOf("Invalid");
        strErrDesc = strTemp.mid(nstart);
        qInfo() << strErrDesc;
    }
    myProcess->deleteLater();
    return bConvert;
}

bool DMultiOperateProcess::cutVideoToEnd(const QString &fileName, const QString &startTime, const QString &endTime,
                                         const DMultiOperateInterface::muxerName &outMuxerFormat, const QString &sOutFileName, QString &strErrDesc)
{
    if(!m_isFFmpgEnv) {
        qInfo() << "system has't install ffmpeg env!";
        return false;
    }
    QFileInfo fileinfo(fileName);
    if(!fileinfo.exists()) {
        qInfo() << "file" << fileName << " not exist!";
        return false;
    }
    bool bConvert = false;
    strErrDesc.clear();
    if (fileName.isEmpty() || fileName.isNull()) {
        strErrDesc = "Please enter the correct file name!";
        return bConvert;
    }
    QFileInfo info;
    info.setFile(QDir(outFilePath()), sOutFileName + "." + nameOfMuxer(outMuxerFormat));
    QString sName = info.absoluteFilePath();
    QString program = "/usr/bin/ffmpeg";
    QStringList arguments;

    arguments << "-i"
              << fileName
              << "-ss"
              << startTime
              << "-to"
              << endTime
              << "-c"
              << "copy"
              << sName
              << "-y";

    QProcess *myProcess = new QProcess(NULL);
    myProcess->start(program, arguments);
    connect(myProcess, &QProcess::readyReadStandardError, [=]() {
        QString strTemp = myProcess->readAllStandardError();
        QString strDesc = myProcess->property("errdesc").toString();
        myProcess->setProperty("errdesc", strDesc + strTemp);
        qInfo() << strTemp;
    });
    myProcess->waitForFinished(-1);
    bConvert = (myProcess->exitCode() == 0);
    if (!bConvert) {
        QString strTemp = myProcess->property("errdesc").toString();
        int nstart = strTemp.indexOf("Invalid");
        strErrDesc = strTemp.mid(nstart);
        qInfo() << strErrDesc;
    }
    myProcess->deleteLater();
    return bConvert;
}
