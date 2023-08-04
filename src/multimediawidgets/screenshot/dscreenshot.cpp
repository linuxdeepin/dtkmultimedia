// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dscreenshot_p.h"
#include <QScreen>
#include <QPixmap>
#include <QGuiApplication>
#include <QApplication>
#ifdef BUILD_Qt6
#include <QWindow>

#else
#include <QDesktopWidget>
#endif

DMULTIMEDIA_USE_NAMESPACE
DScreenShot::DScreenShot(QObject *parent)
    : QObject(parent), d_ptr(new DScreenShotPrivate(this))
{
}

DScreenShot::~DScreenShot()
{
}

QPixmap DScreenShot::fullScreenShot()
{
    Q_D(DScreenShot);
    return d->fullScreenShot();
}

QPixmap DScreenShot::screenShot(const QPoint point, const QSize size)
{
    Q_D(DScreenShot);
    return d->screenShot(point, size);
}

QPixmap DScreenShotPrivate::fullScreenShot()
{
    QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
#ifdef BUILD_Qt6
    return t_primaryScreen->grabWindow();

#else
    return t_primaryScreen->grabWindow(QApplication::desktop()->winId());
#endif
}

QPixmap DScreenShotPrivate::screenShot(const QPoint point, const QSize size)
{
    QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
#ifdef BUILD_Qt6
    return t_primaryScreen->grabWindow(0, point.x(), point.y(), size.width(), size.height());

#else
    return t_primaryScreen->grabWindow(QApplication::desktop()->winId(), point.x(), point.y(), size.width(), size.height());
#endif
}
