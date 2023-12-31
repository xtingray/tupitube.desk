/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez                                                     *
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

#include "tupexposuresheet.h"
#include "tupscenenamedialog.h"
#include "talgorithm.h"
#include "tuivalues.h"

#include <QHBoxLayout>
#include <QButtonGroup>
#include <QGroupBox>
#include <QPushButton>
#include <QActionGroup>
#include <QPixmap>

TupExposureSheet::TupExposureSheet(QWidget *parent, TupProject *work) : TupModuleWidgetBase(parent, "Exposure Sheet")
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet()]";
    #endif

    project = work;
    currentTable = nullptr;
    localRequest = false;
    previousScene = 0;
    previousLayer = 0;
    // movingTab = false;

    setWindowTitle(tr("Exposure Sheet"));
    setWindowIcon(QPixmap(ICONS_DIR + "exposure_sheet.png"));

    QList<TupProjectActionBar::Action> generalActions;
    generalActions << TupProjectActionBar::InsertLayer << TupProjectActionBar::RemoveLayer;
    generalActions << TupProjectActionBar::Separator;
    generalActions << TupProjectActionBar::InsertScene << TupProjectActionBar::RemoveScene;

    topActionBar = new TupProjectActionBar(QString("Exposure"), generalActions);

    connect(topActionBar, SIGNAL(actionSelected(int)), this, SLOT(applyAction(int)));
    addChild(topActionBar, Qt::AlignHCenter);

    QList<TupProjectActionBar::Action> frameActions;
    frameActions << TupProjectActionBar::InsertFrame
                 << TupProjectActionBar::DuplicateFrame
                 << TupProjectActionBar::RemoveFrame;

    frameActions << TupProjectActionBar::Separator;

    frameActions << TupProjectActionBar::MoveFrameBackward
                 << TupProjectActionBar::MoveFrameForward
                 << TupProjectActionBar::ReverseFrameSelection;

    frameActions << TupProjectActionBar::Separator;

    frameActions << TupProjectActionBar::CopyFrame;
    frameActions << TupProjectActionBar::PasteFrame;

    bottomActionBar = new TupProjectActionBar(QString("Exposure"), frameActions);

    connect(bottomActionBar, SIGNAL(actionSelected(int)), this, SLOT(applyAction(int)));
    addChild(bottomActionBar, Qt::AlignHCenter);

    scenesContainer = new TupExposureSceneTabWidget(this);
    connect(scenesContainer, SIGNAL(currentChanged(int)), this, SLOT(requestChangeScene(int)));
    connect(scenesContainer, SIGNAL(layerOpacityChanged(double)), this, SLOT(requestUpdateLayerOpacity(double)));
    connect(scenesContainer, SIGNAL(sceneRenameRequested(int)), this, SLOT(showRenameSceneDialog(int)));
    connect(scenesContainer, SIGNAL(sceneMoved(int,int)), this, SLOT(requestSceneMove(int,int)));

    addChild(scenesContainer);
    createMenuForAFrame();
    // createMenuForSelection();

    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    if (screenWidth > HD_WIDTH)
        setMinimumWidth(FIT_EXPOSURESHEET_WIDTH(screenWidth));
}

TupExposureSheet::~TupExposureSheet()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupExposureSheet()]";
    #endif

    delete project;
    delete scenesContainer;
    delete currentTable;
    delete topActionBar;
    delete bottomActionBar;
    delete singleMenu;

    framesList.clear();
    statesList.clear();
}

void TupExposureSheet::createMenuForAFrame()
{
    singleMenu = new QMenu(tr("actions"));
    QMenu *insertMenu = new QMenu(tr("Insert"));

    QAction *insertOne = new QAction(QIcon(THEME_DIR + "icons/add_frame.png"), tr("1 frame"), this); 
    insertOne->setIconVisibleInMenu(true);
    insertMenu->addAction(insertOne);

    QAction *insertFive = new QAction(QIcon(THEME_DIR + "icons/add_frame.png"), tr("5 frames"), this);                        
    insertFive->setIconVisibleInMenu(true);
    insertMenu->addAction(insertFive);

    QAction *insertTen = new QAction(QIcon(THEME_DIR + "icons/add_frame.png"), tr("10 frames"), this);
    insertTen->setIconVisibleInMenu(true);
    insertMenu->addAction(insertTen);

    QAction *insertTwenty = new QAction(QIcon(THEME_DIR + "icons/add_frame.png"), tr("20 frames"), this);
    insertTwenty->setIconVisibleInMenu(true);
    insertMenu->addAction(insertTwenty);

    QAction *insertFifty = new QAction(QIcon(THEME_DIR + "icons/add_frame.png"), tr("50 frames"), this);
    insertFifty->setIconVisibleInMenu(true);
    insertMenu->addAction(insertFifty);

    QAction *insertHundred = new QAction(QIcon(THEME_DIR + "icons/add_frame.png"), tr("100 frames"), this);
    insertHundred->setIconVisibleInMenu(true);
    insertMenu->addAction(insertHundred);

    connect(insertMenu, SIGNAL(triggered(QAction*)), this, SLOT(insertFramesFromMenu(QAction*)));

    singleMenu->addMenu(insertMenu);

    QAction *removeAction = new QAction(QIcon(THEME_DIR + "icons/remove_frame.png"), tr("Remove frame"), this);
    removeAction->setIconVisibleInMenu(true);
    singleMenu->addAction(removeAction);
    connect(removeAction, SIGNAL(triggered()), this, SLOT(removeFrame()));

    QAction *clearAction = new QAction(QIcon(THEME_DIR + "icons/new.png"), tr("Clear frame"), this);
    clearAction->setIconVisibleInMenu(true);
    singleMenu->addAction(clearAction);
    connect(clearAction, SIGNAL(triggered()), this, SLOT(clearFrame()));

    QAction *copyAction = new QAction(QIcon(THEME_DIR + "icons/copy.png"), tr("Copy frame"), this);
    copyAction->setIconVisibleInMenu(true);
    singleMenu->addAction(copyAction);
    connect(copyAction, SIGNAL(triggered()), this, SLOT(requestCopyFrameSelection()));

    QAction *pasteAction = new QAction(QIcon(THEME_DIR + "icons/paste.png"), tr("Paste frame"), this);
    pasteAction->setIconVisibleInMenu(true);
    singleMenu->addAction(pasteAction);
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(requestPasteSelectionInCurrentFrame()));

    /* SQA: Feature temporarily disabled
    QMenu *timeLineMenu = new QMenu(tr("Copy TL forward"));

    QAction *copyTLOnce = new QAction(QIcon(THEME_DIR + "icons/copy.png"), tr("1 time"), this);
    copyTLOnce->setIconVisibleInMenu(true);
    timeLineMenu->addAction(copyTLOnce);

    QAction *copyTLTwice = new QAction(QIcon(THEME_DIR + "icons/copy.png"), tr("2 times"), this);
    copyTLTwice->setIconVisibleInMenu(true);
    timeLineMenu->addAction(copyTLTwice);

    QAction *copyTLThird = new QAction(QIcon(THEME_DIR + "icons/copy.png"), tr("3 times"), this);
    copyTLThird->setIconVisibleInMenu(true);
    timeLineMenu->addAction(copyTLThird);

    QAction *copyTLFourth = new QAction(QIcon(THEME_DIR + "icons/copy.png"), tr("4 times"), this);
    copyTLFourth->setIconVisibleInMenu(true);
    timeLineMenu->addAction(copyTLFourth);

    QAction *copyTLFifth = new QAction(QIcon(THEME_DIR + "icons/copy.png"), tr("5 times"), this);
    copyTLFifth->setIconVisibleInMenu(true);
    timeLineMenu->addAction(copyTLFifth);
    connect(timeLineMenu, SIGNAL(triggered(QAction *)), this, SLOT(copyTimeLineFromMenu(QAction*)));

    singleMenu->addMenu(timeLineMenu);
    */

    connect(singleMenu, SIGNAL(triggered(QAction *)), this, SLOT(actionTriggered(QAction*)));
}

