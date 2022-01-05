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

#ifndef TUPNEWPROYECT_H
#define TUPNEWPROYECT_H

#include "tglobal.h"
#include "tabdialog.h"
#include "twizard.h"
#include "txyspinbox.h"
#include "tupprojectmanagerparams.h"

#include <QLineEdit>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QColorDialog>
#include <QStyleOptionButton>
#include <QComboBox>
#include <QLineEdit>

// class TupProjectManagerParams;

class TupNewProject : public TabDialog
{
    Q_OBJECT

    public:
        enum Format { FREE = 0, 
                      FORMAT_520,
                      FORMAT_640,
                      FORMAT_480,
                      FORMAT_576,
                      FORMAT_720,
                      FORMAT_MOBILE,
                      FORMAT_1080_VERTICAL,
                      FORMAT_1080
                    };

        TupNewProject(QWidget *parent = nullptr);
        ~TupNewProject();

        TupProjectManagerParams *parameters();
        bool useNetwork() const;
        void focusProjectLabel();
        QString login() const;

    public slots:
        void ok();

    private:
        void setupNetOptions();

    private slots:
        void setBgColor();
        void setPresets(int index);
        void updateFormatCombo();

    public slots:
        void enableNetOptions(bool isEnabled);

    private:
        QString labelColor() const;

        QLineEdit *projectName;
        QLineEdit *authorName;
        QLineEdit *description;

        QColor color;
        QPushButton *colorButton;
        QSpinBox *fps;

        QComboBox *presets;
        TXYSpinBox *size;
        bool enableUseNetwork;

        QGroupBox *netOptions;
        QBoxLayout *netLayout;

        QLineEdit *server;
        QSpinBox *port;
        QLineEdit *username;
        QLineEdit *password;

        QCheckBox *storePassword;
};

#endif
