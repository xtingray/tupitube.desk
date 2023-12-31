/***************************************************************************
 *   Project TUPITUBE DESK                                                 *
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

#include "tuppaintarea.h"
#include "toptionaldialog.h"
#include "tuppaintareaevent.h"
#include "tosd.h"
#include "tupitemgroup.h"
#include "tuppixmapitem.h"
#include "tupsvg2qt.h"
#include "tmouthtarget.h"
#include "tupfilemanager.h"
#include "talgorithm.h"
#include "tupprojectimporterdialog.h"
#include "tupprojectscanner.h"

#include <cmath> // fabs
#include <QMimeData>
#include <QNetworkAccessManager>

TupPaintArea::TupPaintArea(TupProject *work, QWidget *parent): TupPaintAreaBase(parent, work->getDimension(),
                                                                                 work->getLibrary())
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea()]";
    #endif

    setAccessibleName("WORKSPACE");
    setAcceptDrops(true);

    project = work;
    canvasEnabled = false;
    globalSceneIndex = 0;
    deleteMode = false;
    menuOn = false;
    copyIsValid = false;
    currentToolID = TAction::Pencil;
    webLock = false;

    QPair<int, int> dimension = TAlgorithm::screenDimension();
    screenWidth = dimension.first;
    screenHeight = dimension.second;

    setBgColor(work->getCurrentBgColor());

    setCurrentScene(0);
    graphicsScene()->setCurrentFrame(0, 0);
}

TupPaintArea::~TupPaintArea()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupPaintArea()]";
    #endif

    graphicsScene()->clear();
    delete graphicsScene();
}

void TupPaintArea::setCurrentScene(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::setCurrentScene()] - Scene index: " << index;
    #endif

    if (project->scenesCount() > 0) {
        TupScene *scene = project->sceneAt(index);
        if (scene) {
            globalSceneIndex = index;
            graphicsScene()->setCurrentScene(scene);
            setBgColor(scene->getBgColor());
        } else {
            if (project->scenesCount() == 1) {
                setDragMode(QGraphicsView::NoDrag);
                globalSceneIndex = 0;
                graphicsScene()->setCurrentScene(nullptr);
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupPaintArea::setCurrentScene()] - [ Fatal Error ] -  No scenes available. Invalid index -> "
                             << index;
                    qDebug() << "[TupPaintArea::setCurrentScene()] - Scenes total -> " << project->scenesCount();
                #endif
            }
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPaintArea::setCurrentScene()] - No scenes available!";
        #endif
    }
}

void TupPaintArea::mousePressEvent(QMouseEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::mousePressEvent()]";
    #endif

    if (!canvasEnabled)
        return;

    TupFrame *frame = graphicsScene()->currentFrame();
    if (frame) {
        if (frame->isFrameLocked()) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPaintArea::mousePressEvent()] - Frame is locked!";
            #endif
            return;
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPaintArea::mousePressEvent()] - Frame is NULL!";
        #endif
        return;
    }

    if (currentToolID == TAction::LipSyncTool) {
        // If a node is the mouth target... abort!
        if (event->buttons() == Qt::RightButton) {
            if (qgraphicsitem_cast<TMouthTarget *>(scene()->itemAt(mapToScene(event->pos()), QTransform())))
                return;
        }
    }

    if (currentToolID == TAction::NodesEditor) {
        // If a node is the target... abort!
        if (event->buttons() == Qt::RightButton) {
            if (qgraphicsitem_cast<TControlNode *>(scene()->itemAt(mapToScene(event->pos()), QTransform())))
                return;
        }
    }

    if (event->buttons() == Qt::RightButton) {
        if (currentToolID == TAction::Polyline) {
            emit closePolyLine();
            return;
        }
        if (currentToolID == TAction::Line) {
            emit closeLine();
            return;
        }
    }

    if (currentToolID == TAction::Text) {
        if (event->buttons() == Qt::RightButton) {
            bool activeSelection = !scene()->selectedItems().isEmpty();
            if (activeSelection) {
                QMenu *menu = new QMenu(tr("Drawing area"));
                menu->addAction(kApp->findGlobalAction("undo"));
                menu->addSeparator();
                menu->addAction(tr("Add to library..."), this, SLOT(addSelectedItemsToLibrary()));
                menuOn = true;
                menu->exec(event->globalPos());
                return;
            }
        }
    }

    if (currentToolID == TAction::ObjectSelection) {
        if (event->buttons() == Qt::RightButton) {
            QMenu *menu = new QMenu(tr("Drawing area"));
            menu->addAction(kApp->findGlobalAction("undo"));
            menu->addAction(kApp->findGlobalAction("redo"));

            bool activeSelection = !scene()->selectedItems().isEmpty();

            if (activeSelection) {
                menu->addSeparator();
                menu->addAction(tr("Cut"), this, SLOT(cutItems()), QKeySequence(tr("Ctrl+X")));
                menu->addAction(tr("Copy"), this, SLOT(copyItems()));
            }

            if (!copiesXml.isEmpty()) {
                if (!activeSelection)
                    menu->addSeparator();
                menu->addAction(tr("Paste"), this, SLOT(pasteItems()));

                QMenu *pasteMenu = new QMenu(tr("Paste in..."));
                QAction *pasteFive = pasteMenu->addAction(tr("next 5 frames"), this, SLOT(pasteNextFive()));
                QAction *pasteTen = pasteMenu->addAction(tr("next 10 frames"), this, SLOT(pasteNextTen()));
                QAction *pasteTwenty = pasteMenu->addAction(tr("next 20 frames"), this, SLOT(pasteNextTwenty()));
                QAction *pasteFifty = pasteMenu->addAction(tr("next 50 frames"), this, SLOT(pasteNextFifty()));
                QAction *pasteHundred = pasteMenu->addAction(tr("next 100 frames"), this, SLOT(pasteNextHundred()));

                pasteMenu->addAction(pasteFive);
                pasteMenu->addAction(pasteTen);
                pasteMenu->addAction(pasteTwenty);
                pasteMenu->addAction(pasteFifty);
                pasteMenu->addAction(pasteHundred);

                menu->addMenu(pasteMenu);
            }

            if (activeSelection) {
                menu->addAction(tr("Delete"), this, SLOT(deleteItems()));
                menu->addSeparator();
                QMenu *order = new QMenu(tr("Send"));

                connect(order, SIGNAL(triggered(QAction*)), this, SLOT(requestItemMovement(QAction*)));
                order->addAction(tr("To back"))->setData(TupFrame::MoveBack);
                order->addAction(tr("To front"))->setData(TupFrame::MoveToFront);
                order->addAction(tr("One level to back"))->setData(TupFrame::MoveOneLevelBack);
                order->addAction(tr("One level to front"))->setData(TupFrame::MoveOneLevelToFront);

                menu->addMenu(order);
                menu->addSeparator();

                // Code commented temporary while SQA is done
                QAction *addItem = menu->addAction(tr("Add to library..."), this, SLOT(addSelectedItemsToLibrary()));
                menu->addSeparator();

                QList<QGraphicsItem *> selected = scene()->selectedItems();
                foreach (QGraphicsItem *item, selected) {
                    QDomDocument dom;
                    dom.appendChild(dynamic_cast<TupAbstractSerializable *>(item)->toXml(dom));
                    QDomElement root = dom.documentElement();

                    if (root.tagName() == "symbol") {
                       QString key = root.attribute("id").toUpper();
                       if (key.endsWith("JPG") || key.endsWith("PNG") || key.endsWith("GIF") || key.endsWith("XPM")) {
                           addItem->setEnabled(false);
                           break;
                       }
                    } else if (root.tagName() == "svg") {
                       addItem->setEnabled(false);
                       break;
                    }
                }
            }

            /* SQA: This menu is a pending feature to *consider*
            if (QMenu *toolMenu = graphicsScene()->currentTool()->menu()) {
                menu->addSeparator();
                menu->addMenu(toolMenu);
            }
            */

            position = viewPosition();
            menuOn = true;
            menu->exec(event->globalPos());
        }
    } 

    TupPaintAreaBase::mousePressEvent(event);
}

void TupPaintArea::tabletEvent(QTabletEvent *event)
{
    if (currentToolID == TAction::Ink) {
        if (event->pressure() > 0)
            graphicsScene()->currentTool()->updatePressure(event->pressure());
    }

    TupPaintAreaBase::tabletEvent(event);
}