/* SQA:: Check if it's possible to use a pop up menu for multiple frames selection
void TupExposureSheet::createMenuForSelection()
{
    multipleMenu = new QMenu(tr("actions"));

    QAction *removeAction = new QAction(QIcon(THEME_DIR + "icons/remove_frame.png"), tr("Remove frames"), this);
    removeAction->setIconVisibleInMenu(true);
    multipleMenu->addAction(removeAction);
    connect(removeAction, SIGNAL(triggered()), this, SLOT(removeFrame()));
}
*/

void TupExposureSheet::addScene(int sceneIndex, const QString &name)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::addScene()] - index -> "
                 << sceneIndex << " - name -> " << name;
    #endif

    TupScene *tupScene = project->sceneAt(sceneIndex);
    int fps = tupScene->getFPS();
    TupExposureTable *scene = new TupExposureTable(fps);
    scene->setSinglePopUpMenu(singleMenu);

    connect(scene, SIGNAL(frameUsed(int,int)), this, SLOT(insertFrame(int,int)));
    connect(scene, SIGNAL(frameRenamed(int, int, const QString &)), this, SLOT(renameFrame(int, int, const QString &)));
    connect(scene, SIGNAL(frameSelected(int,int)), SLOT(selectFrame(int,int)));
    connect(scene, SIGNAL(selectionCopied()), SLOT(requestCopyFrameSelection())); 
    connect(scene, SIGNAL(selectionPasted()), SLOT(requestPasteSelectionInCurrentFrame()));
    connect(scene, SIGNAL(selectionRemoved()), SLOT(removeFrame()));
    connect(scene, SIGNAL(frameExtended(int,int)), SLOT(extendFrameForward(int,int)));

    connect(scene, SIGNAL(layerNameChanged(int, const QString &)), this, SLOT(requestRenameLayer(int, const QString &)));
    connect(scene, SIGNAL(layerMoved(int,int)), this, SLOT(moveLayer(int,int)));
    connect(scene, SIGNAL(layerVisibilityChanged(int,bool)), this, SLOT(changeLayerVisibility(int,bool)));
    connect(scene, SIGNAL(newPerspective(int)), this, SIGNAL(newPerspective(int)));

    scenesContainer->addScene(sceneIndex, name, scene);
}

void TupExposureSheet::renameScene(int sceneIndex, const QString &name)
{
    scenesContainer->renameScene(sceneIndex, name);
}

