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

#include "tupitemfactory.h"
#include "tupsvg2qt.h"
#include "tuppathitem.h"
#include "tuppixmapitem.h"
#include "tuptextitem.h"
#include "tupbuttonitem.h"
#include "tuprectitem.h"
#include "tupellipseitem.h"
#include "tuplineitem.h"
#include "tupgraphiclibraryitem.h"
#include "tuplibrary.h"
#include "tupgraphicalgorithm.h"
#include "tupserializer.h"

TupItemFactory::TupItemFactory() : TXmlParserBase()
{
    item = nullptr;
    addToGroup = false;
    isLoading = false;
    library = nullptr;
}

TupItemFactory::~TupItemFactory()
{
}

void TupItemFactory::setLibrary(const TupLibrary *assets)
{
    library = assets;
}

QGraphicsItem* TupItemFactory::createItem(const QString &root)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupItemFactory::createItem()] - root -> " << root;
    #endif
    */

    QGraphicsItem* item = nullptr;
    type = TupItemFactory::Vectorial;

    if (root == "path") {
        item = new TupPathItem;
    } else if (root == "rect") {
        item = new TupRectItem;
    } else if (root == "ellipse") {
        item = new TupEllipseItem;
    } else if (root == "button") {
        item = new TupButtonItem;
    } else if (root == "text") {
        item = new TupTextItem;
    } else if (root == "line") {
        item = new TupLineItem;
    } else if (root == "group") {
        item = new TupItemGroup;
    } else if (root == "symbol") {
        item = new TupGraphicLibraryItem;
        type = TupItemFactory::Library;
    }

    return item;
}

