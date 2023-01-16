// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DOcr>
#include <iostream>

#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QtDebug>
#include <cstring>
#include <dlfcn.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        qWarning() << "Usage: ./ocrcmd [picture full path]";
        return 1;
    }

    // 1.Create a dtkocr object
    Dtk::Ocr::DOcr ocr;

    // 2.Load default plugin of dtkocr
    ocr.loadDefaultPlugin();

    // 3.Let plugin try to use vulkan device
    // If your computer have no vulkan device, you may see "vkCreateInstance failed -9" in stderr
    ocr.setUseHardware({{Dtk::Ocr::GPU_Vulkan, 0}});

    // 4.Set recognition language
    ocr.setLanguage("zh-Hans_en");

    // 5.Set image file
    ocr.setImageFile(argv[1]);

    // 6.Run analyze
    bool analyzeSuccess = ocr.analyze();

    // 7.Get the recognition result
    if (analyzeSuccess) {
        qInfo() << "The recognition result is:";
        qInfo() << ocr.simpleResult();
    } else {
        qInfo() << "OCR analyze have no result";
    }

    return 0;
}
