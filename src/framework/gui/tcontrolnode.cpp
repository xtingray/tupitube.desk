/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2019:                                                                 *
 *    Alejandro Carrasco Rodríguez                                         *
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

#include "tcontrolnode.h"
#include "tnodegroup.h"

TControlNode::TControlNode(int index, TNodeGroup *group, const QPointF & pos, 
                           QGraphicsItem *graphicParent, QGraphicsScene *gScene, int level): QGraphicsItem()
{
    nodeIndex  = index;
    unchanged = true;

    itemParent = nullptr;
    leftNode = nullptr;
    rightNode = nullptr;
    cNode = nullptr;

    nodeGroup = group;
    globalScene = gScene;
    QGraphicsItem::setCursor(QCursor(Qt::PointingHandCursor));
    setFlag(ItemIsSelectable, true);
    setFlag(ItemIsMovable, true);
    setFlag(ItemSendsGeometryChanges, true);
    setPos(pos);

    setZValue(level);
    setGraphicParent(graphicParent);
}

TControlNode::~TControlNode()
{
}

void TControlNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *w)
{
    Q_UNUSED(option);
    Q_UNUSED(w);
    
    // SQA: Check Antialiasing management for this painter
    // bool antialiasing = painter->renderHints() & QPainter::Antialiasing;
    // painter->setRenderHint(QPainter::Antialiasing, antialiasing);

    QColor color;

    if (cNode) {
        color = QColor("white");
    } else {
        color = QColor(55, 155, 55);
        color.setAlpha(200);
    }

    painter->setBrush(color);
    paintLinesToChildNodes(painter);
    // painter->drawRoundRect(boundingRect()); 
    painter->drawRoundedRect(boundingRect(), 1, 1, Qt::AbsoluteSize);
}

void TControlNode::paintLinesToChildNodes(QPainter *painter)
{
    QTransform inverted = sceneTransform().inverted();
    painter->save();
    
    painter->setPen(QPen(QColor(Qt::gray)));
    painter->setRenderHint(QPainter::Antialiasing, true);

    if (rightNode) {
        if (rightNode->isVisible())
            painter->drawLine(inverted.map(pos()), inverted.map(rightNode->pos()));
    }

    if (leftNode) {
        if (leftNode->isVisible())
            painter->drawLine(inverted.map(pos()), inverted.map(leftNode->pos()));
    }
    
    painter->restore();
}

QRectF TControlNode::boundingRect() const
{
    QSizeF size(10, 10);
    QRectF rect(QPointF(-size.width()/2, -size.height()/2), size);

    if (rightNode) {
        if (rightNode->isVisible())
            rect = rect.united(rightNode->boundingRect());
    }

    if (leftNode) {
        if (leftNode->isVisible())
           rect = rect.united(leftNode->boundingRect());
    }

    return rect;
}

QVariant TControlNode::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        if (!unchanged) {
            if (qgraphicsitem_cast<QGraphicsPathItem*>(itemParent)) {
                QPointF diff = value.toPointF() - pos();
                if (leftNode)
                    leftNode->moveBy(diff.x(), diff.y());

                if (rightNode)
                    rightNode->moveBy(diff.x(), diff.y());

                QPointF scenePos = itemParent->mapFromScene(value.toPointF());

                if (nodeGroup)
                    nodeGroup->moveElementTo(nodeIndex, scenePos);
           } 
        } else {
           unchanged = false;
        }

    } else if (change == QGraphicsItem::ItemSelectedChange) {
        if (value.toBool()) {
            itemParent->setSelected(true);
            showChildNodes(true);
        } else {
            if (leftNode) {
                if (leftNode->isSelected())
                    leftNode->setVisible(true);
                else
                    leftNode->setVisible(false);
            }

            if (rightNode) {
                if (rightNode->isSelected())
                    rightNode->setVisible(true);
                else
                    rightNode->setVisible(false);
            }
            update();
        }
    }

    return QGraphicsItem::itemChange(change, value);
}

void TControlNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (cNode) {
        setSelected(true);
        cNode->setSelected(true);

        if (cNode->left()) {
            if (cNode->left() != this)
                cNode->left()->setSelected(false);
        }

        if (cNode->right()) {
            if (cNode->right() != this)
                cNode->right()->setSelected(false);
        }

    } else {
        setSeletedChilds(false);
    }

    itemParent->setSelected(true);
    showChildNodes(true);
  
    // These instructions are required for painting updates
    QGraphicsItem::mousePressEvent(event); 
    event->accept();

    // SQA: Possible code for the future
    // nodeGroup->emitNodeClicked(Pressed);
}

void TControlNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TControlNode::mouseReleaseEvent()]";
        #else
           T_FUNCINFO;
        #endif
    #endif

    Q_UNUSED(event);

    nodeGroup->emitNodeClicked(Released);

    // SQA: Why this instruction makes the system crash in Qt 4.7? 
    // QGraphicsItem::mouseReleaseEvent(event);
    // event->accept();
}

void TControlNode::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    /* SQA: Check if this code is required for other features than Position tween
    foreach (QGraphicsItem *item, scene()->selectedItems()) {
             if (qgraphicsitem_cast<TControlNode*>(item)) {
                 if (!k-centralNode) {
                     if (item != this)
                         item->moveBy(event->pos().x(), event->pos().y());
                 }
             } 
    }
    */

    setPos(event->scenePos());
    event->accept();
}

void TControlNode::setLeft(TControlNode *left)
{
    // SQA: check if this condition is required
    if (leftNode)
        delete leftNode;

    leftNode = left;
    leftNode->setVisible(false);
    leftNode->setCentralNode(this);
    leftNode->setZValue(zValue()+1);
}

void TControlNode::setRight(TControlNode *right)
{
    // SQA: check if this condition is required
    if (right)
        delete rightNode;

    rightNode = right;
    rightNode->setVisible(false);
    rightNode->setCentralNode(this);
    rightNode->setZValue(zValue()+2);
}

void TControlNode::setCentralNode(TControlNode *node)
{
    cNode = node;
}

void TControlNode::showChildNodes(bool visible)
{
    if (leftNode)
        leftNode->setVisible(visible);

    if (rightNode)
        rightNode->setVisible(visible);
}

void TControlNode::setSeletedChilds(bool select)
{
    if (leftNode)
        leftNode->setSelected(select);

    if (rightNode)
        rightNode->setSelected(select);
}

TControlNode *TControlNode::left()
{
    return leftNode;
}

TControlNode *TControlNode::right()
{
    return rightNode;
}

TControlNode *TControlNode::centralNode()
{
    return cNode;
}

int TControlNode::index() const
{
    return nodeIndex;
}

void TControlNode::setGraphicParent(QGraphicsItem *newParent)
{
    itemParent = newParent;
}

QGraphicsItem * TControlNode::graphicParent()
{
    return itemParent;
}

void TControlNode::hasChanged(bool noChange)
{
    unchanged = noChange;
}

void TControlNode::resize(qreal factor)
{
    setScale(factor);
}