void TupExposureSheet::applyAction(int action)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::applyAction()] - action -> " << action;
    #endif

    currentTable = scenesContainer->getCurrentTable();
    if (!currentTable) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupExposureSheet::applyAction()] - No layer view!";
        #endif

        return;
    }

    // SQA: Refactor scene, layer and frame index variables. Set them before the switch
    // i.e. int sceneIndex = scenesContainer->currentIndex();

    switch (action) {
        case TupProjectActionBar::InsertFrame:
        {
            insertFrames(1);
        }
        break;

        case TupProjectActionBar::DuplicateFrame:
        {
            extendFrameForward(currentTable->currentLayer(), currentTable->currentFrame());
        }
        break;

        case TupProjectActionBar::RemoveFrame:
        {
            localRequest = true;
            QList<int> coords = currentTable->currentSelection();

            if (coords.count() == 4) {
                int scene = scenesContainer->currentIndex();
                int layers = coords.at(1) - coords.at(0) + 1;
                int frames = coords.at(3) - coords.at(2) + 1;

                QString flags = "";
                for (int i=coords.at(0); i<=coords.at(1); i++) {
                    if (currentTable->framesCountAtLayer(i) > frames)
                        flags += "0,";
                    else
                        flags += "1,";
                }
                flags.chop(1);

                QString selection = QString::number(layers) + "," + QString::number(frames) + ":" + flags;
                TupProjectRequest request = TupRequestBuilder::createFrameRequest(scene, coords.at(0), coords.at(2),
                                                                                  TupProjectRequest::RemoveSelection, selection);
                emit requestTriggered(&request);
            }
        }
        break;

        case TupProjectActionBar::MoveFrameBackward:
            {
                int frameIndex = currentTable->currentFrame();
                if (frameIndex > 0) {
                    TupProjectRequest request = TupRequestBuilder::createFrameRequest(scenesContainer->currentIndex(),
                                                currentTable->currentLayer(),  frameIndex,
                                                TupProjectRequest::Exchange, currentTable->currentFrame()-1);
                    emit requestTriggered(&request);
                 }
            }
        break;

        case TupProjectActionBar::MoveFrameForward:
            {
                int origin = currentTable->currentFrame();
                int destination = currentTable->currentFrame() + 1;
                if (destination == currentTable->framesCountAtCurrentLayer())
                    insertFrames(1);

                TupProjectRequest request = TupRequestBuilder::createFrameRequest(scenesContainer->currentIndex(),
                                            currentTable->currentLayer(), origin,
                                            TupProjectRequest::Exchange, destination);
                emit requestTriggered(&request);
            }
        break;

        case TupProjectActionBar::ReverseFrameSelection:
            {
                QList<int> coords = currentTable->currentSelection();
                if (coords.count() == 4) {
                    if (coords.at(1) != coords.at(3)) {
                        QString selection = QString::number(coords.at(0)) + "," + QString::number(coords.at(1)) + ","
                                            + QString::number(coords.at(2)) + "," + QString::number(coords.at(3));
                        TupProjectRequest request = TupRequestBuilder::createFrameRequest(scenesContainer->currentIndex(),
                                                                       currentTable->currentLayer(),
                                                                       currentTable->currentFrame(),
                                                                       TupProjectRequest::ReverseSelection, selection);
                        emit requestTriggered(&request);
                    } else {
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupExposureSheet::applyAction()] - Selection must include at least 2 frames of the same layer";
                        #endif
                    }
                }
            }
        break;

        case TupProjectActionBar::CopyFrame:
            {
                requestCopyFrameSelection();
            }
        break;

        case TupProjectActionBar::PasteFrame:
            {
                requestPasteSelectionInCurrentFrame();
            }
        break;

        case TupProjectActionBar::InsertLayer:
            {
                int layer = currentTable->columnCount();
                TupProjectRequest request = TupRequestBuilder::createLayerRequest(scenesContainer->currentIndex(),
                                                                                   layer, TupProjectRequest::Add, tr("Layer %1").arg(layer + 1));
                emit requestTriggered(&request);

                int framesNum = currentTable->usedFrames(currentTable->currentColumn());
                for (int i=0;i < framesNum;i++) {
                     request = TupRequestBuilder::createFrameRequest(scenesContainer->currentIndex(), layer, i,
                                                                     TupProjectRequest::Add, tr("Frame"));
                     emit requestTriggered(&request);
                }
            }
        break;

        case TupProjectActionBar::RemoveLayer:
            {
                TupProjectRequest request = TupRequestBuilder::createLayerRequest(scenesContainer->currentIndex(),
                                                                                  currentTable->currentLayer(),
                                                                                  TupProjectRequest::Remove);
                emit requestTriggered(&request);
            }
        break;

        case TupProjectActionBar::InsertScene:
            {
                int sceneTarget = scenesContainer->count();
                QString sceneName = tr("Scene %1").arg(sceneTarget + 1);
                TupSceneNameDialog *dialog = new TupSceneNameDialog(TupSceneNameDialog::Add, sceneName);
                if (dialog->exec() == QDialog::Accepted) {
                    sceneName = dialog->getSceneName();
                    TupProjectRequest request = TupRequestBuilder::createSceneRequest(sceneTarget, TupProjectRequest::Add, sceneName);
                    emit requestTriggered(&request);

                    request = TupRequestBuilder::createLayerRequest(sceneTarget, 0, TupProjectRequest::Add, tr("Layer 1"));
                    emit requestTriggered(&request);

                    request = TupRequestBuilder::createFrameRequest(sceneTarget, 0, 0, TupProjectRequest::Add, tr("Frame"));
                    emit requestTriggered(&request);

                    request = TupRequestBuilder::createSceneRequest(sceneTarget, TupProjectRequest::Select);
                    emit requestTriggered(&request);
                }
            }
        break;

        case TupProjectActionBar::RemoveScene:
            {
                int scenesTotal = scenesContainer->count();
                int sceneIndex = scenesContainer->currentIndex();

                TupProjectRequest request;
                if (scenesTotal > 1) {
                    request = TupRequestBuilder::createSceneRequest(sceneIndex, TupProjectRequest::Remove);
                    emit requestTriggered(&request);

                    QString selection = "0,0,0,0";
                    request = TupRequestBuilder::createFrameRequest(sceneIndex - 1, 0, 0, TupProjectRequest::Select, selection);
                    emit requestTriggered(&request);
                } else {
                    request = TupRequestBuilder::createSceneRequest(sceneIndex, TupProjectRequest::Reset, tr("Scene 1"));
                    emit requestTriggered(&request);
                }
            }
        break;
    }
}

void TupExposureSheet::setScene(int sceneIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::setScene()]";
    #endif

    if (scenesContainer->isTableIndexValid(sceneIndex)) {
        scenesContainer->blockSignals(true);
        scenesContainer->setCurrentIndex(sceneIndex);
        currentTable = scenesContainer->getTable(sceneIndex);
        scenesContainer->blockSignals(false);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupExposureSheet::setScene()] - Invalid scene index ->" << sceneIndex;
            qDebug() << "[TupExposureSheet::setScene()] - Scenes total ->" << scenesContainer->count();
        #endif
    }
}

void TupExposureSheet::requestChangeScene(int sceneIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::requestChangeScene()] - sceneIndex ->" << sceneIndex;
    #endif

    // if (!movingTab) {
        if (scenesContainer->count() > 1) {
            TupProjectRequest request = TupRequestBuilder::createSceneRequest(sceneIndex, TupProjectRequest::Select);
            emit localRequestTriggered(&request);
            emit sceneChanged(sceneIndex);
        }
    // }
}

void TupExposureSheet::requestSceneMove(int from, int to)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::requestSceneMove()] - from ->" << from;
        qDebug() << "[TupExposureSheet::requestSceneMove()] - to ->" << to;
    #endif

    // movingTab = true;
    if (scenesContainer->count() > 1) {
        qDebug() << "[TupExposureSheet::requestSceneMove()] - Calling move request...";
        TupProjectRequest request = TupRequestBuilder::createSceneRequest(to, TupProjectRequest::Move, from);
        emit requestTriggered(&request);
    } else {
        qDebug() << "[TupExposureSheet::requestSceneMove()] - Fatal Error: Can't request scene move. "
                    "Scenes container size ->" << scenesContainer->count();
    }
}

void TupExposureSheet::requestCopyFrameSelection()
{
    QList<int> coords = currentTable->currentSelection();
    if (coords.count() == 4) {
        QString selection = QString::number(coords.at(0)) + "," + QString::number(coords.at(1)) + ","
                            + QString::number(coords.at(2)) + "," + QString::number(coords.at(3));

        TupProjectRequest request = TupRequestBuilder::createFrameRequest(scenesContainer->currentIndex(),
                                                       currentTable->currentLayer(),
                                                       currentTable->currentFrame(),
                                                       TupProjectRequest::CopySelection, selection);
        emit requestTriggered(&request);
    }
}

