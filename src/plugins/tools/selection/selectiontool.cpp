/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustav Gonzalez / xtingray                                           *
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

#include "selectiontool.h"
#include "taction.h"
#include "talgorithm.h"
#include "tupgraphicalgorithm.h"
#include "tupscene.h"
#include "tuplayer.h"
#include "tupsvgitem.h"
#include "tupsvg2qt.h"
#include "tupitemgroup.h"
#include "tupgraphicobject.h"
#include "tupinputdeviceinformation.h"
#include "tupgraphicsscene.h"
#include "nodemanager.h"
#include "tupserializer.h"
#include "tuprequestbuilder.h"
#include "tupprojectresponse.h"
#include "tosd.h"

SelectionTool::SelectionTool(): panel(nullptr)
{
    setupActions();
}

SelectionTool::~SelectionTool()
{
}

void SelectionTool::init(TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "SelectionTool::init()";
    #endif

    scene = gScene;
    targetIsIncluded = false;

    clearSelection();
    scene->clearSelection();
    nodeZValue = (2*ZLAYER_LIMIT) + (scene->currentScene()->layersCount() * ZLAYER_LIMIT);
    initItems(scene);
}

void SelectionTool::initItems(TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[SelectionTool::initItems()]";
    #endif

    foreach (QGraphicsView *view, gScene->views())
             view->setDragMode(QGraphicsView::RubberBandDrag);

    panel->enableFormControls(false);
}

void SelectionTool::removeTarget()
{
    if (targetIsIncluded) {
        scene->removeItem(center);
        scene->removeItem(target1);
        scene->removeItem(target2);
        targetIsIncluded = false;
    }
}

QStringList SelectionTool::keys() const
{
    return QStringList() << tr("Selection");
}

void SelectionTool::press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[SelectionTool::press()]";
    #endif

    Q_UNUSED(brushManager)

    activeSelection = false;
    frame = getCurrentFrame();

    // If Control key is pressed / allow multiple selection picking items one by one 
    if (input->keyModifiers() != Qt::ControlModifier) {
        foreach (NodeManager *nodeManager, nodeManagers) {
            if (!nodeManager->isPressed()) {
                nodeManager->parentItem()->setSelected(false);
                nodeManagers.removeAll(nodeManager);
                gScene->drawCurrentPhotogram();
            } 
        }
        selectedObjects.clear();
    }

    if (frame->indexOf(gScene->mouseGrabberItem()) != -1) {
        selectedObjects << gScene->mouseGrabberItem();
    } else {
        if (gScene->selectedItems().count() > 0)
            selectedObjects = gScene->selectedItems();
    }

    foreach (QGraphicsItem *item, selectedObjects) {
        QDomDocument doc;
        doc.appendChild(TupSerializer::properties(item, doc));

        TupSvgItem *svg = qgraphicsitem_cast<TupSvgItem *>(item);
        int itemIndex = -1;
        TupLibraryObject::Type type = TupLibraryObject::Item;
        if (svg) {
            type = TupLibraryObject::Svg;
            itemIndex = frame->indexOf(svg);
        } else {
            itemIndex = frame->indexOf(item);
        }
        if (itemIndex >= 0)
            frame->checkTransformationStatus(type, itemIndex);

        if (item && (dynamic_cast<TupAbstractSerializable* > (item))) {
            if (item->group())
                item = qgraphicsitem_cast<QGraphicsItem *>(item->group());
            bool found = false;
            foreach (NodeManager *nodeManager, nodeManagers) {
                if (item == nodeManager->parentItem()) {
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                NodeManager *manager = new NodeManager(item, gScene, nodeZValue);
                connect(manager, SIGNAL(rotationUpdated(int)), panel, SLOT(updateRotationAngle(int)));
                connect(manager, SIGNAL(scaleUpdated(double, double)), panel, SLOT(updateScaleFactor(double, double)));
                manager->show();
                manager->resizeNodes(realFactor);
                nodeManagers << manager;
            }
        } 
    }
}

