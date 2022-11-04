// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDir>
#include <QJsonDocument>
#include <QLibrary>
#include <QLibraryInfo>

#include "eventlogutils.h"

EventLogUtils *EventLogUtils::m_instance(nullptr);

EventLogUtils &EventLogUtils::get()
{
    if(m_instance == nullptr) {
        m_instance = new EventLogUtils;
    }
    return *m_instance;
}

EventLogUtils::EventLogUtils()
{
    QLibrary library("libdeepin-event-log.so");

    init          = reinterpret_cast<bool (*)(const std::string &, bool)>(library.resolve("Initialize"));
    writeEventLog = reinterpret_cast<void (*)(const std::string &)>(library.resolve("WriteEventLog"));

    if(init == nullptr) return;

    init("deepin-movie", true);
}

void EventLogUtils::writeLogs(QJsonObject &data)
{
    if(writeEventLog == nullptr) return;

    writeEventLog(QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString());
}