void TupPaintArea::frameResponse(TupFrameResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::frameResponse()] - [" << response->getSceneIndex()
                 << ", " << response->getLayerIndex() << ", "
                 << response->getFrameIndex() << "] | request -> "
                 << response->getAction();
    #endif

    TupGraphicsScene *guiScene = graphicsScene();
    if (!guiScene->currentScene()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPaintArea::frameResponse()] - Fatal Error: No TupScene available!";
        #endif
        return;
    }

    if (!guiScene->userIsDrawing()) {
        switch (response->getAction()) {
            case TupProjectRequest::PasteSelection:
            case TupProjectRequest::RemoveSelection:
            case TupProjectRequest::ReverseSelection:
            case TupProjectRequest::Exchange:
              {
                  if (spaceMode == TupProject::FRAMES_MODE)
                      guiScene->drawCurrentPhotogram();
              }
            break;
            case TupProjectRequest::Add:
            case TupProjectRequest::Select:
            case TupProjectRequest::Paste:
            case TupProjectRequest::Reset:
              {
                  if (response->getAction() == TupProjectRequest::Select) {
                      if (guiScene->currentFrameIndex() != response->getFrameIndex())
                          emit frameChanged(response->getFrameIndex());
                  } else {
                      emit frameChanged(response->getFrameIndex());
                  }
                  guiScene->setCurrentFrame(response->getLayerIndex(), response->getFrameIndex());

                  if (spaceMode == TupProject::FRAMES_MODE) {
                      guiScene->drawPhotogram(response->getFrameIndex(), true);
                  } else if (spaceMode == TupProject::VECTOR_FG_MODE) {
                      guiScene->cleanWorkSpace();
                      guiScene->drawVectorFg();
                  } else {
                      guiScene->cleanWorkSpace();
                      guiScene->drawSceneBackground(guiScene->currentFrameIndex());
                  }

                  if (guiScene->currentTool()->toolType() == TupToolInterface::Selection)
                      guiScene->resetCurrentTool();
              }
            break;
            default:
              #ifdef TUP_DEBUG
                  qDebug() << "[TupPaintArea::frameResponse()] - Action not recognized -> " << response->getAction();
              #endif
            break;
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPaintArea::frameResponse()] - isDrawing() == true! - No action taken!";
        #endif
    }

    guiScene->frameResponse(response);
}

void TupPaintArea::layerResponse(TupLayerResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::layerResponse()] - [" << response->getSceneIndex() << ", "
                 << response->getLayerIndex() << "]";
    #endif

    TupGraphicsScene *guiScene = graphicsScene();
    if (!guiScene->currentScene())
        return;

    int frameIndex = guiScene->currentFrameIndex();

    switch (response->getAction()) {
        case TupProjectRequest::Add:
          {
              if (response->getMode() == TupProjectResponse::Redo || response->getMode() == TupProjectResponse::Undo) {
                  if (spaceMode == TupProject::FRAMES_MODE)
                      guiScene->drawCurrentPhotogram();
              }
              return;
          }
        case TupProjectRequest::Remove:
          {
              TupScene *scene = project->sceneAt(globalSceneIndex);

              if (scene) {
                  if (scene->layersCount() > 1) {
                      if (response->getLayerIndex() != 0)
                          guiScene->setCurrentFrame(response->getLayerIndex() - 1, frameIndex);
                      else
                          guiScene->setCurrentFrame(response->getLayerIndex() + 1, frameIndex);

                      if (spaceMode == TupProject::FRAMES_MODE) {
                          guiScene->drawCurrentPhotogram();
                      } else if (spaceMode == TupProject::VECTOR_FG_MODE) {
                          guiScene->cleanWorkSpace();
                          guiScene->drawVectorFg();
                      } else {
                          guiScene->cleanWorkSpace();
                          guiScene->drawSceneBackground(frameIndex);
                      }
                  } else {
                      if (scene->layersCount() == 1) {
                          guiScene->setCurrentFrame(0, frameIndex);
                          if (spaceMode == TupProject::FRAMES_MODE) {
                              guiScene->drawCurrentPhotogram();
                          } else if (spaceMode == TupProject::VECTOR_FG_MODE) {
                              guiScene->cleanWorkSpace();
                              guiScene->drawVectorFg();
                          } else {
                              guiScene->cleanWorkSpace();
                              guiScene->drawSceneBackground(frameIndex);
                          }
                      }
                  }

                  viewport()->update();
              }
          }
        break;
        case TupProjectRequest::UpdateOpacity:
          {
              guiScene->drawCurrentPhotogram();
              if (scene())
                  viewport()->update(guiScene->sceneRect().toRect());
          }
        break;
        case TupProjectRequest::View:
          {
              guiScene->updateLayerVisibility(response->getLayerIndex(), response->getArg().toBool());
              if (spaceMode == TupProject::FRAMES_MODE) {
                  guiScene->drawCurrentPhotogram();
              } else if (spaceMode == TupProject::VECTOR_FG_MODE) {
                  guiScene->cleanWorkSpace();
                  guiScene->drawVectorFg();
              } else {
                  guiScene->cleanWorkSpace();
                  guiScene->drawSceneBackground(frameIndex);
              }

              viewport()->update(guiScene->sceneRect().toRect());
          }
        break;
        case TupProjectRequest::Move:
          {
              guiScene->setCurrentFrame(response->getArg().toInt(), frameIndex);
              if (spaceMode == TupProject::FRAMES_MODE) {
                  guiScene->drawCurrentPhotogram();
              } else if (spaceMode == TupProject::VECTOR_FG_MODE) {
                  guiScene->cleanWorkSpace();
                  guiScene->drawVectorFg();
              } else {
                  guiScene->cleanWorkSpace();
                  guiScene->drawSceneBackground(frameIndex);
              }

              viewport()->update(guiScene->sceneRect().toRect());
          }
        break;
        case TupProjectRequest::UpdateLipSync:
          {
              guiScene->layerResponse(response);
              return;
          }
        break;
        default:
          {
              if (spaceMode == TupProject::FRAMES_MODE) {
                  guiScene->drawCurrentPhotogram();
              } else if (spaceMode == TupProject::VECTOR_FG_MODE) {
                  guiScene->cleanWorkSpace();
                  guiScene->drawVectorFg();
              } else {
                  guiScene->cleanWorkSpace();
                  guiScene->drawSceneBackground(frameIndex);
              }

              viewport()->update(guiScene->sceneRect().toRect());
          }
          break;
    }

    guiScene->layerResponse(response);
}

void TupPaintArea::sceneResponse(TupSceneResponse *event)
{
    #ifdef TUP_DEBUG
        qInfo() << "[TupPaintArea::sceneResponse()] - [" << event->getSceneIndex() << "]";
    #endif

    TupGraphicsScene *guiScene = graphicsScene();
    if (!guiScene->currentScene())
        return;

    if (!guiScene->userIsDrawing()) {
        int sceneIndex = event->getSceneIndex();
        switch(event->getAction()) {
            case TupProjectRequest::Select:
              {
                  if (sceneIndex >= 0) {
                      if (project->scenesCount() == 1)
                          setCurrentScene(0);
                      else
                          setCurrentScene(sceneIndex);
                  }
              }
            break;
            case TupProjectRequest::Remove:
              {
                  if (project->scenesCount() > 0) {
                      if (project->scenesCount() == 1) {
                          setCurrentScene(0);
                      } else {
                          if (sceneIndex == (project->scenesCount() - 1))
                              setCurrentScene(sceneIndex - 1);
                          else
                              setCurrentScene(sceneIndex);
                      }
                  }
              }
            break;
            case TupProjectRequest::Move:
              {
                  qDebug() << "[TupPaintArea::sceneResponse()] - Tracing scene move action!";
                  qDebug() << "[TupPaintArea::sceneResponse()] - pos ->" << sceneIndex;
                  qDebug() << "[TupPaintArea::sceneResponse()] - newPos ->" << event->getArg().toInt();

                  setCurrentScene(event->getArg().toInt());
              }
            break;
            case TupProjectRequest::Reset:
              {
                  setCurrentScene(sceneIndex);
              }
            break;
            case TupProjectRequest::BgColor:
              {
                  QString colorName = event->getArg().toString();
                  QColor color(colorName);
                  setBgColor(color);
              }
            break;
            default: 
              {
                  #ifdef TUP_DEBUG
                      qDebug() << "[TupPaintArea::sceneResponse()] <- TupProjectRequest::Default";
                  #endif
              }
            break;
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPaintArea::sceneResponse()] - isDrawing() == true! - No action taken!";
        #endif
    }

    guiScene->sceneResponse(event);
}

