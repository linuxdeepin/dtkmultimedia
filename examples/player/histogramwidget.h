// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include <QThread>
#include <QVideoFrame>
#include <QAudioBuffer>
#include <QWidget>

class QAudioLevel;

class FrameProcessor: public QObject
{
    Q_OBJECT

public slots:
    void processFrame(QVideoFrame frame, int levels);

signals:
    void histogramReady(const QVector<qreal> &histogram);
};

class HistogramWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HistogramWidget(QWidget *parent = nullptr);
    ~HistogramWidget();
    void setLevels(int levels) { m_levels = levels; }

public slots:
    void processFrame(const QVideoFrame &frame);
    void processBuffer(const QAudioBuffer &buffer);
    void setHistogram(const QVector<qreal> &histogram);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<qreal> m_histogram;
    int m_levels = 128;
    FrameProcessor m_processor;
    QThread m_processorThread;
    bool m_isBusy = false;
    QVector<QAudioLevel *> m_audioLevels;
};

#endif // HISTOGRAMWIDGET_H
