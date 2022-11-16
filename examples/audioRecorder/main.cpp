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
    recoder.record();
    QThread::sleep(15);
    recoder.stop();

    return 0;
}
