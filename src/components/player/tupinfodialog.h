/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez / xtingray                                          *
 *                                                                         *
 *   KTooN's versions:                                                     * 
 *                                                                         *
 *   2006:                                                                 *
 *    David Cuadrado                                                       *
 *    Jorge Cuadrado                                                       *
 *   2003:                                                                 *
 *    Fernado Roldan                                                       *
 *    Simena Dinas                                                         *
 *                                                                         *
 *   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       *
 *   License:                                                              *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef TUPINFODIALOG_H
#define TUPINFODIALOG_H

#include "tglobal.h"
#include "tconfig.h"
#include "tapplicationproperties.h"

#ifdef TUP_DEBUG
#ifdef Q_OS_WIN
  #include <QDebug>
#else
  #include "tdebug.h"
#endif
#endif

#include <QDialog>
#include <QLineEdit>
#include <QPlainTextEdit>

class TupInfoDialog : public QDialog
{
    Q_OBJECT

    public:
        TupInfoDialog(const QString &tags, const QString &author, const QString &desc, QWidget *parent = 0);
        ~TupInfoDialog();

        void focusProjectLabel();

    signals:
        void dataSent(const QString &, const QString &, const QString &);

    private slots:
        void updateInfo();

    private:
        QLineEdit *projectTags;
        QLineEdit *authorName;
        QPlainTextEdit *descText;
};

#endif
