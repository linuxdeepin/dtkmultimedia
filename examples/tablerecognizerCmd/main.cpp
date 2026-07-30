// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <dtablerecognizer.h>

#include <QCoreApplication>
#include <QFile>
#include <QImage>
#include <QtDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    if (argc < 2) {
        qWarning() << "Usage: ./tablerecognizercmd <table-image>";
        return 1;
    }

    QImage image(QString::fromUtf8(argv[1]));
    if (image.isNull()) {
        qWarning() << "Failed to load image:" << argv[1];
        return 1;
    }

    Dtk::TableRecognizer::DTableRecognizer recognizer;

    QObject::connect(&recognizer, &Dtk::TableRecognizer::DTableRecognizer::recognitionDone,
                     [](const Dtk::TableRecognizer::DTableResult &result) {
        if (result.success) {
            qInfo() << "Recognized via" << result.source;
            qInfo() << "Cells:" << result.cells.size();
            qInfo() << "HTML:\n" << result.html;
        } else {
            qWarning() << "Recognition failed:" << result.errorMessage;
        }
        QCoreApplication::quit();
    });

    recognizer.recognizeAsync(image);
    return app.exec();
}
