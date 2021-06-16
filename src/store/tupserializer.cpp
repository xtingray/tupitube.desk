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

#include "tupserializer.h"
#include "tupgraphicobject.h"
#include "tupsvg2qt.h"
#include "tuptextitem.h"

#include <QTextDocument>

TupSerializer::TupSerializer()
{
}

TupSerializer::~TupSerializer()
{
}

QDomElement TupSerializer::properties(const QGraphicsItem *item, QDomDocument &doc, const QString &text, int textWidth, Qt::Alignment textAlign)
{
    QDomElement properties = doc.createElement("properties");
    
    QString matrixStr = "matrix(";
    QTransform m = item->transform();
    qreal a = m.m11();
    qreal b = m.m12();
    qreal c = m.m21();
    qreal d = m.m22();
    qreal e = m.dx();
    qreal f = m.dy();
    
    matrixStr += QString::number(a) + "," + QString::number(b) + "," + QString::number(c) + "," + QString::number(d) + "," + QString::number(e) + "," + QString::number(f) + ")" ; 

    properties.setAttribute("transform", matrixStr);
    properties.setAttribute("rotation", item->data(TupGraphicObject::Rotate).toInt());
    double sx = item->data(TupGraphicObject::ScaleX).toDouble();
    properties.setAttribute("scale_x", QString::number(sx));
    double sy = item->data(TupGraphicObject::ScaleY).toDouble();
    properties.setAttribute("scale_y", QString::number(sy));
    properties.setAttribute("pos", "(" + QString::number(item->pos().x()) + "," + QString::number(item->pos().y()) + ")");
    properties.setAttribute("enabled", item->isEnabled());
    properties.setAttribute("flags", item->flags());

    if (textWidth > 0) {
        properties.setAttribute("text_width", textWidth);
        properties.setAttribute("text_alignment", textAlign);
        properties.setAttribute("text", text);
    }

    return properties;
}

void TupSerializer::loadProperties(QGraphicsItem *item, const QXmlAttributes &atts)
{
    QTransform transform;
    TupSvg2Qt::svgmatrix2qtmatrix(atts.value("transform"), transform);
    item->setTransform(transform);

    QPointF pos;
    TupSvg2Qt::parsePointF(atts.value("pos"), pos);

    item->setPos(pos);
    item->setEnabled(atts.value("pos") != "0"); // default true
    item->setFlags(QGraphicsItem::GraphicsItemFlags(atts.value("flags").toInt()));
    item->setData(TupGraphicObject::Rotate, atts.value("rotation").toInt());
    double sx = atts.value("scale_x").toDouble();
    item->setData(TupGraphicObject::ScaleX, sx);
    double sy = atts.value("scale_y").toDouble();
    item->setData(TupGraphicObject::ScaleY, sy);

    if (TupTextItem *textItem = qgraphicsitem_cast<TupTextItem *>(item)) {
        textItem->setTextWidth(atts.value("text_width").toInt());

        Qt::Alignment alignment = Qt::Alignment(atts.value("text_alignment").toInt());
        QTextOption option = textItem->document()->defaultTextOption();
        option.setAlignment(alignment);
        textItem->document()->setDefaultTextOption(option);

        textItem->setData(0, atts.value("text"));
        textItem->setPlainText(atts.value("text"));
    }
}

void TupSerializer::loadProperties(QGraphicsItem *item, const QDomElement &element)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSerializer::loadProperties()]";
    #endif

    if (element.tagName() == "properties") {
        QTransform transform;
        TupSvg2Qt::svgmatrix2qtmatrix(element.attribute("transform"), transform);
        item->setTransform(transform); 

        QPointF pos;
        TupSvg2Qt::parsePointF(element.attribute("pos"), pos);
        #ifdef TUP_DEBUG
            qDebug() << "[TupSerializer::loadProperties()] - pos -> " << pos;
        #endif
        item->setPos(pos);

        item->setEnabled(element.attribute("pos") != "0");
        item->setFlags(QGraphicsItem::GraphicsItemFlags(element.attribute("flags").toInt()));
        item->setData(TupGraphicObject::Rotate, element.attribute("rotation").toInt());
        double sx = element.attribute("scale_x").toDouble();
        item->setData(TupGraphicObject::ScaleX, sx);
        double sy = element.attribute("scale_y").toDouble();
        item->setData(TupGraphicObject::ScaleY, sy);

        if (TupTextItem *textItem = qgraphicsitem_cast<TupTextItem *>(item)) {
            textItem->setTextWidth(element.attribute("text_width").toInt());

            Qt::Alignment alignment = Qt::Alignment(element.attribute("text_alignment").toInt());
            QTextOption option = textItem->document()->defaultTextOption();
            option.setAlignment(alignment);
            textItem->document()->setDefaultTextOption(option);

            textItem->setData(0, element.attribute("text"));
            textItem->setPlainText(element.attribute("text"));
        }
    }
}

