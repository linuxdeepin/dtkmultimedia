// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSCREENSHOT_P_H
#define DSCREENSHOT_P_H

#include "dscreenshot.h"
#include <QObject>

DMULTIMEDIA_BEGIN_NAMESPACE
class DScreenShotPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DScreenShot)
public:
    explicit DScreenShotPrivate(DScreenShot *parent = nullptr)
        : QObject(parent), q_ptr(parent)
    {
    }
    ~DScreenShotPrivate() {}

public:
    QPixmap fullScreenShot();
    QPixmap screenShot(const QPoint point, const QSize size);

private:
    DScreenShot *q_ptr;
};
DMULTIMEDIA_END_NAMESPACE
#endif   // DSCREENSHOT_P_H
