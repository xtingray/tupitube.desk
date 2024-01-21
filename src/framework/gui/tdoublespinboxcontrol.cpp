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

#include "tdoublespinboxcontrol.h"
#include <QHBoxLayout>

TDoubleSpinBoxControl::TDoubleSpinBoxControl(double value, double minValue, double maxValue, double step, QString text,
                                 const char *name, QWidget *parent) : QGroupBox(parent)
{
    setObjectName(name);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(2);
    layout->setSpacing(0);

    setTitle(text);
    setLayout(layout);

    doubleSpinBox = new QDoubleSpinBox(this);
    doubleSpinBox->setDecimals(1);
    doubleSpinBox->setMinimum(minValue);
    doubleSpinBox->setMaximum(maxValue);
    doubleSpinBox->setSingleStep(step);
    doubleSpinBox->setValue(value);
    layout->addWidget(doubleSpinBox);
    layout->addSpacing(5);

    slider = new QSlider(Qt::Horizontal, this);
    slider->setMinimum(minValue*10);
    slider->setMaximum(maxValue*10);
    slider->setSingleStep(step*10);
    value = value*10;
    int size = (int) value;
    slider->setValue(size);
    
    layout->addWidget(slider);
    setupConnections();
    setMinimumHeight(sizeHint().height());
}

TDoubleSpinBoxControl::~TDoubleSpinBoxControl()
{
}

void TDoubleSpinBoxControl::setupConnections()
{
    connect(doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(processValuesFromSpinBox(double)));
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(processValuesFromSlider(int)));
}

void TDoubleSpinBoxControl::setRange(double min, double max)
{
    doubleSpinBox->setMinimum(min);
    doubleSpinBox->setMaximum(max);
    slider->setMinimum(min*10);
    slider->setMaximum(max*10);
}

void TDoubleSpinBoxControl::setValue(double value)
{
    doubleSpinBox->blockSignals(true);
    doubleSpinBox->setValue(value);
    doubleSpinBox->blockSignals(false);

    slider->blockSignals(true);
    slider->setValue(value*10);
    slider->blockSignals(false);
}

double TDoubleSpinBoxControl::value()
{
    return doubleSpinBox->value();
}

void TDoubleSpinBoxControl::processValuesFromSpinBox(double value)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TDoubleSpinBoxControl::processValuesFromSpinBox()] - value ->" << value;
    #endif

    int size = (int)(value * 10);
    slider->blockSignals(true);
    slider->setValue(size);
    slider->blockSignals(false);

    emit valueChanged(value);
}

void TDoubleSpinBoxControl::processValuesFromSlider(int value)
{
    double input = (double) value;
    double size = input / 10.0;
    doubleSpinBox->blockSignals(true);
    doubleSpinBox->setValue(size);
    doubleSpinBox->blockSignals(false);

    #ifdef TUP_DEBUG
        qDebug() << "[TDoubleSpinBoxControl::processValuesFromSlider()] - size ->" << size;
    #endif

    emit valueChanged(size);
}
