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

#ifndef TUPCOLORFORM_H
#define TUPCOLORFORM_H

#include "tglobal.h"
#include "tupformitem.h"

#include <QWidget>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>

class TUPITUBE_EXPORT TupColorForm: public QWidget
{
    Q_OBJECT

    public:
        TupColorForm(QWidget *parent = nullptr);
        ~TupColorForm();

    private:
       void setupForm();

    public slots:
       void setColor(const QBrush &);

    private slots:
       void syncRgbValues();
       void syncHsvValues();
       void updateAlphaValueFromSpin(int alpha);
       void updateAlphaValueFromSlider(int alpha);

    signals:
       void brushChanged(const QBrush &);
       void hueChanged(int);
       void saturationChanged(int);
       void valueChanged(int);

    private:
       TupFormItem *valueR;
       TupFormItem *valueG;
       TupFormItem *valueB;

       TupFormItem *valueH;
       TupFormItem *valueS;
       TupFormItem *valueV;

       QSpinBox *alphaBox;
       QSlider *alphaSlider;
};

#endif
