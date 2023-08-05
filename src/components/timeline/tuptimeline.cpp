/***************************************************************************
 *   Project TUPITUBE DESK                                                 *
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

#include "tuptimeline.h"
#include "tupprojectactionbar.h"
#include "tupprojectrequest.h"

#include "tuplibraryobject.h"
#include "tuplayer.h"
#include "tuplibrary.h"
#include "tupscenenamedialog.h"

#include <QList>

#define RETURN_IF_NOT_LIBRARY if (!library) return;

TupTimeLine::TupTimeLine(TupProject *projectData, QWidget *parent) : TupModuleWidgetBase(parent, "TupTimeLine")
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLine()]";
    #endif
    
    setWindowTitle(tr("Time Line"));
    setWindowIcon(QPixmap(ICONS_DIR + "time_line.png"));

    project = projectData;
    library = project->getLibrary();
    currentTable = nullptr;
    doSelection = false;

    // SQA: Pending to add the feature "Layer Opacity" as part of this action bar

    QList<TupProjectActionBar::Action> actions;
    actions << TupProjectActionBar::InsertFrame << TupProjectActionBar::ExtendFrame << TupProjectActionBar::RemoveFrame; 

    actions << TupProjectActionBar::MoveFrameBackward
            << TupProjectActionBar::MoveFrameForward
            << TupProjectActionBar::ReverseFrameSelection;

    actions << TupProjectActionBar::Separator;
    actions << TupProjectActionBar::CopyFrame << TupProjectActionBar::PasteFrame;
    actions << TupProjectActionBar::Separator;
    actions << TupProjectActionBar::InsertLayer << TupProjectActionBar::RemoveLayer;
    actions << TupProjectActionBar::Separator;
    actions << TupProjectActionBar::InsertScene << TupProjectActionBar::RemoveScene;

    actionBar = new TupProjectActionBar(QString("TimeLine"), actions);

    // Opacity Component

    QHBoxLayout *toolsLayout = new QHBoxLayout;
    toolsLayout->setAlignment(Qt::AlignHCenter);

    QLabel *header = new QLabel();
    QPixmap pix(THEME_DIR + "icons/layer_opacity.png");
    header->setToolTip(tr("Current Layer Opacity"));
    header->setPixmap(pix);

    opacitySpinBox = new QDoubleSpinBox(this);
    opacitySpinBox->setRange(0.1, 1.0);
    opacitySpinBox->setSingleStep(0.1);
    opacitySpinBox->setValue(1.0);
    opacitySpinBox->setToolTip(tr("Current Layer Opacity"));
    connect(opacitySpinBox, SIGNAL(valueChanged(double)), this, SLOT(requestUpdateLayerOpacity(double)));

    toolsLayout->addWidget(header);
    toolsLayout->addWidget(opacitySpinBox);

    // ---

    QWidget *toolBar = new QWidget;
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolBar);
    toolbarLayout->addWidget(actionBar);
    toolbarLayout->addSpacing(3);
    toolbarLayout->addWidget(new TSeparator(Qt::Vertical));
    toolbarLayout->addSpacing(3);
    toolbarLayout->addLayout(toolsLayout);

    addChild(toolBar, Qt::AlignHCenter);

    scenesContainer = new TupTimelineSceneContainer;
    addChild(scenesContainer);
    
    connect(actionBar, SIGNAL(actionSelected(int)), this, SLOT(requestCommand(int)));
    connect(scenesContainer, SIGNAL(currentChanged(int)), this, SLOT(requestSceneSelection(int)));
    connect(scenesContainer, SIGNAL(sceneRenameRequested(int)), this, SLOT(showRenameSceneDialog(int)));
}

TupTimeLine::~TupTimeLine()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupTimeLine()]";
    #endif
}

TupTimeLineTable *TupTimeLine::framesTable(int sceneIndex)
{
    TupTimeLineTable *framesTable = qobject_cast<TupTimeLineTable *>(scenesContainer->widget(sceneIndex));
    if (framesTable)
        return framesTable;
 
    return nullptr;
}

void TupTimeLine::addScene(int sceneIndex, const QString &name)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLine::addScene()] - sceneIndex -> " << sceneIndex << " - name -> " << name;
    #endif

    if (sceneIndex < 0 || sceneIndex > scenesContainer->count()) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupTimeLine::addScene()] - Fatal Error: invalid scene index -> " << sceneIndex;
        #endif

        return;
    }

    TupScene *tupScene = project->sceneAt(sceneIndex);
    int fps = tupScene->getFPS();
    TupTimeLineTable *framesTable = new TupTimeLineTable(sceneIndex, fps, scenesContainer);
    connect(framesTable, SIGNAL(frameSelected(int,int)), this, SLOT(requestFrameSelection(int,int)));
    connect(framesTable, SIGNAL(selectionCopied()), SLOT(requestCopyFrameSelection()));
    connect(framesTable, SIGNAL(selectionPasted()), SLOT(requestPasteSelectionInCurrentFrame()));
    connect(framesTable, SIGNAL(selectionRemoved()), SLOT(removeFrameSelection()));
    connect(framesTable, SIGNAL(frameExtended(int, int)), SLOT(extendFrameForward(int, int)));
    connect(framesTable, SIGNAL(visibilityChanged(int, bool)), this, SLOT(requestLayerVisibilityAction(int, bool)));

    connect(framesTable, SIGNAL(layerNameChanged(int, const QString &)), this, SLOT(requestLayerRenameAction(int, const QString &))); 
    connect(framesTable, SIGNAL(layerMoved(int, int)), this, SLOT(requestLayerMove(int, int)));
    connect(framesTable, SIGNAL(newPerspective(int)), this, SIGNAL(newPerspective(int)));

    scenesContainer->addScene(sceneIndex, framesTable, name);
}

void TupTimeLine::removeScene(int sceneIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLine::removeScene()] - sceneIndex -> " << sceneIndex;
    #endif

    if (sceneIndex >= 0 && sceneIndex < scenesContainer->count())
        scenesContainer->removeScene(sceneIndex, true);
}

void TupTimeLine::closeAllScenes()
{
    scenesContainer->removeAllScenes();
}

void TupTimeLine::sceneResponse(TupSceneResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLine::sceneResponse()] - action -> " << response->getAction();
    #endif

    int sceneIndex = response->getSceneIndex();

    switch (response->getAction()) {
        case TupProjectRequest::Add:
        {
            if (response->getMode() == TupProjectResponse::Do) {
                addScene(sceneIndex, response->getArg().toString());
            } else { 
                scenesContainer->restoreScene(sceneIndex, response->getArg().toString());
                TupProjectRequest request = TupRequestBuilder::createSceneRequest(sceneIndex, TupProjectRequest::Select);
                emit requestTriggered(&request);
            }

            return;
        }
        case TupProjectRequest::Remove:
        {
            removeScene(sceneIndex);
        }
        break;
        case TupProjectRequest::Reset:
        {
            if (response->getMode() == TupProjectResponse::Do || response->getMode() == TupProjectResponse::Redo) {
                scenesContainer->removeScene(sceneIndex, true);
                addScene(sceneIndex, tr("Scene %1").arg(sceneIndex + 1));

                currentTable = scenesContainer->getTable(sceneIndex);
                currentTable->insertLayer(0, tr("Layer 1"));
                currentTable->insertFrame(0);

                currentTable->blockSignals(true);
                currentTable->selectFrame(0, 0);
                currentTable->blockSignals(false);
            } else {
                scenesContainer->removeScene(sceneIndex, false);
                scenesContainer->restoreScene(sceneIndex, response->getArg().toString());

                currentTable = scenesContainer->getTable(sceneIndex);
                currentTable->blockSignals(true);
                currentTable->selectFrame(0, 0);
                currentTable->blockSignals(false);
            }
        }
        break;
        /*
        case TupProjectRequest::Move:
        {
            
        }
        break;
        case TupProjectRequest::Lock:
        {
            
        }
        break;
        */
        case TupProjectRequest::Rename:
        {
            scenesContainer->renameScene(sceneIndex, response->getArg().toString());
        }
        break;
        case TupProjectRequest::Select:
        {
            scenesContainer->setCurrentIndex(sceneIndex);
        }
        break;
        default:
            #ifdef TUP_DEBUG
                qWarning() << "[TupTimeLine::sceneResponse()] : Unknown action -> " << response->getAction();
            #endif
        break;
    }
}

