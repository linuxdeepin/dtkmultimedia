// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "videostreamffmpeg.h"
#include <QLibraryInfo>
#include <QImage>
#include <QDir>

VideoStreamFfmpeg::VideoStreamFfmpeg(QObject *parent)
    : VideoStreamInterface(parent)
{
}

VideoStreamFfmpeg::~VideoStreamFfmpeg()
{
    // TODO 卸载动态库
}

void VideoStreamFfmpeg::record()
{
    // TODO 开始录屏
}

void VideoStreamFfmpeg::stop()
{
    // TODO 停止录屏
}

void VideoStreamFfmpeg::encodeWork()
{
    // TODO 编码线程
}

bool VideoStreamFfmpeg::openInputVideoCtx()
{
    // TODO 打开输入流信息，及编码
    return true;
}

bool VideoStreamFfmpeg::openOutputVideoCtx()
{
    // TODO 打开输出流信息，及编码
    return true;
}

bool VideoStreamFfmpeg::loadFunction()
{
    // TODO 加载动态链接库
    return true;
}