void TupExposureSheet::requestPasteSelectionInCurrentFrame()
{
    if (!framesList.isEmpty()) {
        TupProjectRequest request = TupRequestBuilder::createFrameRequest(scenesContainer->currentIndex(),
                                                       currentTable->currentLayer(),
                                                       currentTable->currentFrame(),
                                                       TupProjectRequest::PasteSelection);
        emit requestTriggered(&request);
    }
}

void TupExposureSheet::requestExtendCurrentFrame(int times)
{
    TupProjectRequest request = TupRequestBuilder::createFrameRequest(scenesContainer->currentIndex(),
                                                   currentTable->currentLayer(),
                                                   currentTable->currentFrame(),
                                                   TupProjectRequest::Extend, times);
    emit requestTriggered(&request);
}

void TupExposureSheet::insertFrame(int layerIndex, int frameIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::insertFrame()] - frameIndex -> " << frameIndex << " - layerIndex -> " << layerIndex;
    #endif

    TupProjectRequest request = TupRequestBuilder::createFrameRequest(scenesContainer->currentIndex(),
                                                   layerIndex, frameIndex, TupProjectRequest::Add, tr("Frame"));
    emit requestTriggered(&request);
}

void TupExposureSheet::renameFrame(int layerIndex, int frameIndex, const QString &name)
{
    TupProjectRequest request = TupRequestBuilder::createFrameRequest(scenesContainer->currentIndex(), layerIndex, frameIndex,
                                                   TupProjectRequest::Rename, name);
    emit requestTriggered(&request);
}

void TupExposureSheet::selectFrame(int layerIndex, int frameIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::selectFrame()] - layerIndex, frameIndex -> " << layerIndex << "," << frameIndex;
    #endif

    // QList<QTableWidgetItem *> list = currentTable->selectedItems();
    QString selection = "";
    QList<int> coords = currentTable->currentSelection();

    if (coords.count() == 4) {
        if ((layerIndex >= coords.at(0) && layerIndex <= coords.at(1)) && 
            (frameIndex >= coords.at(2) && frameIndex <= coords.at(3))) {
            selection = QString::number(coords.at(0)) + "," + QString::number(coords.at(1)) + "," 
                        + QString::number(coords.at(2)) + "," + QString::number(coords.at(3));
        } else {
            selection = QString::number(layerIndex) + "," + QString::number(layerIndex) + "," 
                        + QString::number(frameIndex) + "," + QString::number(frameIndex);  
        }
    } else {
        selection = QString::number(layerIndex) + "," + QString::number(layerIndex) + ","
                    + QString::number(frameIndex) + "," + QString::number(frameIndex);
    }

    TupProjectRequest request = TupRequestBuilder::createFrameRequest(scenesContainer->currentIndex(), layerIndex,
                                                   frameIndex, TupProjectRequest::Select, selection);
    emit localRequestTriggered(&request);
}

void TupExposureSheet::removeFrame()
{
    topActionBar->emitActionSelected(TupProjectActionBar::RemoveFrame);
}

void TupExposureSheet::extendFrameForward(int layerIndex, int frameIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::extendFrameForward()]";
    #endif

    int sceneIndex = scenesContainer->currentIndex();
    TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, frameIndex,
                                                                      TupProjectRequest::Extend, 1);
    emit requestTriggered(&request);
}

void TupExposureSheet::changeLayerVisibility(int layerIndex, bool visibility)
{
    TupProjectRequest request = TupRequestBuilder::createLayerRequest(scenesContainer->currentIndex(), layerIndex,
                                                   TupProjectRequest::View, visibility);
    emit localRequestTriggered(&request);
}

void TupExposureSheet::requestRenameLayer(int layerIndex, const QString & name)
{
    TupProjectRequest request = TupRequestBuilder::createLayerRequest(scenesContainer->currentIndex(), layerIndex,
                                                   TupProjectRequest::Rename, name);
    emit requestTriggered(&request);
}

void TupExposureSheet::moveLayer(int oldIndex, int newIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::moveLayer()] - oldIndex -> " << oldIndex << " - newIndex -> " << newIndex;
    #endif

    TupProjectRequest request = TupRequestBuilder::createLayerRequest(scenesContainer->currentIndex(), oldIndex,
                                                   TupProjectRequest::Move, newIndex);
    emit requestTriggered(&request);
}

void TupExposureSheet::actionTriggered(QAction *action)
{
    bool ok;
    int id = action->data().toInt(&ok);

    if (ok)
        applyAction(id);
}

void TupExposureSheet::closeAllScenes()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::closeAllScenes()]";
    #endif

    blockSignals(true);
    currentTable = nullptr;
    scenesContainer->removeAllTabs();
    blockSignals(false);
}

void TupExposureSheet::sceneResponse(TupSceneResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::sceneResponse()] - action -> " << response->getAction();
    #endif

    int sceneIndex = response->getSceneIndex();
    switch(response->getAction()) {
        case TupProjectRequest::Add:
            {
                if (response->getMode() == TupProjectResponse::Do) {
                    addScene(sceneIndex, response->getArg().toString());
                    return;
                }

                if (response->getMode() == TupProjectResponse::Redo || response->getMode() == TupProjectResponse::Undo) {
                    TupScene *scene = project->sceneAt(sceneIndex);
                    if (scene)
                        scenesContainer->restoreScene(sceneIndex, scene->getSceneName());
                    return;
                }
            }
        break;
        case TupProjectRequest::Remove:
            {
                scenesContainer->removeScene(sceneIndex, true);
            }
        break;
        case TupProjectRequest::Reset:
            {
                if (response->getMode() == TupProjectResponse::Do || response->getMode() == TupProjectResponse::Redo) {
                    scenesContainer->removeScene(sceneIndex, true);
                    addScene(sceneIndex, tr("Scene %1").arg(sceneIndex + 1));

                    setScene(sceneIndex);
                    currentTable->insertLayer(0, tr("Layer 1"));
                    currentTable->insertFrame(0, 0, tr("Frame"), false);

                    currentTable->blockSignals(true);
                    currentTable->selectFrame(0, 0);
                    currentTable->blockSignals(false);
                }

                if (response->getMode() == TupProjectResponse::Undo) {
                    scenesContainer->removeScene(sceneIndex, false);
                    scenesContainer->restoreScene(sceneIndex, response->getArg().toString());

                    setScene(sceneIndex);

                    currentTable->blockSignals(true);
                    currentTable->selectFrame(0, 0);
                    currentTable->blockSignals(false);
                }
            }
        break;
        case TupProjectRequest::Move:
            {
                 qDebug() << "[TupExposureSheet::sceneResponse()] - Tracing move action...";
                 // movingTab = false;
                 scenesContainer->moveScene(sceneIndex, response->getArg().toInt());
            }
        break;
        /*
        case TupProjectRequest::Lock:
            {
            }
        break;
           */
        case TupProjectRequest::Rename:
            {
                renameScene(sceneIndex, response->getArg().toString());
            }
        break;
        case TupProjectRequest::Select:
            {
                setScene(sceneIndex);
                if (currentTable && scenesContainer) {
                    scenesContainer->blockSignals(true);
                    currentTable->selectFrame(0, 0);
                    scenesContainer->blockSignals(false);
                    if (previousScene != sceneIndex) {
                        previousScene = sceneIndex;
                        previousLayer = 0;
                        updateLayerOpacity(sceneIndex, 0);
                    }
                }
            }
        break;
    }
}

