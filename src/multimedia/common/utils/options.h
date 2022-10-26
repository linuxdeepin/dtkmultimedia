// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOPTIONS_H
#define DOPTIONS_H

#include <QtCore>
#include <dtkmultimedia.h>

DMULTIMEDIA_BEGIN_NAMESPACE
class CommandLineManager: public QCommandLineParser {
public:
    static CommandLineManager& get();
    bool verbose() const;
    bool debug() const;
    QString openglMode() const;
    QString overrideConfig() const;
    
    QString dvdDevice() const;

private:
    CommandLineManager();
};

DMULTIMEDIA_END_NAMESPACE

#endif /* ifndef DOPTIONS_H */
