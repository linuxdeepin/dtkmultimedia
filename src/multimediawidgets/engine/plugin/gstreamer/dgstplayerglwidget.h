// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQTPLAYER_GLWIDGET_H
#define DQTPLAYER_GLWIDGET_H

#include <QtWidgets>
#undef Bool
#include <DGuiApplicationHelper>
#include <dtkmultimedia.h>

#ifdef BUILD_Qt6
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QtOpenGL/QOpenGLVertexArrayObject>
#include <QtOpenGL/QOpenGLBuffer>
#include <QtOpenGL/QOpenGLTexture>
#include <QtOpenGL/QOpenGLShaderProgram>
#include <QtOpenGL/QOpenGLFramebufferObject>
#endif

DMULTIMEDIA_BEGIN_NAMESPACE
class DGstPlayerGLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    friend class MpvProxy;

    explicit DGstPlayerGLWidget(QWidget *parent);
    virtual ~DGstPlayerGLWidget();

    /**
     * rounded clipping consumes a lot of resources, and performs bad on 4K video
     */
    void toggleRoundedClip(bool bFalse);

    void setPlaying(bool);

    void setVideoTex(QImage image);

#ifdef __x86_64__
    //更新全屏时影院播放进度
    void updateMovieProgress(qint64 duration, qint64 pos);
#endif
    void setRawFormatFlag(bool bRawFormat);

protected:
    /**
     * @brief opengl初始化 cppcheck误报
     */
    void initializeGL() override;
    void resizeGL(int nWidth, int nHeight) override;
    void paintGL() override;

    void setMiniMode(bool);

private:
    void initMember();
    void updateVbo();
    void updateVboCorners();
    void updateVboBlend();

    void updateMovieFbo();
    void updateCornerMasks();

    void setupBlendPipe();
    void setupIdlePipe();

    void prepareSplashImages();

private:
    bool m_bPlaying = false;
    bool m_bInMiniMode = false;
    bool m_bDoRoundedClipping = false;

    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QOpenGLTexture *m_pDarkTex = { nullptr };
    QOpenGLTexture *m_pLightTex = { nullptr };
    QOpenGLShaderProgram *m_pGlProg = { nullptr };

    QOpenGLVertexArrayObject m_vaoBlend;
    QOpenGLBuffer m_vboBlend;
    QOpenGLShaderProgram *m_pGlProgBlend;
    QOpenGLFramebufferObject *m_pFbo = { nullptr };
    QOpenGLShaderProgram *m_pGlProgBlendCorners = { nullptr };

    QOpenGLVertexArrayObject m_vaoCorner;
    QOpenGLTexture *m_pCornerMasks[4] = { nullptr };
    QOpenGLBuffer m_vboCorners[4];
    QOpenGLShaderProgram *m_pGlProgCorner;

    QImage m_imgBgDark;
    QImage m_imgBgLight;

    QOpenGLTexture *m_pVideoTex = { nullptr };
    int m_currWidth;
    int m_currHeight;
#ifdef __x86_64__
    qreal m_pert;
    QString m_strPlayTime;
#endif
    bool m_bRawFormat = false;
};

DMULTIMEDIA_END_NAMESPACE

#endif /* ifndef DQTPLAYER_GLWIDGET_H */
