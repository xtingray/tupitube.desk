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

#include "nodestool.h"
#include "tupprojectrequest.h"
#include "tupprojectresponse.h"
#include "tuprequestbuilder.h"
#include "tupproxyitem.h"

NodesTool::NodesTool()
{
    setupActions();
}

NodesTool::~NodesTool()
{
}

void NodesTool::init(TupGraphicsScene *gScene)
{
    activeSelection = false;
    shiftEnabled = false;
    ctrlEnabled = false;
    scene = gScene;

    if (scene->selectedItems().count() > 0) {
        scene->clearSelection();
        if (nodeGroup) {
            nodeGroup->clear();
            nodeGroup = nullptr;
        }
    }

    nodeZValue = ((BG_LAYERS + 1) * ZLAYER_LIMIT) + (scene->currentScene()->layersCount() * ZLAYER_LIMIT);
    if (scene->getSpaceContext() == TupProject::VECTOR_FG_MODE)
        nodeZValue += ZLAYER_LIMIT;
}

QList<TAction::ActionId> NodesTool::keys() const
{
    return QList<TAction::ActionId>() << TAction::NodesEditor;
}

void NodesTool::press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    Q_UNUSED(input)
    Q_UNUSED(brushManager)
    Q_UNUSED(gScene)
}

void NodesTool::move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    Q_UNUSED(input)
    Q_UNUSED(brushManager)
    Q_UNUSED(gScene)
}