bool TupItemFactory::startTag(const QString& qname, const QXmlAttributes& atts)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupItemFactory::startTag()] - qname -> " << qname;
    #endif

    if (qname == "path") {
        QPainterPath path;
        TupSvg2Qt::svgpath2qtpath(atts.value("coords"), path);

        if (addToGroup) {
            QGraphicsItem *item = createItem(qname);
            qgraphicsitem_cast<TupPathItem *>(item)->setPath(path);
            objects.push(item);
        } else {
            if (!item)
                item = createItem(qname);

            qgraphicsitem_cast<TupPathItem *>(item)->setPath(path);

            objects.push(item);
        }
    } else if (qname == "rect") {
        QRectF rect(atts.value("x").toDouble(), atts.value("y").toDouble(), atts.value("width").toDouble(),
                    atts.value("height").toDouble());

        if (addToGroup) {
            TupRectItem *item = qgraphicsitem_cast<TupRectItem *>(createItem(qname));
            item->setRect(rect);
            objects.push(item);
        } else {
            if (!item)
                item = createItem(qname);
            qgraphicsitem_cast<TupRectItem *>(item)->setRect(rect);
            objects.push(item);
        }
    } else if (qname == "ellipse") {
        QRectF rect(QPointF((atts.value("cx").toDouble() - atts.value("rx").toDouble()),
                    atts.value("cy").toDouble() - atts.value("ry").toDouble()),
                    QSizeF(2 * atts.value("rx").toDouble(), 2 * atts.value("ry").toDouble()));
        // QRectF rect(QPointF(0, 0), QSizeF(2 * atts.value("rx").toDouble(), 2 * atts.value("ry").toDouble()));
        if (addToGroup) {
            TupEllipseItem *item = qgraphicsitem_cast<TupEllipseItem *>(createItem(qname));
            item->setRect(rect);
            objects.push(item);
        } else {
            if (!item)
                item = createItem(qname);

            qgraphicsitem_cast<TupEllipseItem *>(item)->setRect(rect);
            objects.push(item);
        }
    } else if (qname == "button") {
        if (!item) {
            item = createItem(qname);
            objects.push(item);
        }

        if (addToGroup) {
            // groups.last()->addToGroup(createItem(qname));
        }
    } else if (qname == "text") {
        if (addToGroup) {
            TupTextItem *item = qgraphicsitem_cast<TupTextItem *>(createItem(qname));
            objects.push(item);
        } else {
            if (!item)
                item = createItem(qname);

            objects.push(item);
        }
        setReadText(true);
        textReaded = "";
    } else if (qname == "line") {
        QLineF line(atts.value("x1").toDouble(), atts.value("y1").toDouble(), atts.value("x2").toDouble(), atts.value("y2").toDouble());

        if (addToGroup) {
            TupLineItem *item = qgraphicsitem_cast<TupLineItem *>(createItem(qname));
            item->setLine(line);

            objects.push(item);
        } else {
            if (!item)
                item = createItem(qname);

            qgraphicsitem_cast<TupLineItem *>(item)->setLine(line);
            objects.push(item);
        }
    } else if (qname == "group") {
        if (addToGroup) {
            TupItemGroup *group = qgraphicsitem_cast<TupItemGroup *>(createItem(qname));
            groups.push(group);
            objects.push(group);
        } else {
            if (!item)
                item = createItem(qname);
            groups.push(qgraphicsitem_cast<TupItemGroup *>(item));
            objects.push(item);
        }

        addToGroup = true;
    } else if (qname == "symbol") {
        if (addToGroup) {
            TupGraphicLibraryItem *item = qgraphicsitem_cast<TupGraphicLibraryItem *>(createItem(qname));
            QString id = atts.value("id");
            item->setSymbolName(id);
            if (library)
                item->setObject(library->getObject(id));

            objects.push(item);
        } else {
            if (!item)
                item = createItem(qname);

            QString id = atts.value("id");

            qgraphicsitem_cast<TupGraphicLibraryItem *>(item)->setSymbolName(id);

            if (library)
                qgraphicsitem_cast<TupGraphicLibraryItem *>(item)->setObject(library->getObject(id));

            objects.push(item);
        }
    } else if (qname == "properties" && !objects.isEmpty()) {
        TupSerializer::loadProperties(objects.last(), atts);
    } else if (qname == "brush") {
        QBrush brush;
        TupSerializer::loadBrush(brush, atts);

        if (currentTag() == "pen") {
            loading = "pen";
            QPen pen = itemPen();
            pen.setBrush(brush);
            setItemPen(pen);
        } else {
            loading = qname;
            setItemBrush(brush);
        }
    } else if (qname == "pen") {
        QPen pen;
        loading = qname;
        TupSerializer::loadPen(pen, atts);
        setItemPen(pen);
    } else if (qname == "font") {
        QFont font;
        TupSerializer::loadFont(font, atts);

        if (TupTextItem *text = qgraphicsitem_cast<TupTextItem *>(objects.last()))
            text->setFont(font);
    } else if (qname == "stop") {
        if (gradient) {
            QColor c(atts.value("colorName"));
            c.setAlpha(atts.value("alpha").toInt());
            gradient->setColorAt(atts.value("value").toDouble(), c);
        }
    } else if (qname == "gradient") {
        gradient = TupSerializer::createGradient( atts);
    }

    return true;
}

void TupItemFactory::text(const QString &input)
{
    textReaded += input;
}

bool TupItemFactory::endTag(const QString& qname)
{
    /*
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupItemFactory::endTag()] - qname: " << qname;
        #else
            T_FUNCINFOX("items") << qname;
        #endif
    #endif
    */

    if (qname == "path") {
        if (addToGroup)
            groups.last()->addToGroup(objects.last());
        objects.pop();
    } else if (qname == "rect") {
               if (addToGroup)
                   groups.last()->addToGroup(objects.last());
               objects.pop();
    } else if (qname == "ellipse") {
               if (addToGroup)
                   groups.last()->addToGroup(objects.last());
               objects.pop();
    } else if (qname == "symbol") {
               if (addToGroup)
                   groups.last()->addToGroup(objects.last());
               objects.pop();
    } else if (qname == "line") {
               if (addToGroup)
                   groups.last()->addToGroup(objects.last());
               objects.pop();
    } else if (qname == "button") {
               if (addToGroup)
                   groups.last()->addToGroup(objects.last());
               objects.pop();
    } else if (qname == "text") {
               if (addToGroup)
                   groups.last()->addToGroup(objects.last());

               if (TupTextItem *text = qgraphicsitem_cast<TupTextItem *>(objects.last()))
                   text->setPlainText(textReaded);
                   // text->setHtml(textReaded);
               objects.pop();
    } else if (qname == "group") {
               groups.pop();
               addToGroup = !groups.isEmpty();

               if (addToGroup)
                   groups.last()->addToGroup(objects.last());

               objects.pop();
    } else if (qname == "gradient") {
               if (loading == "brush")
                   setItemGradient(*gradient, true);
               else
                   setItemGradient(*gradient, false);
    } else {
               /*
               #ifdef TUP_DEBUG
                   QString msg = "TupItemFactory::endTag() - Unknown tag: " + qname;
                   #ifdef Q_OS_WIN
                       qWarning() << msg;
                   #else
                       tWarning("items") << msg;
                   #endif
               #endif
               */
    }

    return true;
}

