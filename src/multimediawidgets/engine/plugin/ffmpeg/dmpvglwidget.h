// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DMPV_GLWIDGET_H
#define DMPV_GLWIDGET_H

#include <QtWidgets>
#ifdef BUILD_Qt6
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QtOpenGL/QOpenGLBuffer>
#include <QtOpenGL/QOpenGLVertexArrayObject>
#include <QtOpenGL/QOpenGLTexture>
#include <QtOpenGL/QOpenGLShaderProgram>
#include <QtOpenGL/QOpenGLFramebufferObject>
#endif
#include <mpv/render.h>
#include <mpv/render_gl.h>
#include <DMpvProxy>
#undef Bool
#include <DGuiApplicationHelper>
#include <dtkmultimedia.h>

//add by heyi
typedef void (*mpv_render_contextSet_update_callback)(mpv_render_context *ctx,
                                                      mpv_render_update_fn callback,
                                                      void *callback_ctx);
typedef void (*mpv_render_contextReport_swap)(mpv_render_context *ctx);
typedef void (*mpv_renderContext_free)(mpv_render_context *ctx);
typedef int (*mpv_renderContext_create)(mpv_render_context **res, mpv_handle *mpv,
                                        mpv_render_param *params);
typedef int (*mpv_renderContext_render)(mpv_render_context *ctx, mpv_render_param *params);
typedef uint64_t (*mpv_renderContext_update)(mpv_render_context *ctx);

DMULTIMEDIA_BEGIN_NAMESPACE
class DMpvGLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    friend class DMpvProxy;

    DMpvGLWidget(QWidget *parent, MpvHandle h);
    virtual ~DMpvGLWidget();
    void toggleRoundedClip(bool bFalse);
    void setHandle(MpvHandle h);

protected:
    void initializeGL() override;
    void resizeGL(int nWidth, int nHeight) override;
    void paintGL() override;

public:
    void setPlaying(bool);
    void setMiniMode(bool);
    void initMpvFuns();
#ifdef __x86_64__
    void updateMovieProgress(qint64 duration, qint64 pos);
#endif
    void setRawFormatFlag(bool bRawFormat);

protected slots:
    void onNewFrame();
    void onFrameSwapped();

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
    MpvHandle m_handle;
    mpv_render_context *m_pRenderCtx = { nullptr };

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
    QOpenGLShaderProgram *m_pGlProgBlend = { nullptr };
    QOpenGLFramebufferObject *m_pFbo = { nullptr };
    QOpenGLShaderProgram *m_pGlProgBlendCorners = { nullptr };

    QOpenGLVertexArrayObject m_vaoCorner;
    QOpenGLTexture *m_pCornerMasks[4] = { nullptr };
    QOpenGLBuffer m_vboCorners[4];
    QOpenGLShaderProgram *m_pGlProgCorner;

    QImage m_imgBgDark;
    QImage m_imgBgLight;

    mpv_render_contextSet_update_callback m_callback = { nullptr };
    mpv_render_contextReport_swap m_context_report = { nullptr };
    mpv_renderContext_free m_renderContex = { nullptr };
    mpv_renderContext_create m_renderCreat = { nullptr };
    mpv_renderContext_render m_renderContexRender = { nullptr };
    mpv_renderContext_update m_renderContextUpdate = { nullptr };
#ifdef __x86_64__
    qreal m_pert;
    QString m_strPlayTime;
#endif
    bool m_bRawFormat = false;
};

DMULTIMEDIA_END_NAMESPACE

#endif /* ifndef DMPV_GLWIDGET_H */
