// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class QPushButton;
class QLineEdit;
class QTextEdit;
class QComboBox;

namespace Dtk {
namespace Ocr {
    class DOcr;
}
}  // namespace Dtk

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private:
    void initOcrEngine();
    void initUI();
    void initConnect();
    void runOcrAnalyze(const QString &path);
    void flushLabel();
    void switchBox(int index);

    // image
    QLabel *imageLabel;
    QLineEdit *pathEdit;
    QPushButton *fileDialogButton;
    QPushButton *openImageButton;
    QString pathCached;

    // rect
    QComboBox *languageBox;
    QPushButton *preButton;
    QPushButton *nextButton;
    QPushButton *showCharRectButton;

    // text
    QTextEdit *rectTextEdit;
    QTextEdit *simpleTextEdit;

    // engine
    Dtk::Ocr::DOcr *ocrEngine;
    int currentRectIndex = -1;
    bool needDrawCharRect = false;
};

#endif