void TupExposureSheet::layerResponse(TupLayerResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::layerResponse()] - action -> " << response->getAction();
    #endif

    int sceneIndex = response->getSceneIndex();
    TupExposureTable *framesTable = scenesContainer->getTable(sceneIndex);

    if (framesTable) {
        int layerIndex = response->getLayerIndex();
        switch (response->getAction()) {
            case TupProjectRequest::Add:
                {
                    if (response->getMode() == TupProjectResponse::Do) {
                        framesTable->insertLayer(layerIndex, response->getArg().toString());
                        return;
                    }

                    if (response->getMode() == TupProjectResponse::Redo || response->getMode() == TupProjectResponse::Undo) {
                        TupScene *scene = project->sceneAt(sceneIndex);
                        if (scene) {
                            TupLayer *layer = scene->layerAt(layerIndex);
                            if (layer) {
                                framesTable->insertLayer(layerIndex, layer->getLayerName());
                                QList<TupFrame *> frames = layer->getFrames();
                                int total = frames.count();
                                for(int i=0; i<total; i++) {
                                    TupFrame *frame = frames.at(i);
                                    framesTable->insertFrame(layerIndex, i, frame->getFrameName(), response->external());
                                    if (!frame->isEmpty())
                                        framesTable->updateFrameState(layerIndex, i, TupExposureTable::Used);
                                }
                            }
                        }

                        return;
                    }
                }
            break;
            case TupProjectRequest::Remove:
                {
                    framesTable->removeLayer(layerIndex);
                    TupScene *scene = project->sceneAt(sceneIndex);
                    if (scene) {
                        int index = layerIndex;
                        if (layerIndex == scene->layersCount())
                            index--;
                        updateLayerOpacity(sceneIndex, index);
                    }
                }
            break;
            case TupProjectRequest::Move:
                {
                    framesTable->moveLayer(layerIndex, response->getArg().toInt());
                }
            break;
            case TupProjectRequest::Rename:
                {
                    framesTable->setLayerName(layerIndex, response->getArg().toString());
                }
            break;
            /*
            case TupProjectRequest::Lock:
                {
                    framesTable->setLockLayer(layerIndex, response->arg().toBool());
                }
            */
            case TupProjectRequest::Select:
                {
                    setScene(sceneIndex);
                    framesTable->blockSignals(true);
                    framesTable->selectFrame(layerIndex, 0);
                    framesTable->blockSignals(false);

                    if ((previousScene != sceneIndex) || (previousLayer != layerIndex)) {
                        previousScene = sceneIndex;
                        previousLayer = layerIndex;
                        updateLayerOpacity(sceneIndex, layerIndex);
                    }
                }
            break;
            case TupProjectRequest::View:
                {
                    framesTable->setLayerVisibility(layerIndex, response->getArg().toBool());
                }
            break;
            case TupProjectRequest::UpdateOpacity:
                {
                    updateLayerOpacity(sceneIndex, layerIndex);
                    if (response->getMode() == TupProjectResponse::Undo || response->getMode() == TupProjectResponse::Redo) {
                        QString layer = QString::number(layerIndex);
                        framesTable->selectFrame(layerIndex, 0, layer + "," + layer + ",0,0");
                    }
                }
            break;
            case TupProjectRequest::RemoveLipSync:
                {
                    TupScene *scene = project->sceneAt(sceneIndex);
                    if (scene) {
                        TupLayer *layer = scene->layerAt(layerIndex);
                        if (layer) {
                            int total = layer->framesCount();
                            for (int i=0; i<total; i++) {
                                 TupFrame *frame = layer->frameAt(i);
                                 if (frame->isEmpty()) 
                                     currentTable->updateFrameState(layerIndex, i, TupExposureTable::Empty);
                            }
                        }
                    }
                }
            break;
            default:
                #ifdef TUP_DEBUG
                    qDebug() << "[TupExposureSheet::layerResponse()] - Layer option undefined! -> "
                             << response->getAction();
                #endif
            break;
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupExposureSheet::layerResponse()] - Scene index invalid -> "
                     << sceneIndex;
        #endif
    }
}

