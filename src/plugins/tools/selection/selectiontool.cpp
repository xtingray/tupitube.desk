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
#include "tupellipseitem.h"
#include "tupgraphicobject.h"
#include "tupinputdeviceinformation.h"
#include "tupgraphicsscene.h"
#include "nodemanager.h"
#include "tupserializer.h"
#include "tuprequestbuilder.h"
#include "tupprojectresponse.h"
#include "tosd.h"

struct SelectionTool::Private
{
    QMap<QString, TAction *> actions;
    QList<QGraphicsItem *> selectedObjects;
    QList<NodeManager*> nodeManagers;
    TupGraphicsScene *scene;
    bool activeSelection;
    qreal realFactor;
    int nodeZValue;

    TupEllipseItem *center;
    QGraphicsLineItem *target1;
    QGraphicsLineItem *target2;
    bool targetIsIncluded;
    QString key;

    int currentLayer;
    int currentFrame;
    TupFrame *frame;
};

SelectionTool::SelectionTool(): k(new Private), panel(0)
{
    setupActions();
}

SelectionTool::~SelectionTool()
{
    delete k;
}

void SelectionTool::init(TupGraphicsScene *scene)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[SelectionTool::init()]";
        #else
            T_FUNCINFOX("tools");
        #endif
    #endif

    k->scene = scene;
    k->targetIsIncluded = false; 

    clearSelection();
    k->scene->clearSelection();
    k->nodeZValue = (2*ZLAYER_LIMIT) + (scene->currentScene()->layersCount() * ZLAYER_LIMIT);
    initItems(scene);
}

void SelectionTool::initItems(TupGraphicsScene *scene)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[SelectionTool::initItems()]";
        #else
            T_FUNCINFOX("tools");
        #endif
    #endif

    foreach (QGraphicsView *view, scene->views())
             view->setDragMode(QGraphicsView::RubberBandDrag);

    panel->enableFormControls(false);
}

void SelectionTool::removeTarget()
{
    if (k->targetIsIncluded) {
        k->scene->removeItem(k->center);
        k->scene->removeItem(k->target1);
        k->scene->removeItem(k->target2);
        k->targetIsIncluded = false;
    }
}

QStringList SelectionTool::keys() const
{
    return QStringList() << tr("Selection");
}

void SelectionTool::press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[SelectionTool::press()]";
        #else
            T_FUNCINFOX("tools");
        #endif
    #endif

    Q_UNUSED(brushManager);

    k->activeSelection = false;
    k->frame = currentFrame();

    // If Control key is pressed / allow multiple selection picking items one by one 
    if (input->keyModifiers() != Qt::ControlModifier) {
        foreach (NodeManager *nodeManager, k->nodeManagers) {
            if (!nodeManager->isPressed()) {
                nodeManager->parentItem()->setSelected(false);
                k->nodeManagers.removeAll(nodeManager);
                scene->drawCurrentPhotogram();
            } 
        }
        k->selectedObjects.clear();
    }

    if (k->frame->indexOf(scene->mouseGrabberItem()) != -1) {
        k->selectedObjects << scene->mouseGrabberItem();
    } else {
        if (scene->selectedItems().count() > 0)
            k->selectedObjects = scene->selectedItems();
    }

    foreach (QGraphicsItem *item, k->selectedObjects) {
        QDomDocument doc;
        doc.appendChild(TupSerializer::properties(item, doc));

        TupSvgItem *svg = qgraphicsitem_cast<TupSvgItem *>(item);
        int itemIndex = -1;
        TupLibraryObject::Type type = TupLibraryObject::Item;
        if (svg) {
            type = TupLibraryObject::Svg;
            itemIndex = k->frame->indexOf(svg);
        } else {
            itemIndex = k->frame->indexOf(item);
        }
        if (itemIndex >= 0)
            k->frame->checkTransformationStatus(type, itemIndex);

        if (item && (dynamic_cast<TupAbstractSerializable* > (item))) {
            if (item->group())
                item = qgraphicsitem_cast<QGraphicsItem *>(item->group());
            bool found = false;
            foreach (NodeManager *nodeManager, k->nodeManagers) {
                if (item == nodeManager->parentItem()) {
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                NodeManager *manager = new NodeManager(item, scene, k->nodeZValue);
                connect(manager, SIGNAL(rotationUpdated(int)), panel, SLOT(updateRotationAngle(int)));
                connect(manager, SIGNAL(scaleUpdated(double, double)), panel, SLOT(updateScaleFactor(double, double)));
                manager->show();
                manager->resizeNodes(k->realFactor);
                k->nodeManagers << manager;
            }
        } 
    }
}

