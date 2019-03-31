/***************************************************************************
 *   Project TUPITUBE DESK                                                 *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *                                                                         *
 *   Developers:                                                           *
 *   2019:                                                                 *
 *    Alejandro Carrasco Rodríguez                                         *
 *   2012:                                                                 *
 *    Andres Calderon / @andresfcalderon                                   *
 *    Antonio Vanegas / @hpsaturn                                          *
 *   2010:                                                                 *
 *    Gustavo Gonzalez / xtingray                                          *
 *                                                                         *
 *   TupiTube Desk is a fork of the KTooN project                          *
 *   KTooN's versions:                                                     *
 *   2006:                                                                 *
 *    David Cuadrado                                                       *
 *    Jorge Cuadrado                                                       *
 *   2003:                                                                 *
 *    Fernado Roldan                                                       *
 *    Simena Dinas                                                         *
 *                                                                         *
 *   Copyright (C) 2012 Mae Floresta - http://www.maefloresta.com          *
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

#include "tcolorcell.h"
#include "tconfig.h"
// #include "tapplicationproperties.h"

TColorCell::TColorCell(FillType typeIndex, const QBrush &b, const QSize &dimension)
{
    TCONFIG->beginGroup("General");
    themeName = TCONFIG->value("Theme", "Light").toString();

    index = typeIndex;
    enabled = true;
    checked = false;
    cellBrush = b;
    size = dimension;
    setFixedSize(size);
}

TColorCell::~TColorCell()
{
}

QSize TColorCell::sizeHint() const 
{
    return size;
}

void TColorCell::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.fillRect(rect(), cellBrush);
    
    /*
    if (k-brush.color() == Qt::transparent) {
        QImage transparent(THEME_DIR + "icons/trans_big.png");
        painter.drawImage(rect().topLeft(), transparent);
    } else {
        painter.fillRect(rect(), k-brush);
    }
    */

    QRect border = rect();
    if (enabled) {
        if (checked) {
            QColor borderColor1 = QColor(200, 200, 200); 
            QColor borderColor2 = QColor(190, 190, 190);
            QColor borderColor3 = QColor(150, 150, 150);
            
            if (themeName.compare("Dark") == 0) {
                borderColor1 = QColor(120, 120, 120);
                borderColor2 = QColor(110, 110, 110);
                borderColor3 = QColor(70, 70, 70);
            }

            painter.setPen(QPen(borderColor1, 8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.drawRect(border);
            painter.setPen(QPen(borderColor2, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.drawRect(border);
            painter.setPen(QPen(borderColor3, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.drawRect(border);
        } else {
            QRect frame = QRect(border.topLeft(), QSize(size.width()-1, size.height()-1));
            if (cellBrush.color() == Qt::transparent)
                painter.setPen(QPen(QColor(30, 30, 30), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            else
                painter.setPen(QPen(QColor(190, 190, 190), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.drawRect(frame);
        }
    } else {
        QRect frame = QRect(border.topLeft(), QSize(size.width()-1, size.height()-1));
        painter.setPen(QPen(QColor(190, 190, 190), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawRect(frame);
    }
}

void TColorCell::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    click();
}

void TColorCell::click()
{
    if (enabled) {
        setChecked(true);
        emit clicked(index);
    }
}

void TColorCell::setEnabled(bool isEnabled)
{
    enabled = isEnabled;
    update();
}

QColor TColorCell::color()
{
    return cellBrush.color();
}

QBrush TColorCell::brush()
{
    return cellBrush;
}

void TColorCell::setChecked(bool isChecked)
{
    checked = isChecked;
    update();
}

bool TColorCell::isChecked()
{
    return checked;
}

void TColorCell::setBrush(const QBrush &b) 
{
    cellBrush = b;
    update();
}