void TupTimeLine::layerResponse(TupLayerResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLine::layerResponse()] - action -> " << response->getAction();
    #endif

    int sceneIndex = response->getSceneIndex();
    TupTimeLineTable *framesTable = this->framesTable(sceneIndex);
    if (framesTable) {
        int layerIndex = response->getLayerIndex();
        switch (response->getAction()) {
            case TupProjectRequest::Add:
            {
                TupScene *scene = project->sceneAt(sceneIndex);
                if (scene) {
                    TupLayer *layer = scene->layerAt(layerIndex);
                    if (layer) {
                        if (response->getMode() == TupProjectResponse::Do) {
                            framesTable->insertLayer(layerIndex, response->getArg().toString());

                            return;
                        } else {
                            framesTable->insertLayer(layerIndex, layer->getLayerName());
                            QList<TupFrame *> frames = layer->getFrames();
                            int total = frames.count();
                            for (int i=0; i<total; i++)
                                framesTable->insertFrame(layerIndex);

                            return;
                        }
                    }
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

                if (framesTable->layersCount() == 0) {
                    TupProjectRequest request = TupRequestBuilder::createLayerRequest(sceneIndex, 0, TupProjectRequest::Add,
                                                                                      tr("Layer %1").arg(1));
                    emit requestTriggered(&request);

                    request = TupRequestBuilder::createFrameRequest(sceneIndex, 0, 0, TupProjectRequest::Add, tr("Frame"));
                    emit requestTriggered(&request);
                }
            }
            break;
            case TupProjectRequest::Move:
            {
                framesTable->moveLayer(layerIndex, response->getArg().toInt());
            }
            break;
            /*
            case TupProjectRequest::Lock:
            {
                // SQA: Pending for implementation
            }
            break;
            */
            case TupProjectRequest::Rename:
            {
                framesTable->setLayerName(layerIndex, response->getArg().toString());
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
            case TupProjectRequest::AddLipSync:
                {
                    QString xml = response->getArg().toString();
                    TupLipSync *lipsync = new TupLipSync();
                    lipsync->fromXml(xml);
                    framesTable->setAttribute(layerIndex, lipsync->getInitFrame(), TupTimeLineTableItem::IsEmpty, false);
                }
            break;
            case TupProjectRequest::UpdateLipSync:
                {
                    QString xml = response->getArg().toString();
                    TupLipSync *lipsync = new TupLipSync();
                    lipsync->fromXml(xml);
                    framesTable->setAttribute(layerIndex, lipsync->getInitFrame(), TupTimeLineTableItem::IsEmpty, false);
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
                                     framesTable->setAttribute(layerIndex, i, TupTimeLineTableItem::IsEmpty, true);
                            }
                        }
                    }
                }
            break;
            default:
            break;
        }
    }
}