void NodesTool::release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodesTool::release()]";
    #endif

    QList<QGraphicsItem *> currentSelection = gScene->selectedItems();

    if ((shiftEnabled || ctrlEnabled) && activeSelection) {
        qDebug() << "CTRL/SHIFT ENABLED!";
        qDebug() << "";
        QPointF coord = input->pos();
        QString pathStr = "";
        if (!currentSelection.isEmpty()) {
            qDebug() << "ITEM WAS SELECTED!";
            // QGraphicsItem *selectedItem;
            TupPathItem *pathItem = nullptr;

            foreach(QGraphicsItem *item, currentSelection) {
                if (pathItem = qgraphicsitem_cast<TupPathItem *>(item))
                    break;
            }

            if (pathItem) {
                qDebug() << "COORD ->" << coord;
                // if (pathItem->contains(coord)) {
                    qDebug() << "IN: The path rect was clicked!";
                    QPainterPath path = pathItem->path();
                    if (pathItem->containsOnPath(coord, brushManager->penWidth())) { // Point is part of the path
                        qDebug() << "IN - Adding node on the path!";

                        /*
                        pathItem->setZValue(0);
                        QPainterPath route = pathItem->path();
                        QPolygonF points = route.toFillPolygon();
                        qDebug() << "POINTS";
                        qDebug() << points;
                        QPainterPath linePath(points.at(0));
                        int total = points.size();
                        qDebug() << "IN - Points total -> " << total;
                        for(int i=1; i<total-1; i++) {
                            linePath.lineTo(points.at(i));
                        }

                        QGraphicsPathItem *line = new QGraphicsPathItem;
                        QPen pen;
                        pen.setBrush(QBrush(Qt::red));
                        pen.setWidth(5);
                        line->setPen(pen);
                        line->setPath(linePath);
                        line->setZValue(20000);
                        gScene->addItem(line);

                        QList<QPointF> nodes = pathItem->keyNodes();
                        QList<QColor> colors = pathItem->nodeColors();
                        QList<QString> tips = pathItem->nodeTips();
                        total = nodes.size();
                        QPen pen1;
                        pen1.setBrush(QBrush(Qt::blue));
                        pen1.setWidth(1);
                        for(int i=0; i<total; i++) {
                            QPointF point = nodes.at(i);
                            QGraphicsEllipseItem *dot = new QGraphicsEllipseItem(point.x() - 5, point.y() - 5, 10, 10);
                            dot->setToolTip(tips.at(i));
                            dot->setPen(pen1);
                            dot->setBrush(colors.at(i));
                            dot->setZValue(20000);
                            gScene->addItem(dot);
                        }

                        QGraphicsEllipseItem *target = new QGraphicsEllipseItem(coord.x() - 8, coord.y() - 8, 16, 16);
                        target->setToolTip("Target");
                        target->setPen(pen);
                        target->setBrush(QColor(255, 100, 0));
                        target->setZValue(20000);
                        gScene->addItem(target);
                        */

                        pathStr = pathItem->addNewNode(brushManager->penWidth());

                        nodeGroup->clear();
                        nodeGroup->createNodes(pathItem);
                        nodeGroup->resizeNodes(realFactor);

                        TupFrame *frame = getCurrentFrame();
                        int position = frame->indexOf(pathItem);
                        if (position >= 0) {
                            qDebug() << "Calling EditNodes action...";
                            TupProjectRequest event = TupRequestBuilder::createItemRequest(gScene->currentSceneIndex(),
                                                                                           currentLayer, currentFrame, position,
                                                                                           QPointF(), gScene->getSpaceContext(), TupLibraryObject::Item,
                                                                                           TupProjectRequest::EditNodes, pathStr);
                            emit requested(&event);
                        }
                    } else { // Point is out the path but inside the item rect
                        if (ctrlEnabled) {
                            qDebug() << "OUT - Adding straight line around the path!";
                            path.lineTo(coord);
                        } else if (shiftEnabled) {
                            qDebug() << "OUT - Adding curve line around the path!";
                            QPointF lastPoint = path.pointAtPercent(1);
                            qDebug() << "lastPoint ->" << lastPoint;
                            qDebug() << "coord ->" << coord;
                            // path.cubicTo(c1, c2, coord);
                            path.lineTo(coord);
                        }

                        pathItem->setPath(path);
                        nodeGroup->clear();
                        nodeGroup->createNodes(pathItem);
                        nodeGroup->resizeNodes(realFactor);
                        pathStr = pathItem->pathToString();

                        TupFrame *frame = getCurrentFrame();
                        int position = frame->indexOf(pathItem);
                        if (position >= 0) {
                            qDebug() << "Calling EditNodes action...";
                            TupProjectRequest event = TupRequestBuilder::createItemRequest(gScene->currentSceneIndex(),
                                                                                           currentLayer, currentFrame, position,
                                                                                           QPointF(), gScene->getSpaceContext(), TupLibraryObject::Item,
                                                                                           TupProjectRequest::EditNodes, pathStr);
                            emit requested(&event);
                        }
                    }
                /*
                } else {
                    qDebug() << "OUT: Point out of the path and out of the item rect!";
                    QPainterPath path = pathItem->path();
                    // qDebug() << "SHIFT ->" << shiftEnabled;
                    if (ctrlEnabled) {
                        qDebug() << "OUT - Adding straight line out of the item rect!";
                        path.lineTo(coord);
                    } else if (shiftEnabled) {
                        qDebug() << "OUT - Adding curve line out of the item rect!";
                        QPointF lastPoint = path.pointAtPercent(1);
                        qDebug() << "lastPoint ->" << lastPoint;
                        qDebug() << "coord ->" << coord;
                        // path.cubicTo(c1, c2, coord);
                        path.lineTo(coord);
                    }

                    qDebug() << "Updating nodes...";

                    pathItem->setPath(path);
                    nodeGroup->clear();
                    nodeGroup->createNodes(pathItem);
                    nodeGroup->resizeNodes(realFactor);
                    QString pathStr = pathItem->pathToString();

                    TupFrame *frame = getCurrentFrame();
                    int position = frame->indexOf(pathItem);
                    if (position >= 0) {
                        qDebug() << "Calling EditNodes action...";
                        TupProjectRequest event = TupRequestBuilder::createItemRequest(gScene->currentSceneIndex(),
                                                                                       currentLayer, currentFrame, position,
                                                                                       QPointF(), gScene->getSpaceContext(), TupLibraryObject::Item,
                                                                                       TupProjectRequest::EditNodes, pathStr);
                        emit requested(&event);
                    }
                }
                */
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[NodesTool::release()] - Fatal Error: Path item is NULL!!!";
                #endif
            }
        } else {
            qDebug() << "NO PATH SELECTED!";
            if (TupPathItem *item = qgraphicsitem_cast<TupPathItem *>(nodeGroup->parentItem())) {
                if (item->containsOnPath(coord, brushManager->penWidth())) {
                    pathStr = item->addNewNode(brushManager->penWidth());
                } else {
                    qDebug() << "Click outside of the path!";
                    QPainterPath path = item->path();
                    if (ctrlEnabled) {
                        qDebug() << "OUT - Adding straight line!";
                        path.lineTo(coord);
                    } else if (shiftEnabled) {
                        qDebug() << "OUT - Adding curve line!";
                        QPointF lastPoint = path.pointAtPercent(1);
                        qDebug() << "lastPoint ->" << lastPoint;
                        qDebug() << "coord ->" << coord;
                        // path.cubicTo(c1, c2, coord);
                        path.lineTo(coord);
                    }

                    item->setPath(path);
                    pathStr = item->pathToString();
                }

                qDebug() << "Updating nodes...";

                nodeGroup->clear();
                nodeGroup->createNodes(item);
                nodeGroup->resizeNodes(realFactor);

                TupFrame *frame = getCurrentFrame();
                int position = frame->indexOf(item);
                if (position >= 0) {
                    qDebug() << "Calling EditNodes action...";
                    TupProjectRequest event = TupRequestBuilder::createItemRequest(gScene->currentSceneIndex(),
                                                                                   currentLayer, currentFrame, position,
                                                                                   QPointF(), gScene->getSpaceContext(), TupLibraryObject::Item,
                                                                                   TupProjectRequest::EditNodes, pathStr);
                    emit requested(&event);
                }
            }
        }

        return;
    }

    if (!currentSelection.isEmpty()) {
        QGraphicsItem *selectedItem = currentSelection.at(0);
        TupFrame *frame = getCurrentFrame();
        int itemIndex = frame->indexOf(selectedItem);

        // SVG items are not allowed
        if (qgraphicsitem_cast<TupSvgItem *>(selectedItem)) {
            TOsd::self()->display(TOsd::Error, tr("SVG objects cannot be edited!"));

            return;
        }

        // Raster images are not allowed
        if (TupGraphicLibraryItem *libraryItem = qgraphicsitem_cast<TupGraphicLibraryItem *>(selectedItem)) {
            if (libraryItem->getItemType() == TupLibraryObject::Image) {
                TOsd::self()->display(TOsd::Error, tr("Images have no nodes!"));

                return;
            }
        }

        // Item is a group, so it must be split
        if (qgraphicsitem_cast<TupItemGroup *>(selectedItem)) {
            if (activeSelection)
                nodeGroup->clear();

            QPointF coord = input->pos();
            if (itemIndex >= 0) {
                TupProjectRequest event = TupRequestBuilder::createItemRequest(
                                          gScene->currentSceneIndex(),
                                          currentLayer, currentFrame,
                                          itemIndex, coord,
                                          gScene->getSpaceContext(), TupLibraryObject::Item,
                                          TupProjectRequest::Ungroup);
                emit requested(&event);
            }

            return;
        }

        // Check if the selected item is either a node or a path
        if (!qgraphicsitem_cast<TControlNode*>(selectedItem)) {
            if (!qgraphicsitem_cast<TupPathItem *>(selectedItem)) {
                TOsd::self()->display(TOsd::Error, tr("Only pencil/ink lines can be edited!"));

                return;
            }
        }

        if (itemIndex == -1) {
            // Node was selected
            if (qgraphicsitem_cast<TControlNode*>(selectedItem)) {
                QGraphicsItem *item = nodeGroup->parentItem();
                int position = frame->indexOf(item);
                if (position >= 0) {
                    #ifdef TUP_DEBUG
                        qDebug() << "[NodesTool::release()] - Requesting EditNodes action...";
                    #endif
                    QString path = qgraphicsitem_cast<TupPathItem *>(item)->pathToString();
                    TupProjectRequest event = TupRequestBuilder::createItemRequest(gScene->currentSceneIndex(),
                                              currentLayer, currentFrame, position,
                                              QPointF(), gScene->getSpaceContext(), TupLibraryObject::Item,
                                              TupProjectRequest::EditNodes, path);
                    emit requested(&event);
                    nodeGroup->clearChangedNodes();
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "[NodesTool::release()] - Fatal Error: Invalid position -> " << position;
                    #endif
                }
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[NodesTool::release()] - Fatal Error: Invalid selected item index -> " << itemIndex;
                #endif
            }

            return;
        }

        // Avoiding to select the same item twice 
        if (activeSelection) {
            TupFrame *frame = getCurrentFrame();
            int oldIndex = frame->indexOf(nodeGroup->parentItem());

            qDebug() << "OLDINDEX -> " << oldIndex;
            qDebug() << "ITEMINDEX ->" << itemIndex;

            if (oldIndex != itemIndex) { // New selection
                #ifdef TUP_DEBUG
                    qDebug() << "[NodesTool::release()] - A new object has been selected! (active selection was on)";
                #endif
                nodeGroup->clear();

                nodeGroup = new TNodeGroup(selectedItem, gScene, TNodeGroup::PathSelection, nodeZValue);
                connect(nodeGroup, SIGNAL(nodeRemoved(QPointF)), this, SLOT(removeNodeFromPath(QPointF)));
                nodeGroup->show();
                nodeGroup->resizeNodes(realFactor);

                if (TupPathItem *path = qgraphicsitem_cast<TupPathItem *>(selectedItem)) {
                    path->resetPathHistory();
                    if (path->isNotEdited()) 
                        path->saveOriginalPath();
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "[NodesTool::release()] - Warning: item is not a TupPathItem!";
                    #endif
                }

                #ifdef TUP_DEBUG
                    qDebug() << "---";
                #endif
                configPanel->setNodesTotal(nodeGroup->mainNodesCount());
            } else {
                if (nodeGroup->hasChangedNodes()) { // If path was edited
                    #ifdef TUP_DEBUG
                        qDebug() << "[NodesTool::release()] - Path was edited!";
                    #endif

                    QGraphicsItem *item = nodeGroup->parentItem();
                    int position = frame->indexOf(item);
                    if (position >= 0) {
                        QString path = qgraphicsitem_cast<TupPathItem *>(item)->pathToString();
                        TupProjectRequest event = TupRequestBuilder::createItemRequest(gScene->currentSceneIndex(),
                                                  currentLayer, currentFrame, position,
                                                  QPointF(), gScene->getSpaceContext(), TupLibraryObject::Item,
                                                  TupProjectRequest::EditNodes, path);
                        emit requested(&event);
                        nodeGroup->clearChangedNodes();
                    } else {
                        #ifdef TUP_DEBUG
                            qDebug() << "[NodesTool::release()] - Fatal Error: Invalid position -> " << position;
                        #endif
                    }
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "[NodesTool::release()] - Same item selected. Node group has NO changes!";
                    #endif
                }
            }
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[NodesTool::release()] - Adding nodes to the selected path for the first time...";
            #endif

            nodeGroup = new TNodeGroup(selectedItem, gScene, TNodeGroup::PathSelection, nodeZValue);
            connect(nodeGroup, SIGNAL(nodeRemoved(QPointF)), this, SLOT(removeNodeFromPath(QPointF)));
            nodeGroup->show();
            activeSelection = true;

            nodeGroup->resizeNodes(realFactor);
            if (TupPathItem *path = qgraphicsitem_cast<TupPathItem *>(selectedItem)) {
                path->resetPathHistory();
                if (path->isNotEdited())
                    path->saveOriginalPath();
            }

            configPanel->setNodesTotal(nodeGroup->mainNodesCount());
        }
    } else {
        if (activeSelection) {
            #ifdef TUP_DEBUG
                qDebug() << "[NodesTool::release()] - Empty selection! Removing nodes...";
            #endif

            if (nodeGroup) {
                nodeGroup->clear();
                disconnect(nodeGroup, SIGNAL(nodeRemoved(QPointF)), this, SLOT(removeNodeFromPath(QPointF)));
                nodeGroup = nullptr;
            }
            activeSelection = false;
            configPanel->showClearPanel(false);
        }
    } 
}

