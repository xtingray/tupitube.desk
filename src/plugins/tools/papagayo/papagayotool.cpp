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
#include "configurator.h"
#include "mouthtarget.h"
#include "taction.h"
#include "tupinputdeviceinformation.h"
#include "tupbrushmanager.h"
#include "tupgraphicsscene.h"
#include "tuprequestbuilder.h"
#include "tupprojectrequest.h"
#include "tupscene.h"

struct PapagayoTool::Private
{
    QMap<QString, TAction *> pgActions;
    Configurator *configPanel;

    TupGraphicsScene *scene;

    TupLipSync *currentLipSync;
    int initScene;

    QPointF origin;
    MouthTarget *target;

    TupToolPlugin::Mode mode;

    int baseZValue;

    QGraphicsItem *mouth;
    QPointF mouthOffset;
    QString currentMouth;
    int currentMouthIndex;
    bool targetIncluded;
};

PapagayoTool::PapagayoTool() : TupToolPlugin(), k(new Private)
{
    setupActions();
    k->configPanel = 0;
    k->target = 0;
    k->targetIncluded = false;
}

PapagayoTool::~PapagayoTool()
{
    delete k;
}

/* This method initializes the plugin */

void PapagayoTool::init(TupGraphicsScene *scene)
{
    k->scene = scene;
    k->mode = TupToolPlugin::View;
    k->baseZValue = ZLAYER_BASE + (scene->currentScene()->layersCount() * ZLAYER_LIMIT);

    k->initScene = k->scene->currentSceneIndex();

    removeTarget();

    k->configPanel->resetUI();

    QList<QString> lipSyncList = k->scene->currentScene()->getLipSyncNames();
    if (lipSyncList.size() > 0)
        k->configPanel->loadLipSyncList(lipSyncList);
}

/* This method returns the plugin name */

