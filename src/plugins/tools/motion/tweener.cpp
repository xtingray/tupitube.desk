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

#include "tweener.h"
#include "tconfig.h"
#include "tosd.h"
#include "tupsvg2qt.h"
#include "tupinputdeviceinformation.h"
#include "tupbrushmanager.h"
#include "tupgraphicsscene.h"
#include "tupgraphicobject.h"
#include "tuppathitem.h"
#include "tuprectitem.h"
#include "tuplineitem.h"
#include "tupsvgitem.h"
#include "tupitemgroup.h"
#include "tupitemtweener.h"
#include "tuprequestbuilder.h"
#include "tupprojectrequest.h"
#include "tuplibraryobject.h"
#include "tupscene.h"
#include "tuplayer.h"
#include "talgorithm.h"

#include <QMessageBox>

Tweener::Tweener() : TupToolPlugin()
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::Tweener()]";
    #endif

    setupActions();

    isPathInScene = false;
    configPanel = nullptr;
    linePath = nullptr;
    nodesGroup = nullptr;
    currentTween = nullptr;

    initFrame = 0;
    initLayer = 0;
    initScene = 0;
}

Tweener::~Tweener()
{
}

// This method initialize the context of the plugin
void Tweener::init(TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::init()] - isPathInScene ->" << isPathInScene;
        qDebug() << "[Motion Tweener::init()] - Current scene index ->" << gScene->currentSceneIndex();
    #endif

    doList.clear();
    undoList.clear();
    dots.clear();

    if (isPathInScene) {
        if (linePath) {
            gScene->removeItem(linePath);
            delete linePath;
            linePath = nullptr;
        }

        isPathInScene = false;
    }

    scene = gScene;
    lineStraightMode = false;
    objects.clear();
    baseZValue = ZLAYER_BASE + (gScene->currentScene()->layersCount() * ZLAYER_LIMIT);

    pathOffset = QPointF(0, 0);
    firstNode = QPointF(0, 0);
    itemObjectReference = QPointF(0, 0);

    mode = TupToolPlugin::View;
    editMode = TupToolPlugin::None;

    configPanel->resetUI();

    QList<QString> tweenList = scene->currentScene()->getTweenNames(TupItemTweener::Motion);
    if (tweenList.size() > 0) {
        configPanel->loadTweenList(tweenList);
        setCurrentTween(tweenList.at(0));
    } else {
        configPanel->activeButtonsPanel(false);
    }

    int total = framesCount();
    configPanel->initStartCombo(total, scene->currentFrameIndex());
}

void Tweener::updateStartFrame(int index)
{
    #ifdef TUP_DEBUG
       qDebug() << "[Motion Tweener::updateStartFrame()]";
       qDebug() << "*** index ->" << index;
    #endif

    if (index == 1)
        index--;

    if (initFrame != index && index >= 0) {
        initFrame = index;
        #ifdef TUP_DEBUG
           qDebug() << "[Motion Tweener::updateStartFrame()] - initFrame ->" << initFrame;
        #endif
    }
}

// This method returns the plugin name
QList<TAction::ActionId> Tweener::keys() const
{
    return QList<TAction::ActionId>() << TAction::Motion;
}

/*
  This method makes an action when the mouse is pressed on the workspace 
  depending on the active mode: Selecting an object or Creating a path  
*/

void Tweener::press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::press()] -> Motion";
    #endif

    Q_UNUSED(brushManager)
    Q_UNUSED(gScene)

    if (editMode == TupToolPlugin::Properties && scene->currentFrameIndex() == initFrame) {
        if (linePath) {
            QPointF point = linePath->mapFromParent(input->pos());
            if (lineStraightMode) {
                QPainterPath pathVar = linePath->path();
                pathVar.cubicTo(lastPoint, lastPoint, lastPoint);
                linePath->setPath(pathVar);
                currentPoint = lastPoint;
            } else {
                QPainterPath pathVar = linePath->path();
                pathVar.cubicTo(point, point, point);
                linePath->setPath(pathVar);
                currentPoint = point;
            }
        }
    } 
}

// This method is executed while the mouse is pressed and on movement

void Tweener::move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    Q_UNUSED(input)
    Q_UNUSED(brushManager)
    Q_UNUSED(gScene)
}

/*
  This method finishes the action started on the press method depending
  on the active mode: Selecting an object or Creating a path
*/