void TupTimeLine::frameResponse(TupFrameResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLine::frameResponse()] - action -> " << response->getAction();
    #endif

    int sceneIndex = response->getSceneIndex();
    TupTimeLineTable *framesTable = this->framesTable(sceneIndex);
    if (framesTable) {
        int layerIndex = response->getLayerIndex();
        int frameIndex = response->getFrameIndex();

        switch (response->getAction()) {
            case TupProjectRequest::Add:
              {
                  framesTable->insertFrame(layerIndex);
              }
            break;
            case TupProjectRequest::Remove:
              {
                  if (response->getMode() == TupProjectResponse::Redo || response->getMode() == TupProjectResponse::Undo) {
                      int lastFrame = framesTable->framesCountAtCurrentLayer() - 1;
                      int target = frameIndex;
                      if (target == lastFrame) { // Removing last frame
                          if (frameIndex > 0) {
                              framesTable->removeFrame(layerIndex, frameIndex);
                              frameIndex--;
                          }
                          framesTable->selectFrame(layerIndex, frameIndex);
                      } else { // When the item deleted is not the last one
                          framesTable->removeFrame(layerIndex, frameIndex);
                      }
                  } else {
                      framesTable->removeFrame(layerIndex, frameIndex);
                  }
              }
            break;
            case TupProjectRequest::RemoveSelection:
              {
                  QString selection = response->getArg().toString();
                  if (response->getMode() == TupProjectResponse::Do || response->getMode() == TupProjectResponse::Redo) {
                      QStringList blocks = selection.split(":");
                      QStringList params = blocks.at(0).split(",");
                      int layers = params.at(0).toInt();
                      int frames = params.at(1).toInt();
                      framesTable->removeFrameSelection(layerIndex, frameIndex, layers, frames, doSelection);
                  } else {
                      framesTable->restoreFrameSelection(layerIndex, frameIndex, selection);
                  }

                  doSelection = false;
              }
            break;
            case TupProjectRequest::Exchange:
              {
                  framesTable->exchangeFrame(frameIndex, layerIndex, response->getArg().toInt(), layerIndex);
              }
            break;
            case TupProjectRequest::Extend:
              {
                  int times = response->getArg().toInt();
                  if (response->getMode() == TupProjectResponse::Do || response->getMode() == TupProjectResponse::Redo) {
                      for (int i=0; i<times; i++)
                          framesTable->insertFrame(layerIndex);
                  } else {
                      framesTable->removeFrameSelection(layerIndex, frameIndex, 1, times);
                  }
              }
            break;
            case TupProjectRequest::Select:
              {
                  if (selectedLayer != layerIndex)
                      updateLayerOpacity(sceneIndex, layerIndex);

                  QString selection = response->getArg().toString();
                  selectedLayer = layerIndex;

                  framesTable->selectFrame(layerIndex, frameIndex, selection);
              }
            break;
            case TupProjectRequest::CopySelection:
              {
                  if (response->getMode() == TupProjectResponse::Do)
                      frameSelection = response->getArg().toString();
              }
            break;
            case TupProjectRequest::PasteSelection:
              {
                  if (!frameSelection.isEmpty()) {
                      QString selection = response->getArg().toString();
                      QStringList params = selection.split(",");
                      if (params.count() == 4) {
                          QList<int> coords;
                          foreach(QString item, params)
                              coords << item.toInt();

                          int layers = coords.at(1) - coords.at(0) + 1;
                          int frames = coords.at(3) - coords.at(2) + 1;
                          if (response->getMode() == TupProjectResponse::Do || response->getMode() == TupProjectResponse::Redo)
                              framesTable->pasteFrameSelection(layerIndex, frameIndex, layers, frames);
                          else
                              framesTable->removeFrameSelection(layerIndex, frameIndex, layers, frames);
                      }
                  }
              }
            break;
            default:
              // Do nothing
            break;
        }
    }
}

