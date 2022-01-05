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

#ifndef TOPTIONALDIALOG_H
#define TOPTIONALDIALOG_H

#include "tglobal.h"
#include "tseparator.h"

#include <QBoxLayout>
#include <QLabel>
#include <QDialog>
#include <QCheckBox>
#include <QPushButton>

class T_GUI_EXPORT TOptionalDialog : public QDialog
{
    Q_OBJECT

    public:
        enum Result {Accepted = 0, Cancelled, Discarded};
        TOptionalDialog(const QString &text, const QString &title = QString(), bool showAgainBox = true,
                        bool showDiscardButton = false, QWidget *parent = nullptr);
        ~TOptionalDialog();
        
        bool shownAgain();
        TOptionalDialog::Result getResult();

    private slots:
        void callAcceptAction();
        void callCancelAction();
        void callDiscardAction();

    private:
        void setUIStyle();

        QBoxLayout *mainLayout;
        QCheckBox *checkBox;
        Result result;
};

#endif