void Tweener::release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::release()]";
    #endif

    Q_UNUSED(input)
    Q_UNUSED(brushManager)

    if (gScene->currentFrameIndex() == initFrame) {
        if (editMode == TupToolPlugin::Properties) {
            #ifdef TUP_DEBUG
                qDebug() << "[Motion Tweener::release()] - Tracing properties mode";
            #endif
            if (nodesGroup) {
                updateTweenPath();

                nodesGroup->createNodes(linePath);
                nodesGroup->show();
                nodesGroup->resizeNodes(realFactor);
                nodesGroup->expandAllNodes();

                configPanel->updateSteps(linePath);

                QPainterPath::Element e = linePath->path().elementAt(0);
                QPointF begin = QPointF(e.x, e.y);

                if (begin != firstNode) { // Object has been moved to a new position!
                    QPointF oldPos = firstNode;
                    QPointF newPos = begin;

                    int distanceX = static_cast<int> (newPos.x() - oldPos.x());
                    int distanceY = static_cast<int> (newPos.y() - oldPos.y());

                    if (objects.size() > 0) {
                        foreach (QGraphicsItem *item, objects)
                            item->moveBy(distanceX, distanceY);

                        QGraphicsItem *item = objects.at(0);
                        QRectF rect = item->sceneBoundingRect();
                        itemObjectReference = rect.center();
                    }

                    firstNode = newPos;
                    currentPoint = firstNode;
                }

                updateTweenPoints();
            }
        } else { // Selection mode
            #ifdef TUP_DEBUG
                qDebug() << "[Motion Tweener::release()] - Tracing selection mode";
            #endif
            if (gScene->selectedItems().size() > 0) {
                #ifdef TUP_DEBUG
                    qDebug() << "[Motion Tweener::release()] - selection size -> " << gScene->selectedItems().size();
                #endif

                objects = gScene->selectedItems();
                foreach (QGraphicsItem *item, objects) {
                    QString tip = item->toolTip();
                    if (tip.contains(tr("Motion"))) {
                        QPair<int, int> dimension = TAlgorithm::screenDimension();
                        int screenWidth = dimension.first;
                        int screenHeight = dimension.second;

                        QMessageBox msgBox;
                        msgBox.setWindowTitle(tr("Warning"));
                        msgBox.setIcon(QMessageBox::Warning);
                        msgBox.setText(tr("The selected items already have this kind of tween assigned."));
                        msgBox.setInformativeText(tr("Please, edit the previous tween of these objects."));
                        msgBox.addButton(QString(tr("Accept")), QMessageBox::AcceptRole);
                        msgBox.show();
                        msgBox.move(static_cast<int>((screenWidth - msgBox.width()) / 2),
                                    static_cast<int>((screenHeight - msgBox.height()) / 2));
                        msgBox.exec();

                        objects.clear();
                        gScene->clearSelection();

                        return;
                    }
                }

                #ifdef TUP_DEBUG
                    qDebug() << "[Motion Tweener::release()] - Notifying selection...";
                #endif
                configPanel->notifySelection(true);

                QGraphicsItem *item = objects.at(0);
                QRectF rect = item->sceneBoundingRect();
                QPointF newPos = rect.center();
                QPointF oldPos = itemObjectReference;
                itemObjectReference = newPos;

                if (!linePath) {
                    linePath = new QGraphicsPathItem;
                    linePath->setZValue(baseZValue);

                    QColor pathColor = configPanel->getPathColor();
                    pathColor.setAlpha(200);
                    QPen pen(QBrush(pathColor), configPanel->getPathThickness(), Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
                    linePath->setPen(pen);

                    QPainterPath pathVar;
                    pathVar.moveTo(newPos);
                    firstNode = newPos;
                    currentPoint = newPos;
                    linePath->setPath(pathVar);
                    gScene->addItem(linePath);
                    isPathInScene = true;

                    pathOffset = QPointF(0, 0);

                    // Include guide line
                    guideLine = new TupLineItem();
                    QPen guidePen;
                    guidePen.setWidth(configPanel->getPathThickness());
                    guidePen.setBrush(pathColor);
                    guideLine->setPen(guidePen);

                    guideLine->setLine(QLineF(firstNode, firstNode));
                } else {
                    int distanceX = static_cast<int> (newPos.x() - oldPos.x());
                    int distanceY = static_cast<int> (newPos.y() - oldPos.y());
                    linePath->moveBy(distanceX, distanceY);
                    pathOffset = QPointF(distanceX, distanceY);
                }
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[Motion Tweener::release()] - Selection mode: no items selected";
                #endif
            }
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[Motion Tweener::release()] - scene->currentFrameIndex() != initFrame";
            qDebug() << "scene->currentFrameIndex() -> " << gScene->currentFrameIndex();
            qDebug() << "initFrame -> " << initFrame;
        #endif
    }
}

void Tweener::updateTweenPath()
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::updateTweenPath()]";
    #endif

    QString route = pathToCoords();
    foreach (QGraphicsItem *item, objects) {
        TupLibraryObject::ObjectType type = TupLibraryObject::Item;
        int objectIndex = scene->currentFrame()->indexOf(item);
        TupProjectRequest request = TupRequestBuilder::createItemRequest(
                                    initScene, initLayer, initFrame, objectIndex,
                                    QPointF(), scene->getSpaceContext(), type,
                                    TupProjectRequest::UpdateTweenPath, route);
        emit requested(&request);
    }
    doList << linePath->path();
}

// This method returns the list of actions defined in this plugin

QMap<TAction::ActionId, TAction *> Tweener::actions() const
{
    return posActions;
}

TAction * Tweener::getAction(TAction::ActionId toolId)
{
    return posActions[toolId];
}

// This method returns the list of actions defined in this plugin

int Tweener::toolType() const
{
    return TupToolInterface::Tweener;
}

// This method returns the tool panel associated to this plugin