void TupTimeLine::libraryResponse(TupLibraryResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLine::libraryResponse()]";
    #endif

    if (response->getAction() == TupProjectRequest::InsertSymbolIntoFrame) {
        switch (response->symbolType()) {
            case TupLibraryObject::Audio:
            {
                TupTimeLineTable *framesTable = this->framesTable(response->getSceneIndex());
                if (framesTable) {
                    framesTable->insertSoundLayer(response->getLayerIndex() + 1, response->getArg().toString());
                    framesTable->insertFrame(response->getLayerIndex() + 1);
                }
            }
            break;
            default:
                // Do nothing
            break;
        }
    }
}

void TupTimeLine::itemResponse(TupItemResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLine::itemResponse()]";
    #endif

    int sceneIndex = response->getSceneIndex();
    TupTimeLineTable *framesTable = this->framesTable(sceneIndex);
    if (framesTable) {
        int layerIndex = response->getLayerIndex();
        int frameIndex = response->getFrameIndex();

        switch (response->getAction()) {
            case TupProjectRequest::Add:
              {
                  TupScene *scene = project->sceneAt(sceneIndex);
                  if (scene) {
                      if (!scene->frameIsEmpty(layerIndex, frameIndex))
                          framesTable->setAttribute(layerIndex, frameIndex, TupTimeLineTableItem::IsEmpty, false);
                  }
              }
            break;
            case TupProjectRequest::Remove:
              {
                  TupScene *scene = project->sceneAt(sceneIndex);
                  if (scene) {
                      if (scene->frameIsEmpty(layerIndex, frameIndex))
                          framesTable->setAttribute(layerIndex, frameIndex, TupTimeLineTableItem::IsEmpty, true);
                  }
              }
            break;
            default:
            break;
        }
    }
}

