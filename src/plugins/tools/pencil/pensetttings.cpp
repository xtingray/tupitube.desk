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

#include "pensettings.h"
#include "timagebutton.h"
#include "tconfig.h"
#include "tseparator.h"

#include <QBoxLayout>
#include <QLabel>

PenSettings::PenSettings(QWidget *parent) : QWidget(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "PenSettings()";
    #endif

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
    layout->setAlignment(Qt::AlignHCenter);

    QLabel *toolTitle = new QLabel;
    toolTitle->setAlignment(Qt::AlignHCenter);
    QPixmap pic(THEME_DIR + "icons/pencil.png"); 
    toolTitle->setPixmap(pic.scaledToWidth(16, Qt::SmoothTransformation));
    toolTitle->setToolTip(tr("Pencil Properties"));
    layout->addWidget(toolTitle);
    layout->addWidget(new TSeparator(Qt::Horizontal));

    smoothLabel = new QCheckBox(tr("Smoothness"));
    smoothLabel->setChecked(true);
    connect(smoothLabel, SIGNAL(toggled(bool)), this, SLOT(updateSmoothBox(bool)));
    layout->addWidget(smoothLabel);

    smoothBox = new QDoubleSpinBox();
    smoothBox->setDecimals(2);
    smoothBox->setSingleStep(0.1);
    smoothBox->setMinimum(0);
    smoothBox->setMaximum(20);
    connect(smoothBox, SIGNAL(valueChanged(double)), this, SIGNAL(smoothnessUpdated(double)));
    layout->addWidget(smoothBox);

    mainLayout->addLayout(layout);
    mainLayout->addStretch(2);
}

PenSettings::~PenSettings()
{
}

void PenSettings::updateSmoothBox(bool enabled)
{
    smoothBox->setEnabled(enabled);
    if (!enabled)
        emit smoothnessUpdated(0);
    else
        emit smoothnessUpdated(smoothBox->value());
}

void PenSettings::updateSmoothness(double value)
{
    smoothBox->blockSignals(true);
    smoothBox->setValue(value);
    smoothBox->blockSignals(false);
}