void TupPaintArea::itemResponse(TupItemResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::itemResponse()] - [" << response->getSceneIndex()
                 << ", " << response->getLayerIndex() << ", "
                 << response->getFrameIndex() << "]";
    #endif

    TupGraphicsScene *guiScene = graphicsScene();
    if (!guiScene->currentScene())
        return;

    if (!guiScene->userIsDrawing()) {
        switch(response->getAction()) {
            case TupProjectRequest::Transform:
              {
                  viewport()->update();
              }
            break;
            case TupProjectRequest::Remove:
              { 
                  if (!deleteMode) {
                      if (spaceMode == TupProject::FRAMES_MODE) {
                          guiScene->drawCurrentPhotogram();
                      } else if (spaceMode == TupProject::VECTOR_FG_MODE) {
                          guiScene->cleanWorkSpace();
                          guiScene->drawVectorFg();
                      } else {
                          guiScene->cleanWorkSpace();
                          guiScene->drawSceneBackground(guiScene->currentFrameIndex());
                      }

                      viewport()->update(guiScene->sceneRect().toRect());
                  } 
              }
            break;
            case TupProjectRequest::Move:
              {
                  if (spaceMode == TupProject::FRAMES_MODE) {
                      guiScene->drawCurrentPhotogram();
                  } else if (spaceMode == TupProject::VECTOR_FG_MODE) {
                      guiScene->cleanWorkSpace();
                      guiScene->drawVectorFg();
                  } else {
                      guiScene->cleanWorkSpace();
                      guiScene->drawSceneBackground(guiScene->currentFrameIndex());
                  }

                  viewport()->update(guiScene->sceneRect().toRect());
              }
            case TupProjectRequest::UpdateTweenPath:
              {
                  // Do nothing
              }
            break;
            default:
              {
                  if (spaceMode == TupProject::FRAMES_MODE) {
                      guiScene->drawCurrentPhotogram();
                  } else if (spaceMode == TupProject::VECTOR_FG_MODE) {
                      guiScene->cleanWorkSpace();
                      guiScene->drawVectorFg();
                  } else {
                      guiScene->cleanWorkSpace();
                      guiScene->drawSceneBackground(guiScene->currentFrameIndex());
                  }

                  viewport()->update(guiScene->sceneRect().toRect());

                  if (guiScene->currentTool()->toolType() != TupToolInterface::Tweener
                      && currentToolID != TAction::Polyline && currentToolID != TAction::Line)
                      guiScene->resetCurrentTool();
              }
            break;
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPaintArea::itemResponse()] - isDrawing() == true! - No action taken!";
        #endif
    }

    guiScene->itemResponse(response);
}

void TupPaintArea::projectResponse(TupProjectResponse *)
{
}

void TupPaintArea::libraryResponse(TupLibraryResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::libraryResponse()] - Response Action: " << response->getAction();
    #endif

    TupGraphicsScene *guiScene = graphicsScene();

    if (!guiScene->currentScene())
        return;

    if (!guiScene->userIsDrawing()) {
        int frameIndex = guiScene->currentFrameIndex();
        switch (response->getAction()) {
            case TupProjectRequest::InsertSymbolIntoFrame:
              {
                  if (spaceMode == TupProject::FRAMES_MODE) {
                      guiScene->drawCurrentPhotogram();
                  } else if (spaceMode == TupProject::VECTOR_FG_MODE) {
                      guiScene->cleanWorkSpace();
                      guiScene->drawVectorFg();
                  } else {
                      guiScene->cleanWorkSpace();
                      guiScene->drawSceneBackground(frameIndex);
                  }

                  viewport()->update(guiScene->sceneRect().toRect());

                  if (currentToolID == TAction::ObjectSelection)
                      emit itemAddedOnSelection(guiScene);
              }
            break;
            case TupProjectRequest::Remove:
            case TupProjectRequest::RemoveSymbolFromFrame:
              {
                  if (spaceMode == TupProject::FRAMES_MODE) {
                      guiScene->drawCurrentPhotogram();
                  } else if (spaceMode == TupProject::VECTOR_FG_MODE) {
                      guiScene->cleanWorkSpace();
                      guiScene->drawVectorFg();
                  } else {
                      guiScene->cleanWorkSpace();
                      guiScene->drawSceneBackground(frameIndex);
                  }

                  viewport()->update(guiScene->sceneRect().toRect());
              }
            break;
        } 
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPaintArea::libraryResponse()] - isDrawing() == true! - No action taken!";
        #endif
    }

    guiScene->libraryResponse(response);
}

bool TupPaintArea::canPaint() const
{
    TupGraphicsScene *guiScene = graphicsScene();

    if (guiScene->currentScene()) {
        if (guiScene->currentFrameIndex() >= 0 && guiScene->currentLayerIndex() >= 0) 
            return true;
    }

    return false;
}

void TupPaintArea::deleteItems()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::deleteItems()]";
    #endif

    if (currentToolID != TAction::ObjectSelection && currentToolID != TAction::NodesEditor && currentToolID != TAction::Text) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPaintArea::deleteItems()] - Aborting procedure!";
        #endif
        return;
    }

    QList<QGraphicsItem *> selected = scene()->selectedItems();
    if (!selected.empty()) {
        foreach (QGraphicsItem *item, selected) {
            if (qgraphicsitem_cast<TControlNode *> (item))
                selected.removeOne(item);
        }

        TupGraphicsScene* currentScene = graphicsScene();
        if (currentScene) {
            int counter = 0;
            int total = selected.count();
            deleteMode = true;
            foreach (QGraphicsItem *item, selected) {
                 if (counter == total-1)
                     deleteMode = false;

                 TupLibraryObject::ObjectType type = TupLibraryObject::Svg;
                 TupSvgItem *svg = qgraphicsitem_cast<TupSvgItem *>(item);
                 int itemIndex = -1;
                 int frameIndex = -1;
                 int layerIndex = -1;

                 if (spaceMode == TupProject::FRAMES_MODE) {
                     frameIndex = currentScene->currentFrameIndex();
                     layerIndex = currentScene->currentLayerIndex();
                     if (svg) {
                         itemIndex = currentScene->currentFrame()->indexOf(svg);
                     } else {
                         type = TupLibraryObject::Item;
                         itemIndex = currentScene->currentFrame()->indexOf(item);
                     }
                 } else {
                     TupBackground *bg = currentScene->currentScene()->sceneBackground();
                     if (bg) {
                         TupFrame *frame;
                         if (spaceMode == TupProject::VECTOR_STATIC_BG_MODE)
                             frame = bg->vectorStaticFrame();
                         else if (spaceMode == TupProject::VECTOR_FG_MODE)
                             frame = bg->vectorForegroundFrame();
                         else
                             frame = bg->vectorDynamicFrame();

                         if (frame) {
                             if (svg) {
                                 itemIndex = frame->indexOf(svg);
                             } else {
                                 type = TupLibraryObject::Item;
                                 itemIndex = frame->indexOf(item);
                             }
                         } else {
                             #ifdef TUP_DEBUG
                                 qDebug() << "[TupPaintArea::deleteItems()] - Fatal Error: Background frame is NULL!";
                             #endif
                         }
                     } else {
                         #ifdef TUP_DEBUG
                             qDebug() << "[TupPaintArea::deleteItems()] - Fatal Error: Scene has no background element!";
                         #endif
                     }
                 }

                 if (itemIndex >= 0) {
                     TupProjectRequest event = TupRequestBuilder::createItemRequest(
                                               currentScene->currentSceneIndex(), layerIndex, frameIndex,
                                               itemIndex, QPointF(), spaceMode, type,
                                               TupProjectRequest::Remove);
                     emit requestTriggered(&event);
                 } else {
                     #ifdef TUP_DEBUG
                         qDebug() << "[TupPaintArea::deleteItems()] - Fatal Error: Invalid item index -> "
                                  << itemIndex;
                     #endif
                 }

                 counter++;
            }
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPaintArea::deleteItems()] - Warning: No items to remove!";
        #endif
    }
}