void TupTimeLine::requestCommand(int action)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLine::requestCommand()]";
    #endif

    int sceneIndex = scenesContainer->currentIndex();
    if (sceneIndex < 0) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupTimeLine::requestCommand()] - Fatal Error: Scene index is invalid -> " << sceneIndex;
        #endif

        return;
    }

    int layerIndex = framesTable(sceneIndex)->currentLayer();
    if (layerIndex < 0) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupTimeLine::requestCommand()] - Fatal Error: Layer index is invalid -> " << layerIndex;
        #endif

        return;
    }

    int frameIndex = framesTable(sceneIndex)->lastFrameByLayer(layerIndex);
    if (frameIndex < 0) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupTimeLine::requestCommand()] - Fatal Error: Frame index is invalid -> " << frameIndex;
        #endif

        return;
    }

    if (TupProjectActionBar::FrameActions & action) {
        if (!requestFrameAction(action, frameIndex, layerIndex, sceneIndex)) {
            #ifdef TUP_DEBUG
                qWarning() << "[TupTimeLine::requestCommand()] - Fatal Error: Frame action has failed!";
            #endif
        }

        return;
    }

    if (TupProjectActionBar::LayerActions & action) {
        if (!requestLayerAction(action, layerIndex, sceneIndex)) {
            #ifdef TUP_DEBUG
                qWarning() << "[TupTimeLine::requestCommand()] - Fatal Error: Layer action has failed!";
            #endif
        }

        return;
    }

    if (TupProjectActionBar::SceneActions & action) {
        if (!requestSceneAction(action, sceneIndex)) {
            #ifdef TUP_DEBUG
                qWarning() << "[TupTimeLine::requestCommand()] - Fatal Error: Scene action has failed! - sceneIndex -> " << sceneIndex;
            #endif
        }

        return;
    }
}

bool TupTimeLine::requestFrameAction(int action, int frameIndex, int layerIndex, int sceneIndex, const QVariant &arg)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLine::requestFrameAction()]";
    #endif

    Q_UNUSED(frameIndex)
    Q_UNUSED(arg)

    TupProjectRequest request;
    int currentFrame = framesTable(sceneIndex)->currentColumn();

    switch (action) {
        case TupProjectActionBar::InsertFrame:
        {
            int lastFrame = framesTable(sceneIndex)->lastFrameByLayer(layerIndex);
            if (currentFrame == lastFrame) {
                request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, lastFrame + 1,
                                             TupProjectRequest::Add, tr("Frame"));
                emit requestTriggered(&request);
            } else {
                request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, currentFrame + 1,
                                             TupProjectRequest::Add, tr("Frame"));
                emit requestTriggered(&request);
                int target = currentFrame + 2;
                for (int index=target; index <= lastFrame+1; index++) {
                    target++;
                    request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, index, TupProjectRequest::Rename, tr("Frame"));
                    emit requestTriggered(&request);
                }
            }

            requestFrameSelection(layerIndex, lastFrame + 1);

            return true;
        }
        case TupProjectActionBar::ExtendFrame:
        {
            extendFrameForward(layerIndex, currentFrame);

            return true;
        }
        case TupProjectActionBar::RemoveFrame:
        {
            requestRemoveFrame(true);

            return true;
        }
        case TupProjectActionBar::MoveFrameBackward:
        {
            TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, currentFrame, TupProjectRequest::Exchange, currentFrame - 1);
            emit requestTriggered(&request);

            return true;
        }
        case TupProjectActionBar::MoveFrameForward:
        {
            int lastFrame = framesTable(sceneIndex)->lastFrameByLayer(layerIndex);

            if (currentFrame == lastFrame) {
                TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, lastFrame + 1, TupProjectRequest::Add, tr("Frame"));
                emit requestTriggered(&request);
            }

            TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, currentFrame, TupProjectRequest::Exchange, currentFrame + 1);
            emit requestTriggered(&request);

            return true;
        }
        case TupProjectActionBar::ReverseFrameSelection:
        {
            requestReverseFrameSelection();

            return true;
        }
        case TupProjectActionBar::CopyFrame:
        {
            requestCopyFrameSelection();

            return true;
        }
        case TupProjectActionBar::PasteFrame:
        {
            requestPasteSelectionInCurrentFrame();

            return true;
        }
        default:
            // Do nothing
        break;
    }
    
    return false;
}

