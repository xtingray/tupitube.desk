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

#include "tuprectitem.h"
#include "tupgraphicalgorithm.h"
#include "tupserializer.h"

TupRectItem::TupRectItem(QGraphicsItem *parent) : QGraphicsRectItem(parent)
{
    setAcceptDrops(true);
}

TupRectItem::TupRectItem(const QRectF& rect, QGraphicsItem * parent) : QGraphicsRectItem(rect, parent)
{
}

TupRectItem::~TupRectItem()
{
}

void TupRectItem::fromXml(const QString &xml)
{
    Q_UNUSED(xml)
}

QDomElement TupRectItem::toXml(QDomDocument &doc) const
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "TupRectItem::toXml() - doc -> " << doc.toString();
    #endif
    */

    QDomElement root = doc.createElement("rect");
    
    root.setAttribute("x", QString::number(rect().x()));
    root.setAttribute("y", QString::number(rect().y()));
    root.setAttribute("width", QString::number(rect().width()));
    root.setAttribute("height", QString::number(rect().height()));
    
    root.appendChild(TupSerializer::properties(this, doc));
    
    QBrush brush = this->brush();
    root.appendChild(TupSerializer::brush(&brush, doc));
    
    QPen pen = this->pen();
    root.appendChild(TupSerializer::pen(&pen, doc));

    return root;
}

void TupRectItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasColor()) {
        event->setAccepted(true);
        dragOver = true;
        update();
    } else {
        event->setAccepted(false);
    }
}

void TupRectItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event)

    dragOver = false;
    update();
}

void TupRectItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    dragOver = false;
    if (event->mimeData()->hasColor()) {
        // setBrush(QBrush(qVariantValue<QColor>(event->mimeData()->colorData())));
        QVariant color = event->mimeData()->colorData();
        setBrush(QBrush(color.value<QColor>()));
    } else if (event->mimeData()->hasImage()) {
               // setBrush(QBrush(qVariantValue<QPixmap>(event->mimeData()->imageData())));
               QVariant pixmap = event->mimeData()->imageData();
               setBrush(QBrush(pixmap.value<QPixmap>()));
    }
    update();
}

bool TupRectItem::contains(const QPointF &point) const
{
    return QGraphicsRectItem::contains(point);
}