void TupPaintArea::copyItems()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::copyItems()]";
    #endif

    QList<QGraphicsItem *> selected = scene()->selectedItems();

    if (!selected.isEmpty()) {
        copiesXml.clear();
        copyCoords.clear();

        TupGraphicsScene* currentScene = graphicsScene();
        if (currentScene) {
            int itemsCount = selected.size();

            // Sort the array of items by zValue
            for (int i=0; i<itemsCount-1; i++) {
                qreal iLevel = selected.at(i)->zValue();
                for (int j=i+1; j<itemsCount; j++) {
                    qreal jLevel = selected.at(j)->zValue();
                    if (jLevel < iLevel) {
                        selected.swapItemsAt(i, j);
                        iLevel = jLevel;
                    }
                }
            }

            int minX = 0;
            int maxX = 0;
            int minY = 0;
            int maxY = 0;

            // Processing every item to save them as XML
            for (int i=0; i<itemsCount; i++) {
                QGraphicsItem *item = selected.at(i);
                if (qgraphicsitem_cast<TControlNode *> (item))
                    continue;

                QDomDocument dom;
                dom.appendChild(dynamic_cast<TupAbstractSerializable *>(item)->toXml(dom));
                QDomElement root = dom.documentElement();
                QDomElement properties = root.firstChild().toElement();

                copiesXml << dom.toString();

                if (itemsCount == 1) { // One item selection
                    copyCoords << item->boundingRect().topLeft();

                    minX = 0;
                    maxX = item->boundingRect().width();
                    minY = 0;
                    maxY = item->boundingRect().height();
                } else { // Looking for the whole dimension of the selection
                    QPointF left = item->boundingRect().topLeft();
                    QPointF right = item->boundingRect().bottomRight();
                    copyCoords << left;

                    if (i == 0) {
                        minX = left.x();
                        maxX = right.x();
                        minY = left.y();
                        maxY = right.y();
                    } else {
                        int leftX = left.x();
                        int leftY = left.y();
                        if (leftX < minX)
                            minX = leftX;
                         if (leftY < minY)
                            minY = leftY;

                        int rightX = right.x();
                        int rightY = right.y();
                        if (rightX > maxX)
                            maxX = rightX;
                        if (rightY > maxY)
                            maxY = rightY;
                    }                    
                }

                // Paint it to clipbard
                QPixmap toPixmap(item->boundingRect().size().toSize());
                toPixmap.fill(Qt::transparent);

                QPainter painter(&toPixmap);
                painter.setRenderHint(QPainter::Antialiasing);

                QStyleOptionGraphicsItem opt;
                opt.state = QStyle::State_None;

                if (item->isEnabled())
                    opt.state |= QStyle::State_Enabled;
                if (item->hasFocus())
                    opt.state |= QStyle::State_HasFocus;
                if (item == currentScene->mouseGrabberItem())
                    opt.state |= QStyle::State_Sunken;

                opt.exposedRect = item->boundingRect();
                opt.levelOfDetail = 1;
                // opt.transform = item->sceneTransform();
                opt.palette = palette();

                item->paint(&painter, &opt, this);
                painter.end();

                QApplication::clipboard()->setPixmap(toPixmap);
            }

            TCONFIG->beginGroup("PaintArea");
            bool onMouse = TCONFIG->value("PasteOnMousePos", false).toBool();

            if (itemsCount == 1) { // One item selection
                if (onMouse)
                    centerCoord = QPointF((maxX - minX)/2, (maxY - minY)/2);
            } else {
                if (onMouse)
                    centerCoord = QPointF(minX + ((maxX - minX)/2), minY + ((maxY - minY)/2));
            }
        }
    } else {
        copyCurrentFrame();
    }
}

void TupPaintArea::pasteItems()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::pasteItems()]";
    #endif

    if (!copiesXml.isEmpty()) {
        QPointF currentPos = viewPosition();
        TupGraphicsScene* currentScene = graphicsScene();
        if (!menuOn)
            position = currentPos;

        int itemsCount = copiesXml.size();
        TCONFIG->beginGroup("PaintArea");
        bool onMouse = TCONFIG->value("PasteOnMousePos", false).toBool();

        for (int i=0; i<itemsCount; i++) {
            QString xml = copiesXml.at(i);
            #ifdef TUP_DEBUG
                qDebug() << "[TupPaintArea::pasteItems()] - xml:";
                qDebug() << xml;
            #endif

            TupFrame *frame = currentScene->currentFrame();
            if (frame) {
                int total = frame->graphicsCount();
                TupLibraryObject::ObjectType type = TupLibraryObject::Item;

                if (xml.startsWith("<svg")) {
                    type = TupLibraryObject::Svg;
                    total = frame->svgItemsCount();
                }

                QPointF pos = QPointF(0, 0);
                if (itemsCount == 1) { // One item selection
                    if (onMouse) { // Dynamic position
                        if (xml.startsWith("<group")) {
                            double x = currentPos.x() - (copyCoords.at(i).x() + centerCoord.x());
                            if (currentPos.x() >= copyCoords.at(i).x())
                                x = fabs(x);

                            double y = currentPos.y() - (copyCoords.at(i).y() + centerCoord.y());
                            if (currentPos.y() >= copyCoords.at(i).y())
                                y = fabs(y);

                            pos = QPointF(x, y);
                            xml = "<group f=\"1\"" + xml.right(xml.length() - 6);
                        } else {
                            double x = currentPos.x() - (copyCoords.at(i).x() + centerCoord.x());
                            if (currentPos.x() >= copyCoords.at(i).x())
                                x = fabs(x);

                            double y = currentPos.y() - (copyCoords.at(i).y() + centerCoord.y());
                            if (currentPos.y() >= copyCoords.at(i).y())
                                y = fabs(y);

                            pos = QPointF(x, y);

                            int initIndex = xml.indexOf("pos=");
                            int lastIndex = xml.indexOf(" ", initIndex);
                            QString newPos = "pos=\"(" + QString::number(x) + ", " + QString::number(y) + ")\"";
                            xml = xml.left(initIndex) + newPos + xml.right(xml.length() - lastIndex);
                        }
                    } else { // Same position
                        // Symbol - SVG
                        if (xml.startsWith("<symbol") || xml.startsWith("<svg")) {
                            QDomDocument dom;
                            dom.setContent(xml);
                            QDomElement root = dom.documentElement();
                            QDomElement properties = root.firstChild().toElement();

                            QPointF shift;
                            TupSvg2Qt::parsePointF(properties.attribute("pos"), shift);
                            if (shift != QPointF(0,0))
                                pos = shift;
                        }
                    }
                } else { // Several items selection
                    if (onMouse) { // Dynamic position
                        double x = currentPos.x() - centerCoord.x();
                        double y = currentPos.y() - centerCoord.y();
                        pos = QPointF(x, y);

                        int firstIndex = xml.indexOf(">");
                        QString init = xml.left(firstIndex) + " f=\"1\"";
                        xml = init + xml.right(xml.length() - firstIndex);
                    } else { // Same position
                        if (xml.startsWith("<symbol") || xml.startsWith("<svg")) {
                            QDomDocument dom;
                            dom.setContent(xml);
                            QDomElement root = dom.documentElement();
                            QDomElement properties = root.firstChild().toElement();
                            QPointF shift;
                            TupSvg2Qt::parsePointF(properties.attribute("pos"), shift);
                            if (shift != QPointF(0,0))
                                pos = shift;
                        }
                    }
                }

                TupProjectRequest event = TupRequestBuilder::createItemRequest(currentScene->currentSceneIndex(),
                                          currentScene->currentLayerIndex(),
                                          currentScene->currentFrameIndex(),
                                          total, pos, spaceMode, type,
                                          TupProjectRequest::Add, xml);
                emit requestTriggered(&event);
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupPaintArea::pasteItems()] - Fatal Error: Frame is NULL!";
                #endif
            }
        }
        menuOn = false;
    } else {
        pasteCurrentFrame();
    }
}

