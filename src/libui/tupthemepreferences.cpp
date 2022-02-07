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
    QString bgColor = TCONFIG->value("BgColor", "#a0a0a0").toString();
    currentColor = QColor(bgColor);

    QHBoxLayout *layout = new QHBoxLayout(this);

    QVBoxLayout *blockLayout = new QVBoxLayout;

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

    blockLayout->addWidget(widget);
    blockLayout->setAlignment(widget, Qt::AlignLeft);
    blockLayout->addWidget(bgLabel);

    QList<QColor> initList;
    initList << QColor(160, 160 ,160) << QColor(150, 139, 139) << QColor(156, 144, 129);
    QList<QColor> endList;
    endList << Qt::white << Qt::white << Qt::white;

    for(int i=0; i<3; i++)
        blockLayout->addLayout(addColorEntry(i, initList.at(i), endList.at(i)));

    blockLayout->addStretch(3);

    layout->addLayout(blockLayout);
    layout->addStretch();
}

QHBoxLayout * TupThemePreferences::addColorEntry(int id, const QColor &initColor, const QColor &endColor)
{
    TRadioButton *colorRadio = new TRadioButton(id, "", this);
    radioList << colorRadio;
    connect(colorRadio, SIGNAL(clicked(int)), this, SLOT(updateCurrentRow(int)));

    QSize cellSize(30, 30);
    QBrush cellBrush(initColor);
    TupColorButton *colorCell = new TupColorButton(1, "", cellBrush, cellSize, "6,4,2");
    colorCell->setEditable(false);
    cellList << colorCell;

    TSlider *colorSlider = new TSlider(Qt::Horizontal, TSlider::Color, initColor, endColor);
    colorSlider->setRange(0, 100);
    colorSlider->setValue(100);
    sliderList << colorSlider;
    connect(colorSlider, SIGNAL(colorChanged(const QColor&)), this, SLOT(updateCurrentColor(const QColor&)));

    QHBoxLayout *blackLayout = new QHBoxLayout;
    blackLayout->addWidget(colorRadio);
    blackLayout->addSpacing(5);
    blackLayout->addWidget(colorCell);
    blackLayout->addSpacing(5);
    blackLayout->addWidget(colorSlider);
    blackLayout->addStretch();

    bool flag = false;
    if (id == currentRow)
        flag = true;

    colorRadio->setChecked(flag);
    colorSlider->setEnabled(flag);

    return blackLayout;
}

void TupThemePreferences::saveValues()
{       
    TCONFIG->beginGroup("Theme");
    TCONFIG->setValue("ColorRow", currentRow);
    TCONFIG->value("BgColor", currentColor.name());
    TCONFIG->sync();
}

void TupThemePreferences::showRestartMsg(bool enabled)
{
    if (enabled)
        TOsd::self()->display(TOsd::Warning, tr("Please restart TupiTube"));
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
}

void TupThemePreferences::updateCurrentColor(const QColor &color)
{
    currentColor = color;
    cellList.at(currentRow)->setBrush(QBrush(color));
    emit colorPicked(color);
}
