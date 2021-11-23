/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *                                                                         *
 *   2019:                                                                 *
 *    Alejandro Carrasco                                                   *
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

#include "taction.h"
#include "tosd.h"
#include "tupinputdeviceinformation.h"
#include "tupbrushmanager.h"
#include "tupgraphicsscene.h"
#include "tupgraphicobject.h"
#include "tuppathitem.h"
#include "tupellipseitem.h"
#include "tuplineitem.h"
#include "tuprectitem.h"
#include "tuppixmapitem.h"
#include "tupsvgitem.h"
#include "tupitemtweener.h"
#include "tuprequestbuilder.h"
#include "tupprojectrequest.h"
#include "tuplibraryobject.h"
#include "tupscene.h"
#include "tuplayer.h"
#include "tupprojectresponse.h"
#include "tupgraphiclibraryitem.h"
#include "tuptextitem.h"

#include <QMessageBox>
#include <QScreen>
#include <QPointF>
#include <QKeySequence>
#include <QGraphicsView>
#include <QDomDocument>
#include <QDir>

Tweener::Tweener() : TupToolPlugin()
{
    setupActions();
    configPanel = nullptr;
}

Tweener::~Tweener()
{
    delete configPanel;
    delete scene;
    delete currentTween;
}

/* This method initializes the plugin */

void Tweener::init(TupGraphicsScene *gScene)
{
    scene = gScene;
    objects.clear();

    mode = TupToolPlugin::View;
    editMode = TupToolPlugin::None;
    initFrame = scene->currentFrameIndex();
    initLayer = scene->currentLayerIndex();
    initScene = scene->currentSceneIndex();

    configPanel->resetUI();

    QList<QString> tweenList = scene->currentScene()->getTweenNames(TupItemTweener::Coloring);
    if (tweenList.size() > 0) {
        configPanel->loadTweenList(tweenList);
        setCurrentTween(tweenList.at(0));
    } else {
        configPanel->activeButtonsPanel(false);
    }

    int total = framesCount();
    configPanel->initStartCombo(total, initFrame);
}

void Tweener::updateStartPoint(int index)
{
     if (initFrame != index && index >= 0)
         initFrame = index;
}

/* This method returns the plugin name */

QList<TAction::ActionId> Tweener::keys() const
{
    return QList<TAction::ActionId>() << TAction::Color;
}

/* This method makes an action when the mouse is pressed on the workspace 
 * depending on the active mode: Selecting an object or Creating a path  
*/

void Tweener::press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[Tweener::press()]";
    #endif

    Q_UNUSED(input)
    Q_UNUSED(brushManager)
    Q_UNUSED(gScene)
}

/* This method is executed while the mouse is pressed and on movement */

void Tweener::move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    Q_UNUSED(input)
    Q_UNUSED(brushManager)
    Q_UNUSED(gScene)
}

/* This method finishes the action started on the press method depending
 * on the active mode: Selecting an object or Creating a path
*/