QWidget *Tweener::configurator()
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::configurator()]";
    #endif

    if (!configPanel) {
        mode = TupToolPlugin::View;

        configPanel = new Configurator;
        connect(configPanel, SIGNAL(startingFrameChanged(int)), this, SLOT(updateStartFrame(int)));
        connect(configPanel, SIGNAL(clickedCreatePath()), this, SLOT(setTweenPath()));
        connect(configPanel, SIGNAL(clickedSelect()), this, SLOT(setSelection()));
        connect(configPanel, SIGNAL(clickedRemoveTween(const QString &)), this, SLOT(removeTween(const QString &)));
        connect(configPanel, SIGNAL(clickedResetInterface()), this, SLOT(applyReset()));
        connect(configPanel, SIGNAL(setMode(TupToolPlugin::Mode)), this, SLOT(updateMode(TupToolPlugin::Mode)));
        connect(configPanel, SIGNAL(clickedApplyTween()), this, SLOT(applyTween()));
        connect(configPanel, SIGNAL(getTweenData(const QString &)), this, SLOT(setCurrentTween(const QString &)));
        connect(configPanel, SIGNAL(framesTotalChanged()), this, SLOT(updateTweenPoints()));
        connect(configPanel, SIGNAL(pathThicknessChanged(int)), this, SLOT(updatePathThickness(int)));
        connect(configPanel, SIGNAL(pathColorUpdated(const QColor &)), this, SLOT(updatePathColor(const QColor &)));
    } else {
        mode = configPanel->mode();
    }

    return configPanel;
}

// This method is called when there's a change on/of scene

void Tweener::aboutToChangeScene(TupGraphicsScene *scene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::aboutToChangeScene()] - Scene index ->" << scene->currentSceneIndex();
    #endif
}

// This method is called when this plugin is off

void Tweener::aboutToChangeTool()
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::aboutToChangeTool()]";
    #endif

    resetGUI();
}

void Tweener::resetGUI()
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::resetGUI()]";
    #endif

    configPanel->clearData();

    if (editMode == TupToolPlugin::Selection) {
        clearSelection();
        disableSelection();
    } else if (editMode == TupToolPlugin::Properties) {
        if (linePath) {
            scene->removeItem(linePath);
            removeTweenPoints();
            isPathInScene = false;
            if (nodesGroup) {
                nodesGroup->clear();
                nodesGroup = nullptr;
            }

            delete linePath;
            linePath = nullptr;
        }
    }
}

// This method defines the actions contained in this plugin

void Tweener::setupActions()
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::setupActions()]";
    #endif
    */

    realFactor = 1;
    QString name = tr("Motion Tween");
    QString shortcut = tr("Shift+W");

    TAction *action = new TAction(QPixmap(ICONS_DIR + "motion_tween.png"), name, this);
    action->setCursor(QCursor(kAppProp->themeDir() + "cursors/tweener.png", 0 ,0));
    action->setShortcut(QKeySequence(shortcut));
    action->setToolTip(name + " - " + shortcut);
    action->setActionId(TAction::Motion);

    posActions.insert(TAction::Motion, action);
}

// This method initializes the "Create path" mode

void Tweener::setTweenPath()
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::setTweenPath()]";
    #endif

    if (linePath) {
        pathOffset = QPointF(0, 0);
        if (!isPathInScene) {
            scene->addItem(linePath);
            isPathInScene = true;
        }

        if (nodesGroup) {
            disconnect(nodesGroup, SIGNAL(nodeReleased()), this, SLOT(updatePath()));
            nodesGroup = nullptr;
        }
        nodesGroup = new TNodeGroup(linePath, scene, TNodeGroup::MotionTween, baseZValue);
        connect(nodesGroup, SIGNAL(nodeReleased()), this, SLOT(updatePath()));

        nodesGroup->createNodes(linePath);
        nodesGroup->show();
        nodesGroup->resizeNodes(realFactor);
        nodesGroup->expandAllNodes();

        paintTweenPoints();
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[Motion Tweener::setTweenPath()] - Warning: linePath variable is NULL!";
        #endif
    }

    editMode = TupToolPlugin::Properties;
    disableSelection();
}

// This method initializes the "Select object" mode

void Tweener::setSelection()
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::setSelection()]";
    #endif

    if (mode == TupToolPlugin::Edit) {
        if (initFrame != scene->currentFrameIndex()) {
            QString selection = QString::number(initLayer) + "," + QString::number(initLayer) + ","
                                + QString::number(initFrame) + "," + QString::number(initFrame);

            TupProjectRequest request = TupRequestBuilder::createFrameRequest(currentTween->getInitScene(),
                                                                              currentTween->getInitLayer(),
                                                                              currentTween->getInitFrame(),
                                                                              TupProjectRequest::Select, selection);
            emit requested(&request);
        }
    }

    if (linePath) {
        removeTweenPoints();
        scene->removeItem(linePath);
        isPathInScene = false;
        if (nodesGroup) {
            nodesGroup->clear();
            nodesGroup = nullptr;
        }
    }

    editMode = TupToolPlugin::Selection;

    scene->enableItemsForSelection();
    foreach (QGraphicsView *view, scene->views())
        view->setDragMode(QGraphicsView::RubberBandDrag);
    // When Object selection is enabled, previous selection is set
    if (objects.size() > 0) {
        foreach (QGraphicsItem *item, objects) {
            item->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
            item->setSelected(true);
        }
    }
}

// This method transforms the path created into a QString representation

