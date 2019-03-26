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

#include "tdualcolorbutton.h"

#include <QMouseEvent>
#include <QPixmap>
#include <QPaintEvent>
#include <QBrush>
#include <qdrawutil.h>
#include <QPainter>

#include "tcolorarrow.xpm"
#include "tcolorreset.xpm"

TDualColorButton::TDualColorButton(QWidget *parent) : QWidget(parent)
{
    arrowBitmap = QPixmap((const char **)dcolorarrow_bits);
    resetPixmap = QPixmap((const char **)dcolorreset_xpm);

    fgBrush = QBrush(Qt::black, Qt::SolidPattern);
    bgBrush = QBrush(QColor(0, 0, 0, 0), Qt::SolidPattern);
	
    currentSpace = Foreground;

    if (sizeHint().isValid())
        setMinimumSize(sizeHint());
}

TDualColorButton::TDualColorButton(const QBrush &fgColor, const QBrush &bgColor, QWidget *parent) : QWidget(parent)
{
    arrowBitmap = QPixmap((const char **)dcolorarrow_bits);
    resetPixmap = QPixmap((const char **)dcolorreset_xpm);

    fgBrush = fgColor;
    bgBrush = bgColor;

    currentSpace = Foreground;

    if (sizeHint().isValid())
        setMinimumSize(sizeHint());
}

TDualColorButton::~TDualColorButton()
{
}

QBrush TDualColorButton::foreground() const
{
    return fgBrush;
}

QBrush TDualColorButton::background() const
{
    return bgBrush;
}

TDualColorButton::ColorSpace TDualColorButton::current() const
{
    return currentSpace;
}

QBrush TDualColorButton::currentColor() const
{
    return (currentSpace == Background ? bgBrush : fgBrush);
}

QSize TDualColorButton::sizeHint() const
{
    return QSize(34, 34);
}

void TDualColorButton::setForeground(const QBrush &c)
{
    fgBrush = c;
    update();
}

void TDualColorButton::setBackground(const QBrush &c)
{
    bgBrush = c;
    update();
}

void TDualColorButton::setCurrentColor(const QBrush &c)
{
    if (currentSpace == Background)
        bgBrush = c;
    else
        fgBrush = c;

    update();
}

void TDualColorButton::setCurrent(ColorSpace s)
{
    currentSpace = s;
    update();
}

void TDualColorButton::metrics(QRect &fgRect, QRect &bgRect)
{
    fgRect = QRect(0, 0, width()-14, height()-14);
    bgRect = QRect(14, 14, width()-14, height()-14);
}

void TDualColorButton::paintEvent(QPaintEvent *)
{
    QPalette pal = palette();
    QPainter painter(this);

    QRect fgRect, bgRect;
    metrics(fgRect, bgRect);

    QBrush defBrush = pal.color(QPalette::Button);

    QBrush bgAdjusted = bgBrush;
    QBrush fgAdjusted = fgBrush;

    qDrawShadeRect(&painter, bgRect, pal, currentSpace == Background, 2, 0, isEnabled() ? &bgAdjusted: &defBrush);
    qDrawShadeRect(&painter, fgRect,  pal, currentSpace == Foreground, 2, 0, isEnabled() ? &fgAdjusted : &defBrush);

    painter.setPen(QPen(palette().shadow().color()));
    painter.drawPixmap(fgRect.right() + 2, 0, arrowBitmap);
    painter.drawPixmap(0, fgRect.bottom() + 2, resetPixmap);
}

void TDualColorButton::mousePressEvent(QMouseEvent *event)
{
    QPoint mPos = event->pos();

    QRect fgRect, bgRect;
    metrics(fgRect, bgRect);

    if (fgRect.contains(mPos)) {
        currentSpace = Foreground;
        emit selectionChanged(Foreground);
    } else if (bgRect.contains(mPos)) {
        currentSpace = Background;
        emit selectionChanged(Background);
    } else if (event->pos().x() > fgRect.width()) {
        QBrush tmpBrush = fgBrush;
        fgBrush = bgBrush;
        bgBrush = tmpBrush;
        emit switchColors();
    } else if (event->pos().x() < bgRect.x()) {
        fgBrush.setColor(Qt::black);
        bgBrush.setColor(QColor(0,0,0,0));
        emit resetColors();
    }

    update();
}
