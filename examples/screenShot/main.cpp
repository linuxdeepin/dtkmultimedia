// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include <DScreenShot>
#include <QApplication>
#include <QPixmap>

DMULTIMEDIA_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DScreenShot shot;
    shot.fullScreenShot().save("full.png");
    shot.screenShot({ 0, 0 }, { 800, 600 }).save("shot.png");
    return 0;
}
