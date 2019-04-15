/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez                                                     *
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

#include "tupbrushmanager.h"

TupBrushManager::TupBrushManager(QObject * parent) : QObject(parent)
{
}

TupBrushManager::TupBrushManager(const QPen &qPen, const QBrush &qBrush, QObject * parent) : QObject(parent)
{
    gPen = qPen;
    gBrush = qBrush;
}

TupBrushManager::~TupBrushManager()
{
}

QPen TupBrushManager::pen() const
{
    return gPen;
}

void TupBrushManager::setPen(const QPen &qPen)
{
    gPen = qPen;
    emit penChanged(gPen);
}

void TupBrushManager::setPenColor(const QColor &color)
{
    QBrush brush = gPen.brush();
    brush.setColor(color);
    gPen.setBrush(brush);

    emit penChanged(gPen);
}

void TupBrushManager::setPenWidth(int width)
{
    return gPen.setWidth(width);
}

QBrush TupBrushManager::brush() const
{
    return gBrush;
}

void TupBrushManager::setBrush(const QBrush &brush)
{
    gBrush = brush;

    emit brushChanged(brush);
}

void TupBrushManager::initBgColor(const QColor &color)
{
    penBgColor = color;
}

void TupBrushManager::setBgColor(const QColor &color)
{
    penBgColor = color;

    emit bgColorChanged(color);
}

QColor TupBrushManager::bgColor()
{
    return penBgColor;
}

int TupBrushManager::penWidth() const
{
    return gPen.width();
}

QColor TupBrushManager::penColor() const
{
    return gPen.color();
}

QBrush TupBrushManager::penBrush() const
{
    return gPen.brush();
}

QBrush TupBrushManager::brushColor() const
{
    return gBrush.color();
}