TupFrame* NodesTool::getCurrentFrame()
{
    TupFrame *frame = nullptr;
    if (scene->getSpaceContext() == TupProject::FRAMES_MODE) {
        frame = scene->currentFrame();
        currentLayer = scene->currentLayerIndex();
        currentFrame = scene->currentFrameIndex();
    } else {
        currentLayer = -1;
        currentFrame = -1;

        TupScene *tupScene = scene->currentScene();
        TupBackground *bg = tupScene->sceneBackground();
        if (tupScene && bg) {
            if (scene->getSpaceContext() == TupProject::VECTOR_STATIC_BG_MODE) {
                frame = bg->vectorStaticFrame();
            } else if (scene->getSpaceContext() == TupProject::VECTOR_FG_MODE) {
                frame = bg->vectorForegroundFrame();
            } else if (scene->getSpaceContext() == TupProject::VECTOR_DYNAMIC_BG_MODE) {
                frame = bg->vectorDynamicFrame();
            }
        }
    }

    return frame;
}

void NodesTool::layerResponse(const TupLayerResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodesTool::layerResponse()] - action -> " << response->getAction();
    #endif

    switch (response->getAction()) {
        case TupProjectRequest::Move:
        {
            init(scene);
        }
    }
}

void NodesTool::itemResponse(const TupItemResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodesTool::itemResponse()]";
    #endif

    QGraphicsItem *item = nullptr;

    if (response->getAction() != TupProjectRequest::Remove) {
        TupFrame *frame = getCurrentFrame();
        if (response->getAction() == TupProjectRequest::Ungroup) {
            QPointF point = response->position();
            item = scene->itemAt(point, QTransform());
        } else {
            item = frame->item(response->getItemIndex());
        }
    }

    switch (response->getAction()) {
        case TupProjectRequest::Convert:
        {
             #ifdef TUP_DEBUG
                 qDebug() << "[NodesTool::itemResponse()] - Convert case";
             #endif

             if (item) {
                 nodeGroup = new TNodeGroup(item, scene, TNodeGroup::PathSelection, nodeZValue);
             } else {
                 #ifdef TUP_DEBUG
                     qDebug() << "[NodesTool::itemResponse()] - Fatal Error: No item was found";
                 #endif
             }
        }
        break;
        case TupProjectRequest::EditNodes:
        {  
             #ifdef TUP_DEBUG
                 qDebug() << "[NodesTool::itemResponse()] - EditNodes case";
             #endif

             if (item) {
                 if (activeSelection) {
                     QGraphicsPathItem *path = qgraphicsitem_cast<QGraphicsPathItem *>(nodeGroup->parentItem());
                     if (path == item) {
                         qDebug() << "   Showing nodes from selected item! (Existing nodeGroup)";
                         nodeGroup->show();
                         nodeGroup->syncNodesFromParent();
                         nodeGroup->saveParentProperties();

                         if (!path->isSelected())
                             path->setSelected(true);
                     }
                 } else {
                     qDebug() << "   Showing nodes from selected item! (Creating new nodeGroup)";
                     nodeGroup = new TNodeGroup(item, scene, TNodeGroup::PathSelection, nodeZValue);
                     connect(nodeGroup, SIGNAL(nodeRemoved(QPointF)), this, SLOT(removeNodeFromPath(QPointF)));
                     nodeGroup->show();
                     activeSelection = true;
                     nodeGroup->resizeNodes(realFactor);
                 }

                 if (response->getMode() == TupProjectResponse::Redo) {
                     configPanel->redo();
                 } else if (response->getMode() == TupProjectResponse::Undo) {
                     configPanel->undo();
                 }
             } else {
                 #ifdef TUP_DEBUG
                     qDebug() << "[NodesTool::itemResponse()] - Fatal Error: No item was found!";
                 #endif
             }
        }
        break;
        case TupProjectRequest::Remove:
        {
             #ifdef TUP_DEBUG
                 qDebug() << "[NodesTool::itemResponse()] - Remove case";
             #endif
             configPanel->showClearPanel(false);

             return;
        }
        case TupProjectRequest::Ungroup:
        {
             #ifdef TUP_DEBUG
                 qDebug() << "[NodesTool::itemResponse()] - Ungroup case";
             #endif

             if (item) {
                 nodeGroup = new TNodeGroup(item, scene, TNodeGroup::PathSelection, nodeZValue);
                 connect(nodeGroup, SIGNAL(nodeRemoved(QPointF)), this, SLOT(removeNodeFromPath(QPointF)));
                 nodeGroup->show();
                 activeSelection = true;
                 nodeGroup->resizeNodes(realFactor);
             } else {
                 #ifdef TUP_DEBUG
                     qDebug() << "[NodesTool::itemResponse()] - Fatal Error: No item was found";
                 #endif
             }

             return;
        }
        default:
        {
             #ifdef TUP_DEBUG
                 qDebug() << "[NodesTool::itemResponse()] - default action";
             #endif

             if (activeSelection) {
                 nodeGroup->show();
                 if (nodeGroup->parentItem()) {
                     nodeGroup->parentItem()->setSelected(true);
                     nodeGroup->syncNodesFromParent();
                 }
             }
        }
    }
}

