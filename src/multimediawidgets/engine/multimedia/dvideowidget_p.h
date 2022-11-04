#ifndef DVIDEOWIDGET_P_H
#define DVIDEOWIDGET_P_H
#include "dvideowidget.h"
#include <QObject>

DMULTIMEDIA_BEGIN_NAMESPACE

class DVideoWidgetPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DVideoWidget)
public:
    explicit DVideoWidgetPrivate(DVideoWidget *parent = nullptr)
        : QObject(parent), q_ptr(parent)
    {
    }

private:
    DVideoWidget *q_ptr;
    QWidget *player = nullptr;
    DPlatformMediaPlayer *mediaPlayer = nullptr;
};

DMULTIMEDIA_END_NAMESPACE
#endif
