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

#include "tupproxyitem.h"

TupProxyItem::TupProxyItem(QGraphicsItem *item) : QGraphicsItem()
{
    setItem(item);
    setPos(0, 0);
}

TupProxyItem::~TupProxyItem()
{
}

void TupProxyItem::setItem(QGraphicsItem *item)
{
    // TODO: Enable this line when filter support is ready
    // if (realItem)
    //     this->removeSceneEventFilter(realItem);

    realItem = item;    
    if (realItem) {
        // TODO: Enable this line when filter support is ready 
        // realItem->installSceneEventFilter(this);
        this->setFlags(realItem->flags());
    }
}

QGraphicsItem *TupProxyItem::item() const
{
    return realItem;
}

QRectF TupProxyItem::boundingRect() const
{
    if (realItem)
        return realItem->boundingRect();
    
    return QRectF(0, 0, 0, 0);
}

void TupProxyItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (realItem) {
        if (QGraphicsItemGroup *group = qgraphicsitem_cast<QGraphicsItemGroup *>(realItem)) {
            foreach (QGraphicsItem *child, group->childItems())
                     child->paint(painter, option, widget);
        } else {
            realItem->paint(painter, option, widget);
        }
    }
}

QPainterPath TupProxyItem::shape() const
{
    if (realItem)
        return realItem->shape();
    
    return QGraphicsItem::shape();
}

bool TupProxyItem::collidesWithItem(const QGraphicsItem *other, Qt::ItemSelectionMode mode) const
{
    if (realItem)
        return realItem->collidesWithItem(other, mode);
    
    return QGraphicsItem::collidesWithItem(other, mode);
}

bool TupProxyItem::collidesWithPath(const QPainterPath &path, Qt::ItemSelectionMode mode) const
{
    if (realItem)
        return realItem->collidesWithPath(path, mode);
    
    return QGraphicsItem::collidesWithPath(path, mode);
}

bool TupProxyItem::contains(const QPointF &point) const
{
    if (realItem)
        return realItem->contains(point);
    
    return QGraphicsItem::contains(point);
}

bool TupProxyItem::isObscuredBy(const QGraphicsItem *item) const
{
    if (realItem)
        return realItem->isObscuredBy(item);
    
    return QGraphicsItem::isObscuredBy(item);
}

QPainterPath TupProxyItem::opaqueArea() const
{
    if (realItem)
        return realItem->opaqueArea();
    
    return QGraphicsItem::opaqueArea();
}
