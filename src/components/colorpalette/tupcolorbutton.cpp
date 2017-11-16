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

struct TupColorButton::Private
{
    QBrush brush;
    int index;
    bool editable;
    bool selected;
    QSize size;
    QString themeName;
};

TupColorButton::TupColorButton(int index, const QString &name, const QBrush &brush, const QSize &size,
                               const QString &params) : k(new Private)
{
    k->index = index;
    k->editable = true;
    k->selected = false;
    k->brush = brush;
    k->size = size;

    TCONFIG->beginGroup("General");
    k->themeName = TCONFIG->value("Theme", "Light").toString();

    setToolTip(name);

    QStringList values = params.split(",");
    border1 = values.at(0).toInt();
    border2 = values.at(1).toInt();
    border3 = values.at(2).toInt();

    setFixedSize(k->size);
}

TupColorButton::~TupColorButton()
{
}

QSize TupColorButton::sizeHint() const
{
    return k->size;
}

void TupColorButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.fillRect(rect(), k->brush);

    /*
    if (k->brush.color() == Qt::transparent) { 
        QImage transparent(THEME_DIR + "icons/trans_small.png");
        painter.drawImage(rect().topLeft(), transparent);
    } else {
        painter.fillRect(rect(), k->brush);
    }
    */

    QRect border = rect();
    if (k->selected && k->editable) {
        QColor borderColor1 = QColor(200, 200, 200);
        QColor borderColor2 = QColor(190, 190, 190);
        QColor borderColor3 = QColor(150, 150, 150);
        if (k->themeName.compare("Dark") == 0) {
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
        if (k->brush.color() == Qt::transparent) 
            painter.setPen(QPen(QColor(30, 30, 30), border3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        else
            painter.setPen(QPen(QColor(190, 190, 190), border3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawRect(border);
    }
}

void TupColorButton::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    emit clicked(k->index);
    setState(true);
}

void TupColorButton::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    emit doubledClicked(k->index);
}

QColor TupColorButton::color()
{
    return k->brush.color();
}

void TupColorButton::setState(bool isSelected)
{
    k->selected = isSelected;
    update();
}

bool TupColorButton::isSelected()
{
    return k->selected;
}

void TupColorButton::setBrush(const QBrush &brush)
{
    k->brush = brush;
    update();
}

void TupColorButton::setEditable(bool flag)
{
    k->editable = flag;
}

int TupColorButton::index()
{
    return k->index;
}
