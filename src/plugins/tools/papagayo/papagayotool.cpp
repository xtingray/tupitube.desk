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

PapagayoTool::PapagayoTool() : TupToolPlugin()
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::PapagayoTool()]";
    #endif

    setupActions();
    configPanel = 0;
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

    target = 0;
    targetIncluded = false;
    scene = gScene;
    mode = TupToolPlugin::View;

    sceneIndex = scene->currentSceneIndex();
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
    Q_UNUSED(input)
    Q_UNUSED(brushManager)
    Q_UNUSED(scene)
}

/* This method is executed while the mouse is pressed and on movement */

void PapagayoTool::move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
{
    Q_UNUSED(input)
    Q_UNUSED(brushManager)
    Q_UNUSED(scene)
}

/* This method finishes the action started on the press method depending
 * on the active mode: Selecting an object or Creating a path
*/

void PapagayoTool::release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
{
    Q_UNUSED(input)
    Q_UNUSED(brushManager)
    Q_UNUSED(scene)
}

/* This method returns the list of actions defined in this plugin */

QMap<TAction::ActionId, TAction *> PapagayoTool::actions() const
{
    return pgActions;
}

TAction * PapagayoTool::getAction(TAction::ActionId toolId)
{
    return pgActions[toolId];
}

/* This method returns the list of actions defined in this plugin */

int PapagayoTool::toolType() const
{
    return TupToolInterface::LipSync;
}

/* This method returns the tool panel associated to this plugin */

QWidget * PapagayoTool::configurator()
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::configurator()]";
    #endif

    if (!configPanel) {
        mode = TupToolPlugin::View;

        configPanel = new Configurator;
        connect(configPanel, SIGNAL(openLipSyncCreator()), this, SIGNAL(openLipSyncCreator()));
        connect(configPanel, &Configurator::editLipSyncSelection, this, &PapagayoTool::editLipSyncSelection);
        connect(configPanel, &Configurator::removeCurrentLipSync, this, &PapagayoTool::removeCurrentLipSync);
        connect(configPanel, SIGNAL(closeLipSyncProperties()), this, SLOT(resetCanvas()));
        connect(configPanel, SIGNAL(initFrameHasChanged(int)), this, SLOT(updateInitFrame(int)));
        connect(configPanel, SIGNAL(xPosChanged(int)), this, SLOT(updateXPosition(int)));
        connect(configPanel, SIGNAL(yPosChanged(int)), this, SLOT(updateYPosition(int)));
    } 

    return configPanel;
}

void PapagayoTool::resetCanvas()
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::resetCanvas()]";
    #endif

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
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::aboutToChangeTool()]";
    #endif

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

    realFactor = 1;
}

/* This method saves the settings of this plugin */

void PapagayoTool::saveConfig()
{
}

/* This method updates the workspace when the plugin changes the scene */

void PapagayoTool::updateScene(TupGraphicsScene *scene)
{ 
    Q_UNUSED(scene)
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
                qDebug() << "[PapagayoTool::editLipSyncSelection()] - No lipsync phoneme at index 0";
            #endif
        }
        configPanel->setPos(voice->mouthPos());
        addTarget();
    }
}

void PapagayoTool::removeCurrentLipSync(const QString &name)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::removeCurrentLipSync()] - name -> " << name;
    #endif

    QList<QGraphicsView *> views = scene->views();
    QGraphicsView * view = views.first();
    foreach (QGraphicsItem *item, view->scene()->items()) {
        QString tip = item->toolTip();
        if (tip.length() > 0) {
            if (tip.startsWith(tr("lipsync:") + name))
                scene->removeItem(item);
        }
    }

    TupProjectRequest request = TupRequestBuilder::createLayerRequest(sceneIndex, 0, TupProjectRequest::RemoveLipSync, name);
    emit requested(&request);

    request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Remove, name, TupLibraryObject::Folder);
    emit requested(&request);
}

void PapagayoTool::setTargetInitPos(const QPointF &point)
{
    targetInitPos = point;
}

void PapagayoTool::updateOriginPoint(const QPointF &point)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::updateOriginPoint()] - point -> " << point;
    #endif

    origin = point;
    mouth->setPos(origin - mouthOffset);

    int sceneIndex = scene->currentFrameIndex();
    currentLipSync->updateMouthPos(origin, (scene->currentFrameIndex() - currentLipSync->getInitFrame()));

    TupProjectRequest request = TupRequestBuilder::createLayerRequest(sceneIndex, scene->currentLayerIndex(),
                                                                      TupProjectRequest::UpdateLipSync, currentLipSync->toString());
    emit requested(&request);

    configPanel->setPos(point);
}

void PapagayoTool::addTarget()
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::addTarget()]";
    #endif

    mode = TupToolPlugin::Edit;

    TupScene *sceneData = scene->currentScene();
    int initLayer = sceneData->getLipSyncLayerIndex(currentLipSync->getLipSyncName());
    int initFrame = currentLipSync->getInitFrame();

    QString selection = QString::number(initLayer) + "," + QString::number(initLayer) + ","
                        + QString::number(initFrame) + "," + QString::number(initFrame);

    TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, initLayer, initFrame,
                                                                      TupProjectRequest::Select, selection);
    emit requested(&request);

    removeTarget();
    setTargetEnvironment();
}