void NodesTool::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();

    if (event->modifiers() == Qt::ShiftModifier)
        shiftEnabled = true;

    if (event->modifiers() == Qt::ControlModifier)
        ctrlEnabled = true;

    if (key == Qt::Key_F11 || key == Qt::Key_Escape) {
        emit closeHugeCanvas();
    } else {
        if (!activeSelection) {
            QPair<int, int> flags = TAction::setKeyAction(key, event->modifiers());
            if (flags.first != -1 && flags.second != -1)
                emit callForPlugin(flags.first, flags.second);
        } else if (key == Qt::Key_X) {
            if (nodeGroup) {
                qDebug() << "   DELETING ELEMENT!";
                qDebug() << "";

                nodeGroup->removeSelectedNode();
            }
        } else if ((key == Qt::Key_Left) || (key == Qt::Key_Up)
                   || (key == Qt::Key_Right) || (key == Qt::Key_Down)) {
            int delta = 5;

            if (event->modifiers() == Qt::ShiftModifier)
                delta = 1;

            if (event->modifiers() == Qt::ControlModifier)
                delta = 10;

            TupFrame *frame = getCurrentFrame();
            QGraphicsItem *item = nodeGroup->parentItem();

            if (key == Qt::Key_Left)
                item->moveBy(-delta, 0);

            if (key == Qt::Key_Up)
                item->moveBy(0, -delta);

            if (key == Qt::Key_Right)
                item->moveBy(delta, 0);

            if (key == Qt::Key_Down)
                item->moveBy(0, delta);

            QTimer::singleShot(0, this, SLOT(syncNodes()));
            requestTransformation(item, frame);
        } else {
            QPair<int, int> flags = TAction::setKeyAction(key, event->modifiers());
            if (flags.first != -1 && flags.second != -1)
                emit callForPlugin(flags.first, flags.second);
        }
    }
}