QStringList PapagayoTool::keys() const
{
    return QStringList() << tr("Papagayo Lip-sync");
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

QMap<QString, TAction *> PapagayoTool::actions() const
{
    return k->pgActions;
}

/* This method returns the list of actions defined in this plugin */

int PapagayoTool::toolType() const
{
    return TupToolInterface::LipSync;
}

/* This method returns the tool panel associated to this plugin */

QWidget *PapagayoTool::configurator()
{
    if (!k->configPanel) {
        k->mode = TupToolPlugin::View;

        k->configPanel = new Configurator;
        connect(k->configPanel, SIGNAL(importLipSync()), this, SIGNAL(importLipSync()));
        connect(k->configPanel, SIGNAL(editLipSyncSelection(const QString &)), this, SLOT(editLipSyncSelection(const QString &)));
        connect(k->configPanel, SIGNAL(removeCurrentLipSync(const QString &)), this, SLOT(removeCurrentLipSync(const QString &)));
        connect(k->configPanel, SIGNAL(selectMouth(const QString &, int)), this, SLOT(addTarget(const QString &, int)));
        connect(k->configPanel, SIGNAL(closeLipSyncProperties()), this, SLOT(resetCanvas()));
        connect(k->configPanel, SIGNAL(initFrameHasChanged(int)), this, SLOT(updateInitFrame(int)));
        connect(k->configPanel, SIGNAL(xPosChanged(int)), this, SLOT(updateXPosition(int)));
        connect(k->configPanel, SIGNAL(yPosChanged(int)), this, SLOT(updateYPosition(int)));
    } 

    return k->configPanel;
}

void PapagayoTool::resetCanvas()
{
    k->mode = TupToolPlugin::View;
    removeTarget();
}

/* This method is called when there's a change on/of scene */
void PapagayoTool::aboutToChangeScene(TupGraphicsScene *)
{
}

/* This method is called when this plugin is off */
void PapagayoTool::aboutToChangeTool()
{
    init(k->scene);
}

/* This method defines the actions contained in this plugin */

void PapagayoTool::setupActions()
{
    TAction *lipsync = new TAction(QPixmap(kAppProp->themeDir() + "icons/papagayo.png"), 
                                      tr("Papagayo Lip-sync"), this);
    lipsync->setShortcut(QKeySequence(tr("Ctrl+Shift+P")));

    k->pgActions.insert(tr("Papagayo Lip-sync"), lipsync);
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
    TupScene *scene = k->scene->currentScene();
    k->currentLipSync = scene->getLipSync(name);

    k->configPanel->openLipSyncProperties(k->currentLipSync);

    TupVoice *voice = k->currentLipSync->getVoices().at(0);
    if (voice) {
        TupPhoneme *phoneme = voice->getPhonemeAt(0);
        if (phoneme) {
            k->configPanel->setPhoneme(phoneme->value());
        } else {
            #ifdef TUP_DEBUG
                QString msg = "PapagayoTool::editLipSyncSelection() - No lipsync phoneme at index 0";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif
        }
        k->configPanel->setPos(voice->mouthPos());
    }
}

void PapagayoTool::removeCurrentLipSync(const QString &name)
{
    QGraphicsView * view = k->scene->views().first();
    foreach (QGraphicsItem *item, view->scene()->items()) {
             QString tip = item->toolTip();
             if (tip.length() > 0) {
                 if (tip.startsWith(tr("lipsync:") + name))
                     k->scene->removeItem(item);
             }
    }

    TupProjectRequest request = TupRequestBuilder::createLayerRequest(k->initScene, 0, TupProjectRequest::RemoveLipSync, name);
    emit requested(&request);
}

void PapagayoTool::updateOriginPoint(const QPointF &point)
{
    k->origin = point;
    k->mouth->setPos(k->origin - k->mouthOffset);
    k->currentLipSync->updateMouthPosition(k->currentMouthIndex, point, k->scene->currentFrameIndex());

    TupScene *scene = k->scene->currentScene();
    scene->updateLipSync(k->currentLipSync);

    k->configPanel->setPos(point);
}

void PapagayoTool::addTarget(const QString &id, int index)
{
    k->mode = TupToolPlugin::Edit;

    k->currentMouth = id;
    k->currentMouthIndex = index;

    TupScene *scene = k->scene->currentScene();
    int initLayer = scene->getLipSyncLayerIndex(k->currentLipSync->getLipSyncName());
    int initFrame = k->currentLipSync->getInitFrame();

    QString selection = QString::number(initLayer) + "," + QString::number(initLayer) + ","
                        + QString::number(initFrame) + "," + QString::number(initFrame);

    TupProjectRequest request = TupRequestBuilder::createFrameRequest(k->initScene, initLayer, initFrame,
                                                                      TupProjectRequest::Select, selection);
    emit requested(&request);

    removeTarget();
    setTargetEnvironment();
}

void PapagayoTool::setTargetEnvironment()
{
    QGraphicsView *view = k->scene->views().at(0);
    foreach (QGraphicsItem *item, view->scene()->items()) {
             QString tip = item->toolTip();
             if (tip.length() > 0) {
                 if (tip.compare(k->currentMouth) == 0) {
                     k->mouthOffset = item->boundingRect().center();
                     k->origin = item->pos() + k->mouthOffset;
                     k->mouth = item;
                 }
             }
    }

    k->target = new MouthTarget(k->origin, k->baseZValue);
    connect(k->target, SIGNAL(positionUpdated(const QPointF &)), this, SLOT(updateOriginPoint(const QPointF &)));
    k->scene->addItem(k->target);
    k->targetIncluded = true;

    TupVoice *voice = k->currentLipSync->getVoices().at(k->currentMouthIndex);
    if (voice) {
        int index = k->scene->currentFrameIndex() - k->currentLipSync->getInitFrame();
        TupPhoneme *phoneme = voice->getPhonemeAt(index);
        if (phoneme) {
            k->configPanel->setPhoneme(phoneme->value());
        } else {
            #ifdef TUP_DEBUG
                QString msg = "TupGraphicsScene::setTargetEnvironment() - No lipsync phoneme at frame " + QString::number(k->scene->currentFrameIndex()) + " - index: " + QString::number(index);
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif
        }
    }

    k->configPanel->setPos(k->origin);
}

void PapagayoTool::removeTarget()
{
    if (k->targetIncluded) {
        if (k->target) {
            k->scene->removeItem(k->target);
            k->target = 0;
            k->targetIncluded = false;
        }
    }
}

void PapagayoTool::sceneResponse(const TupSceneResponse *event)
{
    if ((event->getAction() == TupProjectRequest::Remove || event->getAction() == TupProjectRequest::Reset)
        && (k->scene->currentSceneIndex() == event->getSceneIndex()))
        init(k->scene);

    if (event->getAction() == TupProjectRequest::Select)
        init(k->scene);
}

void PapagayoTool::layerResponse(const TupLayerResponse *event)
{
    if (event->getAction() == TupProjectRequest::AddLipSync) {
        QString xml = event->getArg().toString();
        k->currentLipSync = new TupLipSync();
        k->currentLipSync->fromXml(xml);
        k->configPanel->addLipSyncRecord(k->currentLipSync->getLipSyncName());
    } 
}

void PapagayoTool::frameResponse(const TupFrameResponse *event)
{
    if (event->getAction() == TupProjectRequest::Select) {
        if (k->mode == TupToolPlugin::Edit) {
            int frameIndex = event->getFrameIndex();
            int lastFrame = k->currentLipSync->getInitFrame() + k->currentLipSync->getFramesCount() - 1;
            if (frameIndex >= k->currentLipSync->getInitFrame() && frameIndex <= lastFrame)
                setTargetEnvironment();
        }
    }
}

void PapagayoTool::updateWorkSpaceContext()
{
    if (k->mode == TupToolPlugin::Edit)
        k->configPanel->closePanels();
}

void PapagayoTool::updateInitFrame(int index)
{
    removeTarget();
    k->currentLipSync->setInitFrame(index);

    TupScene *scene = k->scene->currentScene();
    scene->updateLipSync(k->currentLipSync);

    int sceneFrames = scene->framesCount();
    int lipSyncFrames = index + k->currentLipSync->getFramesCount();
    if (lipSyncFrames > sceneFrames) {
        int layersCount = scene->layersCount();
        for (int i = sceneFrames; i < lipSyncFrames; i++) {
             for (int j = 0; j < layersCount; j++) {
                  TupProjectRequest request = TupRequestBuilder::createFrameRequest(k->initScene, j, i, TupProjectRequest::Add, tr("Frame"));
                  emit requested(&request);
             }
        }
    }

    k->configPanel->updateInterfaceRecords();

    int initLayer = scene->getLipSyncLayerIndex(k->currentLipSync->getLipSyncName());
    QString selection = QString::number(initLayer) + "," + QString::number(initLayer) + ","
                        + QString::number(index) + "," + QString::number(index);

    TupProjectRequest request = TupRequestBuilder::createFrameRequest(k->initScene, initLayer, index, TupProjectRequest::Select, selection);
    emit requested(&request);
}

void PapagayoTool::keyPressEvent(QKeyEvent *event)
{
    if (k->mode == TupToolPlugin::Edit) {
        if ((event->key() == Qt::Key_Left) || (event->key() == Qt::Key_Up)
            || (event->key() == Qt::Key_Right) || (event->key() == Qt::Key_Down)) {
            int delta = 5;

            if (event->modifiers()==Qt::ShiftModifier)
                delta = 1;

            if (event->modifiers()==Qt::ControlModifier)
                delta = 10;

            QPointF point;

            if (event->key() == Qt::Key_Left) {
                k->mouth->moveBy(-delta, 0);
                k->target->moveBy(-delta, 0);
                point = k->origin + QPointF(-delta, 0);
            }

            if (event->key() == Qt::Key_Up) {
                k->mouth->moveBy(0, -delta);
                k->target->moveBy(0, -delta);
                point = k->origin + QPointF(0, -delta);
            }

            if (event->key() == Qt::Key_Right) {
                k->mouth->moveBy(delta, 0);
                k->target->moveBy(delta, 0);
                point = k->origin + QPointF(delta, 0);
            }

            if (event->key() == Qt::Key_Down) {
                k->mouth->moveBy(0, delta);
                k->target->moveBy(0, delta);
                point = k->origin + QPointF(0, delta);
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
    QPointF point(x, k->origin.y());
    k->target->setPos(point);
    updateOriginPoint(point);
}

void PapagayoTool::updateYPosition(int y)
{
    QPointF point(k->origin.x(), y);
    k->target->setPos(point);
    updateOriginPoint(point);
}

