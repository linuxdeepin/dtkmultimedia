#ifndef DENGINEPLAYER_P_H
#define DENGINEPLAYER_P_H
#include "dengineplayer.h"
#include <QObject>

DMULTIMEDIA_BEGIN_NAMESPACE

class DEnginePlayerPrivate : public QObject {
    Q_OBJECT
    Q_DECLARE_PUBLIC(DEnginePlayer)
  public:
    explicit DEnginePlayerPrivate(DEnginePlayer *parent = nullptr) : QObject(parent), q_ptr(parent)
    {
    }

  private:
    DEnginePlayer *q_ptr;

  private:
    QWidget *m_pPlayer          = nullptr;
    QMediaPlayer *m_mediaPlayer = nullptr;
    QUrl m_media;
    PlayerEngine *m_engine      = nullptr;
    DAudioOutput *m_audioOutput = nullptr;
    MovieInfo m_movieInfo;
};

DMULTIMEDIA_END_NAMESPACE
#endif
