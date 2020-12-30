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

#include "tupwatermark.h"
#include <QFont>

TupWaterMark::TupWaterMark(QObject *parent) : QObject(parent)
{
}

TupWaterMark::~TupWaterMark()
{
}

QGraphicsTextItem * TupWaterMark::generateWaterMark(const QColor &bgColor, const QSize &size, int zLevel)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupWaterMark::generateWaterMark()]";
        qDebug() << "bgColor: " << bgColor;
        qDebug() << "size: " << size;
    #endif

    int imgW = size.width();
    int imgH = size.height();

    double wLimit;
    if (imgW > imgH)
        wLimit = imgW * (0.2);
    else
        wLimit = imgW * (0.3);

    QColor fgColor = waterMarkColor(bgColor);

    QGraphicsTextItem *watermark = new QGraphicsTextItem("@tupitube");
    watermark->setDefaultTextColor(fgColor);
    QFont font("Paytone One");

    int textWidth = 0;
    int fontSize = 10;
    while (textWidth < wLimit) {
        font.setPointSize(fontSize);
        watermark->setFont(font);
        QRectF rect = watermark->boundingRect();
        textWidth = rect.width();
        fontSize++;
    }

    int x = (imgW - textWidth)/2;
    watermark->setPos(x, -5);
    watermark->setZValue(zLevel);

    return watermark;
}

QColor TupWaterMark::waterMarkColor(const QColor &bgColor)
{
    QColor fgColor(120, 120, 120);

    if (bgColor == Qt::white) {
        fgColor = QColor(180, 180, 180);
    } else {
        if (bgColor == Qt::black) {
            fgColor = QColor(255, 255, 255);
        } else {
            if ((bgColor.green() <= 210) && (bgColor.red() <= 210) && (bgColor.blue() <= 210)) {
                fgColor = QColor(255, 255, 255);
            } else {
                if (bgColor.red() > 210 && bgColor.green() > 210 && bgColor.blue() > 210)
                    fgColor = QColor(180, 180, 180);
                else if (bgColor.red() > 220 && bgColor.green() > 220 && bgColor.blue() < 220)
                    fgColor = QColor(150, 150, 150);
                else if (bgColor.green() > 200 && (bgColor.red() < 200 || bgColor.blue() < 200))
                    fgColor = QColor(120, 120, 120);
                else if (bgColor.red() > 200 && (bgColor.green() < 200 || bgColor.blue() < 200))
                    fgColor = QColor(255, 255, 255);
                else if (bgColor.blue() > 200 && (bgColor.red() < 200 || bgColor.green() < 200))
                    fgColor = QColor(255, 255, 255);
                else if (bgColor.red() < 150 && bgColor.green() < 150 && bgColor.blue() < 150)
                    fgColor = QColor(230, 230, 230);
            }
        }
    }

    return fgColor;
}
