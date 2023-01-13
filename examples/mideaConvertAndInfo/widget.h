// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "DMultiOperateInterface"


namespace Ui {
class Widget;
}

DMULTIMEDIA_USE_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_fileInfo_clicked();

    void on_pushButton_scan_clicked();

    void on_pushButton_videoInfo_clicked();

    void on_pushButton_audioInfo_clicked();

    void on_pushButton_subtitle_clicked();

    void on_pushButton_videoConvert_clicked();

    void on_pushButton_audioConvert_clicked();

    void on_pushButton_muxConvert_clicked();

    void on_pushButton_rateConvert_clicked();

    void on_pushButton_resolutionConvert_clicked();

    void on_pushButton_audioStream_clicked();

    void on_pushButton_videoStream_clicked();

    void on_pushButton_scanAudio_clicked();

    void on_pushButton_addAudioStream_clicked();

    void on_pushButton_cutDuration_clicked();

    void on_pushButton_cutEnd_clicked();

private:
    Ui::Widget *ui;
    DMultiOperateInterface *m_pProc;
};

#endif   // WIDGET_H
