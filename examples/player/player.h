// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PLAYER_H
#define PLAYER_H

#include <QWidget>
#include <DMediaPlayer>
#ifdef BUILD_Qt6
#include <qt6player/qmediaplaylist.h>
#else
#include <DMediaPlaylist>
#endif

class QAbstractItemView;
class QLabel;
class QModelIndex;
class QPushButton;
class QSlider;
class QStatusBar;
// class QVideoProbe;
class QVideoWidget;
// class QAudioProbe;

class PlaylistModel;
DMULTIMEDIA_USE_NAMESPACE

class Player : public QWidget
{
    Q_OBJECT

public:
    explicit Player(QWidget *parent = nullptr);
    ~Player();

    bool isPlayerAvailable() const;

    void addToPlaylist(const QList<QUrl> &urls);
    void setCustomAudioRole(const QString &role);

signals:
    void fullScreenChanged(bool fullScreen);

private slots:
    void open();
    void durationChanged(qint64 duration);
    void positionChanged(qint64 progress);
    void metaDataChanged();

    void previousClicked();

    void seek(int seconds);
    void jump(const QModelIndex &index);
    void playlistPositionChanged(int);

    void statusChanged(QMediaPlayer::MediaStatus status);
    void bufferingProgress(int progress);
    void displayErrorMessage();
#ifndef BUILD_Qt6
    void showColorDialog();
#endif

private:
    void setTrackInfo(const QString &info);
    void setStatusInfo(const QString &info);
    void handleCursor(QMediaPlayer::MediaStatus status);
    void updateDurationInfo(qint64 currentInfo);

    DMediaPlayer *m_player = nullptr;
    QMediaPlaylist *m_playlist = nullptr;
    QVideoWidget *m_videoWidget = nullptr;
//    DVideoWidget *m_videoWidget = nullptr;
    QLabel *m_coverLabel = nullptr;
    QSlider *m_slider = nullptr;
    QLabel *m_labelDuration = nullptr;
    QDialog *m_colorDialog = nullptr;
    QLabel *m_statusLabel = nullptr;
    QStatusBar *m_statusBar = nullptr;
    PlaylistModel *m_playlistModel = nullptr;
    QAbstractItemView *m_playlistView = nullptr;
    QString m_trackInfo;
    QString m_statusInfo;
    qint64 m_duration;
};

#endif // PLAYER_H