void SelectionTool::move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    Q_UNUSED(brushManager)

    if (input->buttons() == Qt::LeftButton && gScene->selectedItems().count() > 0)
        QTimer::singleShot(0, this, SLOT(syncNodes()));
}

void SelectionTool::release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[SelectionTool::release()]";
    #endif

    Q_UNUSED(input)
    Q_UNUSED(brushManager)

    selectedObjects = gScene->selectedItems();
    if (selectedObjects.count() > 0) {
        panel->enableFormControls(true);
        activeSelection = true;
        foreach (NodeManager *manager, nodeManagers) {
            QGraphicsItem *item = manager->parentItem();
            int parentIndex = selectedObjects.indexOf(item);
            if (parentIndex != -1) { // Object is IN the list
                selectedObjects.removeAt(parentIndex); // Remove node's item from selected objects list
            } else { // Object is NOT IN the selected objects list
                int index = nodeManagers.indexOf(manager);
                disconnect(nodeManagers.at(index), SIGNAL(rotationUpdated(int)), panel, SLOT(updateRotationAngle(int)));
                disconnect(nodeManagers.at(index), SIGNAL(scaleUpdated(double, double)), panel, SLOT(updateScaleFactor(double, double)));
                delete nodeManagers.takeAt(index); // Removing node manager from nodes list
            }
        }

        foreach (QGraphicsItem *item, selectedObjects) {
            if (item && (dynamic_cast<TupAbstractSerializable* > (item))) {
                if (item->group())
                    item = qgraphicsitem_cast<QGraphicsItem *>(item->group());
                bool found = false;
                foreach (NodeManager *manager, nodeManagers) {
                    if (item == manager->parentItem()) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    NodeManager *manager = new NodeManager(item, gScene, nodeZValue);
                    connect(manager, SIGNAL(rotationUpdated(int)), panel, SLOT(updateRotationAngle(int)));
                    connect(manager, SIGNAL(scaleUpdated(double, double)), panel, SLOT(updateScaleFactor(double, double)));
                    manager->show();
                    manager->resizeNodes(realFactor);
                    nodeManagers << manager;
                }
            }
        }

        foreach (NodeManager *manager, nodeManagers) {
            if (manager->isModified())
                requestTransformation(manager->parentItem(), frame);
        }
        updateItemPosition();
        updateItemRotation();
        updateItemScale();
    } else {
        panel->enableFormControls(false);
        if (targetIsIncluded)
            targetIsIncluded = false;

        foreach (NodeManager *manager, nodeManagers) {
            manager->parentItem()->setSelected(false);
            nodeManagers.removeAll(manager);
        }

        gScene->drawCurrentPhotogram();
    }
}

TupFrame* SelectionTool::getCurrentFrame()
{
    TupFrame *frame = nullptr;
    if (scene->getSpaceContext() == TupProject::FRAMES_EDITION) {
        frame = scene->currentFrame();
        currentLayer = scene->currentLayerIndex();
        currentFrame = scene->currentFrameIndex();
    } else {
        currentLayer = -1;
        currentFrame = -1;

        TupScene *tupScene = scene->currentScene();
        TupBackground *bg = tupScene->sceneBackground();
        if (scene->getSpaceContext() == TupProject::STATIC_BACKGROUND_EDITION) {
            frame = bg->staticFrame();
        } else if (scene->getSpaceContext() == TupProject::DYNAMIC_BACKGROUND_EDITION) {
            frame = bg->dynamicFrame();
        }
    }

    return frame;
}

