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

#include "polylinetool.h"
#include "tupscene.h"
#include "tuplayer.h"
#include "tuprequestbuilder.h"
#include "tuplibraryobject.h"
#include "taction.h"
#include "tupinputdeviceinformation.h"
#include "tupgraphicsscene.h"
#include "tupprojectrequest.h"
#include "tupbrushmanager.h"
#include "tupprojectresponse.h"

PolyLineTool::PolyLineTool()
{
    configPanel = nullptr;
    nodeGroup = nullptr;
    pathItem = nullptr;
    cutterOn = false;
    movingOn = false;

    cursor = QCursor(kAppProp->themeDir() + "cursors/target.png", 4, 4);
    
    line1 = new QGraphicsLineItem(0, 0, 0, 0);
    line1->setPen(QPen(QColor(55, 177, 50)));
    line2 = new QGraphicsLineItem(0, 0, 0, 0);
    line2->setPen(QPen(QColor(55, 177, 50)));
    
    setupActions();
}

PolyLineTool::~PolyLineTool()
{
    delete configPanel;
}

void PolyLineTool::setupActions()
{
    TAction *polyline = new TAction(QIcon(ICONS_DIR + "polyline.png"), tr("PolyLine"), this);
    polyline->setShortcut(QKeySequence(tr("S")));
    polyline->setToolTip(tr("PolyLine") + " - " + tr("S"));
    polyline->setCursor(cursor);
    polyline->setActionId(TAction::Polyline);

    polyActions.insert(TAction::Polyline, polyline);
}

QMap<TAction::ActionId, TAction *> PolyLineTool::actions() const
{
    return polyActions;
}

TAction * PolyLineTool::getAction(TAction::ActionId toolId)
{
    return polyActions[toolId];
}

void PolyLineTool::init(TupGraphicsScene *gScene)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[PolyLineTool::init()]";
    #endif
    */

    if (gScene) {
        scene = gScene;
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[PolyLineTool::init()] - Fatal Error: TupGraphicsScene variable is NULL!";
        #endif

        return;
    }

    foreach (QGraphicsView *view, scene->views())
        view->setDragMode(QGraphicsView::NoDrag);

    cutterOn = false;
    initEnv();
}

QList<TAction::ActionId> PolyLineTool::keys() const
{
    return QList<TAction::ActionId>() << TAction::Polyline;
}

void PolyLineTool::press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PolyLineTool::press()]";
    #endif

    if (cutterOn)
        return;

    if (begin) { // This condition happens only in the beginning of the polyline
        path = QPainterPath();
        path.moveTo(input->pos());

        pathItem = new TupPathItem();
        pathItem->setPen(brushManager->pen());
        pathItem->setBrush(brushManager->brush());
        pathItem->setPath(path);
    } else { // This condition happens from the second point of the polyline and until the last one
        if (pathItem) {
            if (!gScene->items().contains(pathItem))
                gScene->includeObject(pathItem, true); // SQA: Polyline hack

            if (movingOn) {
                path.cubicTo(right, mirror, input->pos());
                movingOn = false;
            } else {
                path.cubicTo(lastPoint, lastPoint, lastPoint);
            }
            pathItem->setPath(path);
        }
    }
    
    center = input->pos();
    
    if (!gScene->items().contains(line1))
        gScene->includeObject(line1);

    if (!gScene->items().contains(line2))
        gScene->includeObject(line2);
}

void PolyLineTool::move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    Q_UNUSED(brushManager)
    Q_UNUSED(gScene)

    if (cutterOn)
        return;

    movingOn = true;
    mirror = center - (input->pos() - center);
    if (begin) {
        right = input->pos();
    } else {
        for (int i=path.elementCount()-1; i>=0; i--) {
            if (path.elementAt(i).type == QPainterPath::CurveToElement) {
                right = input->pos();
                if (path.elementAt(i + 1).type == QPainterPath::CurveToDataElement)
                    path.setElementPositionAt(i + 1, mirror.x(), mirror.y());
                break;
            }
        }
    }

    if (pathItem)
        pathItem->setPath(path);
    if (line1)
        line1->setLine(QLineF(mirror, center));
    if (line2)
        line2->setLine(QLineF(right, center));
}

