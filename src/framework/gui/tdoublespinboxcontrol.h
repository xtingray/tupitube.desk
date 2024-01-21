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

#ifndef TDOUBLESPINBOXCONTROL_H
#define TDOUBLESPINBOXCONTROL_H

#include "tglobal.h"

#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QSlider>

class T_GUI_EXPORT TDoubleSpinBoxControl : public QGroupBox
{
    Q_OBJECT

    public:
        TDoubleSpinBoxControl(double value, double valueMin, double valueMax, double step, QString text,
                              const char *name = nullptr, QWidget *parent = nullptr);
        ~TDoubleSpinBoxControl();

        void setRange(double min, double max);
        double value();
        
    public slots:
        void setValue(double value);
        void processValuesFromSpinBox(double value);
        void processValuesFromSlider(int value);
        
    signals:
        void valueChanged(double value);

    private:
        void setupConnections();

        QSlider *slider;
        QDoubleSpinBox *doubleSpinBox;
};

#endif
