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

    Q_UNUSED(brushManager)

    QList<QGraphicsItem *> currentSelection = gScene->selectedItems();
    if (!currentSelection.isEmpty()) {
        QGraphicsItem *selectedItem = currentSelection.at(0);
        TupFrame *frame = getCurrentFrame();
        int itemIndex = frame->indexOf(selectedItem);

        if (qgraphicsitem_cast<TupSvgItem *>(selectedItem)) {
            TOsd::self()->display(TOsd::Error, tr("SVG objects cannot be edited!"));
            return;
        }

        if (TupGraphicLibraryItem *libraryItem = qgraphicsitem_cast<TupGraphicLibraryItem *>(selectedItem)) {
            if (libraryItem->getItemType() == TupLibraryObject::Image) {
                TOsd::self()->display(TOsd::Error, tr("Images have no nodes!"));
                return;
            }
        }

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

        if (!qgraphicsitem_cast<TControlNode*>(selectedItem)) {
            if (!qgraphicsitem_cast<TupPathItem *>(selectedItem)) {
                TOsd::self()->display(TOsd::Error, tr("Only pencil/ink lines can be edited!"));
                return;
            }
        }

        if (itemIndex == -1) {
            if (qgraphicsitem_cast<TControlNode*>(selectedItem)) {
                QGraphicsItem *item = nodeGroup->parentItem();
                int position = frame->indexOf(item);
                if (position >= 0) {
                    // if (qgraphicsitem_cast<QGraphicsPathItem *>(item)) {
                        QString path = qgraphicsitem_cast<TupPathItem *>(item)->pathToString();
                        TupProjectRequest event = TupRequestBuilder::createItemRequest(gScene->currentSceneIndex(),
                                                  currentLayer, currentFrame, position,
                                                  QPointF(), gScene->getSpaceContext(), TupLibraryObject::Item,
                                                  TupProjectRequest::EditNodes, path);
                        emit requested(&event);
                        nodeGroup->clearChangedNodes();
                    // }
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
            if (oldIndex != itemIndex) {
                nodeGroup->clear();
                nodeGroup = new TNodeGroup(selectedItem, gScene, TNodeGroup::LineSelection, nodeZValue);
                nodeGroup->show();
                nodeGroup->resizeNodes(realFactor);
                if (TupPathItem *path = qgraphicsitem_cast<TupPathItem *>(selectedItem)) {
                    if (path->isNotEdited()) 
                        path->saveOriginalPath();
                }
            } else {
                if (nodeGroup->hasChangedNodes()) {
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
                        qDebug() << "[NodesTool::release()] - Node group has NO changes!";
                    #endif
                }
            }
        } else {
            nodeGroup = new TNodeGroup(selectedItem, gScene, TNodeGroup::LineSelection, nodeZValue);
            nodeGroup->show();
            activeSelection = true;

            nodeGroup->resizeNodes(realFactor);
            if (TupPathItem *path = qgraphicsitem_cast<TupPathItem *>(selectedItem)) {
                if (path->isNotEdited())
                    path->saveOriginalPath();
            }
        }
    } else {
        if (activeSelection) {
            #ifdef TUP_DEBUG
                qDebug() << "[NodesTool::release()] - Empty selection! Removing nodes...";
            #endif
            nodeGroup->clear();
            nodeGroup = nullptr;
            activeSelection = false;
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
        if (scene->getSpaceContext() == TupProject::VECTOR_STATIC_BG_MODE) {
            frame = bg->vectorStaticFrame();
        } else if (scene->getSpaceContext() == TupProject::VECTOR_FG_MODE) {
            frame = bg->vectorForegroundFrame();
        } else if (scene->getSpaceContext() == TupProject::VECTOR_DYNAMIC_BG_MODE) {
            frame = bg->vectorDynamicFrame();
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
             if (item) {
                 nodeGroup = new TNodeGroup(item, scene, TNodeGroup::LineSelection, nodeZValue);
             } else {
                 #ifdef TUP_DEBUG
                     qDebug() << "[NodesTool::itemResponse()] - Fatal Error: No item was found";
                 #endif
             }
        }
        break;
        case TupProjectRequest::EditNodes:
        {
             if (item) {
                 if (activeSelection) {
                     if (qgraphicsitem_cast<QGraphicsPathItem *>(nodeGroup->parentItem()) == item) {
                         nodeGroup->show();
                         nodeGroup->syncNodesFromParent();
                         nodeGroup->saveParentProperties();
                     }
                 } else {
                     nodeGroup = new TNodeGroup(item, scene, TNodeGroup::LineSelection, nodeZValue);
                     nodeGroup->show();
                     activeSelection = true;
                     nodeGroup->resizeNodes(realFactor);
                 }
             } else {
                 #ifdef TUP_DEBUG
                     qDebug() << "[NodesTool::itemResponse()] - Fatal Error: No item was found";
                 #endif
             }
        }
        break;
        case TupProjectRequest::Remove:
        {
             return;
        }
        case TupProjectRequest::Ungroup:
        {
             // reset(scene);
             if (item) {
                 nodeGroup = new TNodeGroup(item, scene, TNodeGroup::LineSelection, nodeZValue);
                 nodeGroup->show();
                 activeSelection = true;
                 nodeGroup->resizeNodes(realFactor);
             } else {
                 #ifdef TUP_DEBUG
                     qDebug() << "[NodesTool::itemResponse()] - Fatal error: No item was found";
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
    if (event->key() == Qt::Key_F11 || event->key() == Qt::Key_Escape) {
        emit closeHugeCanvas();
    } else {
        if (!activeSelection) {
            QPair<int, int> flags = TupToolPlugin::setKeyAction(event->key(), event->modifiers());
            if (flags.first != -1 && flags.second != -1)
                emit callForPlugin(flags.first, flags.second);
        } else {
            int delta = 5;

            if (event->modifiers() == Qt::ShiftModifier)
                delta = 1;

            if (event->modifiers() == Qt::ControlModifier)
                delta = 10;

            TupFrame *frame = getCurrentFrame();
            QGraphicsItem *item = nodeGroup->parentItem();

            if (event->key() == Qt::Key_Left)
                item->moveBy(-delta, 0);

            if (event->key() == Qt::Key_Up)
                item->moveBy(0, -delta);

            if (event->key() == Qt::Key_Right)
                item->moveBy(delta, 0);

            if (event->key() == Qt::Key_Down)
                item->moveBy(0, delta);

            QTimer::singleShot(0, this, SLOT(syncNodes()));
            requestTransformation(item, frame);
        }
    }
}

void NodesTool::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event)
}

void NodesTool::setupActions()
{
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

int NodesTool::toolType() const
{
    return TupToolInterface::Selection;
}

QWidget *NodesTool::configurator()
{
    return nullptr;
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

QCursor NodesTool::polyCursor() // const
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
                        + QString::number(position) + " ]";
        #endif
    }
}
