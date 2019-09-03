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
        #ifdef Q_OS_WIN
            qDebug() << "[TNodeGroup()]";
        #else
            TINIT;
        #endif
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
        #ifdef Q_OS_WIN
            qDebug() << "[~TNodeGroup()]";
        #else
            TEND;
        #endif
    #endif

    delete nodeParentItem;
    delete nodeScene;
}

void TNodeGroup::clear()
{
    if (nodes.isEmpty())
        return;

    foreach (TControlNode *node, nodes) {
        if (node)
            nodeScene->removeItem(node);
    }

    nodes.clear();
    nodeParentItem->update();
}

void TNodeGroup::syncNodes(const QPainterPath &path)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TNodeGroup::syncNodes()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (nodes.isEmpty())
        return;

    foreach (TControlNode *node, nodes) {
        if (node) {
            node->hasChanged(true);
            node->setPos(path.elementAt(node->index()));
        }
    }
}

void TNodeGroup::syncNodesFromParent()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TNodeGroup::syncNodesFromParent()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (nodeParentItem) {
        if (qgraphicsitem_cast<QGraphicsPathItem *>(nodeParentItem))
            syncNodes(nodeParentItem->sceneTransform().map(qgraphicsitem_cast<QGraphicsPathItem *>(nodeParentItem)->path()));
            // syncNodes(nodeParentItem->sceneMatrix().map(qgraphicsitem_cast<QGraphicsPathItem *>(nodeParentItem)->path()));
    }
}

void TNodeGroup::setParentItem(QGraphicsItem *newParent)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TNodeGroup::setParentItem()]";
        #else
            T_FUNCINFO;
        #endif
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
        #ifdef Q_OS_WIN
            qDebug() << "[TNodeGroup::moveElementTo()]";
        #else
            T_FUNCINFO;
        #endif
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
    if (qgraphicsitem_cast<QGraphicsPathItem *>(nodeParentItem)) {
        path = qgraphicsitem_cast<QGraphicsPathItem *>(nodeParentItem)->path();
        pos = nodeParentItem->scenePos();
    }
}

int TNodeGroup::removeSelectedNodes()
{
    int count = 0;
    foreach (TControlNode *node, nodes) {
        if (node->isSelected()) {
            count++;
            nodes.removeAll(node);
            // SQA: recreate the path
        }
    }
    
    return count;
}

void TNodeGroup::createNodes(QGraphicsPathItem *pathItem)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TNodeGroup::createNodes()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (pathItem) {
        qDeleteAll(nodes);
        nodes.clear();
        
        // QPainterPath path = pathItem->sceneMatrix().map(pathItem->path());
        QPainterPath path = pathItem->sceneTransform().map(pathItem->path());
        saveParentProperties();
        int index = 0;
 
        while (index < path.elementCount()) {
            QPainterPath::Element e = path.elementAt(index);
            
            if (e.type == QPainterPath::CurveToDataElement) {
                if (index - 2 < 0)
                    continue;
                if (path.elementAt(index-2).type == QPainterPath::CurveToElement) {
                    TControlNode *node = new TControlNode(index, this, path.elementAt(index), pathItem, nodeScene, nodeLevel);
                    QPainterPath::Element e1 = path.elementAt(index - 1);
                    node->setLeft(new TControlNode(index - 1, this, e1, pathItem, nodeScene, nodeLevel));
                    
                    if (index+1 < path.elementCount()) {
                        QPainterPath::Element e2 = path.elementAt(index+1);
                        if (e2.type == QPainterPath::CurveToElement) {
                            node->setRight(new TControlNode(index + 1, this, e2, pathItem, nodeScene, nodeLevel));
                            nodes << node->right();
                            index++;
                        }
                    }
                    nodes << node;
                    nodes << node->left();
                }
            } else if ((e.type == QPainterPath::LineToElement || e.type == QPainterPath::MoveToElement)) {
                TControlNode *node;
                if (index+1 < path.elementCount()) {
                    if (path.elementAt(index+1).type == QPainterPath::CurveToElement) {
                        node = new TControlNode(index, this, path.elementAt(index), pathItem, nodeScene, nodeLevel);
                        node->setRight(new TControlNode(index + 1, this, path.elementAt(index + 1), pathItem, nodeScene));
                        
                        index++;
                        nodes << node;
                        nodes << node->right();
                    } else {
                        node = new TControlNode(index, this, path.elementAt(index), pathItem, nodeScene, nodeLevel);
                        nodes << node;
                    }
                } else {
                    node = new TControlNode(index, this, path.elementAt(index), pathItem, nodeScene, nodeLevel);
                    nodes << node;
                }
            }
            index++;
        }
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TNodeGroup::createNodes() - Fatal Error: Item is NULL!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }
}

void TNodeGroup::addControlNode(TControlNode*)
{
}

void TNodeGroup::emitNodeClicked(TControlNode::State state)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TNodeGroup::emitNodeClicked()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    /* SQA: Possible code for the future 
    if (state == TControlNode::Pressed) {
        tFatal() << "TNodeGroup::emitNodeClicked() - Click! -> PRESSED";
        emit nodePressed();
    }
    */

    if (state == TControlNode::Released)
        emit nodeReleased();
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

int TNodeGroup::nodesTotalCount()
{
    return nodes.count();
}

int TNodeGroup::mainNodesCount()
{
    return (nodes.count()/3) + 1;
}

void TNodeGroup::resizeNodes(qreal scaleFactor)
{
    foreach (TControlNode *node, nodes) {
        if (node)
            node->resize(scaleFactor);
    }
}
