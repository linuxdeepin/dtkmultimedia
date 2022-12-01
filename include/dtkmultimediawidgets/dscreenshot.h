// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSCREENSHOT_H
#define DSCREENSHOT_H

#include <QObject>
#include <dtkmultimedia.h>

DMULTIMEDIA_BEGIN_NAMESPACE
class DScreenShotPrivate;
class DScreenShot : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DScreenShot)
public:
    explicit DScreenShot(QObject *parent = nullptr);
    ~DScreenShot();

    QPixmap fullScreenShot();

    QPixmap screenShot(const QPoint point, const QSize size);

protected:
    QScopedPointer<DScreenShotPrivate> d_ptr;
};
DMULTIMEDIA_END_NAMESPACE
#endif   // DSCREENSHOT_H
