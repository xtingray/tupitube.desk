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

#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include "tglobal.h"

#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QPushButton>

/**
 * @author Gustav Gonzalez 
*/

class TUPITUBE_PLUGIN InkSettings : public QWidget
{
    Q_OBJECT

    public:
        // enum Structure { Basic = 0, Axial, Organic };
        enum Device { Mouse = 0, Pen };

        InkSettings(QWidget *parent = nullptr);
        ~InkSettings();

        void updateSmoothness(double value);
        bool smooothnessIsEnabled();
        Device currentDevice();

    private slots:
        void updateDevice(int index);
        void updateBorderOption(bool showBorder);
        void updateFillOption(bool showFill);
        void updateSmoothBox(bool enabled);
        // void setBorderColor();

    signals:
        void deviceUpdated(InkSettings::Device device);
        void borderUpdated(bool borderFlag);
        void fillUpdated(bool fillFlag);
        void borderSizeUpdated(int size);
        void pressureUpdated(int sensibility);
        void smoothnessUpdated(double smoothness);

    private:
        Device device;
        // QComboBox *deviceCombo;
        QWidget *pressureWidget;
        QSpinBox *pressureBox;
        QCheckBox *borderOption;
        QCheckBox *fillOption;
        QSpinBox *borderSizeBox;
        QCheckBox *smoothLabel;
        QDoubleSpinBox *smoothBox;
};

#endif
