// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QApplication>
#ifdef BUILD_Qt6
#include <QScreen>
#else
#include <QDesktopWidget>
#endif

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow window;

#ifdef BUILD_Qt6
    auto screen = QGuiApplication::primaryScreen();
    window.move((screen->availableSize().width() - window.width()) / 2,
                (screen->availableSize().height() - window.height()) / 2 - 100);
#else
    auto desktop = QApplication::desktop();
    window.move((desktop->width() - window.width()) / 2, (desktop->height() - window.height()) / 2 - 100);
#endif

    window.show();

    return a.exec();
}
