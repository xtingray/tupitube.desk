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

#include "tnodegroup.h"

TNodeGroup::TNodeGroup(QGraphicsItem *parent, QGraphicsScene *scene, GroupType type, int level)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TNodeGroup()]";
    #endif

    nodeParentItem = parent;
    nodeScene = scene;
    nodeType = type;
    nodeLevel = level;
    
    if (QGraphicsPathItem *pathItem = qgraphicsitem_cast<QGraphicsPathItem *>(parent))
        createNodes(pathItem);
}

QGraphicsItem *TNodeGroup::parentItem()
{
    return nodeParentItem;
}

TNodeGroup::~TNodeGroup()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TNodeGroup()]";
    #endif

    delete nodeParentItem;
    delete nodeScene;
}

void TNodeGroup::clear()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TNodeGroup::clear()]";
    #endif

    if (nodes.isEmpty())
        return;

    foreach (TControlNode *node, nodes) {
        if (node)
            nodeScene->removeItem(node);
    }

    nodes.clear();
    if (nodeParentItem)
        nodeParentItem->update();
}

void TNodeGroup::syncNodes(const QPainterPath &path)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TNodeGroup::syncNodes()]";
    #endif

    if (path.isEmpty()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TNodeGroup::syncNodes()] - Warning: QPainterPath is empty!";
        #endif

        return;
    }

    if (nodes.isEmpty()) {
        #ifdef TUP_DEBUG
                qDebug() << "[TNodeGroup::syncNodes()] - Warning: Nodes list is empty!";
        #endif

        return;
    }

    foreach (TControlNode *node, nodes) {
        if (node) {
            node->hasChanged(true);
            int index = node->index();
            if (index < path.elementCount()) {
                node->setPos(path.elementAt(index));
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TNodeGroup::syncNodes()] - Warning: Invalid element index ->" << index;
                #endif
            }
        }
    }
}

void TNodeGroup::syncNodesFromParent()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TNodeGroup::syncNodesFromParent()]";
    #endif

    if (nodeParentItem) {
        if (QGraphicsPathItem *item = qgraphicsitem_cast<QGraphicsPathItem *>(nodeParentItem))
            syncNodes(nodeParentItem->sceneTransform().map(item->path()));
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TNodeGroup::syncNodesFromParent()] - Warning: Parent item is NULL!";
        #endif
    }
}

void TNodeGroup::setParentItem(QGraphicsItem *newParent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TNodeGroup::setParentItem()]";
    #endif

    nodeParentItem = newParent;
    foreach (TControlNode *node, nodes) {
        if (node)
            node->setGraphicParent(newParent);
    }
}

void TNodeGroup::moveElementTo(int index, const QPointF& pos)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TNodeGroup::moveElementTo()]";
    #endif
    */

    if (nodeParentItem) {
        QPainterPath path = qgraphicsitem_cast<QGraphicsPathItem *>(nodeParentItem)->path();
        path.setElementPositionAt(index, pos.x(), pos.y());
        qgraphicsitem_cast<QGraphicsPathItem *>(nodeParentItem)->setPath(path);

        hashChangedNodes.insert(index, pos);
        emit itemChanged(nodeParentItem);
    }
}

QHash<int, QPointF> TNodeGroup::changedNodes()
{
    return hashChangedNodes;
}

bool TNodeGroup::hasChangedNodes()
{
    return hashChangedNodes.count() > 0;
}

void TNodeGroup::clearChangedNodes()
{
    if (!hashChangedNodes.isEmpty())
        hashChangedNodes.clear();
}

void TNodeGroup::restoreItem()
{
    qgraphicsitem_cast<QGraphicsPathItem *>(nodeParentItem)->setPath(path);
    nodeParentItem->setPos(pos);
}

void TNodeGroup::show()
{
    foreach (TControlNode *node, nodes) {
        if (qgraphicsitem_cast<QGraphicsPathItem *>(nodeParentItem)) {
            if (!node->scene())
                nodeScene->addItem(node);
        }
    }
}

void TNodeGroup::saveParentProperties()
{
    if (nodeParentItem) {
        if (qgraphicsitem_cast<QGraphicsPathItem *>(nodeParentItem)) {
            path = qgraphicsitem_cast<QGraphicsPathItem *>(nodeParentItem)->path();
            pos = nodeParentItem->scenePos();
        }
    }
}

void TNodeGroup::removeSelectedNode()
{
    int index = 1;
    foreach (TControlNode *node, nodes) {
        if (node->isCentralNode()) {
            if (node->isSelected()) {
                emit nodeRemoved(index);

                return;
            }
            index++;
        }
    }
}

void TNodeGroup::changeSelectedNode()
{
    int index = 1;
    foreach (TControlNode *node, nodes) {
        if (node->isCentralNode()) {
            if (node->isSelected()) {
                emit nodeTypeChanged(index);

                return;
            }
            index++;
        }
    }
}