void Tweener::release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[Tweener::release()]";
    #endif

    Q_UNUSED(input)
    Q_UNUSED(brushManager)

    if (gScene->currentFrameIndex() == initFrame) {
        if (editMode == TupToolPlugin::Selection) {
            #ifdef TUP_DEBUG
                qDebug() << "[Color Tweener::release()] - Tracing selection mode";
            #endif
            if (gScene->selectedItems().size() > 0) {
                #ifdef TUP_DEBUG
                    qDebug() << "[Color Tweener::release()] - selection size -> " << gScene->selectedItems().size();
                #endif
                objects = gScene->selectedItems();
                foreach (QGraphicsItem *item, objects) {
                    QString tip = item->toolTip();
                    if (tip.contains(tr("Coloring"))) {
                        QScreen *screen = QGuiApplication::screens().at(0);
                        QMessageBox msgBox;
                        msgBox.setWindowTitle(tr("Warning"));
                        msgBox.setIcon(QMessageBox::Warning);
                        msgBox.setText(tr("The selected items already have this kind of tween assigned."));
                        msgBox.setInformativeText(tr("Please, edit the previous tween of these objects."));
                        msgBox.addButton(QString(tr("Accept")), QMessageBox::AcceptRole);
                        msgBox.show();
                        msgBox.move(static_cast<int>((screen->geometry().width() - msgBox.width())/2),
                                    static_cast<int>((screen->geometry().height() - msgBox.height())/2));
                        msgBox.exec();

                        objects.clear();
                        gScene->clearSelection();
                        return;
                    }
                }

                foreach (QGraphicsItem *item, objects) {
                    if (TupGraphicLibraryItem *libraryItem = qgraphicsitem_cast<TupGraphicLibraryItem *>(item)) {
                        if (libraryItem->type() == TupLibraryObject::Image) {
                            clearSelection();
                            TOsd::self()->display(TOsd::Warning, tr("Coloring Tween can't be applied to raster images"));
                            return;
                         }
                    }

                    if (qgraphicsitem_cast<TupSvgItem *>(item)) {
                        clearSelection();
                        TOsd::self()->display(TOsd::Warning, tr("Coloring Tween can't be applied to SVG files"));
                        return;
                    }
                }

                QGraphicsItem *item = objects.at(0);
                QColor color = QColor("#fff");
                if (TupTextItem *text = qgraphicsitem_cast<TupTextItem *>(item)) {
                    color = text->defaultTextColor();
                } else if (TupPathItem *path = qgraphicsitem_cast<TupPathItem *>(item)) {
                    color = path->pen().color();
                } else if (TupEllipseItem *ellipse = qgraphicsitem_cast<TupEllipseItem *>(item)) {
                    color = ellipse->pen().color();
                } else if (TupLineItem *line = qgraphicsitem_cast<TupLineItem *>(item)) {
                    color = line->pen().color();
                } else if (TupRectItem *rect = qgraphicsitem_cast<TupRectItem *>(item)) {
                    color = rect->pen().color();
                }

                configPanel->setInitialColor(color);

                #ifdef TUP_DEBUG
                    qDebug() << "[Color Tweener::release()] - Notifying selection...";
                #endif
                configPanel->notifySelection(true);
            }
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[Color Tweener::release()] - Selection mode: no items selected";
            #endif
        }
    }
}

/* This method returns the list of actions defined in this plugin */

QMap<TAction::ActionId, TAction *> Tweener::actions() const
{
    return colorActions;
}

TAction * Tweener::getAction(TAction::ActionId toolId)
{
    return colorActions[toolId];
}

/* This method returns the list of actions defined in this plugin */

int Tweener::toolType() const
{
    return TupToolInterface::Tweener;
}

/* This method returns the tool panel associated to this plugin */

QWidget *Tweener::configurator()
{
    if (!configPanel) {
        mode = TupToolPlugin::View;

        configPanel = new Configurator;
        connect(configPanel, SIGNAL(startingPointChanged(int)), this, SLOT(updateStartPoint(int)));
        connect(configPanel, SIGNAL(clickedApplyTween()), this, SLOT(applyTween()));
        connect(configPanel, SIGNAL(clickedSelect()), this, SLOT(setSelection()));
        connect(configPanel, SIGNAL(clickedDefineProperties()), this, SLOT(setPropertiesMode()));
        connect(configPanel, SIGNAL(clickedResetInterface()), this, SLOT(applyReset()));
        connect(configPanel, SIGNAL(setMode(TupToolPlugin::Mode)), this, SLOT(updateMode(TupToolPlugin::Mode)));
        connect(configPanel, SIGNAL(getTweenData(const QString &)), this, SLOT(setCurrentTween(const QString &)));
        connect(configPanel, SIGNAL(clickedRemoveTween(const QString &)), this, SLOT(removeTween(const QString &)));
    } 

    return configPanel;
}

/* This method is called when there's a change on/of scene */
void Tweener::aboutToChangeScene(TupGraphicsScene *)
{
}