bool TupTimeLine::requestLayerAction(int action, int layerIndex, int sceneIndex, const QVariant &arg)
{
    TupProjectRequest request;

    switch (action) {
        case TupProjectActionBar::InsertLayer:
        {
            int layerIndex = framesTable(sceneIndex)->layersCount();
            request = TupRequestBuilder::createLayerRequest(sceneIndex, layerIndex, TupProjectRequest::Add, tr("Layer %1").arg(layerIndex + 1));
            emit requestTriggered(&request);

            if (layerIndex == 0) {
                request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, 0, TupProjectRequest::Add, tr("Frame"));
                emit requestTriggered(&request);
            } else {
                int total = framesTable(sceneIndex)->lastFrameByLayer(layerIndex - 1);
                for (int j=0; j <= total; j++) {
                    request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, j, TupProjectRequest::Add, tr("Frame"));
                    emit requestTriggered(&request);
                }
            }

            return true;
        }
        case TupProjectActionBar::RemoveLayer:
        {
            request = TupRequestBuilder::createLayerRequest(sceneIndex, layerIndex, TupProjectRequest::Remove, arg);
            emit requestTriggered(&request);

            return true;
        }
    }
    
    return false;
}

bool TupTimeLine::requestSceneAction(int action, int sceneIndex, const QVariant &arg)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLine::requestSceneAction()]";
    #endif

    TupProjectRequest request;

    switch (action) {
        case TupProjectActionBar::InsertScene:
        {            
            int sceneTarget = scenesContainer->count();
            QString sceneName = tr("Scene %1").arg(sceneTarget + 1);
            TupSceneNameDialog *dialog = new TupSceneNameDialog(TupSceneNameDialog::Add, sceneName);
            if (dialog->exec() == QDialog::Accepted) {
                sceneName = dialog->getSceneName();
                request = TupRequestBuilder::createSceneRequest(sceneTarget, TupProjectRequest::Add, sceneName);
                emit requestTriggered(&request);

                request = TupRequestBuilder::createLayerRequest(sceneTarget, 0, TupProjectRequest::Add, tr("Layer 1"));
                emit requestTriggered(&request);

                request = TupRequestBuilder::createFrameRequest(sceneTarget, 0, 0, TupProjectRequest::Add, tr("Frame"));
                emit requestTriggered(&request);

                request = TupRequestBuilder::createSceneRequest(sceneTarget, TupProjectRequest::Select);
                emit requestTriggered(&request);
            }

            return true;
        }
        case TupProjectActionBar::RemoveScene:
        {
            int scenesTotal = scenesContainer->count();
            if (scenesTotal > 1) {
                request = TupRequestBuilder::createSceneRequest(sceneIndex, TupProjectRequest::Remove, arg);
                emit requestTriggered(&request);

                request = TupRequestBuilder::createFrameRequest(sceneIndex - 1, 0, 0, TupProjectRequest::Select);
                emit requestTriggered(&request);
            } else {
                request = TupRequestBuilder::createSceneRequest(sceneIndex, TupProjectRequest::Reset, tr("Scene 1"));
                emit requestTriggered(&request);
            }

            return true;
        }
        case TupProjectActionBar::MoveSceneUp:
        {
            request = TupRequestBuilder::createSceneRequest(sceneIndex, TupProjectRequest::Move, sceneIndex + 1);
            emit requestTriggered(&request);

            return true;
        }
        case TupProjectActionBar::MoveSceneDown:
        {
            request = TupRequestBuilder::createSceneRequest(sceneIndex, TupProjectRequest::Move, sceneIndex - 1);
            emit requestTriggered(&request);

            return true;
        }
        default:
            // Do nothing
        break;
    }
    
    return false;
}

void TupTimeLine::requestLayerVisibilityAction(int layerIndex, bool isVisible)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLine::requestLayerVisibilityAction()]";
    #endif
    */

    int sceneIndex = scenesContainer->currentIndex();

    TupProjectRequest request = TupRequestBuilder::createLayerRequest(sceneIndex, layerIndex, TupProjectRequest::View, isVisible);
    emit requestTriggered(&request);
}

void TupTimeLine::requestLayerRenameAction(int layerIndex, const QString &name)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLine::requestLayerRenameAction()] - name: " << name;
    #endif
    */

    int sceneIndex = scenesContainer->currentIndex();
    
    TupProjectRequest request = TupRequestBuilder::createLayerRequest(sceneIndex, layerIndex, TupProjectRequest::Rename, name);
    emit requestTriggered(&request);
}