QDomElement TupSerializer::gradient(const QGradient *gradient, QDomDocument &doc)
{
    QDomElement element = doc.createElement("gradient");
    element.setAttribute("type", gradient->type());
    element.setAttribute("spread", gradient->spread());
    
    switch (gradient->type()) {
        case QGradient::LinearGradient:
        {
            element.setAttribute("startX", static_cast<const QLinearGradient *>(gradient)->start().x());
            element.setAttribute("startY", static_cast<const QLinearGradient *>(gradient)->start().y());

            element.setAttribute("finalX", static_cast<const QLinearGradient *>(gradient)->finalStop().x());
            element.setAttribute("finalY", static_cast<const QLinearGradient *>(gradient)->finalStop().y());
        }
        break;
        case QGradient::RadialGradient:
        {
            element.setAttribute("centerX", static_cast<const QRadialGradient *>(gradient)->center().x());
            element.setAttribute("centerY", static_cast<const QRadialGradient *>(gradient)->center().y());

            element.setAttribute("focalX", static_cast<const QRadialGradient *>(gradient)->focalPoint().x());
            element.setAttribute("focalY", static_cast<const QRadialGradient *>(gradient)->focalPoint().y());

            element.setAttribute("radius", static_cast<const QRadialGradient *>(gradient)->radius());
        }
        break;
        case QGradient::ConicalGradient:
        {
            element.setAttribute("centerX", static_cast<const QRadialGradient *>(gradient)->center().x());
            element.setAttribute("centerY", static_cast<const QRadialGradient *>(gradient)->center().y());

            element.setAttribute("angle", static_cast<const QConicalGradient *>(gradient)->angle());
        }
        break;
        case QGradient::NoGradient:
        {
        }
        break;
    }

    QGradientStops stops = gradient->stops();

    foreach (QGradientStop stop, stops) {
         QDomElement stopElement = doc.createElement("stop");
         stopElement.setAttribute("value", stop.first);
         stopElement.setAttribute("colorName", stop.second.name());
         stopElement.setAttribute("alpha", stop.second.alpha());
         element.appendChild(stopElement);
    }
    
    return element;
}

QGradient * TupSerializer::createGradient(const QXmlAttributes &atts)
{
    QGradient *result = nullptr;

    switch (atts.value("type").toInt()) {
        case QGradient::LinearGradient:
        {
            result = new QLinearGradient(QPointF(atts.value("startX").toDouble(),
                     atts.value("startY").toDouble()),
                     QPointF(atts.value("finalX").toDouble(),
                     atts.value("finalY").toDouble()));
        }
        break;
        case QGradient::RadialGradient:
        {
            result = new QRadialGradient(
                     QPointF(atts.value("centerX").toDouble(),atts.value("centerY").toDouble()),
                     atts.value("radius").toDouble(),
                     QPointF(atts.value("focalX").toDouble(), atts.value("focalY").toDouble()));
        }
        break;
        case QGradient::ConicalGradient:
        {
            result = new QConicalGradient(QPointF(atts.value("centerX").toDouble(),atts.value("centerY").toDouble()), atts.value("angle").toDouble());
        }
        break;
        case QGradient::NoGradient:
        {
            result = nullptr;
        }
        break;
    }
    
    if (!result)
        return nullptr;

    result->setSpread(QGradient::Spread(atts.value("spread").toInt()));

    return result;
}

QDomElement TupSerializer::brush(const QBrush *brush, QDomDocument &doc)
{
    QDomElement brushElement = doc.createElement("brush");
    
    brushElement.setAttribute("style", brush->style());
    
    if (brush->gradient()) {
        brushElement.appendChild(gradient(brush->gradient() , doc));
    } else if(brush->color().isValid()) {
        brushElement.setAttribute("color", brush->color().name());
        brushElement.setAttribute("alpha", brush->color().alpha());
    }
    
    QString matrixStr = "matrix(";
    QTransform t = brush->transform(); 
    qreal a = t.m11();
    qreal b = t.m12();
    qreal c = t.m21();
    qreal d = t.m22();
    qreal e = t.dx();
    qreal f = t.dy();
    
    matrixStr += QString::number(a) + "," + QString::number(b) + "," + QString::number(c) + "," + QString::number(d) + "," + QString::number(e) + "," + QString::number(f) + ")"; 
    
    brushElement.setAttribute("transform", matrixStr);

    return brushElement;
}