TupFrame* SelectionTool::frameAt(int sceneIndex, int layerIndex, int frameIndex)
{
    TupFrame *frame = nullptr;
    TupProject *project = scene->currentScene()->project();
    TupScene *sceneData = project->sceneAt(sceneIndex);
    if (sceneData) {
        if (scene->getSpaceContext() == TupProject::FRAMES_EDITION) {
            TupLayer *layer = sceneData->layerAt(layerIndex);
            if (layer) {
                frame = layer->frameAt(frameIndex);
            } else {
                #ifdef TUP_DEBUG
                    QString msg = "SelectionTool::frameAt() - Fatal Error: Layer is NULL! -> " + QString::number(layerIndex);
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
            }
        } else {
            TupBackground *bg = sceneData->sceneBackground();
            if (scene->getSpaceContext() == TupProject::STATIC_BACKGROUND_EDITION) {
                frame = bg->staticFrame();
            } else if (scene->getSpaceContext() == TupProject::DYNAMIC_BACKGROUND_EDITION) {
                frame = bg->dynamicFrame();
                bg->scheduleRender(true);
            }
       }
    } else {
       #ifdef TUP_DEBUG
           QString msg = "SelectionTool::frameAt() - Fatal Error: Scene is NULL! -> " + QString::number(sceneIndex);
           #ifdef Q_OS_WIN
               qDebug() << msg;
           #else
               tError() << msg;
           #endif
       #endif
    }

    return frame;
}

void SelectionTool::setupActions()
{
    targetIsIncluded = false;
    activeSelection = false;
    nodeManagers.clear();
    realFactor = 1;

    TAction *select = new TAction(QPixmap(kAppProp->themeDir() + "icons/selection.png"), tr("Object Selection"), this);
    select->setShortcut(QKeySequence(tr("O")));
    select->setToolTip(tr("Object Selection") + " - " + "O");

    selectActions.insert(tr("Selection"), select);
}

QMap<QString, TAction *> SelectionTool::actions() const
{
    return selectActions;
}

int SelectionTool::toolType() const
{
    return TupToolInterface::Selection;
}

QWidget *SelectionTool::configurator()
{
    if (!panel) {
        panel = new PenSettings;
        connect(panel, SIGNAL(callAlignAction(PenSettings::Align)), this, SLOT(applyAlignAction(PenSettings::Align)));
        connect(panel, SIGNAL(callFlip(PenSettings::Flip)), this, SLOT(applyFlip(PenSettings::Flip)));
        connect(panel, SIGNAL(callOrderAction(PenSettings::Order)), this, SLOT(applyOrderAction(PenSettings::Order)));
        connect(panel, SIGNAL(callGroupAction(PenSettings::Group)), this, SLOT(applyGroupAction(PenSettings::Group)));
        connect(panel, SIGNAL(positionUpdated(int, int)), this, SLOT(updateItemPosition(int, int)));
        connect(panel, SIGNAL(rotationUpdated(int)), this, SLOT(updateItemRotation(int)));
        connect(panel, SIGNAL(scaleUpdated(double, double)), this, SLOT(updateItemScale(double, double)));
        connect(panel, SIGNAL(activateProportion(bool)), this, SLOT(enableProportion(bool)));
    }

    return panel;
}

void SelectionTool::aboutToChangeScene(TupGraphicsScene *scene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[SelectionTool::aboutToChangeScene()]";
    #endif

    Q_UNUSED(scene)

    clearSelection();
}

void SelectionTool::aboutToChangeTool()
{
    #ifdef TUP_DEBUG
        qDebug() << "SelectionTool::aboutToChangeTool()";
    #endif

    init(scene);
}