/* This method is called when this plugin is off */
void Tweener::aboutToChangeTool()
{
}

/* This method defines the actions contained in this plugin */
void Tweener::setupActions()
{
    QString name = tr("Coloring Tween");
    QString shortcut = tr("Shift+C");

    TAction *action = new TAction(QPixmap(kAppProp->themeDir() + "icons/coloring_tween.png"), name, this);
    action->setCursor(QCursor(kAppProp->themeDir() + "cursors/tweener.png", 0, 0));
    action->setShortcut(QKeySequence(shortcut));
    action->setToolTip(name + " - " + shortcut);
    action->setActionId(TAction::Color);

    colorActions.insert(TAction::Color, action);
}

/* This method saves the settings of this plugin */
void Tweener::saveConfig()
{
}

/* This method updates the workspace when the plugin changes the scene */
void Tweener::updateScene(TupGraphicsScene *gScene)
{
    mode = configPanel->mode();

    if (mode == TupToolPlugin::Edit) {
        int framesNumber = framesCount();

        if (configPanel->startComboSize() < framesNumber)
            configPanel->initStartCombo(framesNumber, initFrame);

    } else if (mode == TupToolPlugin::Add) {
               int total = framesCount();

               if (editMode == TupToolPlugin::Properties) {
                   if (total > configPanel->startComboSize()) {
                       configPanel->activateMode(TupToolPlugin::Selection);
                       clearSelection();
                       setSelection();
                   }
               } else if (editMode == TupToolPlugin::Selection) {
                   if (gScene->currentFrameIndex() != initFrame)
                       clearSelection();
                   initFrame = gScene->currentFrameIndex();
                   setSelection();
               }

               if (configPanel->startComboSize() < total) {
                   configPanel->initStartCombo(total, initFrame);
               } else {
                   if (gScene->currentFrameIndex() != initFrame)
                       configPanel->setStartFrame(gScene->currentFrameIndex());
               }

    } else {
        if (gScene->currentFrameIndex() != initFrame)
            configPanel->setStartFrame(gScene->currentFrameIndex());
    }
}

void Tweener::setCurrentTween(const QString &name)
{
    TupScene *sceneData = scene->currentScene();
    currentTween = sceneData->tween(name, TupItemTweener::Coloring);
    if (currentTween)
        configPanel->setCurrentTween(currentTween);
}

int Tweener::framesCount()
{
    int total = 1;
    TupLayer *layer = scene->currentScene()->layerAt(scene->currentLayerIndex());
    if (layer)
        total = layer->framesCount();

    return total;
}

/* This method clears selection */
void Tweener::clearSelection()
{
    if (objects.size() > 0) {
        foreach (QGraphicsItem *item, objects) {
            if (item->isSelected())
                item->setSelected(false);
        }
        objects.clear();
        configPanel->notifySelection(false);
    }
}

/* This method disables object selection */
void Tweener::disableSelection()
{
    foreach (QGraphicsView *view, scene->views()) {
        view->setDragMode (QGraphicsView::NoDrag);
        foreach (QGraphicsItem *item, view->scene()->items()) {
            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
            item->setFlag(QGraphicsItem::ItemIsMovable, false);
        }
    }
}

