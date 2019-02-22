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

#include "tupformitem.h"

#include <QHBoxLayout>
#include <QLabel>

TupFormItem::TupFormItem(const QString &text, QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);

    QLabel *labelText = new QLabel(text);
    value = new QSpinBox;
    value->setMaximum(255);
    value->setMinimum(0);
    connect(value, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
    layout->addWidget(labelText);
    layout->addWidget(value);

    setLayout(layout);
}

TupFormItem::~TupFormItem()
{

}

void TupFormItem::setValue(int input)
{
    value->setValue(input);
}

int TupFormItem::getValue()
{
    return value->value();
}

void TupFormItem::setMax(int max)
{
    value->setMaximum(max);
}

void TupFormItem::setRange(int minimum, int maximum)
{
    value->setRange(minimum, maximum);
}

void TupFormItem::setSuffix(const QString &suffix)
{
    value->setSuffix(suffix);
}

