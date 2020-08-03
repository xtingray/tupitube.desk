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

#include "papagayotool.h"
#include "taction.h"
#include "tupinputdeviceinformation.h"
#include "tupbrushmanager.h"
#include "tupgraphicsscene.h"
#include "tuprequestbuilder.h"
#include "tupprojectrequest.h"
#include "tupscene.h"

#ifdef TUP_DEBUG
    #include <QDebug>
#endif

PapagayoTool::PapagayoTool() : TupToolPlugin()
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::PapagayoTool()]";
    #endif

    setupActions();
    configPanel = 0;
    target = 0;
    targetIncluded = false;
}

PapagayoTool::~PapagayoTool()
{
}

/* This method initializes the plugin */

void PapagayoTool::init(TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::init()]";
    #endif

    scene = gScene;
    mode = TupToolPlugin::View;
    baseZValue = ZLAYER_BASE + (scene->currentScene()->layersCount() * ZLAYER_LIMIT);

    initScene = scene->currentSceneIndex();
    removeTarget();
    configPanel->resetUI();

    QList<QString> lipSyncList = scene->currentScene()->getLipSyncNames();
    if (lipSyncList.size() > 0)
        configPanel->loadLipSyncList(lipSyncList);
}

/* This method returns the plugin name */

QList<TAction::ActionId> PapagayoTool::keys() const
{
    return QList<TAction::ActionId>() << TAction::LipSyncTool;
}

/* This method makes an action when the mouse is pressed on the workspace 
 * depending on the active mode: Selecting an object or Creating a path  
*/

void PapagayoTool::press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
{
    Q_UNUSED(input);
    Q_UNUSED(brushManager);
    Q_UNUSED(scene);
}

/* This method is executed while the mouse is pressed and on movement */

void PapagayoTool::move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
{
    Q_UNUSED(input);
    Q_UNUSED(brushManager);
    Q_UNUSED(scene);
}

/* This method finishes the action started on the press method depending
 * on the active mode: Selecting an object or Creating a path
*/

void PapagayoTool::release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
{
    Q_UNUSED(input);
    Q_UNUSED(brushManager);
    Q_UNUSED(scene);
}

/* This method returns the list of actions defined in this plugin */

QMap<TAction::ActionId, TAction *> PapagayoTool::actions() const
{
    return pgActions;
}

/* This method returns the list of actions defined in this plugin */

int PapagayoTool::toolType() const
{
    return TupToolInterface::LipSync;
}

/* This method returns the tool panel associated to this plugin */

QWidget *PapagayoTool::configurator()
{
    if (!configPanel) {
        mode = TupToolPlugin::View;

        configPanel = new Configurator;
        connect(configPanel, SIGNAL(importLipSync()), this, SIGNAL(importLipSync()));
        connect(configPanel, SIGNAL(editLipSyncSelection(const QString &)), this, SLOT(editLipSyncSelection(const QString &)));
        connect(configPanel, SIGNAL(removeCurrentLipSync(const QString &)), this, SLOT(removeCurrentLipSync(const QString &)));
        connect(configPanel, SIGNAL(selectMouth(const QString &, int)), this, SLOT(addTarget(const QString &, int)));
        connect(configPanel, SIGNAL(closeLipSyncProperties()), this, SLOT(resetCanvas()));
        connect(configPanel, SIGNAL(initFrameHasChanged(int)), this, SLOT(updateInitFrame(int)));
        connect(configPanel, SIGNAL(xPosChanged(int)), this, SLOT(updateXPosition(int)));
        connect(configPanel, SIGNAL(yPosChanged(int)), this, SLOT(updateYPosition(int)));
    } 

    return configPanel;
}

void PapagayoTool::resetCanvas()
{
    mode = TupToolPlugin::View;
    removeTarget();
}

/* This method is called when there's a change on/of scene */
void PapagayoTool::aboutToChangeScene(TupGraphicsScene *)
{
}

/* This method is called when this plugin is off */
void PapagayoTool::aboutToChangeTool()
{
    init(scene);
}

/* This method defines the actions contained in this plugin */