void SelectionTool::itemResponse(const TupItemResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[SelectionTool::itemResponse()]";
    #endif

    if (response->getAction() == TupProjectRequest::Remove) {
        if (nodeManagers.count() == 1)
            panel->enableFormControls(false);
        return;
    }

    QGraphicsItem *item = nullptr;
    TupFrame *frame = frameAt(response->getSceneIndex(), response->getLayerIndex(), response->getFrameIndex());
    if (frame) {
        if (response->getItemType() == TupLibraryObject::Svg && frame->svgItemsCount() > 0) {
            item = frame->svgAt(response->getItemIndex());
        } else if (frame->graphicsCount() > 0) {
            item = frame->item(response->getItemIndex());
        }
    } else {
        #ifdef TUP_DEBUG
            QString msg = "SelectionTool::itemResponse - Fatal Error: frame is NULL! (index: " 
                          + QString::number(response->getFrameIndex()) + ")";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return;
    }

    updateItemPosition();
    updateItemRotation();
    updateItemScale();

#ifdef TUP_DEBUG
    qDebug() << "SelectionTool::itemResponse() - response->action() -> " + QString::number(response->getAction());
#endif

    switch (response->getAction()) {
        case TupProjectRequest::Transform:
        {
            if (item) {
                foreach (NodeManager* manager, nodeManagers) {
                    manager->show();
                    manager->syncNodesFromParent();
                    manager->beginToEdit();
                }
            } else {
                #ifdef TUP_DEBUG
                    QString msg = "SelectionTool::itemResponse - No item found";
                        #ifdef Q_OS_WIN
                            qDebug() << msg;
                        #else
                            tError() << msg;
                        #endif
                    #endif
            }
        }
        break;
        case TupProjectRequest::Move:
        {
            syncNodes();
        }
        break;
        case TupProjectRequest::Group:
        {
            nodeManagers.clear();
            selectedObjects.clear();
            /*
            if (item && (dynamic_cast<TupAbstractSerializable* > (item))) {
                if (item->group())
                    item = qgraphicsitem_cast<QGraphicsItem *>(item->group());
            }

            selectedObjects << item;
            item->setSelected(true);
            NodeManager *manager = new NodeManager(item, scene, nodeZValue);
            connect(manager, SIGNAL(rotationUpdated(int)), panel, SLOT(updateRotationAngle(int)));
            connect(manager, SIGNAL(scaleUpdated(double, double)), panel, SLOT(updateScaleFactor(double, double)));
            manager->show();
            manager->resizeNodes(realFactor);
            nodeManagers << manager;

            syncNodes();
            */
        }
        break;
        case TupProjectRequest::Ungroup:
        {
            foreach (QGraphicsItem *graphic, scene->selectedItems())
                graphic->setSelected(false);

            nodeManagers.clear();
            selectedObjects.clear();

            /*
            QString list = response->getArg().toString();
            QString::const_iterator itr = list.constBegin();
            QList<int> positions = TupSvg2Qt::parseIntList(++itr);
            qSort(positions.begin(), positions.end());
            int total = positions.size();
            for (int i=0; i<total; i++) {
                QGraphicsItem *graphic = frame->item(positions.at(i));     
                if (graphic) {
                    selectedObjects << graphic;
                    graphic->setSelected(true);
                    NodeManager *manager = new NodeManager(graphic, scene, nodeZValue);
                    connect(manager, SIGNAL(rotationUpdated(int)), panel, SLOT(updateRotationAngle(int)));
                    connect(manager, SIGNAL(scaleUpdated(double, double)), panel, SLOT(updateScaleFactor(double, double)));
                    // manager->show();
                    manager->resizeNodes(realFactor);
                    nodeManagers << manager;
                }
            }

            syncNodes();
            */
        }
        break;
        default:
        {
            syncNodes();
        }
        break;
    }
}

void SelectionTool::syncNodes()
{
    foreach (NodeManager* node, nodeManagers) {
        if (node) {
            node->show();
            if (node->parentItem())
                node->syncNodesFromParent();
        }
    }
}

void SelectionTool::saveConfig()
{
}

