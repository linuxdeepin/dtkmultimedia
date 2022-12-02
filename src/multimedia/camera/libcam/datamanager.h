// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QSet>

enum GridType {
    Grid_None = 0,
    Grid_Matts = 1,
    Grid_Thirds = 2
};

enum DeviceStatus { NOCAM = 0,
                    CAM_CANNOT_USE,
                    CAM_CANUSE };

enum EncodeEnv {
    FFmpeg_Env = 0,
    QCamera_Env = 1,
    GStreamer_Env = 2
};

class DataManager : public QObject
{
    Q_OBJECT
public:
    static DataManager *instance();

    QSet<int> m_setIndex;
    bool m_bTabEnter;
    uint m_tabIndex;
    Q_DISABLE_COPY(DataManager)

    QWidget *getObject(uint index);

    void clearIndex()
    {
        m_setIndex.clear();
    }

    void insertIndex(int index)
    {
        m_setIndex.insert(index);
    }

    QString &getstrFileName();

    enum DeviceStatus getdevStatus();

    void setdevStatus(enum DeviceStatus devStatus);

    void setNowTabIndex(uint tabindex);

    uint getNowTabIndex();

    void setEncodeEnv(EncodeEnv env);

    EncodeEnv encodeEnv();

private:
    DataManager();
    static DataManager *m_dataManager;
    uint m_tabIndexNow;
    QString m_strFileName;
    int m_videoCount;
    EncodeEnv m_encodeEnv;
    volatile enum DeviceStatus m_devStatus;
};
#endif   // DATAMANAGER_H
