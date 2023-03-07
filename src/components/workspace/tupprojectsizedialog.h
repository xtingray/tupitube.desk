/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez                                                     *
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

#ifndef TUPPROJECTSIZEDIALOG_H
#define TUPPROJECTSIZEDIALOG_H 

#include "tglobal.h"
#include "tapplicationproperties.h"
#include "txyspinbox.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>

class TUPITUBE_EXPORT TupProjectSizeDialog : public QDialog 
{
    Q_OBJECT

    public:
        TupProjectSizeDialog(const QSize &size, QWidget *parent = nullptr);
        ~TupProjectSizeDialog();

        QSize getSize();

    private slots:
        void setPresets(const QSize &size);
        void setPresets(int index);
        void updatePresetsCombo();

    private:
        void setUI(const QSize &size);
        void checkDimensions(const QSize &size);
        void enableOkButton();

        QVBoxLayout *layout;
        QPushButton *okButton;
        QSize projectSize;
        QComboBox *presetsCombo;
        TXYSpinBox *sizeSpin;
};

#endif