void SelectionTool::keyPressEvent(QKeyEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[SelectionTool::keyPressEvent()]";
    #endif

    key = "NONE";

    if (event->key() == Qt::Key_F11 || event->key() == Qt::Key_Escape) {
        emit closeHugeCanvas();
    } else if (event->modifiers() == Qt::AltModifier) {
        if (event->key() == Qt::Key_R) {
            if (selectionIsActive()) {
                foreach (NodeManager *nodeManager, nodeManagers) {
                    nodeManager->toggleAction();
                    break;
                }
            }
        }
    } else if ((event->key() == Qt::Key_Left) || (event->key() == Qt::Key_Up) 
               || (event->key() == Qt::Key_Right) || (event->key() == Qt::Key_Down)) {

        if (!selectionIsActive()) {
            QPair<int, int> flags = TupToolPlugin::setKeyAction(event->key(), event->modifiers());
            if (flags.first != -1 && flags.second != -1)
                emit callForPlugin(flags.first, flags.second);
        } else {
            int delta = 5;

            if (event->modifiers() == Qt::ShiftModifier)
                delta = 1;

            if (event->modifiers() == Qt::ControlModifier)
                delta = 10;

            selectedObjects = scene->selectedItems();
            TupFrame *frame = getCurrentFrame();

            foreach (QGraphicsItem *item, selectedObjects) {
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

            updateItemPosition();
        }
    } else if (event->modifiers() == Qt::ControlModifier) {
        panel->setProportionState(true);
        key = "CONTROL";
        if (selectionIsActive()) {
            foreach (NodeManager *nodeManager, nodeManagers)
                nodeManager->setProportion(true);
        }
    } else {
        QPair<int, int> flags = TupToolPlugin::setKeyAction(event->key(), event->modifiers());
        if (flags.first != -1 && flags.second != -1)
            emit callForPlugin(flags.first, flags.second);
    }
}

void SelectionTool::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event)

    if (key.compare("CONTROL") == 0) {
        panel->setProportionState(false);
        key = "NONE";
        if (selectionIsActive()) {
            foreach (NodeManager *nodeManager, nodeManagers)
                nodeManager->setProportion(false);
        }
    }
}

bool SelectionTool::selectionIsActive()
{
    return activeSelection;
}

void SelectionTool::applyAlignAction(PenSettings::Align align)
{
    QGraphicsView *view = scene->views().at(0);
    QRectF rect = view->sceneRect();
    QPointF center = rect.center();
    QPointF distance;
    foreach (NodeManager *manager, nodeManagers) {
        QGraphicsItem *item = manager->parentItem();
        QRectF rect = item->boundingRect();
        QPointF objectPos = rect.center();
        if (align == PenSettings::hAlign) {
            int y = static_cast<int>(center.y() - objectPos.y());
            item->setPos(item->pos().x(), y);
        } else if (align == PenSettings::vAlign) {
            int x = static_cast<int>(center.x() - objectPos.x());
            item->setPos(x, item->pos().y());
        } else if (align == PenSettings::totalAlign) {
            distance = center - objectPos;
            item->setPos(distance.x(), distance.y());
        }
        manager->syncNodesFromParent();
        requestTransformation(manager->parentItem(), frame);
    }
}

void SelectionTool::applyFlip(PenSettings::Flip flip)
{
    selectedObjects = scene->selectedItems();

    foreach (QGraphicsItem *item, selectedObjects) {
        foreach (NodeManager *manager, nodeManagers) {
            if (flip == PenSettings::Horizontal)
                manager->horizontalFlip();
            else if (flip == PenSettings::Vertical)
                manager->verticalFlip();
            else if (flip == PenSettings::Crossed)
                manager->crossedFlip();

            if (manager->isModified()) {
                QDomDocument doc;
                doc.appendChild(TupSerializer::properties(item, doc));

                TupSvgItem *svg = qgraphicsitem_cast<TupSvgItem *>(manager->parentItem());
                int position = -1;
                TupLibraryObject::Type type = TupLibraryObject::Item;
                TupFrame *frame = getCurrentFrame();
                if (svg) {
                    type = TupLibraryObject::Svg;
                    position = frame->indexOf(svg);
                } else {
                    position = frame->indexOf(manager->parentItem());
                }

                TupProjectRequest event = TupRequestBuilder::createItemRequest(
                    scene->currentSceneIndex(),
                    currentLayer, currentFrame,
                    position, QPointF(), 
                    scene->getSpaceContext(), type,
                    TupProjectRequest::Transform, doc.toString());
                    emit requested(&event);
            }
        }
    }
}