void TupTimeLine::requestFrameSelection(int layerIndex, int frameIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLine::requestFrameSelection()] - layerIndex, frameIndex -> ("
                 << layerIndex << ", " << frameIndex << ")";
    #endif

    int sceneIndex = scenesContainer->currentIndex();
    TupScene *scene = project->sceneAt(sceneIndex);
    if (scene) {
        int lastFrame = framesTable(sceneIndex)->lastFrameByLayer(layerIndex);
        QList<int> coords = framesTable(sceneIndex)->currentSelection();

        if (frameIndex > lastFrame) {
            for (int frame = lastFrame + 1; frame <= frameIndex; frame++) {
                TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, frame,
                                            TupProjectRequest::Add, tr("Frame"));
                emit requestTriggered(&request);
            }
        }

        QString selection = "";
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

        TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex,
                                                       frameIndex, TupProjectRequest::Select, selection);
        emit requestTriggered(&request);
    }
}

void TupTimeLine::removeFrameSelection()
{
    actionBar->emitActionSelected(TupProjectActionBar::RemoveFrame);
}

void TupTimeLine::requestRemoveFrame(bool flag)
{
    doSelection = flag;

    int sceneIndex = scenesContainer->currentIndex();
    QList<int> coords = framesTable(sceneIndex)->currentSelection();
    if (coords.count() == 4) {
        int layers = coords.at(1) - coords.at(0) + 1;
        int frames = coords.at(3) - coords.at(2) + 1;

        QString flags = "";
        for (int i=coords.at(0); i<=coords.at(1); i++) {
            int framesCount = framesTable(sceneIndex)->lastFrameByLayer(i) + 1;
            if (framesCount > frames)
                flags += "0,";
            else
                flags += "1,";
        }
        flags.chop(1);

        QString selection = QString::number(layers) + "," + QString::number(frames) + ":" + flags;
        TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, coords.at(0), coords.at(2),
                                                                          TupProjectRequest::RemoveSelection, selection);
        emit requestTriggered(&request);
    }
}

void TupTimeLine::extendFrameForward(int layerIndex, int frameIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCommandExecutor::copyFrameSelection()]";
    #endif

    int sceneIndex = scenesContainer->currentIndex();
    TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, frameIndex,
                                                                      TupProjectRequest::Extend, 1);
    emit requestTriggered(&request);
}

void TupTimeLine::requestSceneSelection(int sceneIndex)
{
    if (scenesContainer->count() > 1) {
        int previewFrameIndex = scenesContainer->currentScene()->currentFrame();
        int previewLayerIndex = scenesContainer->currentScene()->currentLayer();
        int previewSceneIndex = scenesContainer->currentIndex();

        QString args = QString::number(previewSceneIndex) + ":" + QString::number(previewLayerIndex) + ":" + QString::number(previewFrameIndex);
        TupProjectRequest request = TupRequestBuilder::createSceneRequest(sceneIndex, TupProjectRequest::Select, args);
        emit localRequestTriggered(&request);
        emit sceneChanged(previewSceneIndex);
    }
}

void TupTimeLine::requestLayerMove(int oldLayerIndex, int newLayerIndex)
{
    TupProjectRequest request = TupRequestBuilder::createLayerRequest(scenesContainer->currentIndex(), oldLayerIndex,
                                                   TupProjectRequest::Move, newLayerIndex);
    emit requestTriggered(&request);
}

void TupTimeLine::initLayerVisibility()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLine::initLayerVisibility()]";
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

void TupTimeLine::requestCopyFrameSelection()
{
    int sceneIndex = scenesContainer->currentIndex();
    int layerIndex = framesTable(sceneIndex)->currentLayer();
    int currentFrame = framesTable(sceneIndex)->currentColumn();

    QList<int> coords = framesTable(sceneIndex)->currentSelection();
    if (coords.count() == 4) {
        QString selection = QString::number(coords.at(0)) + "," + QString::number(coords.at(1)) + ","
                            + QString::number(coords.at(2)) + "," + QString::number(coords.at(3));

        TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, currentFrame,
                                                                          TupProjectRequest::CopySelection, selection);
        emit requestTriggered(&request);
    }
}