void PolyLineTool::release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PolyLineTool::release()]";
    #endif

    Q_UNUSED(input)
    Q_UNUSED(brushManager)

    if (cutterOn)
        return;

    if (begin && pathItem) {
        // Adding path item within the scene
        QDomDocument doc;
        doc.appendChild(pathItem->toXml(doc));
        TupProjectRequest request = TupRequestBuilder::createItemRequest(gScene->currentSceneIndex(),
                                                       gScene->currentLayerIndex(),
                                                       gScene->currentFrameIndex(),
                                                       0, QPointF(), gScene->getSpaceContext(), TupLibraryObject::Item,
                                                       TupProjectRequest::Add, doc.toString());
        emit requested(&request);
 
        begin = false;
    } else {
        // Extending current path item
        if (pathItem) {
            if (!nodeGroup) {
                nodeGroup = new TNodeGroup(pathItem, gScene, TNodeGroup::Polyline,
                                              static_cast<int>(pathItem->zValue() + 1));
                connect(nodeGroup, SIGNAL(nodeReleased()), this, SLOT(nodeChanged()));
            } else {
                nodeGroup->createNodes(pathItem);
            }

            nodeGroup->show();
            nodeGroup->resizeNodes(realFactor);

            int position = getItemPosition();
            if (position >= 0) {
                QString pathStr = pathItem->pathToString();
                TupProjectRequest event = TupRequestBuilder::createItemRequest(gScene->currentSceneIndex(), gScene->currentLayerIndex(),
                                                                               gScene->currentFrameIndex(), position, QPointF(), gScene->getSpaceContext(),
                                                                               TupLibraryObject::Item, TupProjectRequest::EditNodes, pathStr);
                emit requested(&event);
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[PolyLineTool::release()] - Fatal Error: Path index is invalid! (-1)";
                #endif
            }
        }
    }
}

void PolyLineTool::itemResponse(const TupItemResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PolyLineTool::itemResponse()] - Action ->" << response->getAction();
    #endif

    QGraphicsItem *item = nullptr;
    TupScene *sceneVar = nullptr;
    TupLayer *layer = nullptr;
    TupFrame *frame = nullptr;

    TupProject *project = scene->currentScene()->project();
    sceneVar = project->sceneAt(response->getSceneIndex());
    if (sceneVar) {
        if (scene->getSpaceContext() == TupProject::FRAMES_MODE) {
            layer = sceneVar->layerAt(response->getLayerIndex());
            if (layer) {
                frame = layer->frameAt(response->getFrameIndex());
                if (frame) {
                    item = frame->item(response->getItemIndex());
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "[PolyLineTool::itemResponse()] - Fatal Error: Frame variable is NULL!";
                    #endif
                }
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[PolyLineTool::itemResponse()] - Fatal Error: Layer variable is NULL!";
                #endif
            }
        } else {
            TupBackground *bg = sceneVar->sceneBackground();
            if (bg) {
                if (scene->getSpaceContext() == TupProject::VECTOR_STATIC_BG_MODE) {
                    frame = bg->vectorStaticFrame();
                    if (frame) {
                        item = frame->item(response->getItemIndex());
                    } else {
                        #ifdef TUP_DEBUG
                            qDebug() << "[PolyLineTool::itemResponse()] - Fatal Error: Static bg frame variable is NULL!";
                        #endif
                    }
                } else if (scene->getSpaceContext() == TupProject::VECTOR_DYNAMIC_BG_MODE) {
                           frame = bg->vectorDynamicFrame();
                           if (frame) {
                               item = frame->item(response->getItemIndex());
                           } else {
                               #ifdef TUP_DEBUG
                                   qDebug() << "[PolyLineTool::itemResponse()] - Fatal Error: Dynamic bg frame variable is NULL!";
                               #endif
                           }
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "[PolyLineTool::itemResponse()] - Fatal Error: Invalid spaceContext!";
                    #endif
                }
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[PolyLineTool::itemResponse()] - Fatal Error: Scene bg variable is NULL!";
                #endif
            }
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[PolyLineTool::itemResponse()] - Fatal Error: Scene variable is NULL!";
        #endif
    }

    switch (response->getAction()) {
        case TupProjectRequest::Add:
        {
            if (TupPathItem *pItem = qgraphicsitem_cast<TupPathItem *>(item)) {
                if (pathItem != pItem) {
                    pathItem = pItem;
                    if (nodeGroup)
                        nodeGroup->setParentItem(pItem);
                }
            }            
        }
        break;
        case TupProjectRequest::Remove:
        {
            // Undo Action
            initEnv();
        }
        break;
        case TupProjectRequest::EditNodes:
        {
            if (item && nodeGroup) {
                if (qgraphicsitem_cast<QGraphicsPathItem *>(nodeGroup->parentItem()) == item) {                    
                    nodeGroup->createNodes(pathItem);
                    nodeGroup->resizeNodes(realFactor);

                    nodeGroup->show();
                    nodeGroup->syncNodesFromParent();
                    nodeGroup->saveParentProperties();
                    path = pathItem->path();
                }
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[PolyLineTool::itemResponse()] - Fatal Error: No item was found";
                #endif
            }
        }
        break;
        default: 
        break;
    }
}