void SelectionTool::applyOrderAction(PenSettings::Order action)
{
    selectedObjects = scene->selectedItems();

    foreach (QGraphicsItem *item, selectedObjects) {
        TupSvgItem *svg = qgraphicsitem_cast<TupSvgItem *>(item);
        int position = -1;
        TupLibraryObject::Type type = TupLibraryObject::Item;
        TupFrame *frame = getCurrentFrame();
        if (svg) {
            type = TupLibraryObject::Svg;
            position = frame->indexOf(svg);
        } else {
            position = frame->indexOf(item);
        }

        TupProjectRequest event = TupRequestBuilder::createItemRequest(scene->currentSceneIndex(),
                                  currentLayer, currentFrame, position, QPointF(),
                                  scene->getSpaceContext(), type, TupProjectRequest::Move, action);
        emit requested(&event);
    }
}

void SelectionTool::applyGroupAction(PenSettings::Group action)
{
    foreach (QGraphicsItem *item, selectedObjects) {
        TupSvgItem *svg = qgraphicsitem_cast<TupSvgItem *>(item);
        if (svg) {
            TOsd::self()->display(tr("Error"), tr("SVG objects can't be grouped/ungrouped yet"), TOsd::Error);
            return;
        }
    }

    TupFrame *frame = getCurrentFrame();

    if (action == PenSettings::GroupItems) {
        selectedObjects = scene->selectedItems();
        int total = selectedObjects.count();
        if (total > 1) {
            QString items = "(";
            int i = 1;
            int position = -1; 
            foreach (QGraphicsItem *item, selectedObjects) {
                int index = frame->indexOf(item);
                if (index > -1) {
                    if (i == 1) {
                        position = index;
                    } else {
                        if (index < position) 
                            position = index;
                    }
                    items += QString::number(index); 
                    if (i < total)
                        items += ", ";
                } else {
                    #ifdef TUP_DEBUG
                        QString msg = "SelectionTool::applyGroupAction() - Fatal Error: Index of item is invalid! -> -1";
                        #ifdef Q_OS_WIN
                            qDebug() << msg;
                        #else
                            tError() << msg;
                        #endif
                    #endif
                }
                i++;
            }
            items += ")";

            foreach (QGraphicsItem *item, selectedObjects)
                item->setSelected(false);

            TupProjectRequest event = TupRequestBuilder::createItemRequest(scene->currentSceneIndex(),
                                      currentLayer, currentFrame,
                                      position, QPointF(), scene->getSpaceContext(),
                                      TupLibraryObject::Item, TupProjectRequest::Group, items);
            emit requested(&event);
        } else if (total == 1) {
            nodeManagers.clear();
            scene->drawCurrentPhotogram();
        }
    } else if (action == PenSettings::UngroupItems) {
        selectedObjects = scene->selectedItems();
        int total = selectedObjects.count();

        if (total > 0) {
            bool noAction = true;
            foreach (QGraphicsItem *item, selectedObjects) {
                item->setSelected(false);
                if (qgraphicsitem_cast<TupItemGroup *> (item)) {
                    noAction = false;
                    int itemIndex = getCurrentFrame()->indexOf(item);

                    TupProjectRequest event = TupRequestBuilder::createItemRequest(
                                              scene->currentSceneIndex(),
                                              currentLayer, currentFrame,
                                              itemIndex, QPointF(),
                                              scene->getSpaceContext(), TupLibraryObject::Item,
                                              TupProjectRequest::Ungroup);
                    emit requested(&event);
                }
            }

            if (noAction) {
                nodeManagers.clear();
                scene->drawCurrentPhotogram();
            }
        }
    }
}

QCursor SelectionTool::polyCursor() const
{
    return QCursor(Qt::ArrowCursor);
}

void SelectionTool::resizeNode(qreal scaleFactor)
{
    realFactor = scaleFactor;
    foreach (NodeManager *manager, nodeManagers)
        manager->resizeNodes(scaleFactor);
}

