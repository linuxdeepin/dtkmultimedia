// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DMPVPROXY_H
#define DMPVPROXY_H

#define MWV206_0

#include <dplayerbackend.h>
#undef Bool

class mpv_handle;

DMULTIMEDIA_BEGIN_NAMESPACE
class DMpvGLWidget;
class PlayingMovieInfo;

class MpvHandle
{
    struct container
    {
    public:
        explicit container(mpv_handle *pHandle);
        ~container();
        mpv_handle *m_pHandle;
    };
    QSharedPointer<container> sptr;

public:
    static MpvHandle fromRawHandle(mpv_handle *pHandle);
    operator mpv_handle *() const;
};

enum DecodeMode { AUTO = 0,
                  HARDWARE,
                  SOFTWARE };

class DMpvProxyPrivate;
class DMpvProxy : public DPlayerBackend
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DMpvProxy)

Q_SIGNALS:
    void has_mpv_events();
    void crashCheck();
    void notifyCreateOpenGL(MpvHandle handle);

public:
    MpvHandle getMpvHandle();
    explicit DMpvProxy(QObject *parent = 0);
    virtual ~DMpvProxy();
    void setDecodeModel(const QVariant &value);
    void refreshDecode();
    void initMpvFuns();
    void firstInit();
    void initSetting();
    const PlayingMovieInfo &playingMovieInfo() override;
    bool isPlayable() const override;
    void pollingEndOfPlayback();
    qint64 duration() const override;
    qint64 elapsed() const override;
    QSize videoSize() const override;
    void setPlaySpeed(double dTimes) override;
    void savePlaybackPosition() override;
    bool loadSubtitle(const QFileInfo &fileInfo) override;
    void toggleSubtitle() override;
    bool isSubVisible() override;
    void selectSubtitle(int nId) override;
    int sid() const override;
    void setSubDelay(double dSecs) override;
    double subDelay() const override;
    void updateSubStyle(const QString &sFont, int nSize) override;
    void setSubCodepage(const QString &sCodePage) override;
    QString subCodepage() override;
    void addSubSearchPath(const QString &sPath) override;
    void selectTrack(int nId) override;
    int aid() const override;
    void changeSoundMode(SoundMode soundMode) override;
    int volume() const override;
    bool muted() const override;
    void setVideoAspect(double dValue) override;
    double videoAspect() const override;
    int videoRotation() const override;
    void setVideoRotation(int nDegree) override;
    QImage takeScreenshot() const override;
    void burstScreenshot() override;
    void stopBurstScreenshot() override;
    QVariant getProperty(const QString &) const override;
    void setProperty(const QString &, const QVariant &) override;
    void nextFrame() override;
    void previousFrame() override;
    void changehwaccelMode(hwaccelMode hwaccelMode) override;
    void makeCurrent() override;

public slots:
    void play() override;
    void pauseResume() override;
    void stop() override;
    void seekForward(int nSecs) override;
    void seekBackward(int nSecs) override;
    void seekAbsolute(int nPos) override;
    void volumeUp() override;
    void volumeDown() override;
    void changeVolume(int nVol) override;
    void toggleMute() override;
    void setMute(bool bMute) override;
    void setWinID(const qint64 &winID) override;

protected:
    void initMember();
    void initGpuInfoFuns();
    bool isSurportHardWareDecode(const QString sDecodeName, const int &nVideoWidth, const int &nVideoHeight);
    int getDecodeProbeValue(const QString sDecodeName);

protected slots:
    void handle_mpv_events();
    void stepBurstScreenshot();

protected:
    QScopedPointer<DMpvProxyPrivate> d_ptr;
};

DMULTIMEDIA_END_NAMESPACE

#endif /* ifndef DMPVPROXY_H */
