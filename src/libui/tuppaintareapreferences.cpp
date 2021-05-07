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

#include "tuppaintareapreferences.h"
#include "tseparator.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QColorDialog>

TupPaintAreaPreferences::TupPaintAreaPreferences(QWidget *parent) : QWidget(parent)
{
    setupPage();
}

TupPaintAreaPreferences::~TupPaintAreaPreferences()
{
}

void TupPaintAreaPreferences::setupPage()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QWidget *widget = new QWidget;
    QVBoxLayout *pageLayout = new QVBoxLayout;

    QLabel *generalLabel = new QLabel(tr("Workspace Preferences"));
    labelFont = font();
    labelFont.setBold(true);
    labelFont.setPointSize(labelFont.pointSize() + 3);
    generalLabel->setFont(labelFont);
    pageLayout->addWidget(generalLabel);
    pageLayout->addSpacing(15);

    QLabel *gridLabel = new QLabel(tr("Grid Settings"));
    labelFont = font();
    labelFont.setBold(true);

    gridLabel->setFont(labelFont);
    pageLayout->addWidget(gridLabel);
    pageLayout->addLayout(gridPanel());

    pageLayout->addSpacing(10);
    pageLayout->addWidget(new TSeparator(Qt::Horizontal));
    pageLayout->addSpacing(10);

    QLabel *ROTLabel = new QLabel(tr("Rule Of Thirds Settings"));
    ROTLabel->setFont(labelFont);
    pageLayout->addWidget(ROTLabel);
    pageLayout->addLayout(ruleOfThirdsPanel());

    pageLayout->addSpacing(10);
    pageLayout->addWidget(new TSeparator(Qt::Horizontal));
    pageLayout->addSpacing(10);

    QLabel *safeAreaLabel = new QLabel(tr("Safe Area Settings"));
    safeAreaLabel->setFont(labelFont);
    pageLayout->addWidget(safeAreaLabel);
    pageLayout->addLayout(safeAreaPanel());

    pageLayout->addSpacing(10);
    pageLayout->addWidget(new TSeparator(Qt::Horizontal));
    pageLayout->addSpacing(10);

    QHBoxLayout *resetLayout = new QHBoxLayout;
    QPushButton *resetButton = new QPushButton(tr("Restore Default Values"));
    connect(resetButton, SIGNAL(clicked()), this, SLOT(restoreValues()));

    resetLayout->addStretch();
    resetLayout->addWidget(resetButton);
    // resetLayout->addStretch();
    pageLayout->addLayout(resetLayout);

    widget->setLayout(pageLayout);

    tabWidget = new QTabWidget;
    tabWidget->addTab(widget, tr("UI Settings"));

    layout->addWidget(tabWidget);
    // layout->setAlignment(tabWidget, Qt::AlignLeft);
    layout->addStretch(3);
}

QGridLayout * TupPaintAreaPreferences::gridPanel()
{
    TCONFIG->beginGroup("PaintArea");
    QString colorName = TCONFIG->value("GridColor", "#0000b4").toString();
    gridColor = QColor(colorName);
    int separation = TCONFIG->value("GridSeparation", 20).toInt();
    int thickness = TCONFIG->value("GridLineThickness", 1).toInt();

    QGridLayout *gridForm = new QGridLayout;

    gridForm->addWidget(new QLabel(tr("Grid Color:")), 0, 0, Qt::AlignLeft);
    gridColorButton = new QPushButton;
    gridColorButton->setText(gridColor.name());
    gridColorButton->setStyleSheet("* { background-color: " + gridColor.name() + " }");
    connect(gridColorButton, SIGNAL(clicked()), this, SLOT(setGridColor()));
    gridForm->addWidget(gridColorButton, 0, 1, Qt::AlignLeft);

    gridForm->addWidget(new QLabel(tr("Grid Separation:")), 1, 0, Qt::AlignLeft);
    gridSeparation = new QSpinBox(this);
    gridSeparation->setMinimum(5);
    gridSeparation->setMaximum(30);
    gridSeparation->setValue(separation);
    gridForm->addWidget(gridSeparation, 1, 1, Qt::AlignLeft);

    gridForm->addWidget(new QLabel(tr("Grid Line Thickness:")), 2, 0, Qt::AlignLeft);
    gridThickness = new QSpinBox(this);
    gridThickness->setMinimum(1);
    gridThickness->setMaximum(5);
    gridThickness->setValue(thickness);
    gridForm->addWidget(gridThickness, 2, 1, Qt::AlignLeft);

    return gridForm;
}

QGridLayout * TupPaintAreaPreferences::ruleOfThirdsPanel()
{
    TCONFIG->beginGroup("PaintArea");
    QString colorName = TCONFIG->value("ROTColor", "#ff0000").toString();
    rotColor = QColor(colorName);
    int thickness = TCONFIG->value("ROTLineThickness", 1).toInt();

    QGridLayout *rotForm = new QGridLayout;

    rotForm->addWidget(new QLabel(tr("Line Color:")), 0, 0, Qt::AlignLeft);
    rotColorButton = new QPushButton;
    rotColorButton->setText(rotColor.name());
    rotColorButton->setStyleSheet("* { background-color: " + rotColor.name() + "; }");
    connect(rotColorButton, SIGNAL(clicked()), this, SLOT(setRotColor()));
    rotForm->addWidget(rotColorButton, 0, 1, Qt::AlignLeft);

    rotForm->addWidget(new QLabel(tr("Line Thickness:")), 2, 0, Qt::AlignLeft);
    rotThickness = new QSpinBox(this);
    rotThickness->setMinimum(1);
    rotThickness->setMaximum(5);
    rotThickness->setValue(thickness);
    rotForm->addWidget(rotThickness, 2, 1, Qt::AlignLeft);

    return rotForm;
}

