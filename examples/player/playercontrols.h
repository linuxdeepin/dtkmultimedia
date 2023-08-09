// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PLAYERCONTROLS_H
#define PLAYERCONTROLS_H

#include <QMediaPlayer>
#include <QWidget>

class QAbstractButton;
class QAbstractSlider;
class QComboBox;

class PlayerControls : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerControls(QWidget *parent = nullptr);

#ifdef BUILD_Qt6
    QMediaPlayer::PlaybackState playbackState() const;
#else
    QMediaPlayer::State state() const;
#endif
    int volume() const;
    bool isMuted() const;
    qreal playbackRate() const;

public slots:
#ifdef BUILD_Qt6
    void setState(QMediaPlayer::PlaybackState state);
#else
    void setState(QMediaPlayer::State state);
#endif
    void setVolume(int volume);
    void setMuted(bool muted);
    void setPlaybackRate(float rate);

signals:
    void play();
    void pause();
    void stop();
    void next();
    void previous();
    void changeVolume(int volume);
    void changeMuting(bool muting);
    void changeRate(qreal rate);

private slots:
    void playClicked();
    void muteClicked();
    void updateRate();
    void onVolumeSliderValueChanged();

private:
#ifdef BUILD_Qt6
    QMediaPlayer::PlaybackState m_playerState = QMediaPlayer::StoppedState;
#else
    QMediaPlayer::State m_playerState = QMediaPlayer::StoppedState;
#endif
    bool m_playerMuted = false;
    QAbstractButton *m_playButton = nullptr;
    QAbstractButton *m_stopButton = nullptr;
    QAbstractButton *m_nextButton = nullptr;
    QAbstractButton *m_previousButton = nullptr;
    QAbstractButton *m_muteButton = nullptr;
    QAbstractSlider *m_volumeSlider = nullptr;
    QComboBox *m_rateBox = nullptr;
};

#endif // PLAYERCONTROLS_H