void TupPaintArea::multipasteObject(int pasteTotal)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::multipasteObject()]";
    #endif

    TupGraphicsScene* currentScene = graphicsScene();
    if (!currentScene) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPaintArea::multipasteObject()] - Fatal Error: graphicsScene() is NULL!";
        #endif
        return;
    }

    if (!menuOn)
        position = viewPosition();
   
    int layerIndex = currentScene->currentLayerIndex();
    int limit = 0;

    int itemsCount = copiesXml.size();
    TCONFIG->beginGroup("PaintArea");
    bool onMouse = TCONFIG->value("PasteOnMousePos", false).toBool();
    QPointF currentPos = viewPosition();

    int currentFrame = currentScene->currentFrameIndex();
    limit = currentFrame + pasteTotal;
    for (int i=currentFrame+1; i<=limit; i++) {
        foreach (QString xml, copiesXml) {
            TupLibraryObject::ObjectType type = TupLibraryObject::Item;
            int total = currentScene->currentFrame()->graphicsCount();
            if (project) {
                TupScene *scene = project->sceneAt(currentScene->currentSceneIndex());
                if (scene) {
                    TupLayer *layer = scene->layerAt(currentScene->currentLayerIndex());
                    if (layer) {
                        int framesCount = layer->framesCount();
                        int newFrameIndex = currentFrame + pasteTotal;
                        int distance = framesCount - (newFrameIndex + 1);

                        if (distance < 0) {
                            for (int i=framesCount; i<=newFrameIndex; i++) {
                                TupProjectRequest request = TupRequestBuilder::createFrameRequest(globalSceneIndex,
                                                            layerIndex, i, TupProjectRequest::Add,
                                                            tr("Frame"));
                                emit requestTriggered(&request);
                            }
                        }

                        if (xml.startsWith("<svg")) {
                            type = TupLibraryObject::Svg;
                            total = currentScene->currentFrame()->svgItemsCount();
                        }

                        QPointF pos = QPointF(0, 0);
                        if (itemsCount == 1) { // One item selection
                            if (onMouse) {
                                QPointF cursorPos = QPointF();
                                if (i < copyCoords.size())
                                    cursorPos = copyCoords.at(i);
                                else
                                    cursorPos = copyCoords.last();

                                double x = currentPos.x() - (cursorPos.x() + centerCoord.x());
                                if (currentPos.x() >= cursorPos.x())
                                    x = fabs(x);

                                double y = currentPos.y() - (cursorPos.y() + centerCoord.y());
                                if (currentPos.y() >= cursorPos.y())
                                    y = fabs(y);

                                pos = QPointF(x, y);
                            } else {
                                // Path - Image - SVG
                                if (xml.startsWith("<path") || xml.startsWith("<symbol") || xml.startsWith("<svg")) {
                                    QDomDocument dom;
                                    dom.setContent(xml);
                                    QDomElement root = dom.documentElement();
                                    QDomElement properties = root.firstChild().toElement();
                                    QPointF shift;
                                    TupSvg2Qt::parsePointF(properties.attribute("pos"), shift);
                                    if (shift != QPointF(0,0))
                                        pos = shift;
                                } else if (xml.startsWith("<group")) {
                                    QDomDocument dom;
                                    dom.setContent(xml);
                                    QDomElement root = dom.documentElement();
                                    QPointF shift;
                                    TupSvg2Qt::parsePointF(root.attribute("pos"), shift);
                                    if (shift != QPointF(0,0))
                                        pos = shift;
                                }
                            }
                        } else { // Several items selection
                            if (onMouse) {
                                double x = currentPos.x() - centerCoord.x();
                                double y = currentPos.y() - centerCoord.y();

                                pos = QPointF(x, y);
                            } else {
                                if (xml.startsWith("<path") || xml.startsWith("<symbol") || xml.startsWith("<svg")) {
                                    QDomDocument dom;
                                    dom.setContent(xml);
                                    QDomElement root = dom.documentElement();
                                    QDomElement properties = root.firstChild().toElement();
                                    QPointF shift;
                                    TupSvg2Qt::parsePointF(properties.attribute("pos"), shift);
                                    if (shift != QPointF(0,0))
                                        pos = shift;
                                } else if (xml.startsWith("<group")) {
                                    QDomDocument dom;
                                    dom.setContent(xml);
                                    QDomElement root = dom.documentElement();
                                    QPointF shift;
                                    TupSvg2Qt::parsePointF(root.attribute("pos"), shift);
                                    if (shift != QPointF(0,0))
                                        pos = shift;
                                }
                            }
                        }

                        TupProjectRequest event = TupRequestBuilder::createItemRequest(globalSceneIndex,
                                                                     layerIndex, i, total, pos, spaceMode, type,
                                                                     TupProjectRequest::Add, xml);
                        emit requestTriggered(&event);
                    } else {
                        #ifdef TUP_DEBUG
                           qDebug() << "[TupPaintArea::multipasteObject()] - Fatal Error: layer variable is NULL!";
                        #endif
                    }
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupPaintArea::multipasteObject()] - Fatal Error: scene variable is NULL!";
                    #endif
                }
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupPaintArea::multipasteObject()] - Fatal Error: project variable is NULL!";
                #endif
            }
         }
     }

     QString selection = QString::number(layerIndex) + "," + QString::number(layerIndex) + ","
                         + QString::number(limit) + "," + QString::number(limit);
     TupProjectRequest request = TupRequestBuilder::createFrameRequest(globalSceneIndex, currentScene->currentLayerIndex(), limit,
                                                                       TupProjectRequest::Select, selection);
     emit localRequestTriggered(&request);
     menuOn = false;
}

void TupPaintArea::pasteNextFive()
{
     multipasteObject(5);
}

void TupPaintArea::pasteNextTen()
{
     multipasteObject(10);
}

void TupPaintArea::pasteNextTwenty()
{
     multipasteObject(20);
}

void TupPaintArea::pasteNextFifty()
{
     multipasteObject(50);
}

void TupPaintArea::pasteNextHundred()
{
     multipasteObject(100);
}

void TupPaintArea::cutItems()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::cutItems()]";
    #endif

    copyItems();
    deleteItems();
}

void TupPaintArea::setNextFramesOnionSkinCount(int n)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::setNextFramesOnionSkinCount()]";
    #endif

    if (TupGraphicsScene* currentScene = graphicsScene())
        currentScene->setNextOnionSkinCount(n);
}

void TupPaintArea::setPreviousFramesOnionSkinCount(int n)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::setPreviousFramesOnionSkinCount()]";
    #endif

    if (TupGraphicsScene* currentScene = graphicsScene())
        currentScene->setPreviousOnionSkinCount(n);
}

void TupPaintArea::addSelectedItemsToLibrary()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::addSelectedItemsToLibrary()]";
    #endif

    QList<QGraphicsItem *> selected = scene()->selectedItems();

    if (selected.isEmpty()) {
        TOsd::self()->display(TOsd::Error, tr("No items selected"));
        return;
    }

    TupLibraryDialog dialog(project->getLibrary());
    foreach (QGraphicsItem *item, selected)
        dialog.addItem(item);

    if (dialog.exec() == QDialog::Accepted) {
        foreach (QGraphicsItem *item, selected) {
            if (TupAbstractSerializable *itemSerializable = dynamic_cast<TupAbstractSerializable *>(item)) {
                QString symName = dialog.symbolName(item) + ".tobj";

                QDomDocument doc;
                doc.appendChild(itemSerializable->toXml(doc));

                TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, 
                                            symName, TupLibraryObject::Item, spaceMode, doc.toString().toLocal8Bit(), QString());
                emit requestTriggered(&request);
            }
        }
    }
}

void TupPaintArea::requestItemMovement(QAction *action)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::requestItemMovement()]";
    #endif

    QList<QGraphicsItem *> selected = scene()->selectedItems();

    if (selected.isEmpty()) {
        TOsd::self()->display(TOsd::Error, tr("No items selected"));
        return;
    }
	
    TupGraphicsScene *currentScene = graphicsScene();
    if (!currentScene)
        return;

    TupFrame *currentFrame = currentScene->currentFrame();

    foreach (QGraphicsItem *item, selected) {
             TupLibraryObject::ObjectType type = TupLibraryObject::Item;
             int index = -1;
             if (TupSvgItem *svg = qgraphicsitem_cast<TupSvgItem *>(item)) {
                 type = TupLibraryObject::Svg;
                 index = currentFrame->indexOf(svg);
             } else {
                 index = currentFrame->indexOf(item);
             }

             if (index >= 0) {
                 bool ok;
                 int moveType = action->data().toInt(&ok);

                 if (ok) {
                     TupProjectRequest event = TupRequestBuilder::createItemRequest(currentScene->currentSceneIndex(),
                                               currentScene->currentLayerIndex(), currentScene->currentFrameIndex(), index, QPointF(), 
                                               spaceMode, type, TupProjectRequest::Move, moveType);
                     emit requestTriggered(&event);
                 } else {
                     #ifdef TUP_DEBUG
                         qDebug() << "[TupPaintArea::requestItemMovement()] - Fatal Error: Invalid action [ "
                                  << moveType << " ]";
                     #endif
                 }
             } else {
                 #ifdef TUP_DEBUG
                     qDebug() << "[TupPaintArea::requestItemMovement()] - Fatal Error: Invalid object index [ "
                              << index << " ]";
                 #endif
             }
    }
}

void TupPaintArea::updatePaintArea() 
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::updatePaintArea()]";
    #endif

    if (spaceMode == TupProject::FRAMES_MODE) {
        TupGraphicsScene* currentScene = graphicsScene();
        currentScene->drawCurrentPhotogram();
    } else if (spaceMode == TupProject::VECTOR_FG_MODE) {
        paintForeground();
    } else {
        paintBackground();
    }
}