QGridLayout * TupPaintAreaPreferences::safeAreaPanel()
{
    TCONFIG->beginGroup("PaintArea");
    QString colorName = TCONFIG->value("SafeAreaRectColor", "#008700").toString();
    safeAreaRectColor = QColor(colorName);
    colorName = TCONFIG->value("SafeAreaLineColor", "#969696").toString();
    safeAreaLineColor = QColor(colorName);
    int thickness = TCONFIG->value("SafeLineThickness", 1).toInt();

    QGridLayout *safeForm = new QGridLayout;

    safeForm->addWidget(new QLabel(tr("Rect Color:")), 0, 0, Qt::AlignLeft);
    safeRectColorButton = new QPushButton;
    safeRectColorButton->setText(safeAreaRectColor.name());
    safeRectColorButton->setStyleSheet("* { background-color: " + safeAreaRectColor.name() + "; }");
    connect(safeRectColorButton, SIGNAL(clicked()), this, SLOT(setSafeAreaRectColor()));
    safeForm->addWidget(safeRectColorButton, 0, 1, Qt::AlignLeft);

    safeForm->addWidget(new QLabel(tr("Line Color:")), 1, 0, Qt::AlignLeft);
    safeLineColorButton = new QPushButton;
    safeLineColorButton->setText(safeAreaLineColor.name());
    safeLineColorButton->setStyleSheet("* { background-color: " + safeAreaLineColor.name() + "; }");
    connect(safeLineColorButton, SIGNAL(clicked()), this, SLOT(setSafeAreaLineColor()));
    safeForm->addWidget(safeLineColorButton, 1, 1, Qt::AlignLeft);

    safeForm->addWidget(new QLabel(tr("Line Thickness:")), 2, 0, Qt::AlignLeft);
    safeThickness = new QSpinBox(this);
    safeThickness->setMinimum(1);
    safeThickness->setMaximum(5);
    safeThickness->setValue(thickness);
    safeForm->addWidget(safeThickness, 2, 1, Qt::AlignLeft);

    return safeForm;
}

void TupPaintAreaPreferences::saveValues()
{
    TCONFIG->beginGroup("PaintArea");

    TCONFIG->setValue("GridColor", gridColor.name());
    TCONFIG->setValue("GridSeparation", gridSeparation->value());
    TCONFIG->setValue("GridLineThickness", gridThickness->value());

    TCONFIG->setValue("ROTColor", rotColor.name());
    TCONFIG->setValue("ROTLineThickness", rotThickness->value());

    TCONFIG->setValue("SafeAreaRectColor", safeAreaRectColor.name());
    TCONFIG->setValue("SafeAreaLineColor", safeAreaLineColor.name());
    TCONFIG->setValue("SafeLineThickness", safeThickness->value());

    TCONFIG->sync();
}

void TupPaintAreaPreferences::restoreValues()
{
    gridColor = QColor("#0000b4");
    gridColorButton->setText(gridColor.name());
    gridColorButton->setStyleSheet("* { background-color: " + gridColor.name() + "; }");
    gridSeparation->setValue(10);
    gridThickness->setValue(1);

    rotColor = QColor("#ff0000");
    rotColorButton->setText(rotColor.name());
    rotColorButton->setStyleSheet("* { background-color: " + rotColor.name() + "; "
                                  " color: #ffffff; }");
    rotThickness->setValue(1);

    safeAreaRectColor = QColor("#008700");
    safeRectColorButton->setText(safeAreaRectColor.name());
    safeRectColorButton->setStyleSheet("* { background-color: " + safeAreaRectColor.name() + "; }");

    safeAreaLineColor = QColor("#969696");
    safeLineColorButton->setText(safeAreaLineColor.name());
    safeLineColorButton->setStyleSheet("* { background-color: " + safeAreaLineColor.name() + "; }");
    safeThickness->setValue(1);
}

void TupPaintAreaPreferences::setGridColor()
{
    gridColor = setButtonColor(gridColorButton, gridColor);
}

void TupPaintAreaPreferences::setRotColor()
{
    rotColor = setButtonColor(rotColorButton, rotColor);
}

void TupPaintAreaPreferences::setSafeAreaRectColor()
{
    safeAreaRectColor = setButtonColor(safeRectColorButton, safeAreaRectColor);
}

void TupPaintAreaPreferences::setSafeAreaLineColor()
{
    safeAreaLineColor = setButtonColor(safeLineColorButton, safeAreaLineColor);
}

QColor TupPaintAreaPreferences::setButtonColor(QPushButton *button, const QColor &currentColor) const
{
     QColor color = QColorDialog::getColor(currentColor);
     if (color.isValid()) {
         button->setText(color.name());
         QString css = "QPushButton { background-color: " + color.name() + " }";
         if (color == Qt::black)
             css = "QPushButton { background-color: " + color.name() + "; color: #ffffff; }";
         button->setStyleSheet(css);
     } else {
         color = currentColor;
     }

     return color;
}
