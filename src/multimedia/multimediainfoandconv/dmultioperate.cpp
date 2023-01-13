// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "dmultioperate.h"
#include <QFileInfo>
DMULTIMEDIA_USE_NAMESPACE

DMultiOperate::DMultiOperate(QObject *parent)
{
    QFileInfo info("/usr/bin/ffmpeg");
    m_isFFmpgEnv = info.exists();
    m_mapMuxer.insert(DMultiOperateInterface::mp4, "mp4");
    m_mapMuxer.insert(DMultiOperateInterface::mkv, "mkv");
    m_mapMuxer.insert(DMultiOperateInterface::webm, "webm");

    m_mapVideoName.insert(DMultiOperateInterface::h264, "h264");
    m_mapVideoName.insert(DMultiOperateInterface::Hevc, "Hevc");
    m_mapVideoName.insert(DMultiOperateInterface::mpeg4, "mpeg4");
    m_mapVideoName.insert(DMultiOperateInterface::vp8, "vp8");
    m_mapVideoName.insert(DMultiOperateInterface::vp9, "vp9");
    m_mapVideoName.insert(DMultiOperateInterface::avs2, "avs2");

    m_mapAudioName.insert(DMultiOperateInterface::aac, "aac");
    m_mapAudioName.insert(DMultiOperateInterface::mp3, "mp3");
    m_mapAudioName.insert(DMultiOperateInterface::ac_3, "ac3");
    m_mapAudioName.insert(DMultiOperateInterface::flac, "flac");
    m_mapAudioName.insert(DMultiOperateInterface::vorbis, "libvorbis");
    m_mapAudioName.insert(DMultiOperateInterface::opus, "libopus");
}

QStringList DMultiOperate::showVideoMuxList(const DMultiOperateInterface::videoCodecName &name)
{
    QStringList sList;

    switch (name) {
    case DMultiOperateInterface::h264: {
        sList.append("mp4");
        sList.append("mkv");
        break;
    }
    case DMultiOperateInterface::Hevc: {
        sList.append("mp4");
        sList.append("mkv");
        break;
    }
    case DMultiOperateInterface::mpeg4: {
        sList.append("mp4");
        sList.append("mkv");
        break;
    }
    case DMultiOperateInterface::vp8: {
        sList.append("webm");
        sList.append("mkv");
        break;
    }
    case DMultiOperateInterface::vp9: {
        sList.append("webm");
        sList.append("mkv");
        break;
    }
    case DMultiOperateInterface::avs2: {
        sList.append("mkv");
        break;
    }
    }
    return sList;
}

QStringList DMultiOperate::showAudioMuxList(const DMultiOperateInterface::audioCodecName &name)
{
    QStringList sList;

    switch (name) {
    case DMultiOperateInterface::aac: {
        sList.append("mp4");
        sList.append("mkv");
        break;
    }
    case DMultiOperateInterface::mp3: {
        sList.append("mp4");
        sList.append("mkv");
        break;
    }
    case DMultiOperateInterface::ac_3: {
        sList.append("mp4");
        sList.append("mkv");
        break;
    }
    case DMultiOperateInterface::flac: {
        sList.append("mp4");
        sList.append("mkv");
        break;
    }
    case DMultiOperateInterface::vorbis: {
        sList.append("mp4");
        sList.append("webm");
        sList.append("mkv");
        break;
    }
    case DMultiOperateInterface::opus: {
        sList.append("mp4");
        sList.append("webm");
        sList.append("mkv");
        break;
    }
    }
    return sList;
}

QString DMultiOperate::nameOfMuxer(const DMultiOperateInterface::muxerName &name)
{
    return m_mapMuxer.value(name);
}

QString DMultiOperate::nameOfVideoCodec(const DMultiOperateInterface::videoCodecName &name)
{
    return m_mapVideoName.value(name);
}

QString DMultiOperate::nameOfAudioCodec(const DMultiOperateInterface::audioCodecName &name)
{
    return m_mapAudioName.value(name);
}

void DMultiOperate::setOutFilePath(const QString &outFilePath)
{
    m_strOutFilePath = outFilePath;
}

QString DMultiOperate::outFilePath()
{
    return m_strOutFilePath;
}
