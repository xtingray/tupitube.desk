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

#include "tupcolorbuttonpanel.h"
#include "tupcolorbutton.h"

#include <QBoxLayout>
#include <QDebug>

struct TupColorButtonPanel::Private
{
    QList<TupColorButton *> baseColors;
    Qt::BrushStyle style;
    int currentColorIndex;
    TupColorButton *trans;
};

TupColorButtonPanel::TupColorButtonPanel(Qt::BrushStyle style, const QSize &baseColorCellSize, int spacing,
                                         const QString &buttonParams, QWidget *parent) : QWidget(parent), k(new Private)
{
    k->style = style;
    k->currentColorIndex = -1;
    this->spacing = spacing;

    setPanel(baseColorCellSize, buttonParams);
}

void TupColorButtonPanel::setPanel(const QSize &cellSize, const QString &buttonParams)
{
    QBrush transBrush(Qt::transparent, k->style);
    k->trans = new TupColorButton(0, tr("Transparent"), transBrush, cellSize, buttonParams);
    connect(k->trans, &TupColorButton::clicked, this, &TupColorButtonPanel::updateSelection);
    k->baseColors << k->trans;

    QColor blackColor(0, 0, 0);
    QBrush blackBrush(blackColor, k->style);
    TupColorButton *black = new TupColorButton(1, tr("Black"), blackBrush, cellSize, buttonParams);
    connect(black, &TupColorButton::clicked, this, &TupColorButtonPanel::updateSelection);
    k->baseColors << black;

    QColor whiteColor(255, 255, 255);
    QBrush whiteBrush(whiteColor, k->style);
    TupColorButton *white = new TupColorButton(2, tr("White"), whiteBrush, cellSize, buttonParams);
    connect(white, &TupColorButton::clicked, this, &TupColorButtonPanel::updateSelection);
    k->baseColors << white;

    QColor redColor(255, 0, 0);
    QBrush redBrush(redColor, k->style);
    TupColorButton *red = new TupColorButton(3, tr("Red"), redBrush, cellSize, buttonParams);
    connect(red, &TupColorButton::clicked, this, &TupColorButtonPanel::updateSelection);
    k->baseColors << red;

    QColor greenColor(0, 255, 0);
    QBrush greenBrush(greenColor, k->style);
    TupColorButton *green = new TupColorButton(4, tr("Green"), greenBrush, cellSize, buttonParams);
    connect(green, &TupColorButton::clicked, this, &TupColorButtonPanel::updateSelection);
    k->baseColors << green;

    QColor blueColor(0, 0, 255);
    QBrush blueBrush(blueColor, k->style);
    TupColorButton *blue = new TupColorButton(5, tr("Blue"), blueBrush, cellSize, buttonParams);
    connect(blue, &TupColorButton::clicked, this, &TupColorButtonPanel::updateSelection);
    k->baseColors << blue;

    QBoxLayout *bottomLayout = new QHBoxLayout(this);
    bottomLayout->setAlignment(Qt::AlignHCenter);
    bottomLayout->setContentsMargins(3, 5, 3, 3);

    bottomLayout->setSpacing(spacing);

    bottomLayout->addWidget(k->trans);
    bottomLayout->addWidget(black);
    bottomLayout->addWidget(white);
    bottomLayout->addWidget(red);
    bottomLayout->addWidget(green);
    bottomLayout->addWidget(blue);
}

TupColorButtonPanel::~TupColorButtonPanel()
{
}

void TupColorButtonPanel::updateSelection(int index)
{
    if (index != k->currentColorIndex) {
        if (k->currentColorIndex >= 0) {
            TupColorButton *button = (TupColorButton *) k->baseColors.at(k->currentColorIndex);
            button->setState(false);
        }

        TupColorButton *selection = (TupColorButton *) k->baseColors.at(index);
        QColor color = selection->color();
        k->currentColorIndex = index;

        emit clickColor(color);
    }
}

void TupColorButtonPanel::setState(int index, bool isSelected)
{
    if (index != k->currentColorIndex && k->currentColorIndex >= 0) {
        TupColorButton *button = (TupColorButton *) k->baseColors.at(k->currentColorIndex);
        button->setState(isSelected);
    }
}

void TupColorButtonPanel::resetPanel()
{
    if (k->currentColorIndex >= 0) {
        TupColorButton *button = (TupColorButton *) k->baseColors.at(k->currentColorIndex);
        button->setState(false);
        k->currentColorIndex = -1; 
    }
}

void TupColorButtonPanel::enableTransparentColor(bool flag)
{
    k->trans->setVisible(flag);
}
