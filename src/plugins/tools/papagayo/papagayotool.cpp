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
#include "tupserializer.h"

PapagayoTool::PapagayoTool() : TupToolPlugin()
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::PapagayoTool()]";
    #endif

    setupActions();
    configPanel = nullptr;
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

    nodesManager = nullptr;
    managerIncluded = false;
    scene = gScene;
    mode = TupToolPlugin::View;

    sceneIndex = scene->currentSceneIndex();
    removeNodesManager();
    configPanel->resetUI();

    nodeZValue = ((BG_LAYERS + 1) * ZLAYER_LIMIT) + (scene->currentScene()->layersCount() * ZLAYER_LIMIT);

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
        connect(configPanel, &Configurator::lipsyncCreatorRequested, this, &PapagayoTool::lipsyncCreatorRequested);
        connect(configPanel, &Configurator::lipsyncEditionRequested, this, &PapagayoTool::lipsyncEditionRequested);
        connect(configPanel, &Configurator::mouthEditionRequested, this, &PapagayoTool::editLipsyncMouth);
        connect(configPanel, &Configurator::currentLipsyncRemoved, this, &PapagayoTool::removeCurrentLipSync);
        connect(configPanel, &Configurator::closeLipSyncProperties, this, &PapagayoTool::resetCanvas);
        connect(configPanel, &Configurator::initFrameHasChanged, this, &PapagayoTool::updateInitFrame);

        connect(configPanel, &Configurator::xPosChanged, this, &PapagayoTool::updateXMouthPositionInScene);
        connect(configPanel, &Configurator::yPosChanged, this, &PapagayoTool::updateYMouthPositionInScene);
        connect(configPanel, &Configurator::rotationChanged, this, &PapagayoTool::updateRotationInScene);
        connect(configPanel, &Configurator::scaleChanged, this, &PapagayoTool::updateScaleInScene);

        connect(configPanel, &Configurator::objectHasBeenReset, this, &PapagayoTool::resetMouthTransformations);
        connect(configPanel, &Configurator::proportionActivated, this, &PapagayoTool::enableProportion);
    } 

    return configPanel;
}

void PapagayoTool::resetCanvas()
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::resetCanvas()]";
    #endif

    mode = TupToolPlugin::View;
    removeNodesManager();
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
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::setupActions()]";
    #endif

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

void PapagayoTool::editLipsyncMouth(const QString &name)
{
    TupScene *sceneData = scene->currentScene();
    currentLipSync = sceneData->getLipSync(name);

    configPanel->openLipSyncProperties(currentLipSync);

    TupVoice *voice = currentLipSync->getVoice();
    if (voice) {
        TupPhoneme *phoneme = voice->getPhonemeAt(0);
        if (phoneme) {
            configPanel->setPhoneme(phoneme);
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[PapagayoTool::editLipsyncMouth()] - No lipsync phoneme at index 0";
            #endif
        }

        addNodesManager();
    }
}