void SelectionTool::move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
{
    Q_UNUSED(brushManager);

    if (input->buttons() == Qt::LeftButton && scene->selectedItems().count() > 0)
        QTimer::singleShot(0, this, SLOT(syncNodes()));
}

void SelectionTool::release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[SelectionTool::release()]";
        #else
            T_FUNCINFOX("tools");
        #endif
    #endif

    Q_UNUSED(input);
    Q_UNUSED(brushManager);

    k->selectedObjects = scene->selectedItems();
    if (k->selectedObjects.count() > 0) {
        panel->enableFormControls(true);
        k->activeSelection = true;
        foreach (NodeManager *manager, k->nodeManagers) {
            QGraphicsItem *item = manager->parentItem();
            int parentIndex = k->selectedObjects.indexOf(item); 
            if (parentIndex != -1) { // Object is IN the list
                k->selectedObjects.removeAt(parentIndex); // Remove node's item from selected objects list 
            } else { // Object is NOT IN the selected objects list
                int index = k->nodeManagers.indexOf(manager);
                disconnect(k->nodeManagers.at(index), SIGNAL(rotationUpdated(int)), panel, SLOT(updateRotationAngle(int)));
                disconnect(k->nodeManagers.at(index), SIGNAL(scaleUpdated(double, double)), panel, SLOT(updateScaleFactor(double, double)));
                delete k->nodeManagers.takeAt(index); // Removing node manager from nodes list
            }
        }

        foreach (QGraphicsItem *item, k->selectedObjects) {
            if (item && (dynamic_cast<TupAbstractSerializable* > (item))) {
                if (item->group())
                    item = qgraphicsitem_cast<QGraphicsItem *>(item->group());
                bool found = false;
                foreach (NodeManager *manager, k->nodeManagers) {
                    if (item == manager->parentItem()) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    NodeManager *manager = new NodeManager(item, scene, k->nodeZValue);
                    connect(manager, SIGNAL(rotationUpdated(int)), panel, SLOT(updateRotationAngle(int)));
                    connect(manager, SIGNAL(scaleUpdated(double, double)), panel, SLOT(updateScaleFactor(double, double)));
                    manager->show();
                    manager->resizeNodes(k->realFactor);
                    k->nodeManagers << manager;
                }
            }
        }

        foreach (NodeManager *manager, k->nodeManagers) {
            if (manager->isModified())
                requestTransformation(manager->parentItem(), k->frame);
        }
        updateItemPosition();
        updateItemRotation();
        updateItemScale();
    } else {
        panel->enableFormControls(false);
        if (k->targetIsIncluded)
            k->targetIsIncluded = false;

        foreach (NodeManager *manager, k->nodeManagers) {
            manager->parentItem()->setSelected(false);
            k->nodeManagers.removeAll(manager);
        }

        scene->drawCurrentPhotogram();
    }
}

TupFrame* SelectionTool::currentFrame()
{
    TupFrame *frame = 0;
    if (k->scene->getSpaceContext() == TupProject::FRAMES_EDITION) {
        frame = k->scene->currentFrame();
        k->currentLayer = k->scene->currentLayerIndex();
        k->currentFrame = k->scene->currentFrameIndex();
    } else {
        k->currentLayer = -1;
        k->currentFrame = -1;

        TupScene *tupScene = k->scene->currentScene();
        TupBackground *bg = tupScene->sceneBackground();
        if (k->scene->getSpaceContext() == TupProject::STATIC_BACKGROUND_EDITION) {
            frame = bg->staticFrame();
        } else if (k->scene->getSpaceContext() == TupProject::DYNAMIC_BACKGROUND_EDITION) {
            frame = bg->dynamicFrame();
        }
    }

    return frame;
}