void SelectionTool::updateZoomFactor(qreal scaleFactor)
{
    realFactor = scaleFactor;
}

void SelectionTool::sceneResponse(const TupSceneResponse *event)
{
    if (event->getAction() == TupProjectRequest::Select)
        initItems(scene);
}

void SelectionTool::updateItemPosition() 
{
    #ifdef TUP_DEBUG
        qDebug() << "[SelectionTool::updateItemPosition()]";
    #endif

    if (nodeManagers.count() == 1) {
        NodeManager *manager = nodeManagers.first();
        QGraphicsItem *item = manager->parentItem();
        QPoint point = item->mapToScene(item->boundingRect().center()).toPoint();
        panel->setPos(point.x(), point.y());
    } else { 
        if (nodeManagers.count() > 1) {
            NodeManager *manager = nodeManagers.first();
            QGraphicsItem *item = manager->parentItem();
            QPoint left = item->mapToScene(item->boundingRect().topLeft()).toPoint();  
            QPoint right = item->mapToScene(item->boundingRect().bottomRight()).toPoint();
            int minX = left.x();
            int maxX = right.x(); 
            int minY = left.y();
            int maxY = right.y();

            foreach (NodeManager *node, nodeManagers) {
                QGraphicsItem *item = node->parentItem();
                QPoint left = item->mapToScene(item->boundingRect().topLeft()).toPoint(); 
                int leftX = left.x();
                int leftY = left.y();
                if (leftX < minX)
                    minX = leftX;
                 if (leftY < minY)
                    minY = leftY;
                QPoint right = item->mapToScene(item->boundingRect().bottomRight()).toPoint();  
                int rightX = right.x();
                int rightY = right.y();
                if (rightX > maxX)
                    maxX = rightX;
                if (rightY > maxY)
                    maxY = rightY;
            }
            int x = minX + ((maxX - minX)/2); 
            int y = minY + ((maxY - minY)/2);
            panel->setPos(x, y);

            if (!targetIsIncluded) {
                center = new TupEllipseItem(QRectF(QPointF(x - 1, y - 1), QSize(2, 2)));
                target1 = new QGraphicsLineItem(x, y - 6, x, y + 6);
                target2 = new QGraphicsLineItem(x - 6, y, x + 6, y);
              
                QPen pen(QColor(255, 0, 0), 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                center->setPen(pen);
                center->setBrush(QColor(255, 0, 0));
                center->setZValue(nodeZValue);
                scene->includeObject(center);

                target1->setPen(pen);
                target1->setZValue(nodeZValue);
                target2->setPen(pen);
                target2->setZValue(nodeZValue);

                scene->includeObject(target1);
                scene->includeObject(target2);

                targetIsIncluded = true;
            } else {
                QPoint current = center->mapToScene(center->boundingRect().topLeft()).toPoint();
                int deltaX = x - current.x() - 1;
                int deltaY = y - current.y() - 1;
                center->moveBy(deltaX, deltaY);
                target1->moveBy(deltaX, deltaY);
                target2->moveBy(deltaX, deltaY);
            }
        }
    }
}

void SelectionTool::updateItemRotation()
{
#ifdef TUP_DEBUG
        qDebug() << "[SelectionTool::updateItemRotation()]";
#endif

    if (nodeManagers.count() > 0) {
        NodeManager *manager = nodeManagers.first();
        QGraphicsItem *item = manager->parentItem();
        qreal angle = item->data(TupGraphicObject::Rotate).toReal();
        panel->updateRotationAngle(static_cast<int>(angle));
    }
}

void SelectionTool::updateItemScale()
{
#ifdef TUP_DEBUG
    qDebug() << "[SelectionTool::updateItemRotation()]";
#endif

    if (nodeManagers.count() > 0) {
        NodeManager *manager = nodeManagers.first();
        QGraphicsItem *item = manager->parentItem();
        qreal scaleX = item->data(TupGraphicObject::ScaleX).toReal();
        if (scaleX == 0.0) {
            scaleX = 1;
            item->setData(TupGraphicObject::ScaleX, 1);
        }
        qreal scaleY = item->data(TupGraphicObject::ScaleY).toReal();
        if (scaleY == 0.0) {
            scaleY = 1;
            item->setData(TupGraphicObject::ScaleY, 1);
        }
        panel->updateScaleFactor(scaleX, scaleY);
    }
}

void SelectionTool::updateItemPosition(int x, int y) 
{
#ifdef TUP_DEBUG
    qDebug() << "[SelectionTool::updateItemPosition(int, int)]";
#endif

    if (nodeManagers.count() == 1) {
        NodeManager *manager = nodeManagers.first();
        QGraphicsItem *item = manager->parentItem();
        item->moveBy(x, y);
        manager->syncNodesFromParent();
        requestTransformation(manager->parentItem(), frame);
    } else {
        if (nodeManagers.count() > 1) {
            foreach (NodeManager *manager, nodeManagers) {
                QGraphicsItem *item = manager->parentItem();
                item->moveBy(x, y);
                manager->syncNodesFromParent();
                requestTransformation(manager->parentItem(), frame);
            }
            center->moveBy(x, y);
            target1->moveBy(x, y);
            target2->moveBy(x, y);
        }
    }
}

void SelectionTool::updateItemRotation(int angle)
{
#ifdef TUP_DEBUG
    qDebug() << "[SelectionTool::updateItemRotation(int)]";
#endif

    if (nodeManagers.count() == 1) {
        NodeManager *manager = nodeManagers.first();
        manager->rotate(angle);
        if (manager->isModified())
            requestTransformation(manager->parentItem(), frame);
    } else {
        if (nodeManagers.count() > 1) {
            foreach (NodeManager *manager, nodeManagers) {
                manager->rotate(angle);
                if (manager->isModified())
                    requestTransformation(manager->parentItem(), frame);
            }
        }
    }
}

void SelectionTool::updateItemScale(double xFactor, double yFactor)
{
#ifdef TUP_DEBUG
    qDebug() << "[SelectionTool::updateItemScale(float, float)]";
#endif

    if (nodeManagers.count() == 1) {
        NodeManager *manager = nodeManagers.first();
        manager->scale(xFactor, yFactor);
        if (manager->isModified())
            requestTransformation(manager->parentItem(), frame);
    } else {
        if (nodeManagers.count() > 1) {
            foreach (NodeManager *manager, nodeManagers) {
                manager->scale(xFactor, yFactor);
                if (manager->isModified())
                    requestTransformation(manager->parentItem(), frame);
            }
        }
    }
}

void SelectionTool::requestTransformation(QGraphicsItem *item, TupFrame *frame)
{
#ifdef TUP_DEBUG
    qDebug() << "[SelectionTool::requestTransformation(QGraphicsItem *, TupFrame *)]";
#endif

    QDomDocument doc;
    doc.appendChild(TupSerializer::properties(item, doc));
    TupSvgItem *svg = qgraphicsitem_cast<TupSvgItem *>(item);
    int position = -1;
    TupLibraryObject::Type type;

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
            QString msg = "SelectionTool::requestTransformation() - Fatal Error: Invalid item position !!! [ "
                    + QString::number(position) + " ]";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }
}

void SelectionTool::clearSelection()
{
    if (activeSelection) {
        if (!nodeManagers.isEmpty()) {
            foreach (NodeManager *nodeManager, nodeManagers) {
                nodeManager->parentItem()->setSelected(false);
                nodeManagers.removeAll(nodeManager);
            }
            nodeManagers.clear();
        }
        selectedObjects.clear();
        activeSelection = false;
        scene->drawCurrentPhotogram();
    }
}

void SelectionTool::enableProportion(bool flag)
{
    key = "NONE";
    if (flag)
        key = "CONTROL";

    if (selectionIsActive()) {
        foreach (NodeManager *nodeManager, nodeManagers)
            nodeManager->setProportion(flag);
    }
}