void PapagayoTool::removeCurrentLipSync(const QString &lipsyncName)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::removeCurrentLipSync()] - name -> " << lipsyncName;
    #endif

    QList<QGraphicsView *> views = scene->views();
    QGraphicsView * view = views.first();
    foreach (QGraphicsItem *item, view->scene()->items()) {
        QString tip = item->toolTip();
        if (tip.length() > 0) {
            if (tip.startsWith(tr("lipsync:") + lipsyncName))
                scene->removeItem(item);
        }
    }

    TupProjectRequest request = TupRequestBuilder::createLayerRequest(sceneIndex, 0, TupProjectRequest::RemoveLipSync, lipsyncName);
    emit requested(&request);

    request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Remove, lipsyncName, TupLibraryObject::Folder);
    emit requested(&request);

    TupScene *sceneData = scene->currentScene();
    TupProject *project = sceneData->project();

    QString pgoPath = project->getDataDir() + "/pgo/" + lipsyncName;
    if (QFile::exists(pgoPath)) {
        if (!QFile::remove(pgoPath)) {
            #ifdef TUP_DEBUG
                qDebug() << "[PapagayoTool::removeCurrentLipSync()] - Fatal Error: Can't remove PGO file -> " << pgoPath;
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[PapagayoTool::removeCurrentLipSync()] - Warning: PGO file doesn't exists -> " << pgoPath;
        #endif
    }

    QString imagesPath = project->getDataDir() + "/images/" + lipsyncName;
    QDir imgDir(imagesPath);
    if (QFile::exists(imagesPath)) {
        if (!imgDir.removeRecursively()) {
            #ifdef TUP_DEBUG
                qDebug() << "[PapagayoTool::removeCurrentLipSync()] - Fatal Error: Can't remove folder -> " << imagesPath;
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[PapagayoTool::removeCurrentLipSync()] - Warning: folder doesn't exists -> " << imagesPath;
        #endif
    }

    QString audioPath = project->getDataDir() + "/audio/" + lipsyncName;
    QDir audioDir(audioPath);
    if (QFile::exists(audioPath)) {
        if (!audioDir.removeRecursively()) {
            #ifdef TUP_DEBUG
                qDebug() << "[PapagayoTool::removeCurrentLipSync()] - Fatal Error: Can't remove folder -> " << audioPath;
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[PapagayoTool::removeCurrentLipSync()] - Warning: folder doesn't exists -> " << audioPath;
        #endif
    }
}

void PapagayoTool::updateMouthTransformation(const QDomElement &dom)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::updateMouthTransformation()]";
    #endif

    int sceneIndex = scene->currentFrameIndex();    
    currentLipSync->updateMouthTransformation(dom, (scene->currentFrameIndex() - currentLipSync->getInitFrame()));

    TupProjectRequest request = TupRequestBuilder::createLayerRequest(sceneIndex, scene->currentLayerIndex(),
                                                                      TupProjectRequest::UpdateLipSync, currentLipSync->toString());
    emit requested(&request);

    configPanel->setTransformations(dom);
}

void PapagayoTool::addNodesManager()
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::addNodesManager()]";
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

    removeNodesManager();
    setNodesManagerEnvironment();
}

void PapagayoTool::setNodesManagerEnvironment()
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::setNodesManagerEnvironment()]";
    #endif

    QGraphicsView *view = scene->views().at(0);
    foreach (QGraphicsItem *item, view->scene()->items()) {
        QString tip = item->toolTip();
        if (tip.length() > 0) {
            QString mouthID = "lipsync:" + currentLipSync->getLipSyncName();
            if (tip.compare(mouthID) == 0) {
                mouth = item;
                mouth->setFlag(QGraphicsItem::ItemIsSelectable, true);
                mouth->setSelected(true);
                break;
            }
        }
    }

    if (mouth) {
        nodesManager = new NodeManager(Node::Papagayo, mouth, scene, nodeZValue);
        connect(nodesManager, &NodeManager::positionUpdated, this, &PapagayoTool::updatePositionRecord);
        connect(nodesManager, &NodeManager::rotationUpdated, this, &PapagayoTool::updateRotationAngleRecord);
        connect(nodesManager, &NodeManager::scaleUpdated, this, &PapagayoTool::updateScaleFactorRecord);
        nodesManager->show();
        nodesManager->resizeNodes(realFactor);
        managerIncluded = true;

        TupVoice *voice = currentLipSync->getVoice();
        if (voice) {
            int index = scene->currentFrameIndex() - currentLipSync->getInitFrame();
            TupPhoneme *phoneme = voice->getPhonemeAt(index);
            if (phoneme) {
                configPanel->setPhoneme(phoneme);
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[PapagayoTool::setNodesManagerEnvironment()] - No lipsync phoneme at frame "
                             << scene->currentFrameIndex() << " - index: " << index;
                #endif
            }            
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[PapagayoTool::setNodesManagerEnvironment()] - No lipsync mouth was found in the scene!";
        #endif
    }
}