TupFrame* SelectionTool::frameAt(int sceneIndex, int layerIndex, int frameIndex)
{
    TupFrame *frame = 0;
    TupProject *project = k->scene->currentScene()->project();
    TupScene *scene = project->sceneAt(sceneIndex);
    if (scene) {
        if (k->scene->getSpaceContext() == TupProject::FRAMES_EDITION) {
            TupLayer *layer = scene->layerAt(layerIndex);
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
            TupBackground *bg = scene->sceneBackground();
            if (k->scene->getSpaceContext() == TupProject::STATIC_BACKGROUND_EDITION) {
                frame = bg->staticFrame();
            } else if (k->scene->getSpaceContext() == TupProject::DYNAMIC_BACKGROUND_EDITION) {
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
    k->targetIsIncluded = false;
    k->activeSelection = false;
    k->nodeManagers.clear();
    k->realFactor = 1;

    TAction *select = new TAction(QPixmap(kAppProp->themeDir() + "icons/selection.png"), tr("Object Selection"), this);
    select->setShortcut(QKeySequence(tr("O")));
    select->setToolTip(tr("Object Selection") + " - " + "O");

    k->actions.insert(tr("Selection"), select);
}

QMap<QString, TAction *> SelectionTool::actions() const
{
    return k->actions;
}

int SelectionTool::toolType() const
{
    return TupToolInterface::Selection;
}

QWidget *SelectionTool::configurator() 
{
    if (!panel) {
        panel = new Settings;
        connect(panel, SIGNAL(callAlignAction(Settings::Align)), this, SLOT(applyAlignAction(Settings::Align)));
        connect(panel, SIGNAL(callFlip(Settings::Flip)), this, SLOT(applyFlip(Settings::Flip)));
        connect(panel, SIGNAL(callOrderAction(Settings::Order)), this, SLOT(applyOrderAction(Settings::Order)));
        connect(panel, SIGNAL(callGroupAction(Settings::Group)), this, SLOT(applyGroupAction(Settings::Group)));
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
        #ifdef Q_OS_WIN
            qDebug() << "[SelectionTool::aboutToChangeScene()]";
        #else
            T_FUNCINFOX("tools");
        #endif
    #endif

    Q_UNUSED(scene);

    clearSelection();
}

void SelectionTool::aboutToChangeTool()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[SelectionTool::aboutToChangeTool()]";
        #else
            T_FUNCINFOX("tools");
        #endif
    #endif

    init(k->scene);
}

void SelectionTool::itemResponse(const TupItemResponse *response)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[SelectionTool::itemResponse()]";
        #else
            T_FUNCINFOX("tools");
        #endif
    #endif

    if (response->action() == TupProjectRequest::Remove) {
        if (k->nodeManagers.count() == 1)
            panel->enableFormControls(false);
        return;
    }

    QGraphicsItem *item = 0;
    TupFrame *frame = frameAt(response->sceneIndex(), response->layerIndex(), response->frameIndex());
    if (frame) {
        if (response->itemType() == TupLibraryObject::Svg && frame->svgItemsCount() > 0) {
            item = frame->svgAt(response->itemIndex());
        } else if (frame->graphicsCount() > 0) {
            item = frame->item(response->itemIndex());
        }
    } else {
        #ifdef TUP_DEBUG
            QString msg = "SelectionTool::itemResponse - Fatal Error: frame is NULL! (index: " 
                          + QString::number(response->frameIndex()) + ")";
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
    QString msg = "SelectionTool::itemResponse() - response->action() -> " + QString::number(response->action());
    #ifdef Q_OS_WIN
        qDebug() << msg;
    #else
        tError() << msg;
    #endif
#endif

    switch (response->action()) {
        case TupProjectRequest::Transform:
        {
            if (item) {
                foreach (NodeManager* manager, k->nodeManagers) {
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
            k->nodeManagers.clear();
            k->selectedObjects.clear();

            k->selectedObjects << item;
            item->setSelected(true);
            NodeManager *manager = new NodeManager(item, k->scene, k->nodeZValue);
            connect(manager, SIGNAL(rotationUpdated(int)), panel, SLOT(updateRotationAngle(int)));
            connect(manager, SIGNAL(scaleUpdated(double, double)), panel, SLOT(updateScaleFactor(double, double)));
            manager->resizeNodes(k->realFactor);
            k->nodeManagers << manager;

            syncNodes();
        }
        break;
        case TupProjectRequest::Ungroup:
        {
            foreach (QGraphicsItem *graphic, k->scene->selectedItems())
                graphic->setSelected(false);

            k->nodeManagers.clear();
            k->selectedObjects.clear();

            QString list = response->arg().toString();
            QString::const_iterator itr = list.constBegin();
            QList<int> positions = TupSvg2Qt::parseIntList(++itr);
            qSort(positions.begin(), positions.end());
            int total = positions.size();
            for (int i=0; i<total; i++) {
                QGraphicsItem *graphic = frame->item(positions.at(i));     
                if (graphic) {
                    k->selectedObjects << graphic;
                    graphic->setSelected(true);
                    NodeManager *manager = new NodeManager(graphic, k->scene, k->nodeZValue);
                    connect(manager, SIGNAL(rotationUpdated(int)), panel, SLOT(updateRotationAngle(int)));
                    connect(manager, SIGNAL(scaleUpdated(double, double)), panel, SLOT(updateScaleFactor(double, double)));
                    manager->resizeNodes(k->realFactor);
                    k->nodeManagers << manager;
                }
            }

            syncNodes();
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
    foreach (NodeManager* node, k->nodeManagers) {
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
        #ifdef Q_OS_WIN
            qDebug() << "[SelectionTool::keyPressEvent()]";
        #else
            T_FUNCINFOX("tools");
        #endif
    #endif

    k->key = "NONE";

    if (event->key() == Qt::Key_F11 || event->key() == Qt::Key_Escape) {
        emit closeHugeCanvas();
    } else if (event->modifiers() == Qt::AltModifier) {
        if (event->key() == Qt::Key_R) {
            if (selectionIsActive()) {
                foreach (NodeManager *nodeManager, k->nodeManagers) {
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

            k->selectedObjects = k->scene->selectedItems();
            TupFrame *frame = currentFrame();

            foreach (QGraphicsItem *item, k->selectedObjects) {
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
        k->key = "CONTROL";
        if (selectionIsActive()) {
            foreach (NodeManager *nodeManager, k->nodeManagers)
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
    Q_UNUSED(event);

    if (k->key.compare("CONTROL") == 0) {
        panel->setProportionState(false);
        k->key = "NONE";
        if (selectionIsActive()) {
            foreach (NodeManager *nodeManager, k->nodeManagers)
                nodeManager->setProportion(false);
        }
    }
}

bool SelectionTool::selectionIsActive()
{
    return k->activeSelection;
}

void SelectionTool::applyAlignAction(Settings::Align align)
{
    QGraphicsView *view = k->scene->views().at(0);
    QRectF rect = view->sceneRect();
    QPointF center = rect.center();
    QPointF distance;
    foreach (NodeManager *manager, k->nodeManagers) {
        QGraphicsItem *item = manager->parentItem();
        QRectF rect = item->boundingRect();
        QPointF objectPos = rect.center();
        if (align == Settings::hAlign) {
            int y = center.y() - objectPos.y();
            item->setPos(item->pos().x(), y);
        } else if (align == Settings::vAlign) {
            int x = center.x() - objectPos.x();
            item->setPos(x, item->pos().y());
        } else if (align == Settings::totalAlign) {
            distance = center - objectPos;
            item->setPos(distance.x(), distance.y());
        }
        manager->syncNodesFromParent();
        requestTransformation(manager->parentItem(), k->frame);
    }
}

void SelectionTool::applyFlip(Settings::Flip flip)
{
    k->selectedObjects = k->scene->selectedItems();

    foreach (QGraphicsItem *item, k->selectedObjects) {
        foreach (NodeManager *manager, k->nodeManagers) {
            if (flip == Settings::Horizontal)
                manager->horizontalFlip();
            else if (flip == Settings::Vertical)
                manager->verticalFlip();
            else if (flip == Settings::Crossed)
                manager->crossedFlip();

            if (manager->isModified()) {
                QDomDocument doc;
                doc.appendChild(TupSerializer::properties(item, doc));

                TupSvgItem *svg = qgraphicsitem_cast<TupSvgItem *>(manager->parentItem());
                int position = -1;
                TupLibraryObject::Type type = TupLibraryObject::Item;
                TupFrame *frame = currentFrame();
                if (svg) {
                    type = TupLibraryObject::Svg;
                    position = frame->indexOf(svg);
                } else {
                    position = frame->indexOf(manager->parentItem());
                }

                TupProjectRequest event = TupRequestBuilder::createItemRequest(
                    k->scene->currentSceneIndex(),
                    k->currentLayer, k->currentFrame,
                    position, QPointF(), 
                    k->scene->getSpaceContext(), type,
                    TupProjectRequest::Transform, doc.toString());
                    emit requested(&event);
            }
        }
    }
}

void SelectionTool::applyOrderAction(Settings::Order action)
{
    k->selectedObjects = k->scene->selectedItems();

    foreach (QGraphicsItem *item, k->selectedObjects) {
        TupSvgItem *svg = qgraphicsitem_cast<TupSvgItem *>(item);
        int position = -1;
        TupLibraryObject::Type type = TupLibraryObject::Item;
        TupFrame *frame = currentFrame();
        if (svg) {
            type = TupLibraryObject::Svg;
            position = frame->indexOf(svg);
        } else {
            position = frame->indexOf(item);
        }

        TupProjectRequest event = TupRequestBuilder::createItemRequest(k->scene->currentSceneIndex(),
                                  k->currentLayer, k->currentFrame, position, QPointF(),
                                  k->scene->getSpaceContext(), type, TupProjectRequest::Move, action);
        emit requested(&event);
    }
}

void SelectionTool::applyGroupAction(Settings::Group action)
{
    foreach (QGraphicsItem *item, k->selectedObjects) {
        TupSvgItem *svg = qgraphicsitem_cast<TupSvgItem *>(item);
        if (svg) {
            TOsd::self()->display(tr("Error"), tr("SVG objects can't be grouped/ungrouped yet"), TOsd::Error);
            return;
        }
    }

    TupFrame *frame = currentFrame();

    if (action == Settings::GroupItems) {
        k->selectedObjects = k->scene->selectedItems();
        int total = k->selectedObjects.count(); 
        if (total > 1) {
            QString items = "(";
            int i = 1;
            int position = -1; 
            foreach (QGraphicsItem *item, k->selectedObjects) {
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

            foreach (QGraphicsItem *item, k->selectedObjects)
                item->setSelected(false);

            TupProjectRequest event = TupRequestBuilder::createItemRequest(k->scene->currentSceneIndex(),
                                      k->currentLayer, k->currentFrame,
                                      position, QPointF(), k->scene->getSpaceContext(),
                                      TupLibraryObject::Item, TupProjectRequest::Group, items);
            emit requested(&event);
        } else if (total == 1) {
            k->nodeManagers.clear();
            k->scene->drawCurrentPhotogram();
        }
    } else if (action == Settings::UngroupItems) {
        k->selectedObjects = k->scene->selectedItems();
        int total = k->selectedObjects.count();

        if (total > 0) {
            bool noAction = true;
            foreach (QGraphicsItem *item, k->selectedObjects) {
                item->setSelected(false);
                if (qgraphicsitem_cast<TupItemGroup *> (item)) {
                    noAction = false;
                    int itemIndex = currentFrame()->indexOf(item);

                    TupProjectRequest event = TupRequestBuilder::createItemRequest(
                                              k->scene->currentSceneIndex(),
                                              k->currentLayer, k->currentFrame,
                                              itemIndex, QPointF(),
                                              k->scene->getSpaceContext(), TupLibraryObject::Item,
                                              TupProjectRequest::Ungroup);
                    emit requested(&event);
                }
            }

            if (noAction) {
                k->nodeManagers.clear();
                k->scene->drawCurrentPhotogram();
            }
        }
    }
}

QCursor SelectionTool::cursor() const
{
    return QCursor(Qt::ArrowCursor);
}

void SelectionTool::resizeNodes(qreal scaleFactor)
{
    k->realFactor = scaleFactor;
    foreach (NodeManager *manager, k->nodeManagers)
        manager->resizeNodes(scaleFactor);
}

void SelectionTool::updateZoomFactor(qreal scaleFactor)
{
    k->realFactor = scaleFactor;
}

void SelectionTool::sceneResponse(const TupSceneResponse *event)
{
    if (event->action() == TupProjectRequest::Select)
        initItems(k->scene);
}

void SelectionTool::updateItemPosition() 
{
#ifdef TUP_DEBUG
    #ifdef Q_OS_WIN
        qDebug() << "[SelectionTool::updateItemPosition()]";
    #else
        T_FUNCINFOX("tools");
    #endif
#endif

    if (k->nodeManagers.count() == 1) {
        NodeManager *manager = k->nodeManagers.first();
        QGraphicsItem *item = manager->parentItem();
        QPoint point = item->mapToScene(item->boundingRect().center()).toPoint();
        panel->setPos(point.x(), point.y());
    } else { 
        if (k->nodeManagers.count() > 1) {
            NodeManager *manager = k->nodeManagers.first();
            QGraphicsItem *item = manager->parentItem();
            QPoint left = item->mapToScene(item->boundingRect().topLeft()).toPoint();  
            QPoint right = item->mapToScene(item->boundingRect().bottomRight()).toPoint();
            int minX = left.x();
            int maxX = right.x(); 
            int minY = left.y();
            int maxY = right.y();

            foreach (NodeManager *node, k->nodeManagers) {
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

            if (!k->targetIsIncluded) {
                k->center = new TupEllipseItem(QRectF(QPointF(x - 1, y - 1), QSize(2, 2)));
                k->target1 = new QGraphicsLineItem(x, y - 6, x, y + 6);
                k->target2 = new QGraphicsLineItem(x - 6, y, x + 6, y);
              
                QPen pen(QColor(255, 0, 0), 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                k->center->setPen(pen);
                k->center->setBrush(QColor(255, 0, 0));
                k->center->setZValue(k->nodeZValue);
                k->scene->includeObject(k->center);

                k->target1->setPen(pen); 
                k->target1->setZValue(k->nodeZValue);
                k->target2->setPen(pen);
                k->target2->setZValue(k->nodeZValue);

                k->scene->includeObject(k->target1);
                k->scene->includeObject(k->target2);

                k->targetIsIncluded = true;
            } else {
                QPoint current = k->center->mapToScene(k->center->boundingRect().topLeft()).toPoint(); 
                int deltaX = x - current.x() - 1;
                int deltaY = y - current.y() - 1;
                k->center->moveBy(deltaX, deltaY);
                k->target1->moveBy(deltaX, deltaY);
                k->target2->moveBy(deltaX, deltaY);
            }
        }
    }
}

void SelectionTool::updateItemRotation()
{
#ifdef TUP_DEBUG
    #ifdef Q_OS_WIN
        qDebug() << "[SelectionTool::updateItemRotation()]";
    #else
        T_FUNCINFOX("tools");
    #endif
#endif

    if (k->nodeManagers.count() > 0) {
        NodeManager *manager = k->nodeManagers.first();
        QGraphicsItem *item = manager->parentItem();
        qreal angle = item->data(TupGraphicObject::Rotate).toReal();
        panel->updateRotationAngle(static_cast<int>(angle));
    }
}

void SelectionTool::updateItemScale()
{
#ifdef TUP_DEBUG
    #ifdef Q_OS_WIN
        qDebug() << "[SelectionTool::updateItemRotation()]";
    #else
        T_FUNCINFOX("tools");
    #endif
#endif

    if (k->nodeManagers.count() > 0) {
        NodeManager *manager = k->nodeManagers.first();
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
    #ifdef Q_OS_WIN
        qDebug() << "[SelectionTool::updateItemPosition(int, int)]";
    #else
        T_FUNCINFOX("tools");
    #endif
#endif

    if (k->nodeManagers.count() == 1) {
        NodeManager *manager = k->nodeManagers.first();
        QGraphicsItem *item = manager->parentItem();
        item->moveBy(x, y);
        manager->syncNodesFromParent();
        requestTransformation(manager->parentItem(), k->frame);
    } else {
        if (k->nodeManagers.count() > 1) {
            foreach (NodeManager *manager, k->nodeManagers) {
                QGraphicsItem *item = manager->parentItem();
                item->moveBy(x, y);
                manager->syncNodesFromParent();
                requestTransformation(manager->parentItem(), k->frame);
            }
            k->center->moveBy(x, y);
            k->target1->moveBy(x, y);
            k->target2->moveBy(x, y);
        }
    }
}

void SelectionTool::updateItemRotation(int angle)
{
#ifdef TUP_DEBUG
    #ifdef Q_OS_WIN
        qDebug() << "[SelectionTool::updateItemRotation(int)]";
    #else
        T_FUNCINFOX("tools");
    #endif
#endif

    if (k->nodeManagers.count() == 1) {
        NodeManager *manager = k->nodeManagers.first();
        manager->rotate(angle);
        if (manager->isModified())
            requestTransformation(manager->parentItem(), k->frame);
    } else {
        if (k->nodeManagers.count() > 1) {
            foreach (NodeManager *manager, k->nodeManagers) {
                manager->rotate(angle);
                if (manager->isModified())
                    requestTransformation(manager->parentItem(), k->frame);
            }
        }
    }
}

void SelectionTool::updateItemScale(double xFactor, double yFactor)
{
#ifdef TUP_DEBUG
    #ifdef Q_OS_WIN
        qDebug() << "[SelectionTool::updateItemScale(float, float)]";
    #else
        T_FUNCINFOX("tools");
    #endif
#endif

    if (k->nodeManagers.count() == 1) {
        NodeManager *manager = k->nodeManagers.first();
        manager->scale(xFactor, yFactor);
        if (manager->isModified())
            requestTransformation(manager->parentItem(), k->frame);
    } else {
        if (k->nodeManagers.count() > 1) {
            foreach (NodeManager *manager, k->nodeManagers) {
                manager->scale(xFactor, yFactor);
                if (manager->isModified())
                    requestTransformation(manager->parentItem(), k->frame);
            }
        }
    }
}

void SelectionTool::requestTransformation(QGraphicsItem *item, TupFrame *frame)
{
#ifdef TUP_DEBUG
    #ifdef Q_OS_WIN
        qDebug() << "[SelectionTool::requestTransformation(QGraphicsItem *, TupFrame *)]";
    #else
        T_FUNCINFOX("tools");
    #endif
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
            if (libraryItem->itemType() == TupLibraryObject::Image)
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
                          k->scene->currentSceneIndex(), k->currentLayer, k->currentFrame,
                          position, QPointF(), k->scene->getSpaceContext(), type,
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
    if (k->activeSelection) {
        if (!k->nodeManagers.isEmpty()) {
            foreach (NodeManager *nodeManager, k->nodeManagers) {
                nodeManager->parentItem()->setSelected(false);
                k->nodeManagers.removeAll(nodeManager);
            }
            k->nodeManagers.clear();
        }
        k->selectedObjects.clear();
        k->activeSelection = false;
        k->scene->drawCurrentPhotogram();
    }
}

void SelectionTool::enableProportion(bool flag)
{
    k->key = "NONE";
    if (flag)
        k->key = "CONTROL";

    if (selectionIsActive()) {
        foreach (NodeManager *nodeManager, k->nodeManagers)
            nodeManager->setProportion(flag);
    }
}