void NodesTool::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event)

    shiftEnabled = false;
    ctrlEnabled = false;
}

void NodesTool::setupActions()
{
    configPanel = nullptr;
    activeSelection = false;

    TAction *nodes = new TAction(QPixmap(kAppProp->themeDir() + "icons/nodes.png"), tr("Nodes Selection"), this);
    nodes->setShortcut(QKeySequence(tr("N")));
    nodes->setToolTip(tr("Nodes Selection") + " - " + tr("N"));
    nodes->setActionId(TAction::NodesEditor);

    nodesActions.insert(TAction::NodesEditor, nodes);
}

QMap<TAction::ActionId, TAction *> NodesTool::actions() const
{
    return nodesActions;
}

TAction * NodesTool::getAction(TAction::ActionId toolId)
{
    return nodesActions[toolId];
}

int NodesTool::toolType() const
{
    return TupToolInterface::Selection;
}

QWidget *NodesTool::configurator()
{
    if (!configPanel) {
        configPanel = new NodeSettings;
        connect(configPanel, SIGNAL(nodesChanged(int)), this, SLOT(updateCurrentPath(int)));
        connect(configPanel, SIGNAL(policyChanged()), this, SLOT(resetPathHistory()));
    }

    return configPanel;
}

void NodesTool::aboutToChangeScene(TupGraphicsScene *scene)
{
    Q_UNUSED(scene)
}