QString Tweener::pathToCoords()
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::pathToCoords()]";
    #endif

    QString strPath = "";
    QChar t;
    int offsetX = static_cast<int> (pathOffset.x());
    int offsetY = static_cast<int> (pathOffset.y());

    QPainterPath pathVar = linePath->path();
    int total = pathVar.elementCount();
    for (int i=0; i < total; i++) {
         QPainterPath::Element e = pathVar.elementAt(i);
         switch (e.type) {
             case QPainterPath::MoveToElement:
             {
                 if (t != 'M') {
                     t = 'M';
                     strPath += "M " + QString::number(e.x + offsetX) + " " + QString::number(e.y + offsetY) + " ";
                 } else {
                     strPath += QString::number(e.x + offsetX) + " " + QString::number(e.y + offsetY) + " ";
                 }
             }
             break;
             case QPainterPath::LineToElement:
             {
                  if (t != 'L') {
                      t = 'L';
                      strPath += " L " + QString::number(e.x + offsetX) + " " + QString::number(e.y + offsetY) + " ";
                  } else {
                      strPath += QString::number(e.x + offsetX) + " " + QString::number(e.y + offsetY) + " ";
                  }
             }
             break;
             case QPainterPath::CurveToElement:
             {
                  if (t != 'C') {
                      t = 'C';
                      strPath += " C " + QString::number(e.x + offsetX) + " " + QString::number(e.y + offsetY) + " ";
                  } else {
                      strPath += "  " + QString::number(e.x + offsetX) + " " + QString::number(e.y + offsetY) + " ";
                  }
             }
             break;
             case QPainterPath::CurveToDataElement:
             {
                  if (t == 'C')
                      strPath +=  " " + QString::number(e.x + offsetX) + "  " + QString::number(e.y + offsetY) + " ";
             }
             break;
         }
    }

    return strPath.trimmed();
}

// This method resets this plugin

void Tweener::applyReset()
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::applyReset()]";
    #endif

    if (mode == TupToolPlugin::Edit && editMode == TupToolPlugin::Properties) {
        if (nodesGroup) {
            nodesGroup->clear();
            nodesGroup = nullptr;
        }
    }

    mode = TupToolPlugin::View;
    editMode = TupToolPlugin::None;

    clearSelection();
    disableSelection();

    if (linePath) {
        removeTweenPoints();
        scene->removeItem(linePath);
        isPathInScene = false;
        linePath = nullptr;
    }

    initFrame = scene->currentFrameIndex();
    initLayer = scene->currentLayerIndex();
    initScene = scene->currentSceneIndex();

    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::applyReset()] - initFrame -> " << initFrame;
    #endif

    configPanel->clearData();
}

// This method applies to the project, the Tween created from this plugin

