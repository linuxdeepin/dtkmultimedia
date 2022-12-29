// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QApplication>
#include <QDesktopWidget>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow window;

    auto desktop = QApplication::desktop();
    window.move((desktop->width() - window.width()) / 2, (desktop->height() - window.height()) / 2 - 100);
    window.show();

    return a.exec();
}