void TupSerializer::loadBrush(QBrush &brush, const QXmlAttributes &atts)
{
    brush.setStyle(Qt::BrushStyle(atts.value("style").toInt()));
    
    if (!atts.value("color").isEmpty()) {
        QColor color(atts.value("color"));
        color.setAlpha(atts.value("alpha").toInt());
        brush.setColor(color);
    } else {
        brush.setColor(Qt::transparent);
    }
    
    QTransform transform;
    TupSvg2Qt::svgmatrix2qtmatrix(atts.value("transform"), transform);
    brush.setTransform(transform);
}

void TupSerializer::loadBrush(QBrush &brush, const QDomElement &element)
{
    brush.setStyle(Qt::BrushStyle(element.attribute("style").toInt()));

    if (!element.attribute("color").isEmpty()) {
        QColor color(element.attribute("color"));
        color.setAlpha(element.attribute("alpha").toInt());
        brush.setColor(color);
    } else {
        brush.setColor(Qt::transparent);
    }

    QTransform transform;
    TupSvg2Qt::svgmatrix2qtmatrix(element.attribute("transform"), transform);
    brush.setTransform(transform);
}

QDomElement TupSerializer::pen(const QPen *pen, QDomDocument &doc)
{
    QDomElement penElement = doc.createElement("pen");
    
    penElement.setAttribute("style", pen->style());
    penElement.setAttribute("color", pen->color().name());
    penElement.setAttribute("alpha", pen->color().alpha());
    penElement.setAttribute("capStyle", pen->capStyle());
    penElement.setAttribute("joinStyle", pen->joinStyle());
    penElement.setAttribute("width", pen->widthF());
    penElement.setAttribute("miterLimit", pen->miterLimit());
    
    QBrush brush = pen->brush();
    penElement.appendChild(TupSerializer::brush(&brush, doc));
    
    return penElement;
}

void TupSerializer::loadPen(QPen &pen, const QXmlAttributes &atts)
{
    pen.setCapStyle(Qt::PenCapStyle(atts.value("capStyle").toInt()));
    pen.setStyle(Qt::PenStyle(atts.value("style").toInt()));
    pen.setJoinStyle(Qt::PenJoinStyle(atts.value("joinStyle").toInt()));
    pen.setWidthF(atts.value("width").toDouble());
    pen.setMiterLimit(atts.value("miterLimit").toInt());
   
    QColor color; 
    QString colorName = atts.value("color");
    if (!colorName.isEmpty()) {
        color = QColor(colorName);
        color.setAlpha(atts.value("alpha").toInt());
    } else {
        color = QColor(Qt::transparent);
    }

    pen.setColor(color);
}

void TupSerializer::loadPen(QPen &pen, const QDomElement &e)
{
    pen.setCapStyle(Qt::PenCapStyle(e.attribute("capStyle").toInt()));
    pen.setStyle(Qt::PenStyle(e.attribute("style").toInt()));
    pen.setJoinStyle(Qt::PenJoinStyle(e.attribute("joinStyle").toInt()));
    pen.setWidthF(e.attribute("width").toDouble());
    pen.setMiterLimit(e.attribute("miterLimit").toInt());

    QDomNode node = e.firstChild();
    QDomElement brushElement = node.toElement();
    QBrush brush; 
    loadBrush(brush, brushElement);

    // pen.setColor(color);
    pen.setBrush(brush);
}

QDomElement TupSerializer::font(const QFont *font, QDomDocument &doc)
{
    QDomElement fontElement = doc.createElement("font");
    
    fontElement.setAttribute("family", font->family());
    fontElement.setAttribute("pointSize", font->pointSize());
    fontElement.setAttribute("weight", font->weight());
    fontElement.setAttribute("italic", font->italic());
    fontElement.setAttribute("bold", font->bold());
    fontElement.setAttribute("style", font->style());
    fontElement.setAttribute("underline", font->underline());
    fontElement.setAttribute("overline", font->overline());
    
    return fontElement;
}

void TupSerializer::loadFont(QFont &font, const QDomElement &e)
{
    font = QFont(e.attribute("family"), e.attribute("pointSize", "-1").toInt(), e.attribute("weight", "-1").toInt(), e.attribute( "italic", "0").toInt());
    
    font.setBold(e.attribute("bold", "0").toInt());
    font.setStyle(QFont::Style(e.attribute("style").toInt()));
    font.setUnderline(e.attribute("underline", "0").toInt());
    font.setOverline(e.attribute("overline", "0").toInt());
}

void TupSerializer::loadFont(QFont &font, const QXmlAttributes &atts)
{
    font = QFont(atts.value("family"), atts.value("pointSize").toInt(), atts.value("weight").toInt(),
                 atts.value("italic").toInt());

    font.setBold(atts.value("bold").toInt());
    font.setStyle(QFont::Style(atts.value("style").toInt()));
    font.setUnderline(atts.value("underline").toInt());
    font.setOverline(atts.value("overline").toInt());
}