void TupPaintArea::paintBackground()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::paintBackground()]";
    #endif

    TupGraphicsScene* currentScene = graphicsScene();
    currentScene->cleanWorkSpace();
    currentScene->drawSceneBackground(currentScene->currentFrameIndex());
}

void TupPaintArea::paintForeground()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::paintForeground()]";
    #endif

    TupGraphicsScene* currentScene = graphicsScene();
    currentScene->cleanWorkSpace();
    currentScene->drawVectorFg();
}

void TupPaintArea::setCurrentTool(TAction::ActionId tool)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::setCurrentTool()] - tool -> " << tool;
    #endif

    currentToolID = tool;
    canvasEnabled = true;
}

void TupPaintArea::updateSpaceContext()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::updateSpaceContext()]";
    #endif

    TupGraphicsScene* currentScene = graphicsScene();
    currentScene->setSpaceMode(project->spaceContext());
    spaceMode = project->spaceContext();
}

void TupPaintArea::setOnionFactor(double value)
{
    TupGraphicsScene* currentScene = graphicsScene();
    currentScene->setOnionFactor(value);
}

void TupPaintArea::keyPressEvent(QKeyEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::keyPressEvent()] - Current tool -> " << currentToolID;
        qDebug() << "[TupPaintArea::keyPressEvent()] - Key -> " << event->key();
        qDebug() << "[TupPaintArea::keyPressEvent()] - Code -> " << event->text();
    #endif

    if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete) {
        deleteItems();
        return;
    }

    if (event->key() == Qt::Key_Return) {
        emit newPerspective(4);
        return;
    }

    if (event->modifiers() == Qt::ControlModifier) {
        if (event->key() == Qt::Key_C) {
            copyItems();
            return;
        }

        if (event->key() == Qt::Key_X) {
            copyItems();
            deleteItems();
            return;
        }

        if (event->key() == Qt::Key_V) {
            pasteItems();
            return;
        }

        if (event->key() == Qt::Key_2) {
            emit newPerspective(1);
            return;
        }

        if (event->key() == Qt::Key_3) {
            emit newPerspective(2);
            return;
        }
    }

#ifdef Q_OS_UNIX // Zoom In
    if ((event->text().compare("+") == 0) || (event->key() == 191) || (event->key() == 61)) {
        emit zoomIn();
        return;
    }
#else // Windows OS - Zoom In
    if ((event->text().compare("+") == 0) || (event->key() == 191) || (event->key() == 61) || (event->key() == 161)) {
        emit zoomIn();
        return;
    }
#endif

    if (event->key() == Qt::Key_Plus) {
        if (event->modifiers() == Qt::NoModifier) {
            emit zoomIn();
            return;
        }
    }

    // Key is next to the + character in the keyboard
    if ((event->text().compare("-") == 0) || (event->key() == 39)) {
        emit zoomOut();
        return;
    }

    if (event->key() == Qt::Key_Minus) {
        if (event->modifiers() == Qt::NoModifier) {
            emit zoomOut();
            return;
        }
    }

    if (event->key() == Qt::Key_E) {
        emit eyeDropperLaunched();
        return;
    }

    if (event->key() == Qt::Key_PageUp) {
        if (event->modifiers() == Qt::ControlModifier)
            removeCurrentFrame();
        else
            goOneFrameBack();
        return;
    }

    if (event->key() == Qt::Key_PageDown) {
        if (event->modifiers() == Qt::ControlModifier)
            copyFrameForward();
        else  
            goOneFrameForward();
        return;
    }

    // Clone frame
    if (event->key() == Qt::Key_8) {
        copyFrameForward();
        return;
    }

    // Redundant shortcut for "Add Frame" feature (as 9)
    if (event->key() == Qt::Key_Insert) {
        TupGraphicsScene *gScene = graphicsScene();
        int sceneIndex = gScene->currentSceneIndex();
        int layerIndex = gScene->currentLayerIndex();
        int frameIndex = gScene->currentFrameIndex() + 1;

        TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, frameIndex,
                                                                          TupProjectRequest::Add, tr("Frame"));
        emit requestTriggered(&request);

        QString selection = QString::number(layerIndex) + "," + QString::number(layerIndex) + ","
                            + QString::number(frameIndex) + "," + QString::number(frameIndex);

        request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, frameIndex, TupProjectRequest::Select, selection);
        emit localRequestTriggered(&request);

        return;
    }

    if (currentToolID == TAction::Polyline) {
        if (event->key() == Qt::Key_X)
            emit closePolyLine();
    }

    TupPaintAreaBase::keyPressEvent(event);
}

/*
void TupPaintArea::keyReleaseEvent(QKeyEvent *event)
{
    TupPaintAreaBase::keyReleaseEvent(event);
}
*/

void TupPaintArea::goOneFrameBack()
{
    TupGraphicsScene *scene = graphicsScene();

    if (scene->currentFrameIndex() > 0) {
        int layerIndex = scene->currentLayerIndex();
        int frameIndex = scene->currentFrameIndex() - 1;
        QString selection = QString::number(layerIndex) + "," + QString::number(layerIndex) + ","
                            + QString::number(frameIndex) + "," + QString::number(frameIndex);

        TupProjectRequest request = TupRequestBuilder::createFrameRequest(scene->currentSceneIndex(), layerIndex,
                                                                          frameIndex, TupProjectRequest::Select, selection);
        emit localRequestTriggered(&request);
    }
}

void TupPaintArea::goOneFrameForward()
{
    TupGraphicsScene *scene = graphicsScene();
    int framesCount = scene->getFramesCount();
    int frameIndex = scene->currentFrameIndex() + 1;

    if (frameIndex == framesCount) {
        TupProjectRequest request = TupRequestBuilder::createFrameRequest(scene->currentSceneIndex(),
                                                     scene->currentLayerIndex(),
                                                     frameIndex,
                                                     TupProjectRequest::Add, tr("Frame"));
        emit requestTriggered(&request);
        frameIndex = scene->currentFrameIndex();
    }

    goToFrame(frameIndex);
}

void TupPaintArea::copyCurrentFrame()
{
    TupGraphicsScene *gScene = graphicsScene();
    int sceneIndex = gScene->currentSceneIndex();
    int layerIndex = gScene->currentLayerIndex();
    int frameIndex = gScene->currentFrameIndex();

    copyFrameName = tr("Frame");
    TupScene *scene = project->sceneAt(sceneIndex);
    if (scene) {
        TupLayer *layer = scene->layerAt(layerIndex);
        if (layer) {
            TupFrame *frame = layer->frameAt(frameIndex);
            if (frame) {
                copyFrameName = frame->getFrameName();
                TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, frameIndex,
                                                                                  TupProjectRequest::Copy);
                emit localRequestTriggered(&request);
                copyIsValid = true;
            }
        }
    }
}

void TupPaintArea::pasteCurrentFrame()
{
    if (copyIsValid) {
        TupGraphicsScene *gScene = graphicsScene();
        int sceneIndex = gScene->currentSceneIndex();
        int layerIndex = gScene->currentLayerIndex();
        int frameIndex = gScene->currentFrameIndex();

        TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, frameIndex,
                                                                          TupProjectRequest::Paste);
        emit localRequestTriggered(&request);

        request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, frameIndex, TupProjectRequest::Rename,
                                                        copyFrameName);
        emit requestTriggered(&request);
    }
}

void TupPaintArea::copyFrameForward()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::copyFrameForward()]";
    #endif

    TupGraphicsScene *gScene = graphicsScene();
    int sceneIndex = gScene->currentSceneIndex();
    int layerIndex = gScene->currentLayerIndex();
    int frameIndex = gScene->currentFrameIndex();

    TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, frameIndex,
                                                                      TupProjectRequest::Extend, 1);
    emit requestTriggered(&request);
}

void TupPaintArea::removeCurrentFrame()
{
    TCONFIG->beginGroup("General");
    bool ask = TCONFIG->value("ConfirmRemoveFrame", true).toBool();
    if (ask) {
        TOptionalDialog dialog(tr("Do you want to remove this frame?"), tr("Confirmation"), true, false, this);
        dialog.setModal(true);
        dialog.move(static_cast<int> ((screenWidth - dialog.sizeHint().width()) / 2),
                    static_cast<int> ((screenHeight - dialog.sizeHint().height()) / 2));

        if (dialog.exec() == QDialog::Rejected)
            return;

        TCONFIG->beginGroup("General");
        TCONFIG->setValue("ConfirmRemoveFrame", dialog.shownAgain());
        TCONFIG->sync();
    }

    TupGraphicsScene *gScene = graphicsScene();
    int sceneIndex = gScene->currentSceneIndex();
    int layerIndex = gScene->currentLayerIndex();
    int frameIndex = gScene->currentFrameIndex();

    TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, frameIndex,
                                                                      TupProjectRequest::RemoveSelection, "1,1:0");
    emit requestTriggered(&request);
}

