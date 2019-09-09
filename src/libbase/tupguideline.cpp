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

#include "tupguideline.h"

#include <QApplication>
#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsView>

TupLineGuide::TupLineGuide(Qt::Orientation direction): QGraphicsItem(nullptr)
{
    orientation = direction;
    enabled = true;
    // setAcceptsHoverEvents(true);
    // setAcceptedMouseButtons(0);
    // setFlag(QGraphicsItem::ItemIsFocusable, false);
}

TupLineGuide::~TupLineGuide()
{
}

QRectF TupLineGuide::boundingRect() const
{
    if (orientation == Qt::Vertical)
        return QRectF(QPointF(0,0), QSizeF(5, scene()->height()));
    else
        return QRectF(QPointF(0,0), QSizeF(scene()->width(), 5));
}

void TupLineGuide::paint(QPainter * painter, const QStyleOptionGraphicsItem * , QWidget *)
{
    painter->setPen(QPen(Qt::black, 1, Qt::DashLine));
    if (orientation == Qt::Vertical) {
        painter->drawLine(static_cast<int>(boundingRect().center().x()), 0,
                          static_cast<int>(boundingRect().center().x()),
                          static_cast<int> (boundingRect().height()));
    } else {
        painter->drawLine(0, static_cast<int>(boundingRect().center().y()),
                          static_cast<int> (boundingRect().width()),
                          static_cast<int> (boundingRect().center().y()));
    }
}

void TupLineGuide::setEnabledSyncCursor(bool flag)
{
    enabled = flag;
}

QVariant TupLineGuide::itemChange(GraphicsItemChange change, const QVariant & value)
{
    if (change == ItemPositionChange) {
        if (orientation == Qt::Vertical)
            return QPointF(value.toPointF().x(), 0);
        else
            return QPointF(0, value.toPointF().y());
    }

    return QGraphicsItem::itemChange(change, value);
}

/* 
void TupLineGuide::hoverEnterEvent(QGraphicsSceneHoverEvent *e)
 {
     QGraphicsSceneMouseEvent *event = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMousePress);
     event->setButtons(Qt::LeftButton);
     event->setButton(Qt::LeftButton);
     
     mousePressEvent(event);
     
     delete event;
     
     setAcceptsHoverEvents(false);
     
     syncCursor();
 }
*/

void TupLineGuide::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (enabled)
        syncCursor();
    else
        setPos(event->scenePos());
}

bool TupLineGuide::sceneEvent(QEvent *event)
{
/*
     switch(e->type())
    {
         case QEvent::GraphicsSceneMouseMove:
         case QEvent::GraphicsSceneHoverEnter:
         case QEvent::GraphicsSceneHoverLeave:
         case QEvent::GraphicsSceneHoverMove:
         {
             QGraphicsSceneMouseEvent *event = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMousePress);
             event->setButtons(Qt::LeftButton);
             event->setButton(Qt::LeftButton);
             
             mousePressEvent(event);
             
             delete event;
         }
         break;
         default: break;
     }
*/
    
    return QGraphicsItem::sceneEvent(event);
}

void TupLineGuide::syncCursor()
{
    QPointF globalPos;
    if (scene()) {
        foreach (QGraphicsView *view, scene()->views())
            globalPos = view->viewport()->mapToGlobal(scenePos().toPoint()
                                                      + view->mapFromScene(QPointF(0, 0))) ;
    }
    
    double distance;
    if (orientation == Qt::Vertical)
        distance = globalPos.x() + 2 - QCursor::pos().x();
    else
        distance = globalPos.y() + 2 - QCursor::pos().y();
    
    if (-QApplication::startDragDistance() < distance && distance < QApplication::startDragDistance()) {
        if (orientation == Qt::Vertical)
            QCursor::setPos(static_cast<int> (globalPos.x() + 2),
                            static_cast<int> (QCursor::pos().y()));
        else
            QCursor::setPos(static_cast<int> (QCursor::pos().x()),
                            static_cast<int> (globalPos.y() + 2));
    }
}