void TupExposureSheet::frameResponse(TupFrameResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::frameResponse()] - action -> " << response->getAction();
    #endif

    int sceneIndex = response->getSceneIndex();
    TupExposureTable *table = scenesContainer->getTable(sceneIndex);

    if (table) {
        int layerIndex = response->getLayerIndex();
        int frameIndex = response->getFrameIndex();
        switch (response->getAction()) {
                case TupProjectRequest::Add:
                  {
                      if (response->getMode() == TupProjectResponse::Do) {
                          table->insertFrame(layerIndex, frameIndex, response->getArg().toString(), response->external());
                          if (layerIndex == 0 && frameIndex == 0) {
                              setScene(sceneIndex);
                              table->blockSignals(true);
                              table->selectFrame(0, 0);
                              table->blockSignals(false);
                          }

                          return;
                      }

                      if (response->getMode() == TupProjectResponse::Redo || response->getMode() == TupProjectResponse::Undo) {
                          TupScene *scene = project->sceneAt(sceneIndex);
                          if (scene) {
                              TupLayer *layer = scene->layerAt(layerIndex);
                              if (layer) {
                                  TupFrame *frame = layer->frameAt(frameIndex);
                                  if (frame) {
                                      table->insertFrame(layerIndex, frameIndex, frame->getFrameName(), response->external());
                                      if (!frame->isEmpty())
                                          table->updateFrameState(layerIndex, frameIndex, TupExposureTable::Used);
                                  }
                              }
                          }

                          return;
                      }
                  }
                break;
                case TupProjectRequest::Remove:
                  {
                     if (response->getMode() == TupProjectResponse::Do) {
                         if (localRequest) {
                             localRequest = false;
                             table->removeFrame(layerIndex, frameIndex);
                         } else {
                             int lastFrame = table->framesCountAtCurrentLayer() - 1;
                             int target = frameIndex;
                             if (target == lastFrame) { // Removing last frame from layer
                                 table->removeFrame(layerIndex, target);
                                 if (target <= 0)
                                     table->clearSelection();
                             } else {
                                 // When the item deleted is not the last one
                                 TupScene *scene = project->sceneAt(sceneIndex);
                                 if (scene) {
                                     TupLayer *layer = scene->layerAt(layerIndex);
                                     if (layer) {
                                         for (int index=target+1; index <= lastFrame; index++) {
                                              TupFrame *frame = layer->frameAt(index-1);
                                              TupExposureTable::FrameType type = TupExposureTable::Empty;
                                              if (!frame->isEmpty())
                                                  type = TupExposureTable::Used;
                                              table->updateFrameState(layerIndex, index - 1, type);
                                              QString label = currentTable->frameName(layerIndex, index);
                                              renameFrame(layerIndex, index - 1, label);
                                         }
                                         table->removeFrame(layerIndex, lastFrame);
                                     }
                                 }
                             }
                         }

                         return;
                     }

                     if (response->getMode() == TupProjectResponse::Redo || response->getMode() == TupProjectResponse::Undo) {
                         int lastFrame = table->framesCountAtCurrentLayer() - 1;
                         int target = frameIndex;
                         if (target == lastFrame) { // Removing last frame
                             if (frameIndex > 0) {
                                 table->removeFrame(layerIndex, frameIndex);
                                 frameIndex--;
                             }
                             table->selectFrame(layerIndex, frameIndex);
                         } else {
                             table->removeFrame(layerIndex, frameIndex);
                         }

                         return;
                     }
                  }
                break;
                case TupProjectRequest::RemoveSelection: 
                  {
                      QString selection = response->getArg().toString();
                      QStringList blocks = selection.split(":");

                      QStringList params = blocks.at(0).split(",");
                      int layers = params.at(0).toInt();
                      int frames = params.at(1).toInt();
                      QStringList flags = blocks.at(1).split(","); 

                      if (response->getMode() == TupProjectResponse::Do || response->getMode() == TupProjectResponse::Redo) {
                          removeBlock(table, layerIndex, frameIndex, layers, frames);
                          if (frameIndex <= (table->framesCountAtLayer(layerIndex)-1))
                              table->updateSelection(layerIndex, frameIndex);
                      } else {
                          TupScene *scene = project->sceneAt(sceneIndex);
                          if (scene) {
                              int layersTotal = layerIndex + layers;
                              for (int i=layerIndex,index=0; i<layersTotal; i++,index++) {
                                  TupLayer *layer = scene->layerAt(i);
                                  if (layer) {
                                      bool remove = flags.at(index).toInt();
                                      if (remove)
                                          table->removeFrame(i, 0);
                                      int framesTotal = frameIndex + frames;
                                      for (int j=frameIndex; j<framesTotal; j++) {
                                          TupFrame *frame = layer->frameAt(j);
                                          if (frame) {
                                              table->insertFrame(i, j, tr("Frame"), response->external());
                                              if (!frame->isEmpty())
                                                  table->updateFrameState(i, j, TupExposureTable::Used);
                                          }
                                      }
                                  }
                              }

                              table->selectFrame(layerIndex, frameIndex);
                          }
                      }

                      return;
                  }
                case TupProjectRequest::Reset:
                  {
                      if (response->getMode() == TupProjectResponse::Do || response->getMode() == TupProjectResponse::Redo) {
                          table->updateFrameState(layerIndex, frameIndex, TupExposureTable::Empty);
                          table->setFrameName(layerIndex, frameIndex, tr("Frame"));
                      } else {
                          TupScene *scene = project->sceneAt(sceneIndex);
                          if (scene) {
                              TupLayer *layer = scene->layerAt(layerIndex);
                              if (layer) {
                                  TupFrame *frame = layer->frameAt(frameIndex);
                                  if (frame) {
                                      TupExposureTable::FrameType state = TupExposureTable::Used;
                                      if (frame->isEmpty())
                                          state = TupExposureTable::Empty;
                                      table->updateFrameState(layerIndex, frameIndex, state);
                                      table->setFrameName(layerIndex, frameIndex, frame->getFrameName());
                                  }
                              }
                          }
                      } 

                      return;
                  }
                case TupProjectRequest::Exchange:
                  {
                      table->exchangeFrame(layerIndex, frameIndex, layerIndex, response->getArg().toInt(), response->external());
                  }
                break;
                /*
                case TupProjectRequest::Move:
                  {
                      // No action required for this specific request
                      return;
                  }
                break;
                */
                case TupProjectRequest::Rename:
                  {
                      table->setFrameName(layerIndex, frameIndex, response->getArg().toString());
                  }
                break;
                case TupProjectRequest::Select:
                  {
                      table->blockSignals(true);
                      table->selectFrame(layerIndex, frameIndex, response->getArg().toString());
                      table->blockSignals(false);

                      table->updateSceneView(layerIndex, frameIndex);

                      if (previousScene != sceneIndex || previousLayer != layerIndex) {
                          previousScene = sceneIndex;
                          previousLayer = layerIndex;
                          updateLayerOpacity(sceneIndex, layerIndex);
                      }
                  }
                break;
                case TupProjectRequest::Extend:
                  {
                      int times = response->getArg().toInt();
                      if (response->getMode() == TupProjectResponse::Do || response->getMode() == TupProjectResponse::Redo) {
                          QString frameName = currentTable->frameName(layerIndex, frameIndex);
                          TupExposureTable::FrameType state = currentTable->frameState(layerIndex, frameIndex);

                          for (int i=1; i<=times; i++) {
                              table->insertFrame(layerIndex, frameIndex + i, frameName, response->external());
                              table->updateFrameState(layerIndex, frameIndex + i, state);
                          }

                          table->clearSelection();

                          table->blockSignals(true);
                          table->selectFrame(layerIndex, frameIndex + times);
                          table->blockSignals(false);

                          selectFrame(layerIndex, frameIndex + times);
                      } else {
                          removeBlock(table, layerIndex, frameIndex, 1, times);
                      }
                  }
                break;
                case TupProjectRequest::CopySelection:
                  {
                      if (response->getMode() == TupProjectResponse::Do) {
                          QString selection = response->getArg().toString();
                          QStringList params = selection.split(",");

                          if (params.count() == 4) {
                              QList<int> coords;
                              foreach(QString item, params) 
                                  coords << item.toInt();

                              framesList.clear();
                              statesList.clear();
                              for (int i=coords.at(0); i<=coords.at(1); i++) {
                                  for (int j=coords.at(2); j<=coords.at(3); j++) {
                                      framesList << table->frameName(i, j);
                                      statesList << table->frameState(i, j);
                                  }
                              }
                          }
                      }
                  }
                break;
                case TupProjectRequest::PasteSelection:
                  {
                      QString selection = response->getArg().toString();
                      QStringList params = selection.split(",");

                      if (params.count() == 4) {
                          QList<int> coords;
                          foreach(QString item, params) 
                              coords << item.toInt();

                          int layersTotal = (coords.at(1) - coords.at(0)) + 1;
                          int framesTotal = (coords.at(3) - coords.at(2)) + 1; 
                          int layerLimit = layerIndex + layersTotal;
                          int frameLimit = frameIndex + framesTotal;

                          if (response->getMode() == TupProjectResponse::Do || response->getMode() == TupProjectResponse::Redo) {
                              int index = 0;
                              for (int i=layerIndex; i<layerLimit; i++) {
                                  if (i < table->layersCount()) {
                                      for (int j=frameIndex; j<frameLimit; j++) {
                                          table->insertFrame(i, j, framesList.at(index), response->external());
                                          table->updateFrameState(i, j, statesList.at(index));
                                          index++;
                                      }
                                  }
                              }

                              table->clearSelection();
                              table->blockSignals(true);
                              table->selectFrame(layerIndex, frameLimit-1);
                              table->blockSignals(false);
                          } else {
                              removeBlock(table, layerIndex, frameIndex, layersTotal, framesTotal);
                          }
                      }
                  }
                break;
                case TupProjectRequest::ReverseSelection:
                  {
                      QString selection = response->getArg().toString();
                      QStringList params = selection.split(",");
                      if (params.count() == 4) {
                          int initLayer = params.at(0).toInt();
                          int endLayer = params.at(1).toInt();
                          int initFrame = params.at(2).toInt();
                          int endFrame = params.at(3).toInt();
                          int segment = endFrame - initFrame;
                          int iterations = 1;
                          if (segment > 1)
                              iterations = (segment + 1) / 2;

                          for (int i=initLayer; i<=endLayer; i++) {
                              int indexA = initFrame;
                              int indexB = endFrame;
                              for (int j=0; j<iterations; j++) {
                                  QString cellA = currentTable->frameName(i, indexA);
                                  QString cellB = currentTable->frameName(i, indexB);
                                  TupExposureTable::FrameType stateA = currentTable->frameState(i, indexA);
                                  TupExposureTable::FrameType stateB = currentTable->frameState(i, indexB);
                                  if (cellA.compare(cellB) != 0) {
                                      currentTable->setFrameName(i, indexA, cellB);
                                      currentTable->setFrameName(i, indexB, cellA);
                                  }
                                  if (stateA != stateB) {
                                      currentTable->updateFrameState(i, indexA, stateB);
                                      currentTable->updateFrameState(i, indexB, stateA);
                                  }
                                  indexA++;
                                  indexB--;
                              }
                          }
                      }
                  }
                break;
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupExposureSheet::frameResponse()] - Fatal Error: Scene index is invalid -> " << sceneIndex;
        #endif
    }
}

