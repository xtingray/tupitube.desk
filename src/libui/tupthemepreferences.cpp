/***************************************************************************
 *   Project TUPITUBE DESK                                                 *
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

#include "tupthemepreferences.h"
#include "tosd.h"
#include "tslider.h"
#include "tupcolorbutton.h"
#include "tradiobutton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

TupThemePreferences::TupThemePreferences(QWidget *parent) : QWidget(parent)
{
    setupPage();
}

TupThemePreferences::~TupThemePreferences()
{
}

void TupThemePreferences::setupPage()
{
    TCONFIG->beginGroup("Theme");
    currentRow = TCONFIG->value("ColorRow", 0).toInt();
    oldRow = currentRow;
    QString bgColor = TCONFIG->value("BgColor", "#a0a0a0").toString();
    currentColor = QColor(bgColor);
    oldColor = currentColor;
    colorPos = TCONFIG->value("ColorPos", 0).toInt();

    themeChanged = false;

    QHBoxLayout *layout = new QHBoxLayout(this);
    QWidget *widget = new QWidget;
    QVBoxLayout *pageLayout = new QVBoxLayout;

    QLabel *generalLabel = new QLabel(tr("Theme Preferences"));
    QFont labelFont = font();
    labelFont.setBold(true);
    labelFont.setPointSize(labelFont.pointSize() + 3);
    generalLabel->setFont(labelFont);
    pageLayout->addWidget(generalLabel);
    pageLayout->addSpacing(15);
    widget->setLayout(pageLayout);

    QLabel *bgLabel = new QLabel(tr("Background Color"));
    labelFont = font();
    labelFont.setBold(true);
    bgLabel->setFont(labelFont);

    QVBoxLayout *blockLayout = new QVBoxLayout;
    blockLayout->addWidget(widget);
    blockLayout->setAlignment(widget, Qt::AlignLeft);
    blockLayout->addWidget(bgLabel);

    QStringList labelList;
    labelList << tr("Gray") << tr("Brown") << tr("Chocolate") << tr("Blue")
              << tr("Honey") << tr("Green") << tr("Violet") << tr("Orange");
    QList<QColor> initList;
    initList << QColor(160, 160, 160) << QColor(150, 139, 139) << QColor(156, 144, 129)
             << QColor(132, 203, 238) << QColor(255, 221, 154) << QColor(149, 184, 140)
             << QColor(238, 196, 206) << QColor(247, 205, 163);

    QWidget *formWidget = new QWidget;
    formLayout = new QGridLayout(formWidget);
    for(int i=0; i<labelList.size(); i++)
        addColorEntry(i, labelList.at(i), initList.at(i), Qt::white);

    blockLayout->addWidget(formWidget);
    blockLayout->addStretch(3);

    layout->addLayout(blockLayout);
    layout->addStretch();
}

void TupThemePreferences::addColorEntry(int id, const QString &label, const QColor &initColor, const QColor &endColor)
{
    TRadioButton *colorRadio = new TRadioButton(id, label, this);
    radioList << colorRadio;
    connect(colorRadio, SIGNAL(clicked(int)), this, SLOT(updateCurrentRow(int)));

    QSize cellSize(30, 30);
    QBrush cellBrush(initColor);
    TupColorButton *colorCell = new TupColorButton(1, "", cellBrush, cellSize, "6,4,10");
    colorCell->setEditable(false);
    cellList << colorCell;

    TSlider *colorSlider = new TSlider(Qt::Horizontal, TSlider::Color, initColor, endColor);
    colorSlider->setRange(0, 100);
    sliderList << colorSlider;
    connect(colorSlider, SIGNAL(colorChanged(const QColor&)), this, SLOT(updateCurrentColor(const QColor&)));

    formLayout->addWidget(colorRadio, id, 0, Qt::AlignLeft);
    formLayout->addWidget(colorCell, id, 1, Qt::AlignCenter);
    formLayout->addWidget(colorSlider, id, 2, Qt::AlignCenter);

    bool flag = false;
    if (id == currentRow) {
        flag = true;
        colorCell->setBrush(QBrush(currentColor));
        colorSlider->setValue(colorPos);
    }

    colorRadio->setChecked(flag);
    colorSlider->setEnabled(flag);
}

void TupThemePreferences::saveValues()
{
    if ((oldRow != currentRow) || (oldColor != currentColor)) {
        TCONFIG->beginGroup("Theme");
        TCONFIG->setValue("ColorRow", currentRow);
        TCONFIG->setValue("BgColor", currentColor.name());
        TCONFIG->setValue("ColorPos", colorPos);
        TCONFIG->sync();

        themeChanged = true;
    }
}

bool TupThemePreferences::showWarning()
{
    return themeChanged;
}

void TupThemePreferences::updateCurrentRow(int row)
{
    currentRow = row;
    for(int i=0; i<sliderList.size(); i++) {
        bool flag = true;
        if (i != currentRow)
            flag = false;

        sliderList.at(i)->setEnabled(flag);
    }

    currentColor = cellList.at(currentRow)->color();
    colorPos = sliderList.at(currentRow)->currentValue();
}

void TupThemePreferences::updateCurrentColor(const QColor &color)
{
    /*
    #ifdef TUP_DEBUG
        int r = color.red();
        int g = color.green();
        int b = color.blue();
        qDebug() << "[TupThemePreferences::updateCurrentColor()] - color -> " << r << "," << g << "," << b;
        qDebug() << "[TupThemePreferences::updateCurrentColor()] - slider value -> " << sliderList.at(currentRow)->currentValue();
    #endif
    */

    currentColor = color;
    colorPos = sliderList.at(currentRow)->currentValue();
    cellList.at(currentRow)->setBrush(QBrush(color));

    emit colorPicked(color);
}