void NodesTool::aboutToChangeTool()
{
}

void NodesTool::saveConfig()
{
}

QCursor NodesTool::toolCursor() // const
{
    return QCursor(Qt::ArrowCursor);
}

void NodesTool::resizeNode(qreal scaleFactor)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodesTool::resizeNodes()]";
    #endif

    realFactor = scaleFactor;
    if (activeSelection)
        nodeGroup->resizeNodes(scaleFactor);
}

void NodesTool::updateZoomFactor(qreal scaleFactor)
{
    realFactor = scaleFactor;
}

void NodesTool::clearSelection()
{
    if (scene->selectedItems().count() > 0) {
        scene->clearSelection();
        if (activeSelection)
            activeSelection = false;
        if (nodeGroup) {
            nodeGroup->clear();
            nodeGroup = nullptr;
        }
    }
}

void NodesTool::requestTransformation(QGraphicsItem *item, TupFrame *frame)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodesTool::requestTransformation(QGraphicsItem *, TupFrame *)]";
    #endif

    QDomDocument doc;
    doc.appendChild(TupSerializer::properties(item, doc));

    TupSvgItem *svg = qgraphicsitem_cast<TupSvgItem *>(item);
    int position = -1;
    TupLibraryObject::ObjectType type;

    if (svg) {
        type = TupLibraryObject::Svg;
        position = frame->indexOf(svg);
    } else {
        if (TupGraphicLibraryItem *libraryItem = qgraphicsitem_cast<TupGraphicLibraryItem *>(item)) {
            if (libraryItem->getItemType() == TupLibraryObject::Image)
                type = TupLibraryObject::Image;
            else
                type = TupLibraryObject::Item;
        } else {
            type = TupLibraryObject::Item;
        }
        position = frame->indexOf(item);
    }

    if (position >= 0) {
        TupProjectRequest event = TupRequestBuilder::createItemRequest(
                          scene->currentSceneIndex(), currentLayer, currentFrame,
                          position, QPointF(), scene->getSpaceContext(), type,
                          TupProjectRequest::Transform, doc.toString());

        emit requested(&event);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "NodesTool::requestTransformation() - Fatal Error: Invalid item position !!! [ "
                     << position << " ]";
        #endif
    }
}

