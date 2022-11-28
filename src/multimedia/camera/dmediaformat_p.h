// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DMEDIAFORMAT_P_H
#define DMEDIAFORMAT_P_H
#include "dmediaformat.h"

DMULTIMEDIA_BEGIN_NAMESPACE
class DMediaFormatPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DMediaFormat)
public:
    explicit DMediaFormatPrivate(DMediaFormat *parent = nullptr)
        : QObject(parent), q_ptr(parent)
    {
    }

private:
    DMediaFormat *q_ptr;
    DMediaFormat::FileFormat fmt;
    DMediaFormat::AudioCodec audio = DMediaFormat::AudioCodec::Unspecified;
    DMediaFormat::VideoCodec video = DMediaFormat::VideoCodec::Unspecified;
};

DMULTIMEDIA_END_NAMESPACE

#endif   // DMEDIAFORMAT_P_H
