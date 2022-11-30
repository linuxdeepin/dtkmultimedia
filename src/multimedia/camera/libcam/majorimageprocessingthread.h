// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MajorImageProcessingThread_H
#define MajorImageProcessingThread_H

#include <QThread>
#include <QPixmap>
#include <QDebug>
#include <QMutex>
#include <QWaitCondition>

#include "datamanager.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "core_io.h"
#include "LPF_V4L2.h"
#include "camview.h"
#include "stdlib.h"
#include "malloc.h"
#include "stdio.h"
#include "math.h"
#include "save_image.h"
#include "colorspaces.h"
#include "render.h"
#ifdef __cplusplus
}
#endif

class MajorImageProcessingThread : public QThread
{
    Q_OBJECT
public:
    MajorImageProcessingThread();

    ~MajorImageProcessingThread();

    void setHorizontalMirror(bool bMirror) { m_bHorizontalMirror = bMirror; }

    void stop();

    void init();

    QAtomicInt getStatus()
    {
        return m_stopped;
    }

    void setFilter(QString filter);

    void setExposure(int exposure);

    void setState(bool bPhoto)
    {
        m_bPhoto = bPhoto;
    }

    void setRecording(bool bRecording)
    {
        m_bRecording = bRecording;
    }

    void setFilterGroupState(bool bDisplay)
    {
        m_filtersGroupDislay = bDisplay;
    }

    int getRecCount();
    QSize getResolution();
    uint8_t *getbuffer();

protected:
    void run();

signals:
    void SendMajorImageProcessing(QImage *image, int result);

    void SendFilterImageProcessing(QImage *image);

    void sigReflushSnapshotLabel();

#ifndef __mips__

    void sigYUVFrame(uchar *yuv, uint width, uint height);
    void sigRenderYuv(bool);

#endif

    void sigRecordFrame(uchar *rgb, uint size);

    void reachMaxDelayedFrames();

public:
    QMutex m_rwMtxImg;
    QString m_strPath;
    QMutex m_rwMtxPath;
    bool m_bTake;

private:
    void ImageHorizontalMirror(const uint8_t *src, uint8_t *dst, int width, int height);

public slots:
    void processingImage(QImage &);

private:
    int m_result;
    uint m_nVdWidth;
    uint m_nVdHeight;
    volatile int m_majorindex;
    QString m_filter;
    QAtomicInt m_stopped;
    v4l2_dev_t *m_videoDevice;
    v4l2_frame_buff_t *m_frame;
    uint8_t *m_yuvPtr;
    uint8_t *m_rgbPtr;

    bool m_bPhoto = true;
    bool m_bRecording;
    bool m_bHorizontalMirror;
    EncodeEnv m_eEncodeEnv;
    int m_exposure = 0;
    bool m_filtersGroupDislay = false;
    int m_nCount;
    uint64_t m_firstPts;

    QImage m_Img;
    QImage m_filterImg;
    QImage m_jpgImage;
};

#endif   // MajorImageProcessingThread_H