void NodesTool::updateCurrentPath(int newTotal)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodesTool::updateCurrentPath()] - Update nodes total to newTotal ->" << newTotal;
    #endif

    if (activeSelection) {
        int nodesTotal = nodeGroup->mainNodesCount();
        qDebug() << "Path nodesTotal ->" << nodesTotal;
        qDebug() << "---";

        TupFrame *frame = getCurrentFrame();
        int position = frame->indexOf(nodeGroup->parentItem());
        QString path = "";

        if (nodesTotal > newTotal) { // Removing nodes
            if (nodesTotal > 2) {
                if (TupPathItem *pathItem = qgraphicsitem_cast<TupPathItem *>(nodeGroup->parentItem())) {
                    int iterations = nodesTotal - newTotal;
                    qDebug() << "Removing nodes to the path...";
                    qDebug() << "Nodes to remove ->" << iterations;
                    int nodesCounter = nodesTotal;
                    for(int i=0; i < iterations; i++) {
                        path = pathItem->refactoringPath(configPanel->policyParam(), nodesCounter);
                        nodesCounter--;
                        qDebug() << "path ->" << path;

                        TupProjectRequest event = TupRequestBuilder::createItemRequest(scene->currentSceneIndex(),
                                                                                       currentLayer, currentFrame, position,
                                                                                       QPointF(), scene->getSpaceContext(), TupLibraryObject::Item,
                                                                                       TupProjectRequest::EditNodes, path);
                        emit requested(&event);
                    }
                }
            }
        } else { // Restoring nodes
            if (TupPathItem *pathItem = qgraphicsitem_cast<TupPathItem *>(nodeGroup->parentItem())) {
                int iterations = newTotal - nodesTotal;
                qDebug() << "Adding nodes to the path...";
                qDebug() << "Nodes to add ->" << iterations;
                int nodesCounter = nodesTotal;
                for(int i=0; i < iterations; i++) {
                    path = pathItem->pathRestored(newTotal);
                    nodesCounter--;
                    qDebug() << "path ->" << path;

                    TupProjectRequest event = TupRequestBuilder::createItemRequest(scene->currentSceneIndex(),
                                                                                   currentLayer, currentFrame, position,
                                                                                   QPointF(), scene->getSpaceContext(), TupLibraryObject::Item,
                                                                                   TupProjectRequest::EditNodes, path);
                    emit requested(&event);
                }
            }
        }
    }
}

