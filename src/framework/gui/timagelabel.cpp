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

#include "timagelabel.h"
#include <QPainter>
#include <QTimer>

TImageLabel::TImageLabel(QWidget *parent, Qt::WindowFlags flags)
{
   Q_UNUSED(parent)
   Q_UNUSED(flags)
}

TImageLabel::TImageLabel(const QString &text, const QColor &color, QWidget *parent, Qt::WindowFlags flags)
{
   Q_UNUSED(parent)
   Q_UNUSED(flags)

   id = text;
   pressed = false;
   markColor = color;
}

TImageLabel::~TImageLabel()
{
}

void TImageLabel::setPixmap(const QPixmap &img)
{
    QLabel::setPixmap(img);
}

void TImageLabel::mousePressEvent(QMouseEvent *)
{
    emit clicked(id);
    activateMark();
}

void TImageLabel::activateMark()
{
    pressed = true;
    update();
    QTimer::singleShot(250, this, SLOT(removeMark()));
}

void TImageLabel::removeMark()
{
    pressed = false;
    update();
}

void TImageLabel::paintEvent(QPaintEvent *event)
{
    // Q_UNUSED(event)

    QLabel::paintEvent(event);
    if (pressed) {
        QPainter dc(this);
        // QColor color(79, 94, 62);
        dc.setPen(markColor);
        dc.drawRect(1, 1, width() - 2, height() - 2);
    }
}
