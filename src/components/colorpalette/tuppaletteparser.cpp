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

TupPaletteParser::TupPaletteParser(): TXmlParserBase()
{
     paletteName = "";
     isEditable = false;
     gradient = nullptr;
}

TupPaletteParser::~TupPaletteParser()
{
}

bool TupPaletteParser::startTag(const QString &tag, const QXmlAttributes &atts)
{
    if (root() == "Palette") {
        if (tag == root()) {
            paletteName = atts.value("name");
            if (atts.value("editable") == "true")
                isEditable = true;
            else
                isEditable = false;
        } else if (tag == "Color") {
            QColor c = QColor(atts.value("colorName"));
            c.setAlpha( atts.value("alpha").toInt() );

            if (c.isValid()) {
                brushes << c;
            } else {
                #ifdef TUP_DEBUG
                   QString msg = "TupPaletteParser::startTag() - Error: Invalid color!";
                   #ifdef Q_OS_WIN
                       qDebug() << msg;
                   #else
                       tError() << msg;
                   #endif
                #endif
            }
        } else if (tag == "Gradient") {
            if (gradient)
                delete gradient;

            gradient = 0;
            gradientStops.clear();

            QGradient::Type type = QGradient::Type(atts.value("type").toInt());
            QGradient::Spread spread = QGradient::Spread(atts.value("spread").toInt());

            switch (type) {
               case QGradient::LinearGradient:
                   {
                       gradient = new QLinearGradient(atts.value("startX").toDouble(),
                                      atts.value("startY").toDouble(),atts.value("finalX").toDouble(),
                                      atts.value("finalY").toDouble());
                   }
               break;
               case QGradient::RadialGradient:
                   {
                       gradient = new QRadialGradient(atts.value("centerX").toDouble(),
                                      atts.value("centerY").toDouble(), atts.value("radius").toDouble(),
                                      atts.value("focalX").toDouble(),atts.value("focalY").toDouble() );
                   }
               break;
               case QGradient::ConicalGradient:
                   {
                       gradient = new QConicalGradient(atts.value("centerX").toDouble(),
                                      atts.value("centerY").toDouble(),atts.value("angle").toDouble());
                   }
               break;
               default:
                   {
                       #ifdef TUP_DEBUG
                           QString msg = "TupPaletteParser::startTag() - No gradient type: " + QString::number(type);
                           #ifdef Q_OS_WIN
                               qDebug() << msg;
                           #else
                               tFatal() << msg;
                           #endif
                       #endif
                   }
            }
            gradient->setSpread(spread);
        } else if (tag == "Stop") {
            QColor c(atts.value("colorName") );
            c.setAlpha(atts.value("alpha").toInt());
            // gradientStops << qMakePair(atts.value("value").toDouble(), c);
            gradientStops << qMakePair((qreal)(atts.value("value").toDouble()), c);
        }
     }

     return true;
}

bool TupPaletteParser::endTag(const QString& tag)
{
    if (root() == "Palette") {
        if (tag == "Gradient" && gradient) {
            gradient->setStops(gradientStops);
            brushes << *gradient;
            gradientStops.clear();
        }
    }

    return true;
}

void TupPaletteParser::text(const QString& )
{

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