void Tweener::applyTween()
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::applyTween()]";
    #endif

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QString name = configPanel->currentTweenName();
    if (name.length() == 0) {
        TOsd::self()->display(TOsd::Error, tr("Tween name is missing!"));
        return;
    }

    // Tween is new
    if (!scene->tweenExists(name, TupItemTweener::Motion)) {
        initFrame = scene->currentFrameIndex();
        initLayer = scene->currentLayerIndex();
        initScene = scene->currentSceneIndex();

        #ifdef TUP_DEBUG
            qDebug() << "[Motion Tweener::applyTween()] - initFrame -> " << initFrame;
        #endif

        foreach (QGraphicsItem *item, objects) {
            TupLibraryObject::ObjectType type = TupLibraryObject::Item;
            int objectIndex = scene->currentFrame()->indexOf(item);
            QPointF point = item->pos();

            if (TupSvgItem *svg = qgraphicsitem_cast<TupSvgItem *>(item)) {
                type = TupLibraryObject::Svg;
                objectIndex = scene->currentFrame()->indexOf(svg);
            }

            QString route = pathToCoords();
            TupProjectRequest request = TupRequestBuilder::createItemRequest(
                                        initScene, initLayer, initFrame,
                                        objectIndex,
                                        QPointF(), scene->getSpaceContext(), type,
                                        TupProjectRequest::SetTween,
                                        configPanel->tweenToXml(initScene, initLayer, initFrame, point, route));
            emit requested(&request);
        }
    } else { // Tween already exists
        removeTweenFromProject(name);
        QList<QGraphicsItem *> newList;

        initFrame = configPanel->startFrame();
        initLayer = currentTween->getInitLayer();
        initScene = currentTween->getInitScene();

        #ifdef TUP_DEBUG
            qDebug() << "[Motion Tweener::applyTween()] - initFrame -> " << initFrame;
        #endif

        foreach (QGraphicsItem *item, objects) {
            TupLibraryObject::ObjectType type = TupLibraryObject::Item;
            TupScene *sceneData = scene->currentScene();
            TupLayer *layer = sceneData->layerAt(initLayer);
            TupFrame *frame = layer->frameAt(currentTween->getInitFrame());
            int objectIndex = frame->indexOf(item);

            /*
            QPainterPath path = path->path();
            QPolygonF points = path.toFillPolygon();
            QPointF point = points.at(0) - QPointF(item->boundingRect().width(), item->boundingRect().height());
            */

            QPointF point = item->pos();
            TupSvgItem *svg = qgraphicsitem_cast<TupSvgItem *>(item);

            if (svg) {
                type = TupLibraryObject::Svg;
                objectIndex = frame->indexOf(svg);
            }

            if (initFrame != currentTween->getInitFrame()) {
                QDomDocument dom;
                if (type == TupLibraryObject::Svg)
                    dom.appendChild(svg->toXml(dom));
                else
                    dom.appendChild(dynamic_cast<TupAbstractSerializable *>(item)->toXml(dom));

                TupProjectRequest request = TupRequestBuilder::createItemRequest(initScene, initLayer,
                                            initFrame, 0, item->pos(),
                                            scene->getSpaceContext(), type,
                                            TupProjectRequest::Add, dom.toString());
                emit requested(&request);

                request = TupRequestBuilder::createItemRequest(currentTween->getInitScene(),
                          currentTween->getInitLayer(),
                          currentTween->getInitFrame(),
                          objectIndex, QPointF(),
                          scene->getSpaceContext(), type,
                          TupProjectRequest::Remove);
                emit requested(&request);

                frame = layer->frameAt(initFrame);
                if (type == TupLibraryObject::Item) {
                    objectIndex = frame->graphicsCount() - 1;
                    newList.append(frame->graphicAt(objectIndex)->item());
                } else {
                    objectIndex = frame->svgItemsCount() - 1;
                    newList.append(frame->svgAt(objectIndex));
                }
            }

            QString route = pathToCoords();
            TupProjectRequest request = TupRequestBuilder::createItemRequest(
                                        initScene, initLayer, initFrame,
                                        objectIndex,
                                        QPointF(), scene->getSpaceContext(), type,
                                        TupProjectRequest::SetTween,
                                        configPanel->tweenToXml(initScene, initLayer, initFrame, point, route));
            emit requested(&request);
       }

       if (newList.size() > 0)
           objects = newList;
    }

    int framesNumber = framesCount();
    int total = initFrame + configPanel->totalSteps();
    TupProjectRequest request;

    if (total > framesNumber) {
        int layersCount = scene->currentScene()->layersCount();
        for (int i = framesNumber; i < total; i++) {
             for (int j = 0; j < layersCount; j++) {
                  request = TupRequestBuilder::createFrameRequest(initScene, j, i, TupProjectRequest::Add, tr("Frame"));
                  emit requested(&request);
             }
        }
    }

    QString selection = QString::number(initLayer) + "," + QString::number(initLayer) + ","
                        + QString::number(initFrame) + "," + QString::number(initFrame);

    request = TupRequestBuilder::createFrameRequest(initScene, initLayer, initFrame,
                                                    TupProjectRequest::Select, selection);
    emit requested(&request);

    setCurrentTween(name);
    TOsd::self()->display(TOsd::Info, tr("Tween %1 applied!").arg(name));

    QApplication::restoreOverrideCursor();

    /*
    // SQA: Debugging code
    // QColor color = TAlgorithm::randomColor(false);
    QColor color = Qt::red;
    QPolygonF points = path->path().toFillPolygon();
    points.removeLast();
    for (int i=0; i<points.size(); i++) {
        QPen inkPen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        TupEllipseItem *ellipse = new TupEllipseItem(QRectF(points.at(i), QSize(3, 3)));
        ellipse->setPen(inkPen);
        ellipse->setBrush(inkPen.brush());
        scene->addItem(ellipse);
    }
    */
}

/*
  This method updates the data of the path into the tool panel 
  and disables edition mode for nodes
*/

void Tweener::updatePath()
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::updatePath()]";
    #endif

    if (linePath) {
        QPainterPath::Element e = linePath->path().elementAt(0);
        QPointF point = QPointF(e.x, e.y);
        if (point != firstNode) {
            int distanceX = static_cast<int> (point.x() - firstNode.x());
            int distanceY = static_cast<int> (point.y() - firstNode.y());
            firstNode = point;
            pathOffset = QPointF(distanceX, distanceY);

            if (objects.size() > 0) {
                int i = 0;
                foreach (QGraphicsItem *item, objects) {
                    item->moveBy(distanceX, distanceY);
                    if (i == 0) {
                        QRectF rect = item->sceneBoundingRect();
                        itemObjectReference = rect.center();
                    }
                    i++;
                }
            }
        }

        doList << linePath->path();

        configPanel->updateSteps(linePath);
        updateTweenPoints();
    }
}

// This method saves the settings of this plugin

void Tweener::saveConfig()
{
}

/* This method updates the workspace when the plugin changes the scene */