void PapagayoTool::setupActions()
{
    QString name = tr("Papagayo Lip-sync");
    QString shortcut = tr("Ctrl+Shift+P");

    TAction *lipsync = new TAction(QPixmap(kAppProp->themeDir() + "icons/papagayo.png"), name, this);
    lipsync->setShortcut(QKeySequence(shortcut));
    lipsync->setToolTip(name + " - " + shortcut);
    lipsync->setActionId(TAction::LipSyncTool);

    pgActions.insert(TAction::LipSyncTool, lipsync);
}

/* This method saves the settings of this plugin */

void PapagayoTool::saveConfig()
{
}

/* This method updates the workspace when the plugin changes the scene */

void PapagayoTool::updateScene(TupGraphicsScene *scene)
{ 
    Q_UNUSED(scene); 
}

void PapagayoTool::editLipSyncSelection(const QString &name)
{
    TupScene *sceneData = scene->currentScene();
    currentLipSync = sceneData->getLipSync(name);

    configPanel->openLipSyncProperties(currentLipSync);

    TupVoice *voice = currentLipSync->getVoices().at(0);
    if (voice) {
        TupPhoneme *phoneme = voice->getPhonemeAt(0);
        if (phoneme) {
            configPanel->setPhoneme(phoneme->value());
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "PapagayoTool::editLipSyncSelection() - No lipsync phoneme at index 0";
            #endif
        }
        configPanel->setPos(voice->mouthPos());
    }
}

void PapagayoTool::removeCurrentLipSync(const QString &name)
{
    QGraphicsView * view = scene->views().first();
    foreach (QGraphicsItem *item, view->scene()->items()) {
        QString tip = item->toolTip();
        if (tip.length() > 0) {
            if (tip.startsWith(tr("lipsync:") + name))
                scene->removeItem(item);
        }
    }

    TupProjectRequest request = TupRequestBuilder::createLayerRequest(initScene, 0, TupProjectRequest::RemoveLipSync, name);
    emit requested(&request);
}

void PapagayoTool::updateOriginPoint(const QPointF &point)
{
    origin = point;
    mouth->setPos(origin - mouthOffset);
    currentLipSync->updateMouthPosition(currentMouthIndex, point, scene->currentFrameIndex());

    TupScene *sceneData = scene->currentScene();
    sceneData->updateLipSync(currentLipSync);

    configPanel->setPos(point);
}

void PapagayoTool::addTarget(const QString &id, int index)
{
    mode = TupToolPlugin::Edit;

    currentMouth = id;
    currentMouthIndex = index;

    TupScene *sceneData = scene->currentScene();
    int initLayer = sceneData->getLipSyncLayerIndex(currentLipSync->getLipSyncName());
    int initFrame = currentLipSync->getInitFrame();

    QString selection = QString::number(initLayer) + "," + QString::number(initLayer) + ","
                        + QString::number(initFrame) + "," + QString::number(initFrame);

    TupProjectRequest request = TupRequestBuilder::createFrameRequest(initScene, initLayer, initFrame,
                                                                      TupProjectRequest::Select, selection);
    emit requested(&request);

    removeTarget();
    setTargetEnvironment();
}

void PapagayoTool::setTargetEnvironment()
{
    QGraphicsView *view = scene->views().at(0);
    foreach (QGraphicsItem *item, view->scene()->items()) {
        QString tip = item->toolTip();
        if (tip.length() > 0) {
            if (tip.compare(currentMouth) == 0) {
                mouthOffset = item->boundingRect().center();
                origin = item->pos() + mouthOffset;
                mouth = item;
            }
        }
    }

    target = new MouthTarget(origin, baseZValue);
    connect(target, SIGNAL(positionUpdated(const QPointF &)), this, SLOT(updateOriginPoint(const QPointF &)));
    scene->addItem(target);
    targetIncluded = true;

    TupVoice *voice = currentLipSync->getVoices().at(currentMouthIndex);
    if (voice) {
        int index = scene->currentFrameIndex() - currentLipSync->getInitFrame();
        TupPhoneme *phoneme = voice->getPhonemeAt(index);
        if (phoneme) {
            configPanel->setPhoneme(phoneme->value());
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "TupGraphicsScene::setTargetEnvironment() - No lipsync phoneme at frame " + QString::number(scene->currentFrameIndex()) + " - index: " + QString::number(index);
            #endif
        }
    }

    configPanel->setPos(origin);
}

void PapagayoTool::removeTarget()
{
    if (targetIncluded) {
        if (target) {
            scene->removeItem(target);
            target = 0;
            targetIncluded = false;
        }
    }
}

