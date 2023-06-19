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
#include "tconfig.h"
#include "tseparator.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>

PenSettings::PenSettings(QWidget *parent) : QWidget(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PenSettings()]";
    #endif

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
    layout->setAlignment(Qt::AlignHCenter);

    QLabel *pencilTitle = new QLabel;
    pencilTitle->setAlignment(Qt::AlignHCenter);
    QPixmap pencilPic(THEME_DIR + "icons/pencil.png");
    pencilTitle->setPixmap(pencilPic.scaledToWidth(16, Qt::SmoothTransformation));
    pencilTitle->setToolTip(tr("Pencil Properties"));

    layout->addWidget(pencilTitle);
    layout->addWidget(new TSeparator(Qt::Horizontal));

    QPixmap pencilTarget(THEME_DIR + "icons/target.png");
    pencilButton = new QPushButton(pencilTarget, "");
    pencilButton->setCheckable(true);
    pencilButton->setToolTip(tr("Pencil Mode"));
    connect(pencilButton, SIGNAL(clicked()), this, SLOT(enablePencilMode()));

    QPixmap eraserPic(THEME_DIR + "icons/eraser.png");
    eraserButton = new QPushButton(eraserPic, "");
    eraserButton->setCheckable(true);
    eraserButton->setToolTip(tr("Eraser Mode"));
    connect(eraserButton, SIGNAL(clicked()), this, SLOT(enableEraserMode()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(pencilButton);
    buttonsLayout->addWidget(eraserButton);

    layout->addLayout(buttonsLayout);

    smoothLabel = new QCheckBox;
    smoothLabel->setIcon(QIcon(QPixmap(THEME_DIR + "icons/smoothness.png")));
    smoothLabel->setToolTip(tr("Smoothness"));
    smoothLabel->setChecked(true);
    connect(smoothLabel, SIGNAL(toggled(bool)), this, SLOT(updateSmoothBox(bool)));

    smoothBox = new QDoubleSpinBox();
    smoothBox->setDecimals(2);
    smoothBox->setSingleStep(0.1);
    smoothBox->setMinimum(0);
    smoothBox->setMaximum(20);
    smoothBox->setAlignment(Qt::AlignCenter);
    connect(smoothBox, SIGNAL(valueChanged(double)), this, SIGNAL(smoothnessUpdated(double)));

    pencilWidget = new QWidget;
    pencilWidget->setFixedWidth(120);
    QVBoxLayout *pencilLayout = new QVBoxLayout(pencilWidget);
    pencilLayout->addWidget(smoothLabel, Qt::AlignHCenter);
    pencilLayout->addWidget(smoothBox, Qt::AlignHCenter);

    layout->addWidget(pencilWidget);

    TCONFIG->beginGroup("BrushParameters");
    int eraserValue = TCONFIG->value("EraserSize", 10).toInt();
    if (eraserValue > 40)
        eraserValue = 10;

    eraserPreview = new TupPenThicknessWidget(this);
    eraserPreview->setColor(Qt::white);
    eraserPreview->setBrush(Qt::SolidPattern);
    eraserPreview->render(eraserValue);

    eraserSlider = new QSlider(Qt::Horizontal, this);
    eraserSlider->setRange(10, 40);
    connect(eraserSlider, SIGNAL(valueChanged(int)), this, SLOT(updateEraserSizeFromSlider(int)));
    connect(eraserSlider, SIGNAL(valueChanged(int)), eraserPreview, SLOT(render(int)));

    eraserSizeBox = new QSpinBox;
    eraserSizeBox->setAlignment(Qt::AlignHCenter);
    eraserSizeBox->setMinimum(10);
    eraserSizeBox->setMaximum(40);
    connect(eraserSizeBox, SIGNAL(valueChanged(int)), this, SLOT(updateEraserSizeFromBox(int)));
    connect(eraserSizeBox, SIGNAL(valueChanged(int)), eraserPreview, SLOT(render(int)));

    eraserSlider->setValue(eraserValue);
    updateEraserSizeFromSlider(eraserValue);

    eraserWidget = new QWidget;
    eraserWidget->setFixedWidth(120);
    QVBoxLayout *eraserLayout = new QVBoxLayout(eraserWidget);
    eraserLayout->addWidget(eraserPreview, Qt::AlignHCenter);
    eraserLayout->addWidget(eraserSlider, Qt::AlignHCenter);
    eraserLayout->addWidget(eraserSizeBox, Qt::AlignHCenter);

    layout->addWidget(eraserWidget, Qt::AlignHCenter);

    mainLayout->addLayout(layout);
    mainLayout->addStretch(2);
}

PenSettings::~PenSettings()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~PenSettings()]";
    #endif
}

void PenSettings::enablePencilMode()
{
    #ifdef TUP_DEBUG
        qDebug() << "[PenSettings::enablePencilMode()]";
    #endif

    pencilButton->setChecked(true);
    if (eraserButton->isChecked())
        eraserButton->setChecked(false);

    eraserWidget->setVisible(false);
    pencilWidget->setVisible(true);

    emit toolEnabled(PencilMode);
}

void PenSettings::enableEraserMode()
{
    #ifdef TUP_DEBUG
        qDebug() << "[PenSettings::enableEraserMode()]";
    #endif

    eraserButton->setChecked(true);
    if (pencilButton->isChecked())
        pencilButton->setChecked(false);

    pencilWidget->setVisible(false);
    eraserWidget->setVisible(true);

    emit toolEnabled(EraserMode);
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

void PenSettings::updateEraserSizeFromSlider(int value)
{
    emit eraserSizeChanged(value);

    eraserSizeBox->blockSignals(true);
    eraserSizeBox->setValue(value);
    eraserSizeBox->blockSignals(false);
}

void PenSettings::updateEraserSizeFromBox(int value)
{
    emit eraserSizeChanged(value);

    eraserSlider->blockSignals(true);
    eraserSlider->setValue(value);
    eraserSlider->blockSignals(false);
}
