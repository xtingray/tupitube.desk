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

#ifndef TEDITSPINBOX_H
#define TEDITSPINBOX_H

#include "tglobal.h"

#include <QHBoxLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QSlider>

class T_GUI_EXPORT TEditSpinBox : public QGroupBox
{
    Q_OBJECT

    public:
        TEditSpinBox(int value, int valueMin, int valueMax, int step, QString text,
                     const char *name = nullptr, QWidget *parent = nullptr);
        ~TEditSpinBox();

        void setRange(int min, int max);
        int value();
        
    private:
        QSlider *m_slider;
        QSpinBox *m_spin;
        void setupConnects();

    public slots:
        void setValue(int value);
        
    signals:
        void valueChanged(int);
};

#endif
