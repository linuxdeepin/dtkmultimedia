// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DAudioRecorder>
#include <QThread>

DMULTIMEDIA_USE_NAMESPACE

int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    DAudioRecorder recoder;
    recoder.setOutputLocation(QUrl("out.aac"));
    recoder.setBitRate(80000);
    recoder.setSampleRate(44100);
    recoder.setChannelCount(DAudioRecorder::CHANNELS_ID_MONO);
    recoder.record();
    qDebug() << recoder.state();
    QThread::sleep(10);
    recoder.stop();
    qDebug() << recoder.state();
    return 0;
}