void Tweener::updateScene(TupGraphicsScene *scene)
{
    mode = configPanel->mode();

    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::updateScene()] - Scene index ->" << scene->currentSceneIndex();
        qDebug() << "[Motion Tweener::updateScene()] - Current mode ->" << mode;
    #endif

    if (mode == TupToolPlugin::Edit) {
        int total = initFrame + configPanel->totalSteps();

        if (editMode == TupToolPlugin::Properties) {
            if (scene->currentFrameIndex() >= initFrame && scene->currentFrameIndex() < total) {
                if (linePath && nodesGroup) {
                    scene->addItem(linePath);
                    nodesGroup->createNodes(linePath);
                    nodesGroup->show();
                    nodesGroup->resizeNodes(realFactor);
                    nodesGroup->expandAllNodes();

                    dots.clear();
                    paintTweenPoints();
                }
            }
        } 

        int framesNumber = framesCount();
        if (configPanel->startComboSize() < framesNumber)
            configPanel->initStartCombo(framesNumber, initFrame);

    } else if (mode == TupToolPlugin::Add) {
        int total = framesCount();
        if (configPanel->startComboSize() < total) {
            configPanel->initStartCombo(total, initFrame);
        } else {
            if (scene->currentFrameIndex() != initFrame)
                configPanel->setStartFrame(scene->currentFrameIndex());
        }

        if (editMode == TupToolPlugin::Properties) {
            linePath = nullptr;
            configPanel->clearData();
            configPanel->activateMode(TupToolPlugin::Selection);
            clearSelection();
            setSelection();
        } else if (editMode == TupToolPlugin::Selection) {
            linePath = nullptr;
            if (scene->currentFrameIndex() != initFrame)
                clearSelection();
            initFrame = scene->currentFrameIndex();            
            #ifdef TUP_DEBUG
                qDebug() << "[Motion Tweener::updateScene()] - initFrame -> " << initFrame;
            #endif
            setSelection();
        }
    } else {
        if (scene->currentFrameIndex() != initFrame)
            configPanel->setStartFrame(scene->currentFrameIndex());
    }
}

void Tweener::updateMode(TupToolPlugin::Mode currentMode)
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::updateMode()] - currentMode ->" << currentMode;
    #endif

    mode = currentMode;

    if (mode == TupToolPlugin::Edit)
        setEditEnv();
}

void Tweener::removeTweenFromProject(const QString &name)
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::removeTweenFromProject()] - name ->" << name;
    #endif

    TupScene *sceneData = scene->currentScene();
    bool removed = sceneData->removeTween(name, TupItemTweener::Motion);

    if (removed) {
        foreach (QGraphicsView * view, scene->views()) {
            foreach (QGraphicsItem *item, view->scene()->items()) {
                QString tip = item->toolTip();
                if (tip.compare("Tweens: " + tr("Motion")) == 0) {
                    item->setToolTip("");
                } else {
                    if (tip.contains(tr("Motion"))) {
                        tip = tip.replace(tr("Motion") + ",", "");
                        tip = tip.replace(tr("Motion"), "");
                        if (tip.endsWith(","))
                            tip.chop(1);
                        item->setToolTip(tip);
                    }
                }
            }
        }
        emit tweenRemoved();
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[Motion Tweener::removeTweenFromProject()] - Motion tween couldn't be removed ->" << name;
        #endif
    }
}

void Tweener::removeTween(const QString &name)
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::removeTween()] - tween name ->" << name;
    #endif

    removeTweenFromProject(name);
    applyReset();

    QString tweenName = configPanel->getTweenNameFromList();
    if (!tweenName.isEmpty())
        setCurrentTween(tweenName);
}

void Tweener::setCurrentTween(const QString &name)
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::setCurrentTween()] - tween name ->" << name;
    #endif

    TupScene *sceneData = scene->currentScene();
    currentTween = sceneData->tween(name, TupItemTweener::Motion);
    if (currentTween)
        configPanel->setCurrentTween(currentTween);
}

void Tweener::setEditEnv()
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::setEditEnv()] - Loading tween ->" << currentTween->getTweenName();
    #endif

    initFrame = currentTween->getInitFrame();
    initLayer = currentTween->getInitLayer();
    initScene = currentTween->getInitScene();

    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::setEditEnv()] - initFrame -> " << initFrame;
    #endif

    if (initFrame != scene->currentFrameIndex() || initLayer != scene->currentLayerIndex() || initScene != scene->currentSceneIndex()) {
        QString selection = QString::number(initLayer) + "," + QString::number(initLayer) + ","
                            + QString::number(initFrame) + "," + QString::number(initFrame);

        TupProjectRequest request = TupRequestBuilder::createFrameRequest(initScene, initLayer, initFrame, TupProjectRequest::Select, selection);
        emit requested(&request);
    }

    mode = TupToolPlugin::Edit;

    TupScene *sceneData = scene->currentScene();
    objects = sceneData->getItemsFromTween(currentTween->getTweenName(), TupItemTweener::Motion);

    if (!objects.isEmpty()) {
        QGraphicsItem *item = objects.at(0);
        QRectF rect = item->sceneBoundingRect();
        itemObjectReference = rect.center();

        linePath = currentTween->graphicsPath();
        linePath->setZValue(baseZValue);

        QPainterPath::Element e = linePath->path().elementAt(0);
        firstNode = QPointF(e.x, e.y);

        QPointF oldPos = QPointF(e.x, e.y);
        QPointF newPos = rect.center();

        int distanceX = static_cast<int> (newPos.x() - oldPos.x());
        int distanceY = static_cast<int> (newPos.y() - oldPos.y());
        linePath->moveBy(distanceX, distanceY);
        pathOffset = QPointF(distanceX, distanceY);

        QColor pathColor = configPanel->getPathColor();
        pathColor.setAlpha(200);
        QPen pen(QBrush(pathColor), configPanel->getPathThickness(), Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
        linePath->setPen(pen);

        setTweenPath();
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[Motion Tweener::setEditEnv()] - Fatal Error: Motion tween wasn't found -> "
                     << currentTween->getTweenName();
        #endif
    }
}

