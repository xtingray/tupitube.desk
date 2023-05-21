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

#include "nodemanager.h"
#include "tupgraphicobject.h"

NodeManager::NodeManager(NodeContext context, QGraphicsItem *parentItem, QGraphicsScene *gScene, int zValue)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeManager::NodeManager()]";
    #endif

    parent = parentItem;
    scene = gScene;
    pressed = false;

    rotationValue = parent->data(TupGraphicObject::Rotate).toReal();
    scaleX = parent->data(TupGraphicObject::ScaleX).toReal();
    if (scaleX == 0.0)
        scaleX = 1;
    scaleY = parent->data(TupGraphicObject::ScaleY).toReal();
    if (scaleY == 0.0)
        scaleY = 1;

    // This condition is only for SVG objects
    if (qgraphicsitem_cast<QGraphicsSvgItem *> (parentItem)) {
        if (static_cast<int> (scaleX) == 0) {
            scaleX = 1;
            parent->setData(TupGraphicObject::ScaleX, 1);
        }
        if (static_cast<int> (scaleY) == 0) {
            scaleY = 1;
            parent->setData(TupGraphicObject::ScaleY, 1);
        }
    } 

    QRectF rect = parentItem->sceneBoundingRect();
    Node *topLeft = new Node(context, TopLeftNode, NodeScale, rect.topLeft(), this, parentItem, zValue);
    Node *topRight = new Node(context, TopRightNode, NodeScale, rect.topRight(), this, parentItem, zValue);
    Node *bottomLeft = new Node(context, BottomLeftNode, NodeScale, rect.bottomLeft(), this, parentItem, zValue);
    Node *bottomRight = new Node(context, BottomRightNode, NodeScale, rect.bottomRight(), this, parentItem, zValue);

    Node *center = new Node(context, CenterNode, NodeScale, rect.center(), this, parentItem, zValue);
    if (context == TextNode || context == PapagayoNode) {
        connect(center, SIGNAL(positionUpdated(const QPointF&)), this, SIGNAL(positionUpdated(const QPointF&)));
        /* SQA: This connection doesn't work on Windows
        connect(center, &Node::positionUpdated, this, &NodeManager::positionUpdated);
        */
    }

    if (context == PapagayoNode) {
        connect(topLeft, SIGNAL(transformationUpdated()), this, SIGNAL(transformationUpdated()));
        connect(topRight, SIGNAL(transformationUpdated()), this, SIGNAL(transformationUpdated()));
        connect(bottomLeft, SIGNAL(transformationUpdated()), this, SIGNAL(transformationUpdated()));
        connect(bottomRight, SIGNAL(transformationUpdated()), this, SIGNAL(transformationUpdated()));
        connect(center, SIGNAL(transformationUpdated()), this, SIGNAL(transformationUpdated()));

        /* SQA: These connections don't work on Windows
        connect(topLeft, &Node::transformationUpdated, this, &NodeManager::transformationUpdated);
        connect(topRight, &Node::transformationUpdated, this, &NodeManager::transformationUpdated);
        connect(bottomLeft, &Node::transformationUpdated, this, &NodeManager::transformationUpdated);
        connect(bottomRight, &Node::transformationUpdated, this, &NodeManager::transformationUpdated);
        connect(center, &Node::transformationUpdated, this, &NodeManager::transformationUpdated);
        */
    }

    nodes.insert(TopLeftNode, topLeft);
    nodes.insert(TopRightNode, topRight);
    nodes.insert(BottomLeftNode, bottomLeft);
    nodes.insert(BottomRightNode, bottomRight);
    nodes.insert(CenterNode, center);

    proportional = false;
    
    beginToEdit();
}

NodeManager::~NodeManager()
{
}

void NodeManager::clear()
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeManager::clear()]";
    #endif

    foreach (Node *node, nodes) {
        if (node) {
            QGraphicsScene *scene = node->scene();
            if (scene)
                scene->removeItem(node);
        }
        delete node;
        node = nullptr;
    }
    nodes.clear();
}

