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

#include "tuppenthicknesswidget.h"

TupPenThicknessWidget::TupPenThicknessWidget(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    opacity = 1.0;
    thickness = 100;
}

TupPenThicknessWidget::~TupPenThicknessWidget()
{
}

void TupPenThicknessWidget::render(int width)
{
    thickness = width;
    update();
}

void TupPenThicknessWidget::render(double transp)
{
    opacity = transp;
    update();
}

void TupPenThicknessWidget::setColor(const QColor c)
{
    color = c;
    update();
}

void TupPenThicknessWidget::setBrush(int index)
{
    brushStyle = index;
    update();
}

void TupPenThicknessWidget::setBrush(const QBrush b)
{
    currentBrush = b;
    brushStyle = -1;
}

QSize TupPenThicknessWidget::minimumSizeHint() const
{
    return QSize(100, 106);
}

QSize TupPenThicknessWidget::sizeHint() const
{
    return QSize(100, 106);
}

void TupPenThicknessWidget::paintEvent(QPaintEvent *)
{
     QPainter painter(this);
     painter.setRenderHint(QPainter::Antialiasing, true);
     painter.fillRect(0, 0, width(), height(), QColor(255, 255, 255));

     QPen border(QColor(0, 0, 0));
     border.setWidth(1);
     painter.setPen(border);
     painter.drawRect(0, 0, width(), height());

     painter.translate(width() / 2, height() / 2);

     QBrush brush;
     Qt::BrushStyle style = Qt::BrushStyle(brushStyle);
     
     if (style != Qt::TexturePattern) {  
         if (brushStyle != -1) {
             brush = QBrush(Qt::BrushStyle(brushStyle));
             brush.setColor(color);
         } else {
             if (currentBrush.gradient()) {
                 brush = currentBrush;
             } else {
                #ifdef TUP_DEBUG
                    qDebug() << "TupPenThicknessWidget::paintEvent() - Warning! NO gradient!";
                #endif
                return;
             }
         }

         QPen pen(Qt::NoPen);
         if (color == Qt::white)
             pen = QPen(QColor(100, 100, 100), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
         painter.setPen(pen);
         painter.setBrush(brush);
         painter.setOpacity(opacity);
         painter.drawEllipse(-(thickness/2), -(thickness/2), thickness, thickness);
     } else {
         QPixmap pixmap(THEME_DIR + "icons/brush_15.png");
         painter.drawPixmap(-(pixmap.width()/2), -(pixmap.height()/2), pixmap);  
     }
}
