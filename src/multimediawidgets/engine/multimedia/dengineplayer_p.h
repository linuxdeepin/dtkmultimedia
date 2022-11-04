#ifndef DENGINEPLAYER_P_H
#define DENGINEPLAYER_P_H
#include "dengineplayer.h"
#include <QObject>

DMULTIMEDIA_BEGIN_NAMESPACE

class DEnginePlayerPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DEnginePlayer)
public:
    explicit DEnginePlayerPrivate(DEnginePlayer *parent = nullptr)
        : QObject(parent), q_ptr(parent)
    {
    }

private:
    DEnginePlayer *q_ptr;
    QWidget *pPlayer = nullptr;
    QMediaPlayer *mediaPlayer = nullptr;
    QUrl media;
    PlayerEngine *engine = nullptr;
    DAudioOutput *audioOutput = nullptr;
    MovieInfo movieInfo;
};

DMULTIMEDIA_END_NAMESPACE
#endif