void PapagayoTool::setTargetEnvironment()
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::setTargetEnvironment()]";
    #endif

    QGraphicsView *view = scene->views().at(0);
    foreach (QGraphicsItem *item, view->scene()->items()) {
        QString tip = item->toolTip();
        if (tip.length() > 0) {
            QString mouthID = "lipsync:" + currentLipSync->getLipSyncName();
            if (tip.compare(mouthID) == 0) {
                mouthOffset = item->boundingRect().center();
                origin = item->pos() + mouthOffset;
                mouth = item;
            }
        }
    }

    target = new TMouthTarget(origin, mouth->zValue() + 1);
    target->resize(realFactor);
    connect(target, &TMouthTarget::initPos, this, &PapagayoTool::setTargetInitPos);
    connect(target, &TMouthTarget::positionUpdated, this, &PapagayoTool::updateOriginPoint);
    scene->addItem(target);
    targetIncluded = true;

    TupVoice *voice = currentLipSync->voiceAt(0);
    if (voice) {
        int index = scene->currentFrameIndex() - currentLipSync->getInitFrame();
        TupPhoneme *phoneme = voice->getPhonemeAt(index);
        if (phoneme) {
            configPanel->setPhoneme(phoneme->value());
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[PapagayoTool::setTargetEnvironment()] - No lipsync phoneme at frame "
                         << scene->currentFrameIndex() << " - index: " << index;
            #endif
        }
    }

    configPanel->setPos(origin);
}

void PapagayoTool::removeTarget()
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::removeTarget()]";
    #endif

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
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::sceneResponse()]";
    #endif

    if ((event->getAction() == TupProjectRequest::Remove || event->getAction() == TupProjectRequest::Reset)
        && (scene->currentSceneIndex() == event->getSceneIndex()))
        init(scene);

    if (event->getAction() == TupProjectRequest::Select)
        init(scene);
}

void PapagayoTool::layerResponse(const TupLayerResponse *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::layerResponse()]";
    #endif

    if (event->getAction() == TupProjectRequest::AddLipSync) {
        QString xml = event->getArg().toString();
        currentLipSync = new TupLipSync();
        currentLipSync->fromXml(xml);
        configPanel->addLipSyncRecord(currentLipSync->getLipSyncName());
    }

    if (event->getAction() == TupProjectRequest::UpdateLipSync) {
        if (mode == TupToolPlugin::Edit)
            setTargetEnvironment();
    }
}

void PapagayoTool::frameResponse(const TupFrameResponse *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::frameResponse()]";
    #endif

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
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::updateWorkSpaceContext()]";
    #endif

    if (mode == TupToolPlugin::Edit)
        configPanel->closePanels();
}

void PapagayoTool::updateInitFrame(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::updateInitFrame()]";
    #endif

    removeTarget();
    currentLipSync->setInitFrame(index);

    TupScene *sceneData = scene->currentScene();

    int sceneFrames = sceneData->framesCount();
    int lipSyncFrames = index + currentLipSync->getFramesCount();
    if (lipSyncFrames > sceneFrames) {
        int layersCount = sceneData->layersCount();
        for (int i = sceneFrames; i < lipSyncFrames; i++) {
             for (int j = 0; j < layersCount; j++) {
                  TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, j, i,
                                                                                    TupProjectRequest::Add, tr("Frame"));
                  emit requested(&request);
             }
        }
    }

    configPanel->updateInterfaceRecords();

    TupProjectRequest request = TupRequestBuilder::createLayerRequest(sceneIndex, scene->currentLayerIndex(),
                                                                      TupProjectRequest::UpdateLipSync, currentLipSync->toString());
    emit requested(&request);

    int initLayer = sceneData->getLipSyncLayerIndex(currentLipSync->getLipSyncName());
    QString selection = QString::number(initLayer) + "," + QString::number(initLayer) + ","
                        + QString::number(index) + "," + QString::number(index);

    request = TupRequestBuilder::createFrameRequest(sceneIndex, initLayer, index, TupProjectRequest::Select, selection);
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
    } else {
        QPair<int, int> flags = TAction::setKeyAction(event->key(), event->modifiers());
        if (flags.first != -1 && flags.second != -1)
            emit callForPlugin(flags.first, flags.second);
    }
}

void PapagayoTool::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event)
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

void PapagayoTool::resizeNode(qreal factor)
{
    realFactor = factor;
    if (target)
        target->resize(realFactor);
}

void PapagayoTool::updateZoomFactor(qreal factor)
{
    realFactor = factor;
}

TupToolPlugin::Mode PapagayoTool::currentMode()
{
    return mode;
}

/*
void PapagayoTool::openLipSyncEditor(const QString &soundFile)
{
}
*/
