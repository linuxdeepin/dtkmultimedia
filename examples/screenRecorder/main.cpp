// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include <DScreenRecorder>
#include <QApplication>
#include <QThread>
#include <QImage>

DMULTIMEDIA_USE_NAMESPACE

void dataReceive(void *obj, int dataLength, unsigned char *data)
{
    QSize s = reinterpret_cast<DScreenRecorder *>(obj)->resolution();
    QImage p(data, s.width(), s.height(), QImage::Format::Format_RGB888);
    //static int cout = 0;
    //p.save(QString("image_%1.png").arg(cout++));
    qDebug() << dataLength << s << p.isNull();
}

class ImageReceive : public QObject
{
    Q_OBJECT
public:
    explicit ImageReceive(QObject *obj = nullptr)
        : QObject(obj) {}
    ~ImageReceive() override {}
public Q_SLOTS:
    void dataReceive(QImage img);

private:
    int cout { 0 };
};

void ImageReceive::dataReceive(QImage img)
{
    qDebug() << img.size() << __FUNCTION__;
    //img.save(QString("image_%1.png").arg(cout++));
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    DScreenRecorder recoder;
    recoder.setFrameRate(12);
    recoder.setTopLeft(100, 100);
    recoder.setResolution(600, 800);
#if 0
    recoder.setStreamAcceptFunc(dataReceive, &recoder);
    recoder.setPixfmt(DScreenRecorder::PixFormatID::PIX_FMT_RGB24);
#else
    ImageReceive obj;
    QObject::connect(&recoder, SIGNAL(screenStreamData(QImage)), &obj, SLOT(dataReceive(QImage)));
#endif
    recoder.record();
    return a.exec();
}
#include "main.moc"
