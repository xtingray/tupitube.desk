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

#ifndef SETTINGS_H
#define SETTINGS_H

#include "tglobal.h"
#include "tapplicationproperties.h"
#include "tuppenthicknesswidget.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>

class TUPITUBE_PLUGIN PenSettings : public QWidget
{
    Q_OBJECT

    public:
        PenSettings(QWidget *parent = nullptr);
        ~PenSettings();

        void enablePencilTool();
        void updateSmoothness(double value);

    signals:
        void smoothnessUpdated(double value);
        void toolEnabled(PenTool tool);
        void eraserSizeChanged(int value);

    private slots:
        void enablePencilMode();
        void enableEraserMode();
        void updateSmoothBox(bool enabled);
        void updateEraserSize(int value);

    private:
        QWidget *pencilWidget;
        QWidget *eraserWidget;

        QPushButton *pencilButton;
        QPushButton *eraserButton;

        TupPenThicknessWidget *eraserPreview;
        QSlider *eraserSize;

        QCheckBox *smoothLabel;
        QDoubleSpinBox *smoothBox;
        QLabel *eraserLabel;
        double smoothness;
};

#endif