void TupTimeLine::requestPasteSelectionInCurrentFrame()
{
    if (!frameSelection.isEmpty()) {
        int sceneIndex = scenesContainer->currentIndex();
        int layerIndex = framesTable(sceneIndex)->currentLayer();
        int currentFrame = framesTable(sceneIndex)->currentColumn();

        TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, currentFrame,
                                                                          TupProjectRequest::PasteSelection);
        emit requestTriggered(&request);
    }
}

void TupTimeLine::requestReverseFrameSelection()
{
    int sceneIndex = scenesContainer->currentIndex();
    int layerIndex = framesTable(sceneIndex)->currentLayer();
    int currentFrame = framesTable(sceneIndex)->currentColumn();

    QList<int> coords = framesTable(sceneIndex)->currentSelection();
    if (coords.count() == 4) {
        if (coords.at(1) != coords.at(3)) {
            QString selection = QString::number(coords.at(0)) + "," + QString::number(coords.at(1)) + ","
                                + QString::number(coords.at(2)) + "," + QString::number(coords.at(3));

            TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, currentFrame,
                                                                              TupProjectRequest::ReverseSelection, selection);
            emit requestTriggered(&request);
        }
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupTimeLine::requestReverseFrameSelection()] - Selection must include at least 2 frames of the same layer";
        #endif
    }
}

void TupTimeLine::requestUpdateLayerOpacity(double opacity)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLine::requestUpdateLayerOpacity()] - opacity -> " << opacity;
    #endif

    int sceneIndex = scenesContainer->currentIndex();
    int layerIndex = framesTable(sceneIndex)->currentLayer();

    TupProjectRequest request = TupRequestBuilder::createLayerRequest(sceneIndex, layerIndex,
                                                                      TupProjectRequest::UpdateOpacity, opacity);
    emit requestTriggered(&request);
}

double TupTimeLine::getLayerOpacity(int sceneIndex, int layerIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLine::getLayerOpacity()] - sceneIndex/layerIndex -> " << sceneIndex << "," << layerIndex;
    #endif

    double opacity = 1.0;
    TupScene *scene = project->sceneAt(sceneIndex);
    if (scene) {
        TupLayer *layer = scene->layerAt(layerIndex);
        if (layer) {
            opacity = layer->getOpacity();
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupTimeLine::getLayerOpacity()] - Fatal Error: No layer at index -> " << layerIndex;
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupTimeLine::getLayerOpacity()] - Fatal Error: No scene at index -> " << sceneIndex;
        #endif
    }

    return opacity;
}

void TupTimeLine::updateLayerOpacity(int sceneIndex, int layerIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLine::updateLayerOpacity()] - sceneIndex/layerIndex -> " << sceneIndex << "," << layerIndex;
    #endif

    double opacity = getLayerOpacity(sceneIndex, layerIndex);
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLine::updateLayerOpacity()] - layer opacity -> " << opacity;
    #endif
    opacitySpinBox->blockSignals(true);
    opacitySpinBox->setValue(opacity);
    opacitySpinBox->blockSignals(false);
}

void TupTimeLine::updateFPS(int fps)
{
    currentTable = scenesContainer->currentScene();
    if (currentTable)
        currentTable->updateFPS(fps);
}

void TupTimeLine::updateFramesState()
{
    for (int i=0; i < project->scenesCount(); i++) {
         TupScene *scene = project->sceneAt(i);
         TupTimeLineTable *table = scenesContainer->getTable(i);
         for (int j=0; j < scene->layersCount(); j++) {
              TupLayer *layer = scene->layerAt(j);
              for (int k=0; k < layer->framesCount(); k++) {
                   TupFrame *frame = layer->frameAt(k);
                   bool isEmpty = false;
                   if (frame->isEmpty())
                       isEmpty = true;
                   table->updateFrameState(k, j, isEmpty);
              }
         }
    }
}

void TupTimeLine::requestSceneRename(const QString &name)
{
    TupProjectRequest event = TupRequestBuilder::createSceneRequest(scenesContainer->currentIndex(),
                                                                    TupProjectRequest::Rename, name);
    emit requestTriggered(&event);
}

void TupTimeLine::showRenameSceneDialog(int sceneIndex)
{
    TupScene *scene = project->sceneAt(sceneIndex);
    QString name = scene->getSceneName();

    TupSceneNameDialog *dialog = new TupSceneNameDialog(TupSceneNameDialog::Rename, name);
    if (dialog->exec() == QDialog::Accepted)
        requestSceneRename(dialog->getSceneName());
}