void TupItemFactory::setItemPen(const QPen &pen)
{
    if (objects.isEmpty())
        return;

    if (QGraphicsLineItem *line = qgraphicsitem_cast<QGraphicsLineItem *>(objects.last())) {
        line->setPen(pen);
    } else if (QAbstractGraphicsShapeItem *shape = qgraphicsitem_cast<QAbstractGraphicsShapeItem *>(objects.last())) {
               shape->setPen(pen);
    }
}

void TupItemFactory::setItemBrush(const QBrush &brush)
{
    if (objects.isEmpty())
        return;

    if (QGraphicsTextItem *textItem = qgraphicsitem_cast<QGraphicsTextItem *>(objects.last())) {
        textItem->setDefaultTextColor(brush.color());
    } else if (QAbstractGraphicsShapeItem *shape = qgraphicsitem_cast<QAbstractGraphicsShapeItem *>(objects.last()))
        shape->setBrush(brush);
}

void  TupItemFactory::setItemGradient(const QGradient& gradient, bool brush)
{
    if (objects.isEmpty())
        return;

    QBrush gBrush(gradient);

    if (QAbstractGraphicsShapeItem *shape = qgraphicsitem_cast<QAbstractGraphicsShapeItem *>(objects.last())) {
        if (brush) {
            gBrush.setMatrix(shape->brush().matrix());
            shape->setBrush(gBrush);
        } else {
            gBrush.setMatrix(shape->pen().brush().matrix());
            QPen pen = shape->pen();
            pen.setBrush(gBrush);
            shape->setPen(pen);
        }
    } else if (QGraphicsLineItem *line = qgraphicsitem_cast<QGraphicsLineItem *>(objects.last())) {
               gBrush.setMatrix(line->pen().brush().matrix());
               QPen pen = line->pen();
               pen.setBrush(gBrush);
               line->setPen(pen);
    }
}

QPen TupItemFactory::itemPen() const
{
    if (!objects.isEmpty()) {
        if (QGraphicsLineItem *line = qgraphicsitem_cast<QGraphicsLineItem *>(objects.last())) {
            return line->pen();
        } else if (QAbstractGraphicsShapeItem *shape = qgraphicsitem_cast<QAbstractGraphicsShapeItem *>(objects.last())) {
                   return shape->pen();
        }
    }

    return QPen(Qt::transparent, 1);
}

QBrush TupItemFactory::itemBrush() const
{
    if (! objects.isEmpty()) {
        if (QAbstractGraphicsShapeItem *shape = qgraphicsitem_cast<QAbstractGraphicsShapeItem *>(objects.last()))
            return shape->brush();
    }

    return Qt::transparent;
}

bool TupItemFactory::loadItem(QGraphicsItem *object, const QString &xml)
{
    item = object;
    isLoading = true;
    bool ok = parse(xml);
    isLoading = false;

    return ok;
}

QGraphicsItem *TupItemFactory::create(const QString &xml)
{
    if (loadItem(nullptr, xml))
        return item;

    return nullptr;
}

QString TupItemFactory::itemID(const QString &xml)
{
    QDomDocument document;

    if (!document.setContent(xml))
        return "item";

    QDomElement root = document.documentElement();
    QString id = root.attribute("id");

    if (id.length() > 0)
        return id;

    return "item";
}

TupItemFactory::Type TupItemFactory::getType() {
    return type;
}
