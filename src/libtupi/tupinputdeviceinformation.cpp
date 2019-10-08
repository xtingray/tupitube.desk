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

#include "tupinputdeviceinformation.h"

// Mouse/tablet events manager

TupInputDeviceInformation::TupInputDeviceInformation(QObject *parent) : QObject(parent)
{
    mouseInfo.button = Qt::NoButton;
    mouseInfo.buttons = Qt::NoButton;
    
    tabletInfo.pressure = -1;
    tabletInfo.rotation = 0;
    tabletInfo.tangentialPressure = -1;
    
    modifiers = Qt::NoModifier;
}

TupInputDeviceInformation::~TupInputDeviceInformation()
{
}

void TupInputDeviceInformation::updateFromMouseEvent(QGraphicsSceneMouseEvent *event)
{
    mouseInfo.button = event->button();
    mouseInfo.buttons = event->buttons();
    
    position = event->scenePos();
    
    modifiers = event->modifiers();
    
    tabletInfo.pressure = -1;
    tabletInfo.rotation = 0;
    tabletInfo.tangentialPressure = -1;
}

void TupInputDeviceInformation::updateFromMouseEvent(QMouseEvent *event)
{
    mouseInfo.button = event->button();
    mouseInfo.buttons = event->buttons();
    
    position = event->pos();
    
    modifiers = event->modifiers();
    
    tabletInfo.pressure = -1;
    tabletInfo.rotation = 0;
    tabletInfo.tangentialPressure = -1;
}

void TupInputDeviceInformation::updateFromTabletEvent(QTabletEvent *event)
{
    // qDebug() << "TupInputDeviceInformation::updateFromTabletEvent() - Pressure: " << event->pressure();

    tabletInfo.pressure = event->pressure();
    tabletInfo.rotation = event->rotation();
    tabletInfo.tangentialPressure = event->tangentialPressure();
    
    position = event->pos();
    
    modifiers = event->modifiers();
}

double TupInputDeviceInformation::pressure() const
{
    //tError() << "TupInputDeviceInformation::pressure() - Testing pressure: " << tabletInfo.pressure;
    return tabletInfo.pressure;
}

double TupInputDeviceInformation::rotation() const
{
    return tabletInfo.rotation;
}

double TupInputDeviceInformation::tangentialPressure() const
{
    return tabletInfo.tangentialPressure;
}

Qt::MouseButton TupInputDeviceInformation::button() const
{
    return mouseInfo.button;
}

Qt::MouseButtons TupInputDeviceInformation::buttons() const
{
    return mouseInfo.buttons;
}

QPointF TupInputDeviceInformation::pos() const
{
    return position;
}

Qt::KeyboardModifiers TupInputDeviceInformation::keyModifiers() const
{
    return modifiers;
}