void Tweener::setSelection()
{
    editMode = TupToolPlugin::Selection;

    /*
    int bottomBoundary = ZLAYER_BASE + (initLayer*ZLAYER_LIMIT);
    int topBoundary = bottomBoundary + ZLAYER_LIMIT;
    foreach (QGraphicsView * view, scene->views()) {
             view->setDragMode(QGraphicsView::RubberBandDrag);
             foreach (QGraphicsItem *item, view->scene()->items()) {
                      if ((item->zValue() >= bottomBoundary) && (item->zValue() < topBoundary) && (item->toolTip().length()==0))
                          item->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
             }
    }
    */

    scene->enableItemsForSelection();
    foreach (QGraphicsView *view, scene->views())
        view->setDragMode(QGraphicsView::RubberBandDrag);
    // When Object selection is enabled, previous selection is set
    if (objects.size() > 0) {
        foreach (QGraphicsItem *item, objects) {
            item->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
            item->setSelected(true);
        }
        QGraphicsItem *item = objects.at(0);
        QColor color = QColor();
        if (TupTextItem *text = qgraphicsitem_cast<TupTextItem *>(item)) {
            color = text->defaultTextColor();
        } else if (TupPathItem *path = qgraphicsitem_cast<TupPathItem *>(item)) {
            color = path->pen().color();
        } else if (TupEllipseItem *ellipse = qgraphicsitem_cast<TupEllipseItem *>(item)) {
            color = ellipse->pen().color();
        } else if (TupLineItem *line = qgraphicsitem_cast<TupLineItem *>(item)) {
            color = line->pen().color();
        } else if (TupRectItem *rect = qgraphicsitem_cast<TupRectItem *>(item)) {
            color = rect->pen().color();
        }
        configPanel->setInitialColor(color);
        configPanel->notifySelection(true);
    }
}

void Tweener::setPropertiesMode()
{
    editMode = TupToolPlugin::Properties;
    disableSelection();
}

/* This method resets this plugin */

void Tweener::applyReset()
{
    disableSelection();
    clearSelection();

    mode = TupToolPlugin::View;
    editMode = TupToolPlugin::None;

    initFrame = scene->currentFrameIndex();
    initLayer = scene->currentLayerIndex();
    initScene = scene->currentSceneIndex();
}

/* This method applies to the project, the Tween created from this plugin */

