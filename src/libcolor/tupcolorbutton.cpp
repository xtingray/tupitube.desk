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

#include "tupcolorbutton.h"

#include <QPainter>
#include <QDebug>

TupColorButton::TupColorButton(int colorIndex, const QString &name, const QBrush &colorBrush,
                               const QSize &dimension, const QString &params)
{
    index = colorIndex;
    editable = true;
    selected = false;
    brush = colorBrush;
    size = dimension;

    TCONFIG->beginGroup("General");
    themeName = TCONFIG->value("Theme", "Light").toString();

    setToolTip(name);

    QStringList values = params.split(",");
    border1 = values.at(0).toInt();
    border2 = values.at(1).toInt();
    border3 = values.at(2).toInt();

    setFixedSize(size);
}

TupColorButton::~TupColorButton()
{
}

QSize TupColorButton::sizeHint() const
{
    return size;
}

void TupColorButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.fillRect(rect(), brush);

    QRect border = rect();
    if (selected && editable) {
        QColor borderColor1 = QColor(200, 200, 200);
        QColor borderColor2 = QColor(190, 190, 190);
        QColor borderColor3 = QColor(150, 150, 150);
        if (themeName.compare("Dark") == 0) {
            borderColor1 = QColor(120, 120, 120);
            borderColor2 = QColor(110, 110, 110);
            borderColor3 = QColor(70, 70, 70);
        }

        painter.setPen(QPen(borderColor1, border1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawRect(border);
        painter.setPen(QPen(borderColor2, border2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawRect(border);
        painter.setPen(QPen(borderColor3, border3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawRect(border);
    } else {
        if (brush.color() == Qt::transparent)
            painter.setPen(QPen(QColor(30, 30, 30), border3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        else
            painter.setPen(QPen(QColor(190, 190, 190), border3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawRect(border);
    }
}

void TupColorButton::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    emit clicked(index);
    setState(true);
}

void TupColorButton::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    emit doubledClicked(index);
}

QColor TupColorButton::color()
{
    return brush.color();
}

void TupColorButton::setState(bool isSelected)
{
    selected = isSelected;
    update();
}

bool TupColorButton::isSelected()
{
    return selected;
}

void TupColorButton::setBrush(const QBrush &cBrush)
{
    brush = cBrush;
    update();
}

void TupColorButton::setEditable(bool flag)
{
    editable = flag;
}

int TupColorButton::getIndex()
{
    return index;
}