void PolyLineTool::keyPressEvent(QKeyEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PolyLineTool::keyPressEvent()]";
    #endif

    if (event->key() == Qt::Key_F11 || event->key() == Qt::Key_Escape) {
        emit closeHugeCanvas();
    } else if (event->key() == Qt::Key_X) {
        cutterOn = true;
        initEnv();
    } else {
        QPair<int, int> flags = TAction::setKeyAction(event->key(), event->modifiers());
        if (flags.first != -1 && flags.second != -1)
            emit callForPlugin(flags.first, flags.second);
    }
}

void PolyLineTool::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_X)
        cutterOn = false;
}

void PolyLineTool::initEnv()
{
    #ifdef TUP_DEBUG
        qDebug() << "[PolyLineTool::initEnv()]";
    #endif

    if (pathItem) {
        clearSelection();
        pathItem = nullptr;
    }

    begin = true;
    movingOn = false;
    path = QPainterPath();

    if (line1) {
        if (scene->items().contains(line1))
            scene->removeItem(line1);
    }

    if (line2) {
        if (scene->items().contains(line2))
            scene->removeItem(line2);
    }
}

int PolyLineTool::getItemPosition()
{
    int position = -1;
    if (scene->getSpaceContext() == TupProject::FRAMES_MODE) {
        position = scene->currentFrame()->indexOf(nodeGroup->parentItem());
    } else {
        TupBackground *bg = scene->currentScene()->sceneBackground();
        if (bg) {
            if (scene->getSpaceContext() == TupProject::VECTOR_STATIC_BG_MODE) {
                TupFrame *frame = bg->vectorStaticFrame();
                if (frame) {
                    position = frame->indexOf(nodeGroup->parentItem());
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "[PolyLineTool::getItemPosition()] - Fatal Error: Static bg frame is NULL!";
                    #endif
                }
            } else if (scene->getSpaceContext() == TupProject::VECTOR_DYNAMIC_BG_MODE) {
                TupFrame *frame = bg->vectorDynamicFrame();
                if (frame) {
                    position = frame->indexOf(nodeGroup->parentItem());
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "[PolyLineTool::getItemPosition()] - Fatal Error: Dynamic bg frame is NULL!";
                    #endif
                }
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[PolyLineTool::getItemPosition()] - Fatal Error: Invalid spaceContext!";
                #endif
            }
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[PolyLineTool::getItemPosition()] - Fatal Error: Scene background variable is NULL!";
            #endif
        }
    }

    return position;
}

void PolyLineTool::nodeChanged()
{
    #ifdef TUP_DEBUG
        qDebug() << "[PolyLineTool::nodeChanged()]";
    #endif

    if (nodeGroup) {
        if (!nodeGroup->changedNodes().isEmpty()) {
            int position = getItemPosition();
            if (position >= 0) {
                TupPathItem *item = qgraphicsitem_cast<TupPathItem *>(nodeGroup->parentItem());
                if (item) {
                    QString pathStr = item->pathToString();
                    TupProjectRequest event = TupRequestBuilder::createItemRequest(scene->currentSceneIndex(), scene->currentLayerIndex(),
                                                                                   scene->currentFrameIndex(), position, QPointF(), scene->getSpaceContext(),
                                                                                   TupLibraryObject::Item, TupProjectRequest::EditNodes, pathStr);
                    emit requested(&event);
                }
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[PolyLineTool::nodeChanged()] - Fatal Error: Invalid object index || No nodeGroup parent item -> " + QString::number(position);
                #endif
                return;
             }
        } else {
          #ifdef TUP_DEBUG
              qDebug() << "[PolyLineTool::nodeChanged()] - Fatal Error: Array of changed nodes is empty!";
          #endif
          return;
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[PolyLineTool::nodeChanged()] - Fatal Error: Array of nodes is empty!";
        #endif
        return;
    }
}

int PolyLineTool::toolType() const
{
    return TupToolInterface::Brush;
}

QWidget *PolyLineTool::configurator()
{
    if (!configPanel)
        configPanel = new PolylineSettings;

    return configPanel;
}

void PolyLineTool::aboutToChangeScene(TupGraphicsScene *)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PolyLineTool::aboutToChangeScene()]";
    #endif

    cutterOn = true;
    initEnv();
    cutterOn = false;
}

void PolyLineTool::aboutToChangeTool()
{
    nodeGroup = nullptr;
    pathItem = nullptr;
}

void PolyLineTool::saveConfig()
{
}

QCursor PolyLineTool::toolCursor() // const
{
    return cursor;
}

void PolyLineTool::resizeNode(qreal scaleFactor)
{
    realFactor = scaleFactor;
    if (nodeGroup)
        nodeGroup->resizeNodes(scaleFactor);
}

void PolyLineTool::updateZoomFactor(qreal scaleFactor)
{
    realFactor = scaleFactor;
}

void PolyLineTool::clearSelection()
{
    if (nodeGroup) {
        nodeGroup->clear();
        nodeGroup = nullptr;
    }
}

void PolyLineTool::updatePos(QPointF pos)
{
    lastPoint = pos;
}
