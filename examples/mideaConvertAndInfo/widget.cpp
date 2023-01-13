// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "widget.h"
#include "ui_widget.h"
#include <QFileDialog>
#include <QMessageBox>

Widget::Widget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::Widget)
{
    m_pProc = new DMultiOperateInterface;
    ui->setupUi(this);
    setWindowTitle("mideaConvertAndInfo");
}

Widget::~Widget()
{
    delete ui;
    m_pProc->deleteLater();
}

void Widget::on_pushButton_scan_clicked()
{
    const QString &file_path = QFileDialog::getOpenFileName(this);
    ui->lineEdit_fileName->setText(file_path);
}

void Widget::on_pushButton_fileInfo_clicked()
{
    QString str = ui->lineEdit_fileName->text();
    if (!str.isEmpty() && !str.isNull()) {
        m_pProc->showFileInfo(str);
    }
}

void Widget::on_pushButton_videoInfo_clicked()
{
    QString str = ui->lineEdit_fileName->text();
    if (!str.isEmpty() && !str.isNull()) {
        m_pProc->showVideoInfo(str);
    }
}

void Widget::on_pushButton_audioInfo_clicked()
{
    QString str = ui->lineEdit_fileName->text();
    if (!str.isEmpty() && !str.isNull()) {
        m_pProc->showAudioInfo(str);
    }
}

void Widget::on_pushButton_subtitle_clicked()
{
    QString str = ui->lineEdit_fileName->text();
    if (!str.isEmpty() && !str.isNull()) {
        m_pProc->showSubtitleInfo(str);
    }
}

void Widget::on_pushButton_videoConvert_clicked()
{
    QString str = ui->lineEdit_fileName->text();
    if (!str.isEmpty() && !str.isNull()) {
        m_pProc->setOutFilePath("/home/myk/Desktop/test");
        QString strDesc;
        if (m_pProc->videoConvert(str, DMultiOperateInterface::vp8, DMultiOperateInterface::webm, "test", strDesc)) {
            QMessageBox::information(this, "conv", "Convert success!");
        } else {
            QMessageBox::information(this, "conv", "Convert failed!");
        }
    }
}

void Widget::on_pushButton_audioConvert_clicked()
{
    QString str = ui->lineEdit_fileName->text();
    if (!str.isEmpty() && !str.isNull()) {
        m_pProc->setOutFilePath("/home/myk/Desktop/test");
        QString strDesc;
        if (m_pProc->audioConvert(str, DMultiOperateInterface::ac_3, DMultiOperateInterface::mkv, "test", strDesc)) {
            QMessageBox::information(this, "conv", "Convert success!");
        } else {
            QMessageBox::information(this, "conv", "Convert failed!");
        }
    }
}

void Widget::on_pushButton_muxConvert_clicked()
{
    QString str = ui->lineEdit_fileName->text();
    if (!str.isEmpty() && !str.isNull()) {
        m_pProc->setOutFilePath("/home/myk/Desktop/test");
        QString strDesc;
        if (m_pProc->muxerConvert(str, DMultiOperateInterface::mkv, "test", strDesc)) {
            QMessageBox::information(this, "conv", "Convert success!");
        } else {
            QMessageBox::information(this, "conv", "Convert failed!");
        }
    }
}

void Widget::on_pushButton_rateConvert_clicked()
{
    QString str = ui->lineEdit_fileName->text();
    if (!str.isEmpty() && !str.isNull()) {
        m_pProc->setOutFilePath("/home/myk/Desktop/test");
        QString strDesc;
        if (m_pProc->adjustRate(str, ui->lineEdit_minRate->text().toInt(), ui->lineEdit_maxRate->text().toInt(), ui->lineEdit_cacheRate->text().toInt(), DMultiOperateInterface::mp4, "test", strDesc)) {
            QMessageBox::information(this, "conv", "Convert success!");
        } else {
            QMessageBox::information(this, "conv", "Convert failed!");
        }
    }
}