void TupPaintArea::goToFrame(int index)
{
    TupGraphicsScene *scene = graphicsScene();
    int layerIndex = scene->currentLayerIndex();
    QString selection = QString::number(layerIndex) + "," + QString::number(layerIndex) + ","
                        + QString::number(index) + "," + QString::number(index);

    TupProjectRequest request = TupRequestBuilder::createFrameRequest(scene->currentSceneIndex(),
                                                   layerIndex, index, TupProjectRequest::Select, selection);
    emit localRequestTriggered(&request);
}

void TupPaintArea::goToFrame(int frameIndex, int layerIndex, int sceneIndex)
{
    QString selection = QString::number(layerIndex) + "," + QString::number(layerIndex) + ","
                        + QString::number(frameIndex) + "," + QString::number(frameIndex);

    TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, frameIndex,
                                                                      TupProjectRequest::Select, selection);
    emit localRequestTriggered(&request);
}

void TupPaintArea::goOneLayerBack()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::goOneLayerBack()]";
    #endif

    TupGraphicsScene *gScene = graphicsScene();
    int sceneIndex = gScene->currentSceneIndex();
    int layerIndex = gScene->currentLayerIndex();
    int frameIndex = gScene->currentFrameIndex();

    if (layerIndex > 0) {
        layerIndex--;
        TupScene *scene = gScene->currentScene();
        TupLayer *layer = scene->layerAt(layerIndex);
        int framesTotal = layer->framesCount();
        if (frameIndex >= framesTotal) {
            for (int i=framesTotal; i<=frameIndex; i++) {
                 TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, i,
                                                                 TupProjectRequest::Add, tr("Frame"));
                 emit requestTriggered(&request);
            }
        }
        goToFrame(frameIndex, layerIndex, sceneIndex);
    }
}

void TupPaintArea::goOneLayerForward()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::goOneLayerForward()]";
    #endif

    TupGraphicsScene *gScene = graphicsScene();
    int sceneIndex = gScene->currentSceneIndex();
    int layerIndex = gScene->currentLayerIndex();
    int frameIndex = gScene->currentFrameIndex();

    layerIndex++;
    TupScene *scene = gScene->currentScene();
    if (layerIndex < scene->layersCount()) {
        TupLayer *layer = scene->layerAt(layerIndex);
        int framesTotal = layer->framesCount();
        if (frameIndex >= framesTotal) {
            for (int i=framesTotal; i<=frameIndex; i++) {
                 TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, i,
                                                                 TupProjectRequest::Add, tr("Frame"));
                 emit requestTriggered(&request);
            }
        }
        goToFrame(frameIndex, layerIndex, sceneIndex);
    }
}

void TupPaintArea::goToScene(int sceneIndex)
{
    TupProjectRequest request = TupRequestBuilder::createSceneRequest(sceneIndex, TupProjectRequest::Select);
    emit localRequestTriggered(&request);
}

int TupPaintArea::currentSceneIndex()
{
    return graphicsScene()->currentSceneIndex();
}

int TupPaintArea::currentLayerIndex()
{
    return graphicsScene()->currentLayerIndex();
}

int TupPaintArea::currentFrameIndex()
{
    return graphicsScene()->currentFrameIndex();
}

void TupPaintArea::resetWorkSpaceCenter(const QSize projectSize)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::resetWorkSpaceCenter()]";
    #endif

    int centerX = projectSize.width()/2;
    int centerY = projectSize.height()/2;

    centerOn(QPointF(centerX, centerY));
    setSceneRect(0, 0, projectSize.width(), projectSize.height());
}

void TupPaintArea::updateLoadingFlag(bool flag)
{
    graphicsScene()->updateLoadingFlag(flag);
}

void TupPaintArea::dragEnterEvent(QDragEnterEvent *e)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::dragEnterEvent()]";
    #endif

    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();
}

void TupPaintArea::dragMoveEvent(QDragMoveEvent *e)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::dragMoveEvent()]";
    #endif
    */

    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();
}

void TupPaintArea::dropEvent(QDropEvent *e)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::dropEvent()]";
    #endif

    QString objectPath = e->mimeData()->text().trimmed();

    if (!objectPath.isEmpty()) {
        QString lowercase = objectPath.toLower();
        if (lowercase.startsWith("http")) {
            getWebAsset(objectPath);
        } else if (lowercase.startsWith("file")) {
            getLocalAsset(objectPath);
        } else if (lowercase.startsWith("asset")) {
            emit libraryAssetDragged();
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupPaintArea::dropEvent()] - Warning: The dropped object is unsupported!";
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupPaintArea::dropEvent()] - Warning: Asset path is empty!";
        #endif
    }
}

void TupPaintArea::importExternalObjects(const QString &tempPath, const QString &projectName, bool scenesSelected,
                                         QList<int> sceneIndexes, QList<bool> libraryFlags,
                                         QList<TupProjectScanner::LibraryObject> objects, const QString &folder)
{
    foreach(TupProjectScanner::LibraryObject object, objects) {
        if (project->getLibrary()->exists(object.key)) { // Object key already exists... renaming it!
            QString key = project->getLibrary()->getItemKey(object.key);
            if (scenesSelected) {
                // Checking if the library is required by at least one scene
                foreach(int index, sceneIndexes) {
                    if (libraryFlags.at(index)) {
                        // Replacing keys in every scene
                        QString oldKey = "<symbol id=\"" + object.key + "\">";
                        key = "<symbol id=\"" + key + "\">";
                        projectScanner->updateLibraryKey(index, oldKey, key);
                    }
                }
            }
        }

        QString typeFolder = "";
        if (object.type == TupLibraryObject::Image)
            typeFolder = "/images/";
        if (object.type == TupLibraryObject::Svg)
            typeFolder = "/svg/";
        if (object.type == TupLibraryObject::Item)
            typeFolder = "/obj/";
        if (object.type == TupLibraryObject::Audio)
            typeFolder = "/audio/";

        emit libraryAssetImported(tempPath + projectName + typeFolder + object.path, object.type, folder);
    }
}