void PapagayoTool::sceneResponse(const TupSceneResponse *event)
{
    if ((event->getAction() == TupProjectRequest::Remove || event->getAction() == TupProjectRequest::Reset)
        && (scene->currentSceneIndex() == event->getSceneIndex()))
        init(scene);

    if (event->getAction() == TupProjectRequest::Select)
        init(scene);
}

void PapagayoTool::layerResponse(const TupLayerResponse *event)
{
    if (event->getAction() == TupProjectRequest::AddLipSync) {
        QString xml = event->getArg().toString();
        currentLipSync = new TupLipSync();
        currentLipSync->fromXml(xml);
        configPanel->addLipSyncRecord(currentLipSync->getLipSyncName());
    } 
}

void PapagayoTool::frameResponse(const TupFrameResponse *event)
{
    if (event->getAction() == TupProjectRequest::Select) {
        if (mode == TupToolPlugin::Edit) {
            int frameIndex = event->getFrameIndex();
            int lastFrame = currentLipSync->getInitFrame() + currentLipSync->getFramesCount() - 1;
            if (frameIndex >= currentLipSync->getInitFrame() && frameIndex <= lastFrame)
                setTargetEnvironment();
        }
    }
}

void PapagayoTool::updateWorkSpaceContext()
{
    if (mode == TupToolPlugin::Edit)
        configPanel->closePanels();
}

void PapagayoTool::updateInitFrame(int index)
{
    removeTarget();
    currentLipSync->setInitFrame(index);

    TupScene *sceneData = scene->currentScene();
    sceneData->updateLipSync(currentLipSync);

    int sceneFrames = sceneData->framesCount();
    int lipSyncFrames = index + currentLipSync->getFramesCount();
    if (lipSyncFrames > sceneFrames) {
        int layersCount = sceneData->layersCount();
        for (int i = sceneFrames; i < lipSyncFrames; i++) {
             for (int j = 0; j < layersCount; j++) {
                  TupProjectRequest request = TupRequestBuilder::createFrameRequest(initScene, j, i, TupProjectRequest::Add, tr("Frame"));
                  emit requested(&request);
             }
        }
    }

    configPanel->updateInterfaceRecords();

    int initLayer = sceneData->getLipSyncLayerIndex(currentLipSync->getLipSyncName());
    QString selection = QString::number(initLayer) + "," + QString::number(initLayer) + ","
                        + QString::number(index) + "," + QString::number(index);

    TupProjectRequest request = TupRequestBuilder::createFrameRequest(initScene, initLayer, index, TupProjectRequest::Select, selection);
    emit requested(&request);
}

void PapagayoTool::keyPressEvent(QKeyEvent *event)
{
    if (mode == TupToolPlugin::Edit) {
        if ((event->key() == Qt::Key_Left) || (event->key() == Qt::Key_Up)
            || (event->key() == Qt::Key_Right) || (event->key() == Qt::Key_Down)) {
            int delta = 5;

            if (event->modifiers()==Qt::ShiftModifier)
                delta = 1;

            if (event->modifiers()==Qt::ControlModifier)
                delta = 10;

            QPointF point;

            if (event->key() == Qt::Key_Left) {
                mouth->moveBy(-delta, 0);
                target->moveBy(-delta, 0);
                point = origin + QPointF(-delta, 0);
            }

            if (event->key() == Qt::Key_Up) {
                mouth->moveBy(0, -delta);
                target->moveBy(0, -delta);
                point = origin + QPointF(0, -delta);
            }

            if (event->key() == Qt::Key_Right) {
                mouth->moveBy(delta, 0);
                target->moveBy(delta, 0);
                point = origin + QPointF(delta, 0);
            }

            if (event->key() == Qt::Key_Down) {
                mouth->moveBy(0, delta);
                target->moveBy(0, delta);
                point = origin + QPointF(0, delta);
            }
     
            updateOriginPoint(point);
        }
    }
}

void PapagayoTool::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
}

void PapagayoTool::updateXPosition(int x)
{
    QPointF point(x, origin.y());
    target->setPos(point);
    updateOriginPoint(point);
}

void PapagayoTool::updateYPosition(int y)
{
    QPointF point(origin.x(), y);
    target->setPos(point);
    updateOriginPoint(point);
}
