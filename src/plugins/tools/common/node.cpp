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

#include "node.h"
#include "nodemanager.h"
#include "tupgraphicobject.h"
#include "tupellipseitem.h"

#include <cmath> // fabs

Node::Node(NodeContext tool, NodePosition nType, NodeAction actionValue, const QPointF &pos, NodeManager *mngr,
           QGraphicsItem *parentItem, int zValue) : QGraphicsItem(0)
{
    QGraphicsItem::setCursor(QCursor(Qt::PointingHandCursor));
    setFlag(ItemIsSelectable, false);
    setFlag(ItemIsMovable, true);
    setFlag(ItemIsFocusable, true);
    setPos(pos);

    context = tool;
    node = nType;
    action = actionValue;
    manager = mngr;
    parent = parentItem;
    size = QSizeF(10, 10);
    generalState = NodeScale;

    setZValue(zValue);
}

Node::~Node()
{
}

void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *w)
{
    Q_UNUSED(w)
    Q_UNUSED(option)
    
    QColor color;
   
    if (node != CenterNode) {
        if (action == NodeRotate) {
            color = QColor(255, 102, 0);
            color.setAlpha(180);
        } else {
            color = QColor("green");
            color.setAlpha(200);
        }
    } else {
        if (generalState == NodeScale) {
            color = QColor(150, 150, 150);
        } else {
           color = QColor(255, 0, 0);
        }
        color.setAlpha(150);
    }

    QRectF square = boundingRect();
    painter->setBrush(color);
    painter->drawRoundedRect(square, 2, 2, Qt::RelativeSize);

    /* SQA: Code for debugging purposes
    #ifdef TUP_DEBUG
        painter->setFont(QFont(painter->font().family(), 5));
        painter->drawText(square, QString::number(type));
    #endif
    */

    if (node == CenterNode) {
        painter->save();
        color = QColor("white");
        color.setAlpha(220);
        painter->setPen(color);

        QPointF point1 = QPointF(square.topLeft().x() + 2, square.topLeft().y() + 2);
        QPointF point2 = QPointF(square.bottomRight().x() - 2, square.bottomRight().y() - 2);
        QPointF point3 = QPointF(square.bottomLeft().x() + 2, square.bottomLeft().y() - 2);
        QPointF point4 = QPointF(square.topRight().x() - 2, square.topRight().y() + 2);

        painter->drawLine(point1, point2);
        painter->drawLine(point3, point4);
        painter->restore();
    }
}

QRectF Node::boundingRect() const
{
    QRectF rect(QPointF(-size.width()/2, -size.height()/2), size);
    return rect;
}

QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[Node::itemChange()]";
    #endif
    */

    if (change == ItemSelectedChange) {
        setVisible(true);
        if (value.toBool())
            parent->setSelected(true);
        manager->show();
    }

    return QGraphicsItem::itemChange(change, value);
}

void Node::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[Node::mousePressEvent()]";
    #endif

    oldPoint = event->scenePos();

    if (manager)
        manager->setPressedStatus(true);

    QGraphicsItem::mousePressEvent(event);
}

void Node::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF newPos(event->scenePos());

    if (node == CenterNode) {
        if (context == SelectionNode || context == TextNode) {
            int x = newPos.x() - scenePos().x();
            int y = newPos.y() - scenePos().y();
            parent->moveBy(x, y);

            if (context == TextNode)
                emit positionUpdated(newPos);
        } else { // Papagayo context
            QPointF center = newPos - QPointF(parent->boundingRect().width()/2, parent->boundingRect().height()/2);
            parent->setPos(center.x(), center.y());
            emit positionUpdated(newPos);
        }
        QGraphicsItem::mouseMoveEvent(event);

        return;
    } else {
        if (action == NodeScale) {
            QPointF center = parent->boundingRect().center();
            QPointF distance = parent->mapToScene(center) - newPos;

            qreal w = parent->boundingRect().width() / 2;
            qreal h = parent->boundingRect().height() / 2;

            qreal sx = fabs(distance.x()) / w;
            qreal sy = fabs(distance.y()) / h;

            if (manager->proportionalScale())
                sy = sx;
            manager->scale(sx, sy);
        } else if (action == NodeRotate) {
            QPointF p1 = newPos;
            QPointF p2 = parent->sceneBoundingRect().center();

            QLineF line(p2, p1);
            QLineF lineRef(p2, oldPoint);
            qreal angle = parent->data(TupGraphicObject::Rotate).toReal() + (lineRef.angle() - line.angle());

            if (angle < 0)
                angle = 360 - fabs(angle);

            if (angle > 360)
                angle = 0;

            manager->rotate(angle);
            oldPoint = newPos;
        }
    }
}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[Node::mouseReleaseEvent()]";
    #endif

    QGraphicsItem::mouseReleaseEvent(event);
    parent->setSelected(true);

    if (manager)
        manager->setPressedStatus(false);

    emit transformationUpdated();
}

void Node::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[Node::mouseDoubleClickEvent()]";
    #endif

    /*
    if (qgraphicsitem_cast<TupEllipseItem *> (parent)) {
        double scaleX = parent->data(TupGraphicObject::ScaleX).toReal();
        double scaleY = parent->data(TupGraphicObject::ScaleY).toReal();
        if (scaleX != scaleY) {
            manager->toggleAction();
            QGraphicsItem::mouseDoubleClickEvent(event);
        }
    } else {
        manager->toggleAction();
        QGraphicsItem::mouseDoubleClickEvent(event);
    }
    */

    manager->toggleAction();
    QGraphicsItem::mouseDoubleClickEvent(event);
}

int Node::nodeType() const
{
    return node;
}

void Node::setAction(NodeAction nAction)
{
    if (node != CenterNode)
        action = nAction;
    else
        action = NodeScale;

    if (generalState == NodeScale)
        generalState = NodeRotate;
    else
        generalState = NodeScale;

    update();
}

int Node::nodeAction()
{
    return action;
}

void Node::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event)

    manager->setProportion(false);
}

void Node::resize(qreal factor)
{
    setScale(factor);
}