int Tweener::framesCount()
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::framesCount()]";
    #endif

    int total = 1;
    TupLayer *layer = scene->currentScene()->layerAt(scene->currentLayerIndex());
    if (layer)
        total = layer->framesCount();

    return total;
}

/* This method clear selection */

void Tweener::clearSelection()
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::clearSelection()]";
    #endif

    if (objects.size() > 0) {
        foreach (QGraphicsItem *item, objects) {
            if (item->isSelected())
                item->setSelected(false);
        }
        objects.clear();
        configPanel->notifySelection(false);
    }
}

// This method disables object selection

void Tweener::disableSelection()
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::disableSelection()]";
    #endif

    foreach (QGraphicsView *view, scene->views()) {
        view->setDragMode (QGraphicsView::NoDrag);
        foreach (QGraphicsItem *item, view->scene()->items()) {
            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
            item->setFlag(QGraphicsItem::ItemIsMovable, false);
        }
    }
}

void Tweener::sceneResponse(const TupSceneResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::sceneResponse()]";
    #endif

    if ((response->getAction() == TupProjectRequest::Remove || response->getAction() == TupProjectRequest::Reset)
        && (scene->currentSceneIndex() == response->getSceneIndex())) {
        init(scene);
    }

    if (response->getAction() == TupProjectRequest::Select)
        init(scene);
}

void Tweener::layerResponse(const TupLayerResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::layerResponse()]";
    #endif

    if (response->getAction() == TupProjectRequest::Remove)
        init(scene);
}

void Tweener::frameResponse(const TupFrameResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::frameResponse()] - index ->" << response->getFrameIndex();
    #endif

    if (response->getAction() == TupProjectRequest::Remove && scene->currentLayerIndex() == response->getLayerIndex()) {
        isPathInScene = false;
        init(scene);

        if (initFrame != scene->currentFrameIndex()) {
            QString selection = QString::number(initLayer) + "," + QString::number(initLayer) + ","
                                + QString::number(initFrame) + "," + QString::number(initFrame);

            TupProjectRequest request = TupRequestBuilder::createFrameRequest(currentTween->getInitScene(),
                                                                              currentTween->getInitLayer(),
                                                                              currentTween->getInitFrame(),
                                                                              TupProjectRequest::Select, selection);
            emit requested(&request);
        }
        return;
    }

    if (response->getAction() == TupProjectRequest::Select) {
        if (mode == TupToolPlugin::Edit) {
            if (editMode == TupToolPlugin::Properties)
                paintTweenPoints();
        }

        if (initLayer != response->getLayerIndex() || initScene != response->getSceneIndex()) {
            resetGUI();
            init(scene);
        }
    }
}

void Tweener::itemResponse(const TupItemResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::itemResponse()] - index ->" << response->getItemIndex();
    #endif

    if (editMode == TupToolPlugin::Properties) {
        if (response->getAction() == TupProjectRequest::UpdateTweenPath) {
            if (response->getMode() == TupProjectResponse::Undo) {
                if (!doList.isEmpty()) {
                    undoList << doList.last();
                    doList.removeLast();
                    scene->removeItem(linePath);

                    if (nodesGroup) {
                        nodesGroup->clear();
                        nodesGroup = nullptr;
                    }
                    removeTweenPoints();

                    QPainterPath painterPath;
                    if (doList.isEmpty()) {
                        linePath = new QGraphicsPathItem;
                        linePath->setZValue(baseZValue);

                        QColor pathColor = configPanel->getPathColor();
                        pathColor.setAlpha(200);
                        QPen pen(QBrush(pathColor), configPanel->getPathThickness(), Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
                        linePath->setPen(pen);

                        painterPath.moveTo(firstNode);
                        linePath->setPath(painterPath);
                        scene->addItem(linePath);
                        currentPoint = firstNode;

                        configPanel->enableSaveOption(false);
                    } else {
                        painterPath = doList.last();

                        QPainterPath::Element e = painterPath.elementAt(painterPath.elementCount() - 1);
                        currentPoint = QPointF(e.x, e.y);
                        linePath->setPath(painterPath);
                        scene->addItem(linePath);
                    }

                    nodesGroup = new TNodeGroup(linePath, scene, TNodeGroup::MotionTween, baseZValue);
                    connect(nodesGroup, SIGNAL(nodeReleased()), this, SLOT(updatePath()));
                    nodesGroup->createNodes(linePath);

                    nodesGroup->show();
                    nodesGroup->resizeNodes(realFactor);
                    nodesGroup->expandAllNodes();

                    if (configPanel->stepsTotal() == nodesGroup->mainNodesCount())
                        configPanel->undoSegment(painterPath);
                    else
                        configPanel->updateSegments(painterPath);

                    /*
                    qDebug() << "Main Nodes size: " << nodesGroup->mainNodesCount();
                    qDebug() << "Nodes Total: " << nodesGroup->nodesTotalCount();
                    qDebug() << "Config panel: " << configPanel->stepsTotal();
                    */

                    paintTweenPoints();
                }
            }

            if (response->getMode() == TupProjectResponse::Redo) {
                if (!undoList.isEmpty()) {
                    doList << undoList.last();
                    undoList.removeLast();
                    scene->removeItem(linePath);

                    if (nodesGroup) {
                        nodesGroup->clear();
                        nodesGroup = nullptr;
                        // disconnect(nodesGroup, SIGNAL(nodeReleased()), this, SLOT(updatePath()));
                    }
                    removeTweenPoints();

                    QPainterPath painterPath = doList.last();
                    QPainterPath::Element e = painterPath.elementAt(painterPath.elementCount() - 1);
                    currentPoint = QPointF(e.x, e.y);

                    linePath->setPath(painterPath);
                    scene->addItem(linePath);

                    nodesGroup = new TNodeGroup(linePath, scene, TNodeGroup::MotionTween, baseZValue);
                    connect(nodesGroup, SIGNAL(nodeReleased()), this, SLOT(updatePath()));
                    nodesGroup->createNodes(linePath);

                    nodesGroup->show();
                    nodesGroup->resizeNodes(realFactor);
                    nodesGroup->expandAllNodes();

                    if (configPanel->stepsTotal() < (nodesGroup->mainNodesCount()-1))
                        configPanel->redoSegment(painterPath);
                    else
                        configPanel->updateSegments(painterPath);

                    /*
                    qDebug() << "Main Nodes size: " << nodesGroup->mainNodesCount();
                    qDebug() << "Nodes Total: " << nodesGroup->nodesTotalCount();
                    qDebug() << "Config panel: " << configPanel->stepsTotal();
                    */

                    paintTweenPoints();
                }
            }
        }
    }
}