void TupExposureSheet::itemResponse(TupItemResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::itemResponse()] - action -> " << response->getAction();
    #endif

    switch (response->getAction()) {
        case TupProjectRequest::Add:
            {
                if (response->spaceMode() == TupProject::FRAMES_MODE && response->getItemIndex() == 0)
                    currentTable->updateFrameState(response->getLayerIndex(), response->getFrameIndex(), TupExposureTable::Used);
            }
        break;
        case TupProjectRequest::Remove:
            {
                if (response->spaceMode() == TupProject::FRAMES_MODE && response->frameIsEmpty())
                    currentTable->updateFrameState(response->getLayerIndex(), response->getFrameIndex(), TupExposureTable::Empty);
            }
        break;
        case TupProjectRequest::SetTween:
            {
                // SQA: Implement the code required to update frames state if they contain a tween
            }
        break;
        default:
        break;
    }
}

void TupExposureSheet::libraryResponse(TupLibraryResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::libraryResponse()] - action -> " << response->getAction();
        qDebug() << "[TupExposureSheet::libraryResponse()] - arg -> " << response->getArg().toString();
        qDebug() << "[TupExposureSheet::libraryResponse()] - symbol type -> " << response->symbolType();
    #endif

    if (response->symbolType() == TupLibraryObject::Folder || response->symbolType() == TupLibraryObject::Audio)
        return;

    switch (response->getAction()) {
        case TupProjectRequest::Add:
        case TupProjectRequest::InsertSymbolIntoFrame:
            {
                if (response->getSpaceMode() == TupProject::FRAMES_MODE)
                    currentTable->updateFrameState(response->getLayerIndex(), response->getFrameIndex(), TupExposureTable::Used);
            }
        break;
        case TupProjectRequest::Remove:
            {
                updateFramesState();
            }
        break;
        default:
        break;
    }
}

void TupExposureSheet::insertFrames(int times)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::insertFrames()]";
    #endif

    int scene = scenesContainer->currentIndex();
    int layer = currentTable->currentLayer();
    int target = currentTable->currentFrame() + 1;
    int lastFrame = currentTable->framesCountAtCurrentLayer() - 1;

    if (target > lastFrame) {
        for (int i=0; i<times; i++)
             insertFrame(layer, currentTable->framesCountAtCurrentLayer());
        selectFrame(layer, currentTable->currentFrame() + 1);
    } else {
        int frame = currentTable->currentFrame() + 1;
        for (int i=0; i<times; i++)
             insertFrame(layer, currentTable->framesCountAtCurrentLayer());

        for (int index=lastFrame; index >= target; index--) {
             TupProjectRequest event = TupRequestBuilder::createFrameRequest(scene, layer, index, 
                                                                             TupProjectRequest::Exchange, index + times);
             emit requestTriggered(&event);
        }
        selectFrame(layer, frame);
    }
}

