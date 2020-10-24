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
#include "tapplicationproperties.h"
#include "tseparator.h"

#include <QBoxLayout>
#include <QLabel>

InkSettings::InkSettings(QWidget *parent) : QWidget(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "InkSettings()";
    #endif

    QFont titleFont = font(); 
    titleFont.setBold(true);

    TCONFIG->beginGroup("InkTool");
    device = Device(TCONFIG->value("Device", Pen).toInt());
    double sensibility = TCONFIG->value("Sensibility", 5).toInt();
    double smoothness = TCONFIG->value("Smoothness", 4.0).toDouble();
    bool showBorder = TCONFIG->value("BorderEnabled", true).toBool();
    bool showFill = TCONFIG->value("FillEnabled", true).toBool();
    int borderSize = TCONFIG->value("BorderSize", 1).toInt();

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);

    QLabel *toolTitle = new QLabel;
    toolTitle->setAlignment(Qt::AlignHCenter);
    QPixmap pic(THEME_DIR + "icons/ink.png");
    toolTitle->setPixmap(pic.scaledToWidth(16, Qt::SmoothTransformation));
    toolTitle->setToolTip(tr("Ink Properties"));
    mainLayout->addWidget(toolTitle);
    mainLayout->addWidget(new TSeparator(Qt::Horizontal));

    QBoxLayout *deviceLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    QLabel *deviceLabel = new QLabel(tr("Device"));
    deviceLabel->setFont(titleFont);
    deviceLabel->setAlignment(Qt::AlignHCenter);

    QComboBox *deviceCombo = new QComboBox();
    deviceCombo->addItem(tr("Mouse"));
    deviceCombo->addItem(tr("Pen"));
    connect(deviceCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateDevice(int)));

    deviceLayout->addWidget(deviceLabel);
    deviceLayout->addWidget(deviceCombo);
    mainLayout->addLayout(deviceLayout);

    QBoxLayout *borderLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    QLabel *appearanceLabel = new QLabel(tr("Appearance"));
    appearanceLabel->setFont(titleFont);
    appearanceLabel->setAlignment(Qt::AlignHCenter);
    borderLayout->addWidget(appearanceLabel);
    mainLayout->addLayout(borderLayout);

    borderOption = new QCheckBox(tr("Enable Border"));
    borderOption->setChecked(showBorder);
    connect(borderOption, SIGNAL(toggled(bool)), this, SLOT(updateBorderOption(bool)));
    mainLayout->addWidget(borderOption);

    fillOption = new QCheckBox(tr("Enable Fill"));
    fillOption->setChecked(showFill);
    connect(fillOption, SIGNAL(toggled(bool)), this, SLOT(updateFillOption(bool)));
    mainLayout->addWidget(fillOption);

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

    connect(borderSizeBox, SIGNAL(valueChanged(int)), this, SIGNAL(borderSizeUpdated(int)));
    borderSizeLayout->addWidget(borderSizeBox);
    mainLayout->addLayout(borderSizeLayout);

    pressureWidget = new QWidget();
    QBoxLayout *pressureLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    QLabel *pressureLabel = new QLabel(tr("Pressure Level"));
    pressureLabel->setAlignment(Qt::AlignHCenter);
    pressureLayout->addWidget(pressureLabel);
    pressureWidget->setLayout(pressureLayout);

    pressureBox = new QSpinBox();
    pressureBox->setSingleStep(1);
    pressureBox->setMinimum(1);
    pressureBox->setMaximum(5);
    pressureBox->setValue(static_cast <int>(sensibility));
    connect(pressureBox, SIGNAL(valueChanged(int)), this, SIGNAL(pressureUpdated(int)));
    pressureLayout->addWidget(pressureBox);
    mainLayout->addWidget(pressureWidget);

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

    if (device != Mouse)
        deviceCombo->setCurrentIndex(device);
    else
        updateDevice(device);
}

InkSettings::~InkSettings()
{
}

InkSettings::Device InkSettings::currentDevice()
{
    return device;
}

void InkSettings::updateDevice(int index)
{
    bool enabled = true;
    if (index == 0)
        enabled = false;

    pressureWidget->setVisible(enabled);
    TCONFIG->beginGroup("InkTool");
    TCONFIG->setValue("Device", index);

    emit deviceUpdated(Device(index));
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

bool InkSettings::smooothnessIsEnabled()
{
    return smoothLabel->isChecked();
}