void NodeManager::syncNodes(const QRectF &rect)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeManager::syncNodes()]";
    #endif

    if (nodes.isEmpty()) {
        #ifdef TUP_DEBUG
            qDebug() << "[NodeManager::syncNodes()] - Warning: No nodes! Leaving...";
        #endif
        return;
    }

    QHash<NodePosition, Node *>::iterator it = nodes.begin();
    while (it != nodes.end()) {
        if ((*it)) {
            switch (it.key()) {
                case TopLeftNode:
                {
                    if ((*it)->scenePos() != rect.topLeft())
                        (*it)->setPos(rect.topLeft());
                    break;
                }
                case TopRightNode:
                {
                    if ((*it)->scenePos() != rect.topRight())
                        (*it)->setPos(rect.topRight());
                    break;
                }
                case BottomRightNode:
                {
                    if ((*it)->scenePos() != rect.bottomRight())
                        (*it)->setPos(rect.bottomRight());
                    break;
                }
                case BottomLeftNode:
                {
                    if ((*it)->scenePos() != rect.bottomLeft())
                        (*it)->setPos(rect.bottomLeft());
                    break;
                }
                case CenterNode:
                {
                    if ((*it)->scenePos() != rect.center())
                        (*it)->setPos(rect.center());
                    break;
                }
            }
        }
        ++it;
    }
}

void NodeManager::syncNodesFromParent()
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeManager::syncNodesFromParent()]";
    #endif

    if (parent)
        syncNodes(parent->sceneBoundingRect());
}

QGraphicsItem *NodeManager::parentItem() const
{
    return parent;
}

bool NodeManager::isModified() const
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeManager::isModified()]";
    #endif

    if (parent) {
        return !((parent->transform() == origTransform) && (parent->pos() == origPos));
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[NodeManager::isModified()] - Fatal Error: Item is NULL!";
        #endif
    }

    return false;
}

void NodeManager::beginToEdit()
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeManager::beginToEdit()]";
    #endif

    origTransform = parent->transform();
    origPos = parent->pos();
}

void NodeManager::restoreItem()
{
    parent->setTransform(origTransform);
    parent->setPos(origPos);
}

void NodeManager::scale(qreal sx, qreal sy)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeManager::scale()] - Scale X ->" << sx;
        qDebug() << "[NodeManager::scale()] - Scale Y ->" << sy;
    #endif

    QTransform transform;
    QPointF point = parent->boundingRect().center();
    transform.translate(point.x(), point.y());
    transform.rotate(rotationValue);
    transform.scale(sx, sy);
    transform.translate(-point.x(), -point.y());

    parent->setTransform(transform);

    syncNodesFromParent();
    scaleX = sx;
    scaleY = sy;
    parent->setData(TupGraphicObject::ScaleX, scaleX);
    parent->setData(TupGraphicObject::ScaleY, scaleY);

    emit scaleUpdated(sx, sy);
}

void NodeManager::rotate(double angle)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeManager::rotate()] - angle -> " << angle;
    #endif

    QTransform transform;
    QPointF point = parent->boundingRect().center();

    transform.translate(point.x(), point.y());
    transform.rotate(angle);
    transform.scale(scaleX, scaleY);
    transform.translate(-point.x(), -point.y());

    parent->setTransformOriginPoint(point);
    parent->setTransform(transform);

    syncNodesFromParent();
    rotationValue = angle;

    parent->setData(TupGraphicObject::Rotate, rotationValue);

    emit rotationUpdated(static_cast<int>(angle));
}

void NodeManager::horizontalFlip()
{
    scale(scaleX*(-1), scaleY);
}

void NodeManager::verticalFlip()
{
    scale(scaleX, scaleY*(-1));
}

void NodeManager::crossedFlip()
{
    scale(scaleX*(-1), scaleY*(-1));
}

void NodeManager::show()
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeManager::show()]";
    #endif

    foreach (Node *node, nodes) {
        if (!node->scene())
            scene->addItem(node);
    }
}

void NodeManager::setPressedStatus(bool pressedFlag)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeManager::setPressedStatus()] - pressedFlag -> " << pressedFlag;
    #endif

    pressed = pressedFlag;
}

bool NodeManager::isPressed()
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeManager::isPressed()] - pressed -> " << pressed;
    #endif

    return pressed;
}

void NodeManager::toggleAction()
{
    foreach (Node *node, nodes) {
        if (node->nodeAction() == NodeScale) {
            node->setAction(NodeRotate);
        } else if (node->nodeAction() == NodeRotate) {
            node->setAction(NodeScale);
        }
    }
}

void NodeManager::setActionNode(NodeAction action)
{
    foreach (Node *node, nodes)
        node->setAction(action);
}

void NodeManager::resizeNodes(qreal factor)
{
    foreach (Node *node, nodes)
        node->resize(factor);
}

void NodeManager::setVisible(bool visible)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeManager::setVisible()] - visible -> " << visible;
    #endif

    foreach (Node *node, nodes)
        node->setVisible(visible);
}

double NodeManager::rotation()
{
    return rotationValue;
}

void NodeManager::setProportion(bool flag)
{
    proportional = flag;
}

bool NodeManager::proportionalScale()
{
    return proportional;
}
