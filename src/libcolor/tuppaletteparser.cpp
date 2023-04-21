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

#include "tuppaletteparser.h"

TupPaletteParser::TupPaletteParser(QIODevice *device): QXmlStreamReader(device)
{
     paletteName = "";
     isEditable = false;
     gradient = nullptr;
}

TupPaletteParser::~TupPaletteParser()
{
}

bool TupPaletteParser::processPalette()
{
    if (readNextStartElement()) {
         if (name() == QString("Palette")){
            paletteName = attributes().value("name").toString();

            isEditable = false;
            if (attributes().value("editable").toString() == "true")
                isEditable = true;

            while (readNextStartElement()) {
                if (name() == QString("Color")) {
                    QColor c = QColor(attributes().value("colorName"));
                    c.setAlpha(attributes().value("alpha").toInt());

                    if (c.isValid()) {
                        brushes << c;
                    } else {
                        #ifdef TUP_DEBUG
                           qDebug() << "[TupPaletteParser::processInput()] - Error: Invalid color!";
                        #endif
                        return false;
                    }
                } else if (name() == QString("Gradient")) {
                    if (gradient)
                        delete gradient;

                    gradient = 0;
                    gradientStops.clear();

                    QGradient::Type type = QGradient::Type(attributes().value("type").toInt());
                    QGradient::Spread spread = QGradient::Spread(attributes().value("spread").toInt());

                    switch (type) {
                       case QGradient::LinearGradient:
                           {
                               gradient = new QLinearGradient(attributes().value("startX").toDouble(),
                                              attributes().value("startY").toDouble(), attributes().value("finalX").toDouble(),
                                              attributes().value("finalY").toDouble());
                           }
                       break;
                       case QGradient::RadialGradient:
                           {
                               gradient = new QRadialGradient(attributes().value("centerX").toDouble(),
                                              attributes().value("centerY").toDouble(), attributes().value("radius").toDouble(),
                                              attributes().value("focalX").toDouble(), attributes().value("focalY").toDouble());
                           }
                       break;
                       case QGradient::ConicalGradient:
                           {
                               gradient = new QConicalGradient(attributes().value("centerX").toDouble(),
                                              attributes().value("centerY").toDouble(), attributes().value("angle").toDouble());
                           }
                       break;
                       default:
                           {
                               #ifdef TUP_DEBUG
                                   qDebug() << "[TupPaletteParser::processInput()] - No gradient type: " << QString::number(type);
                               #endif
                               return false;
                           }
                    }

                    gradient->setSpread(spread);
                } else if (name() == QString("Stop")) {
                    QColor c(attributes().value("colorName") );
                    c.setAlpha(attributes().value("alpha").toInt());
                    gradientStops << qMakePair((qreal)(attributes().value("value").toDouble()), c);
                }
                readElementText();
            }

            if (gradient) {
                gradient->setStops(gradientStops);
                brushes << *gradient;
                gradientStops.clear();
            }
        }
    }

    return true;
}

QList<QBrush> TupPaletteParser::getBrushes() const
{
    return brushes;
}

QString TupPaletteParser::getPaletteName() const
{
    return paletteName;
}

bool TupPaletteParser::paletteIsEditable() const
{
    return isEditable;
}