void TupPaintArea::importLocalProject(const QString &objectPath, bool onlyLibrary)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::importLocalProject()] - objectPath -> " << objectPath;
        qDebug() << "[TupPaintArea::importLocalProject()] - onlyLibrary -> " << onlyLibrary;
    #endif

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QString tempFolder = TAlgorithm::randomString(10);
    QString tempPath = CACHE_DIR + tempFolder + "/";
    projectScanner = new TupProjectScanner;
    bool result = projectScanner->read(objectPath, tempFolder);

    QApplication::restoreOverrideCursor();
    if (result) {
        QList<QString> scenes = projectScanner->getSceneLabels();
        bool isLibraryEmpty = false;
        if (!projectScanner->isLibraryEmpty()) {
            isLibraryEmpty = true;
        } else {
            if (onlyLibrary) {
                TOsd::self()->display(TOsd::Error, tr("Sorry, no library was found!"), 3000);

                return;
            }
        }

        QString projectName = projectScanner->getProjectName();
        TupProjectImporterDialog *dialog = new TupProjectImporterDialog(projectName, scenes, isLibraryEmpty);
        if (dialog->exec() == QDialog::Accepted) {
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

            QList<int> sceneIndexes = dialog->scenes();
            bool includeLibrary = dialog->isLibraryIncluded();
            QList<bool> libraryFlags = projectScanner->getSceneLibraryFlags();
            bool scenesSelected = !sceneIndexes.isEmpty();

            if (scenesSelected) {
                QSize newSize = projectScanner->getProjectDimension();
                QSize currentSize = project->getDimension();

                if (newSize != currentSize) {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupPaintArea::importLocalProject()] - Current project dimension -> " << currentSize;
                        qDebug() << "[TupPaintArea::importLocalProject()] - Imported project dimension -> " << newSize;
                    #endif

                    QMessageBox msgBox;
                    msgBox.setWindowTitle(tr("Project Dimension Conflict"));
                    msgBox.setIcon(QMessageBox::Question);
                    msgBox.setText(tr("Imported scenes require a different dimension than the current to fit accurately."));
                    msgBox.setInformativeText(tr("Do you want to resize your current project?"));
                    msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
                    msgBox.setDefaultButton(QMessageBox::Ok);
                    msgBox.show();

                    msgBox.move(static_cast<int> ((screenWidth - msgBox.width()) / 2),
                                static_cast<int> ((screenHeight - msgBox.height()) / 2));

                    if (msgBox.exec() == QMessageBox::Yes) {
                        msgBox.close();
                        emit resizeActionRequested(newSize);
                    }
                }

                // Checking if the library is required by at least one scene
                foreach(int index, sceneIndexes) {
                    if (libraryFlags.at(index)) {
                        if (!includeLibrary)
                            includeLibrary = true;
                    }
                }
            }

            if (includeLibrary) {
                // Processing library assets
                TupProjectScanner::Folder libraryAssets = projectScanner->getLibrary();
                importExternalObjects(tempPath, projectName, scenesSelected, sceneIndexes, libraryFlags,
                                      libraryAssets.objects, "");

                // Processing library folders
                QList<TupProjectScanner::Folder> folders = libraryAssets.folders;
                foreach(TupProjectScanner::Folder folder, folders) {
                    TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, folder.key,
                                                                                        TupLibraryObject::Folder);
                    emit requestTriggered(&request);

                    QList<TupProjectScanner::LibraryObject> objects = folder.objects;
                    importExternalObjects(tempPath, projectName, scenesSelected, sceneIndexes, libraryFlags, objects,
                                          folder.key);
                }
            }

            if (scenesSelected) { // Importing selected scenes...
                QList<QString> sceneNames = projectScanner->getSceneLabels();
                QList<QString> sceneContents = projectScanner->getSceneContents();
                int scenesCount = project->scenesCount();
                QColor bgColor = Qt::white;
                foreach(int index, sceneIndexes) {
                    project->createScene(sceneNames.at(index), scenesCount, true)->fromXml(sceneContents.at(index));
                    if (projectScanner->getProjectVersion() < 1.1) {
                        bgColor = projectScanner->getProjectBgColor();
                        project->sceneAt(scenesCount)->setBgColor(bgColor);
                    } else {
                        bgColor = project->sceneAt(scenesCount)->getBgColor();
                    }

                    emit sceneCreated(scenesCount);
                    scenesCount++;
                }

                project->setCurrentBgColor(bgColor);
                setBgColor(bgColor);

                TupProjectRequest request = TupRequestBuilder::createSceneRequest(scenesCount - 1, TupProjectRequest::Select);
                emit localRequestTriggered(&request);
            }
            QApplication::restoreOverrideCursor();

            TOsd::self()->display(TOsd::Info, tr("Project assets imported successfully!"));
        }

        // Removing temporary folder
        QDir assetsDir(tempPath);
        #ifdef TUP_DEBUG
            qDebug() << "[TupPaintArea::importLocalProject()] - Removing temporary folder -> " << tempPath;
        #endif
        if (assetsDir.exists()) {
            if (!assetsDir.removeRecursively()) {
                #ifdef TUP_DEBUG
                    qWarning() << "[TupPaintArea::importLocalProject()] - Error: Can't remove temporary folder -> " << tempPath;
                #endif
            }
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPaintArea::importLocalProject()] - Fatal Error: Can't open TUP source file -> " << objectPath;
        #endif
        TOsd::self()->display(TOsd::Error, tr("Sorry, TUP source file is invalid!"));
    }

    delete projectScanner;
}

void TupPaintArea::getLocalAsset(const QString &path)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::getLocalAsset(QString)] - path -> " << path;
    #endif

    QString objectPath = path;
    #ifdef Q_OS_WIN
        objectPath = objectPath.replace("file:///", "");
    #else
        objectPath = objectPath.replace("file://", "");
    #endif

    QString lowercase = objectPath.toLower();

    if (lowercase.endsWith(".tup")) { // Importing elements from external TUP source file
        importLocalProject(objectPath);
    } else { // Processing library assets
        TupLibraryObject::ObjectType type = TupLibraryObject::None;
        if (lowercase.endsWith(".jpeg") || lowercase.endsWith(".jpg") || lowercase.endsWith(".png") || lowercase.endsWith(".webp"))
            type = TupLibraryObject::Image;
        else if (lowercase.endsWith(".svg"))
            type = TupLibraryObject::Svg;
        else if (lowercase.endsWith(".tobj"))
            type = TupLibraryObject::Item;
        else if (lowercase.endsWith(".mp3") || lowercase.endsWith(".wav"))
            type = TupLibraryObject::Audio;
        else if (lowercase.endsWith(".mp4") || lowercase.endsWith(".mov"))
            type = TupLibraryObject::Video;

        if (type != TupLibraryObject::None)
            emit localAssetDropped(objectPath, type);
        else
            TOsd::self()->display(TOsd::Error, tr("Sorry, file format not supported!"));
    }
}

void TupPaintArea::getWebAsset(const QString &urlPath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::getWebAsset()] - url -> " << urlPath;
    #endif

    if (webLock)
        return;

    webLock = true;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(processWebAsset(QNetworkReply*)));
    connect(manager, SIGNAL(finished(QNetworkReply*)), manager, SLOT(deleteLater()));

    QUrl url(urlPath);
    int index = urlPath.lastIndexOf("/") + 1;
    webAssetName = urlPath.right(urlPath.length() - index);

    QNetworkRequest request = QNetworkRequest();
    request.setRawHeader("User-Agent", BROWSER_FINGERPRINT);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    request.setUrl(url);

    QNetworkReply *reply = manager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));

    reply->setParent(manager);
    reply = manager->get(request);

    #ifdef TUP_DEBUG
        qDebug() << "---";
    #endif
}

void TupPaintArea::processWebAsset(QNetworkReply *reply)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPaintArea::processWebAsset()]";
    #endif

    QByteArray data = reply->readAll();
    if (data.size() > 0) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPaintArea::processWebAsset()] - Saving web asset...";
            qDebug() << "[TupPaintArea::processWebAsset()] - Asset size -> " << data.size();
        #endif

        QString content = reply->rawHeader("Content-Disposition");
        int index = content.indexOf("=") + 1;
        QString filename = content.right(content.length() - index);
        filename = filename.replace("\'","");
        filename = filename.replace("\"","");
        filename = filename.replace("\\","");

        if (filename.isEmpty())
            filename = webAssetName;

        QFileInfo info(filename);
        QString base = info.baseName();
        QString extension = info.suffix();
        if (base.length() > 20)
            filename = base.left(20) + "." + extension;
        else
            filename = info.fileName().toLower();

        extension = extension.toUpper();
        QString lowercase = filename.toLower();
        TupLibraryObject::ObjectType type = TupLibraryObject::None;
        if (lowercase.endsWith(".jpeg") || lowercase.endsWith(".jpg") || lowercase.endsWith(".png") || lowercase.endsWith(".webp"))
            type = TupLibraryObject::Image;
        else if (lowercase.endsWith(".svg"))
            type = TupLibraryObject::Svg;
        else if (lowercase.endsWith(".tobj"))
            type = TupLibraryObject::Item;
        else if (lowercase.endsWith(".mp3") || lowercase.endsWith(".wav"))
            type = TupLibraryObject::Audio;
        else if (lowercase.endsWith(".mp4") || lowercase.endsWith(".mov"))
            type = TupLibraryObject::Video;

        if (type != TupLibraryObject::None) {
            filename = project->getLibrary()->getItemKey(filename);
            emit webAssetDropped(filename, extension, type, data);
        } else {
            TOsd::self()->display(TOsd::Error, tr("Sorry, file format not supported!"));
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPaintArea::processWebAsset()] - Fatal Error: No answer from server!";
        #endif
    }

    QApplication::restoreOverrideCursor();
    webLock = false;
}

void TupPaintArea::slotError(QNetworkReply::NetworkError error)
{
    TOsd::self()->display(TOsd::Error, tr("Network Fatal Error. Please, contact us!"));

    switch (error) {
        case QNetworkReply::HostNotFoundError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupPaintArea::slotError()] - Network Error: Host not found";
             #endif
             }
        break;
        case QNetworkReply::TimeoutError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupPaintArea::slotError()] - Network Error: Time out!";
             #endif
             }
        break;
        case QNetworkReply::ConnectionRefusedError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupPaintArea::slotError()] - Network Error: Connection Refused!";
             #endif
             }
        break;
        case QNetworkReply::ContentNotFoundError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupPaintArea::slotError()] - Network Error: Content not found!";
             #endif
             }
        break;
        case QNetworkReply::UnknownNetworkError:
        default:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupPaintArea::slotError()] - Network Error: Unknown Network error!";
             #endif
             }
        break;
    }

    QApplication::restoreOverrideCursor();
    webLock = false;
}