TupToolPlugin::Mode Tweener::currentMode()
{
    return mode;
}

TupToolPlugin::EditMode Tweener::currentEditMode()
{
    return editMode;
}

void Tweener::removeTweenPoints()
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::removeTweenPoints()]";
    #endif

    int total = dots.size();
    for (int i=0; i<total; i++)
         scene->removeItem(dots.at(i));
    dots.clear();
}

void Tweener::paintTweenPoints()
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::paintTweenPoints()]";
    #endif

    if (dots.isEmpty()) {
        QColor color = Qt::black;
        QList<QPointF> points = configPanel->tweenPoints();
        int total = points.size();
        for (int i=0; i < total; i++) {
            QPen inkPen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            TupEllipseItem *ellipse = new TupEllipseItem(QRectF(points.at(i) - QPointF(2,2), QSize(4, 4)));
            ellipse->setPen(inkPen);
            ellipse->setBrush(inkPen.brush());
            scene->addItem(ellipse);
            dots << ellipse;
        }
    } 

    if (nodesGroup) {
        if (nodesGroup->nodesTotalCount() == 4)
            configPanel->enableSaveOption(true);
    } else {
        if (mode == TupToolPlugin::Add)
            configPanel->enableSaveOption(false);
    }
}

void Tweener::updateTweenPoints()
{
    #ifdef TUP_DEBUG
        qDebug() << "[Motion Tweener::updateTweenPoints()]";
    #endif

    removeTweenPoints();
    paintTweenPoints();
}

void Tweener::resizeNode(qreal scaleFactor)
{
    realFactor = scaleFactor;
    if (nodesGroup)
        nodesGroup->resizeNodes(scaleFactor);
}

void Tweener::updateZoomFactor(qreal scaleFactor)
{
    realFactor = scaleFactor;
}

void Tweener::updatePathThickness(int thickness)
{
    if (linePath) {
        QPen pen = linePath->pen();
        pen.setWidth(thickness);
        linePath->setPen(pen);
    }
}

void Tweener::updatePathColor(const QColor &color)
{
    if (linePath) {
        QPen pen = linePath->pen();
        pen.setColor(color);
        linePath->setPen(pen);
    }
}

void Tweener::keyPressEvent(QKeyEvent *event)
{
    if (editMode == TupToolPlugin::Properties) {
        if (event->key() == Qt::Key_Control) {
            lineStraightMode = true;
            if (guideLine)
                scene->addItem(guideLine);
        }
    }
}

void Tweener::keyReleaseEvent(QKeyEvent *event)
{
    if (editMode == TupToolPlugin::Properties) {
        if (event->key() == Qt::Key_Control) {
            lineStraightMode = false;
            if (guideLine)
                scene->removeItem(guideLine);
        }
    }
}

void Tweener::updatePos(QPointF pos)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[Tweener::updatePos()] - pos -> " << pos;
    #endif
    */

    if (editMode == TupToolPlugin::Selection)
        return;

    if (linePath) {
        QLineF lineVar;
        if (lineStraightMode) {
            qreal dx = pos.x() - currentPoint.x();
            qreal dy = pos.y() - currentPoint.y();
            qreal m = fabs(dx/dy);

            if (m > 1) {
                lineVar = QLineF(currentPoint.x(), currentPoint.y(), pos.x(), currentPoint.y());
                lastPoint = QPointF(pos.x(), currentPoint.y());
            } else {
                lineVar = QLineF(currentPoint.x(), currentPoint.y(), currentPoint.x(), pos.y());
                lastPoint = QPointF(currentPoint.x(), pos.y());
            }

            if (guideLine)
                guideLine->setLine(lineVar);
        }
    }
}