void Tweener::applyTween()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QString name = configPanel->currentTweenName();
    if (name.length() == 0) {
        TOsd::self()->display(TOsd::Warning, tr("Tween name is missing!"));
        return;
    }

    // Tween is new
    if (!scene->tweenExists(name, TupItemTweener::Coloring)) {
        initFrame = scene->currentFrameIndex();
        initLayer = scene->currentLayerIndex();
        initScene = scene->currentSceneIndex();

        foreach (QGraphicsItem *item, objects) {
            TupLibraryObject::ObjectType type = TupLibraryObject::Item;
            int objectIndex = -1;

            if (TupSvgItem *svg = qgraphicsitem_cast<TupSvgItem *>(item)) {
                type = TupLibraryObject::Svg;
                objectIndex = scene->currentFrame()->indexOf(svg);
            } else {
                objectIndex = scene->currentFrame()->indexOf(item);
            }

            TupProjectRequest request = TupRequestBuilder::createItemRequest(
                                        initScene, initLayer, initFrame,
                                        objectIndex, QPointF(), scene->getSpaceContext(),
                                        type, TupProjectRequest::SetTween,
                                        configPanel->tweenToXml(initScene, initLayer, initFrame));
            emit requested(&request);
        }
    } else { // Tween already exists
        removeTweenFromProject(name);
        QList<QGraphicsItem *> newList;

        initFrame = configPanel->startFrame();
        initLayer = currentTween->getInitLayer();
        initScene = scene->currentSceneIndex();

        foreach (QGraphicsItem *item, objects) {
            TupLibraryObject::ObjectType type = TupLibraryObject::Item;
            TupScene *tupScene = scene->currentScene();
            TupLayer *layer = tupScene->layerAt(initLayer);
            TupFrame *frame = layer->frameAt(currentTween->getInitFrame());
            int objectIndex = -1;
            TupSvgItem *svg = qgraphicsitem_cast<TupSvgItem *>(item);

            if (svg) {
                type = TupLibraryObject::Svg;
                objectIndex = frame->indexOf(svg);
            } else {
                objectIndex = frame->indexOf(item);
            }

            if (initFrame != currentTween->getInitFrame()) {
                QDomDocument dom;
                if (type == TupLibraryObject::Svg)
                    dom.appendChild(svg->toXml(dom));
                else
                    dom.appendChild(dynamic_cast<TupAbstractSerializable *>(item)->toXml(dom));

                TupProjectRequest request = TupRequestBuilder::createItemRequest(initScene, initLayer, initFrame,
                                                                                 0, item->pos(), scene->getSpaceContext(),
                                                                                 type, TupProjectRequest::Add,
                                                                                 dom.toString());
                emit requested(&request);

                request = TupRequestBuilder::createItemRequest(initScene, initLayer,
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

            TupProjectRequest request = TupRequestBuilder::createItemRequest(
                                        initScene, initLayer, initFrame,
                                        objectIndex, QPointF(), scene->getSpaceContext(),
                                        type, TupProjectRequest::SetTween,
                                        configPanel->tweenToXml(initScene, initLayer, initFrame));
            emit requested(&request);
        }

        if (newList.size() > 0)
            objects = newList;
    }

    int total = initFrame + configPanel->totalSteps();
    int framesNumber = framesCount();
    int layersCount = scene->currentScene()->layersCount();
    TupProjectRequest request;

    if (total >= framesNumber) {
        for (int i = framesNumber; i < total; i++) {
            for (int j = 0; j < layersCount; j++) {
                request = TupRequestBuilder::createFrameRequest(initScene, j, i,
                                                                TupProjectRequest::Add, tr("Frame"));
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
}

void Tweener::removeTweenFromProject(const QString &name)
{
    TupScene *tupScene = scene->currentScene();
    bool removed = tupScene->removeTween(name, TupItemTweener::Coloring);
    if (removed) {
        foreach (QGraphicsView * view, scene->views()) {
            foreach (QGraphicsItem *item, view->scene()->items()) {
                QString tip = item->toolTip();
                if (tip.compare("Tweens: " + tr("Coloring")) == 0) {
                    item->setToolTip("");
                } else {
                    if (tip.contains(tr("Coloring"))) {
                        tip = tip.replace(tr("Coloring") + ",", "");
                        tip = tip.replace(tr("Coloring"), "");
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
            qDebug() << "[Color Tweener::removeTweenFromProject()] - Coloring tween couldn't be removed -> " << name;
        #endif
    }
}

void Tweener::removeTween(const QString &name)
{
    removeTweenFromProject(name);
    applyReset();
}

void Tweener::updateMode(TupToolPlugin::Mode toolMode)
{
    mode = toolMode;

    if (mode == TupToolPlugin::Edit) {
        if (currentTween) {
            initScene = currentTween->getInitScene();
            initLayer = currentTween->getInitLayer();
            initFrame = currentTween->getInitFrame();

            if (initFrame != scene->currentFrameIndex()) {
                QString selection = QString::number(initLayer) + "," + QString::number(initLayer) + ","
                                    + QString::number(initFrame) + "," + QString::number(initFrame);

                TupProjectRequest request = TupRequestBuilder::createFrameRequest(initScene, initLayer, initFrame,
                                                                              TupProjectRequest::Select, selection);
                emit requested(&request);
            }

            if (objects.isEmpty())
                objects = scene->currentScene()->getItemsFromTween(currentTween->getTweenName(), TupItemTweener::Coloring);
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[Color Tweener::updateMode()] - Current tween pointer is NULL!";
            #endif
        }
    }
}

void Tweener::sceneResponse(const TupSceneResponse *event)
{
    if ((event->getAction() == TupProjectRequest::Remove || event->getAction() == TupProjectRequest::Reset)
        && (scene->currentSceneIndex() == event->getSceneIndex())) {
        init(scene);
    }

    if (event->getAction() == TupProjectRequest::Select)
        init(scene);
}

void Tweener::layerResponse(const TupLayerResponse *event)
{
    if (event->getAction() == TupProjectRequest::Remove)
        init(scene);
}

void Tweener::frameResponse(const TupFrameResponse *event)
{
    if (event->getAction() == TupProjectRequest::Remove && scene->currentLayerIndex() == event->getLayerIndex())
        init(scene);

    if (event->getAction() == TupProjectRequest::Select) {
        if (initLayer != event->getLayerIndex() || initScene != event->getSceneIndex())
            init(scene);
    }
}
