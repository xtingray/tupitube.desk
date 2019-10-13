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

#include "inksettings.h"
#include "tconfig.h"

#include <QLabel>
#include <QBoxLayout>
#include <QColorDialog>

InkSettings::InkSettings(QWidget *parent) :QWidget(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "InkSettings()";
    #endif

    QFont titleFont = font(); 
    titleFont.setBold(true);

    TCONFIG->beginGroup("InkTool");
    double sensibility = TCONFIG->value("Sensibility", 5).toInt();
    double smoothness = TCONFIG->value("Smoothness", 4.0).toDouble();
    bool showBorder = TCONFIG->value("BorderEnabled", true).toBool();
    bool showFill = TCONFIG->value("FillEnabled", true).toBool();
    int borderSize = TCONFIG->value("BorderSize", 1).toInt();

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);

    QBoxLayout *borderLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    QLabel *appearanceLabel = new QLabel(tr("Appearance"));
    appearanceLabel->setFont(titleFont);
    appearanceLabel->setAlignment(Qt::AlignHCenter);
    borderLayout->addWidget(appearanceLabel);
    mainLayout->addLayout(borderLayout);

    borderOption = new QCheckBox(tr("Enable border"));
    borderOption->setChecked(showBorder);
    connect(borderOption, SIGNAL(toggled(bool)), this, SLOT(updateBorderOption(bool)));
    mainLayout->addWidget(borderOption);

    borderColor = QColor("#000");
    borderColorButton = new QPushButton();
    borderColorButton->setText(tr("Black"));
    borderColorButton->setPalette(QPalette(borderColor));
    borderColorButton->setAutoFillBackground(true);
    connect(borderColorButton, SIGNAL(clicked()), this, SLOT(setBorderColor()));

    QLabel *borderColorLabel = new QLabel(tr("Border Color") + ": ");
    borderColorLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QHBoxLayout *borderColorLayout = new QHBoxLayout;
    borderColorLayout->setAlignment(Qt::AlignHCenter);
    borderColorLayout->setMargin(0);
    borderColorLayout->setSpacing(0);
    borderColorLayout->addWidget(borderColorLabel);
    borderColorLayout->addWidget(borderColorButton);

    mainLayout->addLayout(borderColorLayout);

    fillOption = new QCheckBox(tr("Enable fill"));
    fillOption->setChecked(showFill);
    connect(fillOption, SIGNAL(toggled(bool)), this, SLOT(updateFillOption(bool)));
    mainLayout->addWidget(fillOption);

    fillColor = QColor("#000");
    fillColorButton = new QPushButton();
    fillColorButton->setText(tr("Black"));
    fillColorButton->setPalette(QPalette(borderColor));
    fillColorButton->setAutoFillBackground(true);
    connect(fillColorButton, SIGNAL(clicked()), this, SLOT(setBorderColor()));

    QLabel *fillColorLabel = new QLabel(tr("Fill Color") + ": ");
    fillColorLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QHBoxLayout *fillColorLayout = new QHBoxLayout;
    fillColorLayout->setAlignment(Qt::AlignHCenter);
    fillColorLayout->setMargin(0);
    fillColorLayout->setSpacing(0);
    fillColorLayout->addWidget(fillColorLabel);
    fillColorLayout->addWidget(fillColorButton);

    mainLayout->addLayout(fillColorLayout);

    QBoxLayout *paramsLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    QLabel *paramsLabel = new QLabel(tr("Parameters"));
    paramsLabel->setFont(titleFont);
    paramsLabel->setAlignment(Qt::AlignHCenter);
    paramsLayout->addWidget(paramsLabel);
    mainLayout->addLayout(paramsLayout);

    QBoxLayout *borderSizeLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    QLabel *borderSizeLabel = new QLabel(tr("Border Size"));
    borderSizeLabel->setAlignment(Qt::AlignHCenter);
    borderSizeLayout->addWidget(borderSizeLabel);

    borderSizeBox = new QSpinBox();
    borderSizeBox->setSingleStep(1);
    borderSizeBox->setMinimum(1);
    borderSizeBox->setMaximum(10);
    borderSizeBox->setValue(borderSize);

    qDebug() << "InkSettings() - borderSize: " <<  borderSize;

    connect(borderSizeBox, SIGNAL(valueChanged(int)), this, SIGNAL(borderSizeUpdated(int)));
    borderSizeLayout->addWidget(borderSizeBox);
    mainLayout->addLayout(borderSizeLayout);

    QBoxLayout *pressurreLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    QLabel *pressureLabel = new QLabel(tr("Pressure Sensibility"));
    pressureLabel->setAlignment(Qt::AlignHCenter);
    pressurreLayout->addWidget(pressureLabel);

    pressureBox = new QSpinBox();
    pressureBox->setSingleStep(1);
    pressureBox->setMinimum(1);
    pressureBox->setMaximum(5);
    pressureBox->setValue(static_cast <int>(sensibility));
    connect(pressureBox, SIGNAL(valueChanged(int)), this, SIGNAL(pressureUpdated(int)));
    pressurreLayout->addWidget(pressureBox);
    mainLayout->addLayout(pressurreLayout);

    QBoxLayout *smoothLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    smoothLayout->setAlignment(Qt::AlignHCenter);
    smoothLabel = new QCheckBox(tr("Smoothness"));
    smoothLabel->setChecked(true);
    connect(smoothLabel, SIGNAL(toggled(bool)), this, SLOT(updateSmoothBox(bool)));
    smoothLayout->addWidget(smoothLabel);
    smoothBox = new QDoubleSpinBox();

    smoothBox->setValue(smoothness);
    smoothBox->setDecimals(2);
    smoothBox->setSingleStep(0.1);
    smoothBox->setMinimum(0);
    smoothBox->setMaximum(10);
    connect(smoothBox, SIGNAL(valueChanged(double)), this, SIGNAL(smoothnessUpdated(double)));
    smoothLayout->addWidget(smoothBox);

    mainLayout->addLayout(smoothLayout);
    mainLayout->addStretch(2);
}

InkSettings::~InkSettings()
{
}

void InkSettings::updateBorderOption(bool showBorder)
{
    borderSizeBox->setEnabled(showBorder);

    if (!showBorder && !fillOption->isChecked())
        fillOption->setChecked(true);

    emit borderUpdated(showBorder);
}

void InkSettings::updateFillOption(bool showFill)
{
    if (!showFill && !borderOption->isChecked())
        borderOption->setChecked(true);

    emit fillUpdated(showFill);
}

void InkSettings::updateSmoothBox(bool enabled)
{
    smoothBox->setEnabled(enabled);
    if (!enabled)
        emit smoothnessUpdated(0);
    else
        emit smoothnessUpdated(smoothBox->value());
}

void InkSettings::updateSmoothness(double value)
{
    smoothBox->blockSignals(true);
    smoothBox->setValue(value);
    smoothBox->blockSignals(false);
}

void InkSettings::setBorderColor()
{
    borderColor = QColorDialog::getColor(borderColor, this, tr("Pick Border Color"),
                                         QColorDialog::ShowAlphaChannel);
    updateColor(borderColor, borderColorButton);
}

void InkSettings::updateColor(QColor color, QPushButton *colorButton)
{
    if (color.isValid()) {
        colorButton->setText(color.name());
        colorButton->setStyleSheet("QPushButton { background-color: " + color.name()
                                    + "; color: " + labelColor(color) + "; }");
    }
}

QString InkSettings::labelColor(QColor color) const
{
    QString text = "white";
    if (color.red() > 50 && color.green() > 50 && color.blue() > 50)
        text = "black";
    return text;
}


