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

    baseZValue = (2*ZLAYER_LIMIT) + (scene->currentScene()->layersCount() * ZLAYER_LIMIT);
}

QStringList NodesTool::keys() const
{
    return QStringList() << tr("Nodes Selection") ;
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
        qDebug() << "NodesTool::release()";
    #endif

    Q_UNUSED(brushManager)

    QList<QGraphicsItem *> currentSelection = gScene->selectedItems();
    if (!currentSelection.isEmpty()) {
        QGraphicsItem *selectedItem = currentSelection.at(0);
        TupFrame *frame = getCurrentFrame();
        int itemIndex = frame->indexOf(selectedItem);

        if (qgraphicsitem_cast<TupSvgItem *>(selectedItem)) {
            TOsd::self()->display(tr("Error"), tr("SVG objects cannot be edited!"), TOsd::Error);
            return;
        }

        if (TupGraphicLibraryItem *libraryItem = qgraphicsitem_cast<TupGraphicLibraryItem *>(selectedItem)) {
            if (libraryItem->getItemType() == TupLibraryObject::Image) {
                TOsd::self()->display(tr("Error"), tr("Images have no nodes!"), TOsd::Error);
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
                TOsd::self()->display(tr("Error"), tr("Only pencil/ink lines can be edited!"), TOsd::Error);
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
                        QString msg = "NodesTool::release() - Fatal Error: Invalid position [ " + QString::number(position) + " ]";
                        #ifdef Q_OS_WIN
                            qDebug() << msg;
                        #else
                            tError() << msg;
                        #endif
                    #endif
                }
            } else {
                #ifdef TUP_DEBUG
                    QString msg = "NodesTool::release() - Invalid selected item index: " + QString::number(itemIndex);
                    #ifdef Q_OS_WIN
                       qDebug() << msg;
                    #else
                       tWarning() << msg;
                    #endif
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
                nodeGroup = new TNodeGroup(selectedItem, gScene, TNodeGroup::LineSelection, baseZValue);
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
                            QString msg = "NodesTool::release() - Fatal Error: Invalid position [ " + QString::number(position) + " ]";
                            #ifdef Q_OS_WIN
                                qDebug() << msg;
                            #else
                                tError() << msg;
                            #endif
                        #endif
                    }
                } else {
                    #ifdef TUP_DEBUG
                        QString msg = "NodesTool::release() - Node group has NO changes!";
                        #ifdef Q_OS_WIN
                            qDebug() << msg;
                        #else
                            tWarning() << msg;
                        #endif
                    #endif
                }
            }
        } else {
            nodeGroup = new TNodeGroup(selectedItem, gScene, TNodeGroup::LineSelection, baseZValue);
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
                QString msg = "NodesTool::release() - Empty selection! Removing nodes...";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tWarning() << msg;
                #endif
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
        } else if (scene->getSpaceContext() == TupProject::VECTOR_DYNAMIC_BG_MODE) {
            frame = bg->vectorDynamicFrame();
        }
    }

    return frame;
}

void NodesTool::itemResponse(const TupItemResponse *response)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[NodesTool::itemResponse()]";
        #else
            T_FUNCINFOX("NodesTool");
        #endif
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
                     nodeGroup = new TNodeGroup(item, scene, TNodeGroup::LineSelection, baseZValue);
                 } else {
                     #ifdef TUP_DEBUG
                         QString msg = "NodesTool::itemResponse() - Fatal Error: No item was found";
                         #ifdef Q_OS_WIN
                             qDebug() << msg;
                         #else
                             tError() << msg;
                         #endif
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
                         nodeGroup = new TNodeGroup(item, scene, TNodeGroup::LineSelection, baseZValue);
                         nodeGroup->show();
                         activeSelection = true;
                         nodeGroup->resizeNodes(realFactor);
                     }
                 } else {
                     #ifdef TUP_DEBUG
                         QString msg = "NodesTool::itemResponse() - Fatal Error: No item was found";
                         #ifdef Q_OS_WIN
                             qDebug() << msg;
                         #else
                             tError() << msg;
                         #endif
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
                     nodeGroup = new TNodeGroup(item, scene, TNodeGroup::LineSelection, baseZValue);
                     nodeGroup->show();
                     activeSelection = true;
                     nodeGroup->resizeNodes(realFactor);
                 } else {
                     #ifdef TUP_DEBUG
                         QString msg = "NodesTool::itemResponse() - Fatal error: No item was found";
                         #ifdef Q_OS_WIN
                             qDebug() << msg;
                         #else
                             tError() << msg;
                         #endif
                     #endif
                 }

                 return;
            }
            default:
            {
                 #ifdef TUP_DEBUG
                     qDebug() << "NodesTool::itemResponse() - default action"; 
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
        QPair<int, int> flags = TupToolPlugin::setKeyAction(event->key(), event->modifiers());
        if (flags.first != -1 && flags.second != -1)
            emit callForPlugin(flags.first, flags.second);
    }
}

void NodesTool::setupActions()
{
    activeSelection = false;

    TAction *nodes = new TAction(QPixmap(kAppProp->themeDir() + "icons/nodes.png"), tr("Nodes Selection"), this);
    nodes->setShortcut(QKeySequence(tr("N")));
    nodes->setToolTip(tr("Nodes Selection") + " - " + "N");

    nodesActions.insert(tr("Nodes Selection"), nodes);
}

QMap<QString, TAction *> NodesTool::actions() const
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

QCursor NodesTool::polyCursor() const
{
    return QCursor(Qt::ArrowCursor);
}

void NodesTool::resizeNode(qreal scaleFactor)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[NodesTool::resizeNodes()]";
        #else
            T_FUNCINFOX("tools");
        #endif
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

