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

#include "tupimagedevice.h"

TupImageDevice::TupImageDevice(QWidget *parent) : QWidget(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupImageDevice()]";
    #endif

    image = QImage(300,300, QImage::Format_RGB32);
    image.fill(Qt::white);

    // setAttribute(Qt::WA_PaintOnScreen);
    // setAttribute(Qt::WA_NoSystemBackground);
    // setAttribute(Qt::WA_OpaquePaintEvent);
    // setAutoFillBackground(true);
}

TupImageDevice::~TupImageDevice()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupImageDevice()]";
    #endif
}

QPaintEngine *TupImageDevice::paintEngine() const
{
    // qDebug("TupImageDevice: paint engine *****************************************");

    return QWidget::paintEngine();
}

void TupImageDevice::paintEvent(QPaintEvent *)
{
    // qDebug("TupImageDevice: Paint event #####################################################");
    QPainter p(this);
    p.drawImage(0, 0, image);
}