void NodesTool::resetPathHistory()
{
    if (activeSelection) {
        if (TupPathItem *pathItem = qgraphicsitem_cast<TupPathItem *>(nodeGroup->parentItem())) {
            #ifdef TUP_DEBUG
                qDebug() << "[NodesTool::resetPathHistory()] - Resetting path history...";
            #endif
            pathItem->resetPathHistory();
        }
    }
}

void NodesTool::removeNodeFromPath(QPointF pos)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodesTool::removeNodeFromPath()] - pos ->" << pos;
    #endif

    if (TupPathItem *pathItem = qgraphicsitem_cast<TupPathItem *>(nodeGroup->parentItem())) {
        TupFrame *frame = getCurrentFrame();
        int position = frame->indexOf(nodeGroup->parentItem());

        int nodesTotal = nodeGroup->mainNodesCount();
        if (nodesTotal == 2) {
            TupProjectRequest event = TupRequestBuilder::createItemRequest(scene->currentSceneIndex(),
                                      currentLayer, currentFrame, position, QPointF(), scene->getSpaceContext(),
                                      TupLibraryObject::Item, TupProjectRequest::Remove);
            emit requested(&event);
        } else {
            QString path = pathItem->removeNodeFromPath(pos);
            qDebug() << "PATH ->" << path;

            TupProjectRequest event = TupRequestBuilder::createItemRequest(scene->currentSceneIndex(),
                                                                           currentLayer, currentFrame, position,
                                                                           QPointF(), scene->getSpaceContext(), TupLibraryObject::Item,
                                                                           TupProjectRequest::EditNodes, path);
            emit requested(&event);
        }
    }
}
