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

#include "tupcolorform.h"
#include "tseparator.h"
#include "tdoublecombobox.h"

#include <QHBoxLayout>
// #include <QLabel>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLineEdit>
#include <cmath>

TupColorForm::TupColorForm(QWidget *parent) : QWidget(parent)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupColorForm()]";
        #else
            TINIT;
        #endif
    #endif

    setupForm();
}

TupColorForm::~TupColorForm()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[~TupColorForm()]";
        #else
            TEND;
        #endif
    #endif
}

void TupColorForm::setupForm()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QGridLayout *gridLayout = new QGridLayout;
    valueR = new TupFormItem("R");
    connect(valueR, SIGNAL(editingFinished()), this, SLOT(syncRgbValues()));

    valueG = new TupFormItem("G");
    connect(valueG, SIGNAL(editingFinished()), this, SLOT(syncRgbValues()));

    valueB = new TupFormItem("B");
    connect(valueB, SIGNAL(editingFinished()), this, SLOT(syncRgbValues()));

    valueH = new TupFormItem("H");
    valueH->setMax(359);
    connect(valueH, SIGNAL(editingFinished()), this, SLOT(syncHsvValues()));

    valueS = new TupFormItem("S");
    connect(valueS, SIGNAL(editingFinished()), this, SLOT(syncHsvValues()));

    valueV = new TupFormItem("V");
    connect(valueV, SIGNAL(editingFinished()), this, SLOT(syncHsvValues()));

    gridLayout->addWidget(valueR, 0, 0, Qt::AlignTop | Qt::AlignLeft);
    gridLayout->addWidget(valueG, 1, 0, Qt::AlignTop | Qt::AlignLeft);
    gridLayout->addWidget(valueB, 2, 0, Qt::AlignTop | Qt::AlignLeft);
    gridLayout->addWidget(valueH, 0, 1, Qt::AlignTop | Qt::AlignLeft);
    gridLayout->addWidget(valueS, 1, 1, Qt::AlignTop | Qt::AlignLeft);
    gridLayout->addWidget(valueV, 2, 1, Qt::AlignTop | Qt::AlignLeft);

    QLabel *alphaLabel = new QLabel(tr("Alpha (Transparency)"));
    alphaLabel->setAlignment(Qt::AlignHCenter);

    alphaSlider = new QSlider(Qt::Horizontal);
    alphaSlider->setMinimum(0);
    alphaSlider->setMaximum(255);
    alphaSlider->setSingleStep(1);
    alphaSlider->setValue(255);
    connect(alphaSlider, SIGNAL(valueChanged(int)), this, SLOT(updateAlphaValue(int)));

    alphaCounter = new QLabel("255");
    alphaCounter->setAlignment(Qt::AlignHCenter);

    layout->addLayout(gridLayout);
    layout->addWidget(new TSeparator(Qt::Horizontal));
    layout->addWidget(alphaLabel);
    layout->addWidget(alphaSlider);
    layout->addWidget(alphaCounter);
}

void TupColorForm::setColor(const QBrush &brush)
{
    QColor color = brush.color();

    blockSignals(true);
    valueR->setValue(color.red());
    valueG->setValue(color.green());
    valueB->setValue(color.blue());
    valueH->setValue(color.hue());
    valueS->setValue(color.saturation());
    valueV->setValue(color.value());

    alphaCounter->setText(QString::number(color.alpha()));
    alphaSlider->blockSignals(true);
    alphaSlider->setValue(color.alpha());
    alphaSlider->blockSignals(false);
    blockSignals(false);
}

void TupColorForm::syncRgbValues()
{
    int r = valueR->getValue();
    int g = valueG->getValue();
    int b = valueB->getValue();
    int a = alphaSlider->value();

    QColor color = QColor::fromRgb(r, g, b, a);
    blockSignals(true);
    valueH->setValue(color.hue());
    valueS->setValue(color.saturation());
    valueV->setValue(color.value());
    blockSignals(false);

    emit brushChanged(color);
}

void TupColorForm::syncHsvValues()
{
    int h = valueH->getValue();
    int s = valueS->getValue();
    int v = valueV->getValue();
    int a = alphaSlider->value();

    QColor color = QColor::fromHsv(h, s, v, a);
    blockSignals(true);
    valueR->setValue(color.red());
    valueG->setValue(color.green());
    valueB->setValue(color.blue());
    blockSignals(false);

    emit brushChanged(color);
}

void TupColorForm::updateAlphaValue(int alpha)
{
    alphaCounter->setText(QString::number(alpha));
    syncRgbValues();
}