void TupExposureSheet::clearFrame()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::clearFrame()]";
    #endif

    int scene = scenesContainer->currentIndex();
    int layer = currentTable->currentLayer();
    int frame = currentTable->currentFrame();

    TupProjectRequest event = TupRequestBuilder::createFrameRequest(scene, layer, frame, TupProjectRequest::Reset);
    emit requestTriggered(&event);

    currentTable->updateFrameState(layer, frame, TupExposureTable::Empty);
}

void TupExposureSheet::updateFramesState()
{
    for (int i=0; i < project->scenesCount(); i++)
        updateSceneFramesState(i);
}

void TupExposureSheet::updateSceneFramesState(int sceneIndex)
{
     TupScene *scene = project->sceneAt(sceneIndex);
     TupExposureTable *tab = scenesContainer->getTable(sceneIndex);
     for (int j=0; j < scene->layersCount(); j++) {
          TupLayer *layer = scene->layerAt(j);
          for (int k=0; k < layer->framesCount(); k++) {
               TupFrame *frame = layer->frameAt(k);
               TupExposureTable::FrameType state = TupExposureTable::Used;
               if (frame->isEmpty())
                   state = TupExposureTable::Empty;
               tab->updateFrameState(j, k, state);
          }
     }
}

void TupExposureSheet::insertFramesFromMenu(QAction *action)
{
    QString actionName = action->text();

    if (actionName.compare(tr("1 frame")) == 0) {
        insertFrames(1);
        return;
    }

    if (actionName.compare(tr("5 frames")) == 0) {
        insertFrames(5);
        return;
    }

    if (actionName.compare(tr("10 frames")) == 0) {
        insertFrames(10);
        return;
    }

    if (actionName.compare(tr("20 frames")) == 0) {
        insertFrames(20);
        return;
    }

    if (actionName.compare(tr("50 frames")) == 0) {
        insertFrames(50);
        return;
    }

    if (actionName.compare(tr("100 frames")) == 0)
        insertFrames(100);
}

void TupExposureSheet::requestUpdateLayerOpacity(double opacity)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::requestUpdateLayerOpacity()] - opacity -> " << opacity;
    #endif

    int layerIndex = currentTable->currentLayer();
    TupProjectRequest request = TupRequestBuilder::createLayerRequest(scenesContainer->currentIndex(),
                                                                      layerIndex, TupProjectRequest::UpdateOpacity, opacity);
    emit requestTriggered(&request);
}

void TupExposureSheet::updateLayerOpacity(int sceneIndex, int layerIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::updateLayerOpacity()] - sceneIndex/layerIndex -> " << sceneIndex << "," << layerIndex;
    #endif

    if (scenesContainer) {
        double opacity = getLayerOpacity(sceneIndex, layerIndex);
        #ifdef TUP_DEBUG
            qDebug() << "[TupExposureSheet::updateLayerOpacity()] - layer opacity -> " << opacity;
        #endif
        scenesContainer->setLayerOpacity(sceneIndex, opacity);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupExposureSheet::updateLayerOpacity()] - scenesContainer is NULL!";
        #endif
    }
}

double TupExposureSheet::getLayerOpacity(int sceneIndex, int layerIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::getLayerOpacity()] - sceneIndex/layerIndex -> " << sceneIndex << "," << layerIndex;
    #endif

    double opacity = 1.0;
    TupScene *scene = project->sceneAt(sceneIndex);
    if (scene) {
        TupLayer *layer = scene->layerAt(layerIndex);
        if (layer) {
            opacity = layer->getOpacity();
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupExposureSheet::getLayerOpacity()] - Fatal Error: No layer at index -> " << layerIndex;
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupExposureSheet::getLayerOpacity()] - Fatal Error: No scene at index -> " << sceneIndex;
        #endif
    }

    return opacity;
}

void TupExposureSheet::initLayerVisibility()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureSheet::initLayerVisibility()]";
    #endif

    int scenes = project->scenesCount();
    for (int sceneIndex=0; sceneIndex < scenes; sceneIndex++) {
         TupScene *scene = project->sceneAt(sceneIndex);
         if (scene) {
             int layers = scene->layersCount();
             for (int layerIndex=0; layerIndex < layers; layerIndex++) {
                  TupLayer *layer = scene->layerAt(layerIndex);
                  scenesContainer->getTable(sceneIndex)->setLayerVisibility(layerIndex, layer->isLayerVisible());
             }
         }
    }
}

void TupExposureSheet::removeBlock(TupExposureTable *table, int layerIndex, int frameIndex, 
                                   int layersTotal, int framesTotal)
{
    int layerPos = layerIndex;
    int framePos;
    for (int i=0; i<layersTotal; i++) {
        framePos = frameIndex;
        for (int j=0; j<framesTotal; j++) {
            table->removeFrame(layerPos, framePos);
            framePos++;
        }
        layerPos++;
    }

    int init = frameIndex + framesTotal;
    int lastIndex = table->framesCountAtCurrentLayer() + framesTotal;
    layerPos = layerIndex;
    for (int i=0; i<layersTotal; i++) {
        framePos = frameIndex;
        for (int j=init; j<lastIndex; j++) {
            QTableWidgetItem *item = table->takeItem(j, layerPos);
            table->setItem(framePos, layerPos, item);
            framePos++;
        }
        layerPos++;
    }

    layerPos = layerIndex;
    for (int i=0; i<layersTotal; i++) {
        if (table->framesCountAtLayer(layerPos) == 0)
            table->insertFrame(layerPos, 0, tr("Frame"), false);
        layerPos++;
    }

    lastIndex = table->framesCountAtLayer(layerIndex) - 1;
    if (lastIndex < frameIndex)
        table->selectFrame(layerIndex, lastIndex);
    else
        table->selectFrame(layerIndex, frameIndex);
}

void TupExposureSheet::updateFPS(int fps)
{
    currentTable->updateFPS(fps);
}

void TupExposureSheet::requestSceneRename(const QString &name)
{
    TupProjectRequest event = TupRequestBuilder::createSceneRequest(scenesContainer->currentIndex(),
                                                                    TupProjectRequest::Rename, name);
    emit requestTriggered(&event);
}

void TupExposureSheet::showRenameSceneDialog(int sceneIndex)
{
    TupScene *scene = project->sceneAt(sceneIndex);
    QString name = scene->getSceneName();

    TupSceneNameDialog *dialog = new TupSceneNameDialog(TupSceneNameDialog::Rename, name);
    if (dialog->exec() == QDialog::Accepted)
        requestSceneRename(dialog->getSceneName());
}