void Widget::on_pushButton_resolutionConvert_clicked()
{
    QString str = ui->lineEdit_fileName->text();
    if (!str.isEmpty() && !str.isNull()) {
        m_pProc->setOutFilePath("/home/myk/Desktop/test");
        QString strDesc;
        if (m_pProc->changeResolution(str, ui->lineEdit_width->text().toInt(), ui->lineEdit_height->text().toInt(), DMultiOperateInterface::mp4, "test", strDesc)) {
            QMessageBox::information(this, "conv", "Convert success!");
        } else {
            QMessageBox::information(this, "conv", "Convert failed!");
        }
    }
}

void Widget::on_pushButton_audioStream_clicked()
{
    QString str = ui->lineEdit_fileName->text();
    if (!str.isEmpty() && !str.isNull()) {
        m_pProc->setOutFilePath("/home/myk/Desktop/test");
        QString strDesc;
        if (m_pProc->audioStreamFile(str, DMultiOperateInterface::mp4, "test", strDesc, 0)) {
            QMessageBox::information(this, "Audio stream", "Audio stream success!");
        } else {
            QMessageBox::information(this, "Audio stream", "Audio stream failed!");
        }
    }
}

void Widget::on_pushButton_videoStream_clicked()
{
    QString str = ui->lineEdit_fileName->text();
    if (!str.isEmpty() && !str.isNull()) {
        m_pProc->setOutFilePath("/home/myk/Desktop/test");
        QString strDesc;
        if (m_pProc->videoStreamFile(str, DMultiOperateInterface::mp4, "test", strDesc)) {
            QMessageBox::information(this, "Video stream", "Video stream success!");
        } else {
            QMessageBox::information(this, "Video stream", "Video stream failed!");
        }
    }
}

void Widget::on_pushButton_scanAudio_clicked()
{
    const QString &file_path = QFileDialog::getOpenFileName(this);
    ui->lineEdit_fileNameAudio->setText(file_path);
}

void Widget::on_pushButton_addAudioStream_clicked()
{
    QString str = ui->lineEdit_fileName->text();
    QString strAudio = ui->lineEdit_fileNameAudio->text();
    if (!str.isEmpty() && !str.isNull() && !strAudio.isEmpty() && !strAudio.isNull()) {
        m_pProc->setOutFilePath("/home/myk/Desktop/test");
        QString strDesc;
        if (m_pProc->addAudioforVideo(str, strAudio, DMultiOperateInterface::mp4, "test", strDesc)) {
            QMessageBox::information(this, "Add Stream", "Add stream success!");
        } else {
            QMessageBox::information(this, "Add Stream", "Add stream failed!");
        }
    }
}

void Widget::on_pushButton_cutDuration_clicked()
{
    QString str = ui->lineEdit_fileName->text();
    if (!str.isEmpty() && !str.isNull()) {
        m_pProc->setOutFilePath("/home/myk/Desktop/test");
        QString strDesc;
        if (m_pProc->cutVideoToDuration(str, ui->lineEdit_durationStart->text(), ui->lineEdit_duration->text(), DMultiOperateInterface::mp4, "test", strDesc)) {
            QMessageBox::information(this, "Cut video", "Cut video success!");
        } else {
            QMessageBox::information(this, "Cut video", "Cut video failed!");
        }
    }
}

void Widget::on_pushButton_cutEnd_clicked()
{
    QString str = ui->lineEdit_fileName->text();
    if (!str.isEmpty() && !str.isNull()) {
        m_pProc->setOutFilePath("/home/myk/Desktop/test");
        QString strDesc;
        if (m_pProc->cutVideoToEnd(str, ui->lineEdit_start->text(), ui->lineEdit_end->text(), DMultiOperateInterface::mp4, "test", strDesc)) {
            QMessageBox::information(this, "Cut video", "Cut video success!");
        } else {
            QMessageBox::information(this, "Cut video", "Cut video failed!");
        }
    }
}