void TNodeGroup::createNodes(QGraphicsPathItem *pathItem)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TNodeGroup::createNodes()]";
    #endif

    if (pathItem) {
        qDeleteAll(nodes);
        nodes.clear();
        mainNodesCounter = 0;

        QPainterPath path = pathItem->sceneTransform().map(pathItem->path());
        saveParentProperties();        
        int index = 0;
        int total = path.elementCount();

        #ifdef TUP_DEBUG
            qDebug() << "[TNodeGroup::createNodes()] - element count ->" << total;
        #endif

        while (index < total) {
            QPainterPath::Element e = path.elementAt(index);
            if (e.type == QPainterPath::CurveToDataElement) { // Extra data required to describe a curve
                if ((index - 2) < 0) // Ignore the first two elements
                    continue; 

                if (path.elementAt(index - 2).type == QPainterPath::CurveToElement) { // Element is a curve
                    TControlNode *node = new TControlNode(index, this, path.elementAt(index), pathItem, nodeScene, nodeLevel);
                    /*
                    #ifdef TUP_DEBUG
                        qDebug() << "* Adding center node - CURVE";
                    #endif
                    */
                    nodes << node; // Adding central node
                    mainNodesCounter++;
                    QPainterPath::Element e1 = path.elementAt(index - 1);
                    node->setLeft(new TControlNode(index - 1, this, e1, pathItem, nodeScene, nodeLevel)); // Setting left wing
                    /*
                    #ifdef TUP_DEBUG
                        qDebug() << "   - Adding left wing node";
                    #endif
                    */
                    nodes << node->left(); // Adding left wing node

                    if ((index + 1) < path.elementCount()) { // If the current element is not the last one, add a right wing
                        QPainterPath::Element e2 = path.elementAt(index + 1);
                        if (e2.type == QPainterPath::CurveToElement) {
                            node->setRight(new TControlNode(index + 1, this, e2, pathItem, nodeScene, nodeLevel)); // Setting right wing
                            /*
                            #ifdef TUP_DEBUG
                                qDebug() << "   - Adding right wing node";
                            #endif
                            */
                            nodes << node->right(); // Adding right wing node
                            index++;
                        }
                    }
                }
            } else if ((e.type == QPainterPath::LineToElement || e.type == QPainterPath::MoveToElement)) { // Element is a rect segment
                TControlNode *node;
                if (index+1 < path.elementCount()) {
                    if (path.elementAt(index + 1).type == QPainterPath::CurveToElement) {
                        node = new TControlNode(index, this, path.elementAt(index), pathItem, nodeScene, nodeLevel);
                        /*
                        #ifdef TUP_DEBUG
                            qDebug() << "* Adding center node - LINE";
                        #endif
                        */
                        nodes << node; // Adding central node
                        mainNodesCounter++;
                        node->setRight(new TControlNode(index + 1, this, path.elementAt(index + 1), pathItem, nodeScene));
                        /*
                        #ifdef TUP_DEBUG
                            qDebug() << "   - Adding right wing node";
                        #endif
                        */
                        nodes << node->right(); // Adding central node

                        index++;
                    } else {
                        /*
                        #ifdef TUP_DEBUG
                            if (e.type == QPainterPath::LineToElement)
                                qDebug() << "* Adding center node - LineToElement ->" << index;
                            else
                                qDebug() << "* Adding center node - MoveToElement ->" << index;
                        #endif
                        */
                        node = new TControlNode(index, this, path.elementAt(index), pathItem, nodeScene, nodeLevel);
                        nodes << node; // Adding node without wings (smoothness == 0)
                        mainNodesCounter++;
                    }
                } else {
                    /*
                    #ifdef TUP_DEBUG
                        if (e.type == QPainterPath::LineToElement)
                            qDebug() << "* Adding center node - LineToElement ->" << index;
                        else
                            qDebug() << "* Adding center node - MoveToElement ->" << index;
                    #endif
                    */
                    node = new TControlNode(index, this, path.elementAt(index), pathItem, nodeScene, nodeLevel);
                    nodes << node; // Adding node without wings (smoothness == 0)
                    mainNodesCounter++;
                }
            }
            index++;
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TNodeGroup::createNodes()] - Fatal Error: Item is NULL!";
        #endif
    }
}

void TNodeGroup::emitNodeClicked(TControlNode::State state)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TNodeGroup::emitNodeClicked()]";
    #endif

    /* SQA: Possible code for the future 
    if (state == TControlNode::Pressed) {
        qWarning() << "[TNodeGroup::emitNodeClicked()] - Click! -> PRESSED";
        emit nodePressed();
    }
    */

    if (state == TControlNode::Released)
        emit nodeReleased();
}

void TNodeGroup::expandNode(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TNodeGroup::expandNode()] - index ->" << index;
    #endif

    int nodesCounter = 1;
    foreach (TControlNode *node, nodes) {
        if (node->isCentralNode()) {
            if (nodesCounter == index) {
                node->showChildNodes(true);
                return;
            }

            nodesCounter++;
        }
    }
}

void TNodeGroup::expandAllNodes()
{
    foreach (TControlNode *node, nodes)
        node->showChildNodes(true);
}

bool TNodeGroup::isSelected()
{
    foreach (TControlNode *node, nodes) {
        if (node->isSelected())
            return true;
    }

    return false;
}

void TNodeGroup::unselectNodes()
{
    foreach (TControlNode *node, nodes) {
        if (node->isSelected())
            node->setSelected(false);
    }
}

int TNodeGroup::nodesTotalCount()
{
    /*
    int total = 0;
    foreach (TControlNode *node, nodes) {
        if (node->isCentralNode())
            total++;
    }

    return total;
    */

    return nodes.count();
}

int TNodeGroup::mainNodesCount()
{
    return mainNodesCounter;
}

void TNodeGroup::resizeNodes(qreal scaleFactor)
{
    foreach (TControlNode *node, nodes) {
        if (node)
            node->resize(scaleFactor);
    }
}