void PapagayoTool::removeNodesManager()
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::removeNodesManager()]";
    #endif

    if (managerIncluded) {
        if (nodesManager) {
            if (nodesManager->parentItem())
                nodesManager->parentItem()->setSelected(false);
            nodesManager->clear();
            nodesManager = nullptr;
            managerIncluded = false;
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
        return;
    }

    if (event->getAction() == TupProjectRequest::UpdateLipSync) {
        if (mode == TupToolPlugin::Edit)
            setNodesManagerEnvironment();
        return;
    }

    if (event->getAction() == TupProjectRequest::RemoveLipSync) {
        QString name = event->getArg().toString();
        configPanel->removeLipSyncRecord(name);
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
            int lastFrame = currentLipSync->getInitFrame() + currentLipSync->getFramesTotal() - 1;
            if (frameIndex >= currentLipSync->getInitFrame() && frameIndex <= lastFrame)
                setNodesManagerEnvironment();
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

    removeNodesManager();
    currentLipSync->setInitFrame(index);

    TupScene *sceneData = scene->currentScene();

    int sceneFrames = sceneData->framesCount();
    int lipSyncFrames = index + currentLipSync->getFramesTotal();
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

            if (event->key() == Qt::Key_Left)
                mouth->moveBy(-delta, 0);

            if (event->key() == Qt::Key_Up)
                mouth->moveBy(0, -delta);

            if (event->key() == Qt::Key_Right)
                mouth->moveBy(delta, 0);

            if (event->key() == Qt::Key_Down)
                mouth->moveBy(0, delta);

            int x = mouth->pos().x() + (mouth->boundingRect().width() / 2);
            int y = mouth->pos().y() + (mouth->boundingRect().height() / 2);

            updatePositionRecord(QPointF(x, y));
        } else if (event->modifiers() == Qt::ControlModifier) {
            configPanel->setProportionState(true);
            key = "CONTROL";
            nodesManager->setProportion(true);
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

    if (mode == TupToolPlugin::Edit) {
        if (key.compare("CONTROL") == 0) {
            configPanel->setProportionState(false);
            key = "NONE";
            nodesManager->setProportion(false);
        }
    }
}

void PapagayoTool::updateXMouthPositionInScene(int x)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::updateXMouthPositionInScene()] - x -> " << x;
    #endif

    mouth->setPos(x, mouth->pos().y());
    if (nodesManager)
        nodesManager->syncNodesFromParent();

    // updateMouthTransformation(point);
}

void PapagayoTool::updateYMouthPositionInScene(int y)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::updateYMouthPositionInScene()] - x -> " << y;
    #endif

    mouth->setPos(mouth->pos().x(), y);
    if (nodesManager)
        nodesManager->syncNodesFromParent();

    // updateMouthTransformation(point);
}

void PapagayoTool::updateRotationInScene(int angle)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::updateRotationInScene()] - angle -> " << angle;
    #endif

    if (nodesManager) {
        nodesManager->rotate(angle);
        nodesManager->syncNodesFromParent();
    }

    // updateMouthTransformation(angle);
}

void PapagayoTool::updateScaleInScene(double xFactor, double yFactor)
{
    if (nodesManager) {
        nodesManager->scale(xFactor, yFactor);
        nodesManager->syncNodesFromParent();
    }

    // updateMouthTransformation(xFactor, yFactor);
}

void PapagayoTool::resizeNode(qreal scaleFactor)
{
    realFactor = scaleFactor;
    if (nodesManager)
        nodesManager->resizeNodes(scaleFactor);
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
void PapagayoTool::storeTransformation(QGraphicsItem *item)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoTool::saveTransformation(QGraphicsItem *)]";
    #endif

    QDomDocument doc;
    doc.appendChild(TupSerializer::properties(item, doc, "", 0));

    currentLipSync->updateMouthTransformation(doc.documentElement(), (scene->currentFrameIndex() - currentLipSync->getInitFrame()));
}
*/

void PapagayoTool::updatePositionRecord(const QPointF &point)
{
    if (nodesManager)
        nodesManager->syncNodesFromParent();

    configPanel->updatePositionCoords(point.x(), point.y());
}

void PapagayoTool::updateRotationAngleRecord(int angle)
{
    configPanel->updateRotationAngle(angle);
}

void PapagayoTool::updateScaleFactorRecord(double x, double y)
{
    configPanel->updateScaleFactor(x, y);
}

void PapagayoTool::enableProportion(bool flag)
{
    if (mode == TupToolPlugin::Edit) {
        key = "NONE";
        if (flag)
            key = "CONTROL";

        if (nodesManager)
            nodesManager->setProportion(flag);
    }
}

void PapagayoTool::resetMouthTransformations()
{
    QSizeF projectSize = scene->currentScene()->getDimension();
    int projectX = projectSize.width() / 2;
    int projectY = projectSize.height() / 2;
    QSizeF mouthSize = mouth->boundingRect().size();
    int mouthX = mouthSize.width() / 2;
    int mouthY = mouthSize.height() / 2;

    updateXMouthPositionInScene(projectX - mouthX);
    updateYMouthPositionInScene(projectY - mouthY);
    updateRotationInScene(0);
    updateScaleInScene(1, 1);
}
