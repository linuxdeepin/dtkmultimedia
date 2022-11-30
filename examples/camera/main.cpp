// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "camera.h"

#include <QtWidgets>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Camera camera;
    camera.show();
    //    camera.dcamInit(argc, argv);

    return app.exec();
};
