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

TupPaintArea::TupPaintArea(TupProject *work, QWidget *parent) : TupPaintAreaBase(parent, work->dimension(), work->library())
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupPaintArea()]";
        #else
            TINIT;
        #endif
    #endif

    setAccessibleName("WORKSPACE");
    project = work;
    canvasEnabled = false;
    globalSceneIndex = 0;
    deleteMode = false;
    menuOn = false;
    copyIsValid = false;
    currentTool = tr("Pencil");

    setBgColor(work->bgColor());

    setCurrentScene(0);
    graphicsScene()->setCurrentFrame(0, 0);
}

TupPaintArea::~TupPaintArea()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[~TupPaintArea()]";
        #else
            TEND;
        #endif
    #endif

    graphicsScene()->clear();
    delete graphicsScene();
}

void TupPaintArea::setCurrentScene(int index)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupPaintArea::setCurrentScene()]";
        #else
            T_FUNCINFO << "Scene index: " << index;
        #endif
    #endif

    if (project->scenesCount() > 0) {
        TupScene *scene = project->sceneAt(index);
        if (scene) {
            globalSceneIndex = index;
            graphicsScene()->setCurrentScene(scene);
        } else {
            if (project->scenesCount() == 1) {
                setDragMode(QGraphicsView::NoDrag);
                globalSceneIndex = 0;
                graphicsScene()->setCurrentScene(nullptr);
            } else {
                #ifdef TUP_DEBUG
                    QString msg1 = "TupPaintArea::setCurrentScene() - [ Fatal Error ] -  No scenes available. Invalid index -> " + QString::number(index);
                    QString msg2 = "TupPaintArea::setCurrentScene() - Scenes total -> " + QString::number(project->scenesCount());
                    #ifdef Q_OS_WIN
                        qDebug() << msg1;
                        qDebug() << msg2;
                    #else
                        tError() << msg1;
                        tError() << msg2;
                    #endif
                #endif
            }
        }
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupPaintArea::setCurrentScene() - No scenes available!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }
}

void TupPaintArea::mousePressEvent(QMouseEvent *event)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupPaintArea::mousePressEvent()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (!canvasEnabled)
        return;

    TupFrame *frame = graphicsScene()->currentFrame();
    if (frame) {
        if (frame->isLocked()) {
            #ifdef TUP_DEBUG
                QString msg = "TupPaintArea::mousePressEvent() - Frame is locked!";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tFatal() << msg;
                #endif
            #endif
            return;
        }
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupPaintArea::mousePressEvent() - Frame is NULL!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tFatal() << msg;
            #endif
        #endif
        return;
    }

    if (currentTool.compare(tr("Nodes Selection")) == 0) {
        // If a node is the target... abort!
        if (event->buttons() == Qt::RightButton) {
            if (qgraphicsitem_cast<TControlNode *>(scene()->itemAt(mapToScene(event->pos()), QTransform())))
                return;
        }
    }

    if (event->buttons() == Qt::RightButton) {
        if (currentTool.compare(tr("PolyLine")) == 0) {
            emit closePolyLine();
            return;
        }
        if (currentTool.compare(tr("Line")) == 0) {
            emit closeLine();
            return;
        }
    }

    if (currentTool.compare(tr("Object Selection")) == 0) {
        if (event->buttons() == Qt::RightButton) {
            // If a node is the target... abort!
            if (qgraphicsitem_cast<Node *>(scene()->itemAt(mapToScene(event->pos()), QTransform())))
                return;

            /* SQA: This code seems to be useless
            if (QGraphicsItem *item = scene()->itemAt(mapToScene(event->pos()), QTransform())) {
                if (item->opacity() == 1) // If target is part of the current frame
                    item->setFlag(QGraphicsItem::ItemIsSelectable, true);
                else 
                    return; // If target is NOT part of the current frame
            }
            */

            QMenu *menu = new QMenu(tr("Drawing area"));
            menu->addAction(kApp->findGlobalAction("undo"));
            menu->addAction(kApp->findGlobalAction("redo"));

            bool activeSelection = !scene()->selectedItems().isEmpty();

            if (activeSelection) {
                menu->addSeparator();
                menu->addAction(tr("Cut"), this, SLOT(cutItems()), QKeySequence(tr("Ctrl+X")));
                // menu->addAction(tr("Copy"), this, SLOT(copyItems()), QKeySequence(tr("Ctrl+C")));
                menu->addAction(tr("Copy"), this, SLOT(copyItems()));
            }

            if (!copiesXml.isEmpty()) {
                if (!activeSelection)
                    menu->addSeparator();
                // menu->addAction(tr("Paste"), this, SLOT(pasteItems()), QKeySequence(tr("Ctrl+V")));
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
                // menu->addAction(tr("Delete"), this, SLOT(deleteItems()), QKeySequence(Qt::Key_Delete));
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

void TupPaintArea::frameResponse(TupFrameResponse *response)
{
    #ifdef TUP_DEBUG
        QString msg = "TupPaintArea::frameResponse() - [" + QString::number(response->sceneIndex()) 
                      + ", " + QString::number(response->layerIndex()) + ", " 
                      + QString::number(response->frameIndex()) + "] | request -> "
                      + QString::number(response->action());
        #ifdef Q_OS_WIN
            qDebug() << msg;
        #else
            tDebug() << msg;
        #endif
    #endif

    TupGraphicsScene *guiScene = graphicsScene();
    if (!guiScene->currentScene()) {
        #ifdef TUP_DEBUG
            QString msg = "TupPaintArea::frameResponse() - Fatal error: No TupScene available!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return;
    }

    if (!guiScene->userIsDrawing()) {
        switch (response->action()) {
            case TupProjectRequest::PasteSelection:
            case TupProjectRequest::RemoveSelection:
            case TupProjectRequest::ReverseSelection:
            case TupProjectRequest::Exchange:
              {
                  if (spaceMode == TupProject::FRAMES_EDITION)
                      guiScene->drawCurrentPhotogram();
              }
            break;
            case TupProjectRequest::Add:
            case TupProjectRequest::Select:
            case TupProjectRequest::Paste:
            case TupProjectRequest::Reset:
              {
                  if (response->action() == TupProjectRequest::Select) {
                      if (guiScene->currentFrameIndex() != response->frameIndex())
                          emit frameChanged(response->frameIndex());
                  } else {
                      emit frameChanged(response->frameIndex());
                  }
                  guiScene->setCurrentFrame(response->layerIndex(), response->frameIndex());

                  if (spaceMode == TupProject::FRAMES_EDITION) {
                      guiScene->drawPhotogram(response->frameIndex(), true);
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
                  QString msg = "TupPaintArea::frameResponse() - Action not recognized -> " + QString::number(response->action());
                  #ifdef Q_OS_WIN
                      qDebug() << msg;
                  #else
                      tWarning() << msg;
                  #endif
              #endif
            break;
        }
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupPaintArea::frameResponse() - isDrawing() == true! - No action taken!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tFatal() << msg;
            #endif
        #endif
    }

    guiScene->frameResponse(response);
}

void TupPaintArea::layerResponse(TupLayerResponse *response)
{
    #ifdef TUP_DEBUG
        QString msg = "TupPaintArea::layerResponse() - [" + QString::number(response->sceneIndex()) + ", " + QString::number(response->layerIndex()) + "]";
        #ifdef Q_OS_WIN
            qDebug() << msg;
        #else
            tDebug() << msg;
        #endif
    #endif

    TupGraphicsScene *guiScene = graphicsScene();
    if (!guiScene->currentScene())
        return;

    int frameIndex = guiScene->currentFrameIndex();

    switch (response->action()) {
        case TupProjectRequest::Add:
          {
              if (response->mode() == TupProjectResponse::Redo || response->mode() == TupProjectResponse::Undo) {
                  if (spaceMode == TupProject::FRAMES_EDITION)
                      guiScene->drawCurrentPhotogram();
              }
              return;
          }
        case TupProjectRequest::Remove:
          {
              TupScene *scene = project->sceneAt(globalSceneIndex);

              if (scene->layersCount() > 1) {
                  if (response->layerIndex() != 0)
                      guiScene->setCurrentFrame(response->layerIndex() - 1, frameIndex);
                  else
                      guiScene->setCurrentFrame(response->layerIndex() + 1, frameIndex);

                  if (spaceMode == TupProject::FRAMES_EDITION) {
                      guiScene->drawCurrentPhotogram();
                  } else {
                      guiScene->cleanWorkSpace();
                      guiScene->drawSceneBackground(frameIndex);
                  }
              } else {
                  if (scene->layersCount() == 1) {
                      guiScene->setCurrentFrame(0, frameIndex);
                      if (spaceMode == TupProject::FRAMES_EDITION) {
                          guiScene->drawCurrentPhotogram();
                      } else {
                          guiScene->cleanWorkSpace();
                          guiScene->drawSceneBackground(frameIndex);
                      }
                  }
              }

              viewport()->update();
          }
        break;
        case TupProjectRequest::UpdateOpacity:
          {
              guiScene->drawCurrentPhotogram();
              viewport()->update(scene()->sceneRect().toRect());
          }
        break;
        case TupProjectRequest::TupProjectRequest::View:
          {
              guiScene->updateLayerVisibility(response->layerIndex(), response->arg().toBool());
              if (spaceMode == TupProject::FRAMES_EDITION) {
                  guiScene->drawCurrentPhotogram();
              } else {
                  guiScene->cleanWorkSpace();
                  guiScene->drawSceneBackground(frameIndex);
              }
              viewport()->update(scene()->sceneRect().toRect());
          }
        break;
        case TupProjectRequest::TupProjectRequest::Move:
          {
              guiScene->setCurrentFrame(response->arg().toInt(), frameIndex);
              if (spaceMode == TupProject::FRAMES_EDITION) {
                  guiScene->drawCurrentPhotogram();
              } else {
                  guiScene->cleanWorkSpace();
                  guiScene->drawSceneBackground(frameIndex);
              }
              viewport()->update(scene()->sceneRect().toRect());
          }
        break;
        default:
          {
              if (spaceMode == TupProject::FRAMES_EDITION) {
                  guiScene->drawCurrentPhotogram();
              } else {
                  guiScene->cleanWorkSpace();
                  guiScene->drawSceneBackground(frameIndex);
              }
              viewport()->update(scene()->sceneRect().toRect());
          }
          break;
    }

    guiScene->layerResponse(response);
}

void TupPaintArea::sceneResponse(TupSceneResponse *event)
{
    #ifdef TUP_DEBUG
        QString msg = "TupPaintArea::sceneResponse() - [" + QString::number(event->sceneIndex()) + "]";
        #ifdef Q_OS_WIN
            qDebug() << msg;
        #else
            tDebug() << msg;
        #endif
    #endif

    TupGraphicsScene *guiScene = graphicsScene();

    if (!guiScene->currentScene())
        return;

    if (!guiScene->userIsDrawing()) {
        switch(event->action()) {
            case TupProjectRequest::Select:
              {
                  if (event->sceneIndex() >= 0) {
                      if (project->scenesCount() == 1)
                          setCurrentScene(0);
                      else
                          setCurrentScene(event->sceneIndex());
                  }
              }
            break;
            case TupProjectRequest::Remove:
              {
                  if (project->scenesCount() > 0)
                      setCurrentScene(event->sceneIndex() - 1);
              }
            break;
            case TupProjectRequest::Reset:
              {
                  setCurrentScene(event->sceneIndex());
              }
            break;
            case TupProjectRequest::BgColor:
              {
                  QString colorName = event->arg().toString();
                  QColor color(colorName);
                  setBgColor(color);
              }
            break;
            default: 
              {
                  #ifdef TUP_DEBUG
                      QString msg = "TupPaintArea::sceneResponse <- TupProjectRequest::Default";
                      #ifdef Q_OS_WIN
                          qDebug() << msg;
                      #else
                          tFatal() << msg;
                      #endif
                  #endif
              }
            break;
        }
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupPaintArea::sceneResponse() - isDrawing() == true! - No action taken!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tFatal() << msg;
            #endif
        #endif
    }

    guiScene->sceneResponse(event);
}

void TupPaintArea::itemResponse(TupItemResponse *response)
{
    #ifdef TUP_DEBUG
        QString msg = "TupPaintArea::itemResponse() - [" + QString::number(response->sceneIndex()) 
                      + ", " + QString::number(response->layerIndex()) + ", " 
                      + QString::number(response->frameIndex()) + "]";
        #ifdef Q_OS_WIN
            qDebug() << msg;
        #else
            tDebug() << msg;
        #endif
    #endif

    TupGraphicsScene *guiScene = graphicsScene();
    if (!guiScene->currentScene())
        return;

    if (!guiScene->userIsDrawing()) {
        switch(response->action()) {
            case TupProjectRequest::Transform:
              {
                  viewport()->update();
              }
            break;
            case TupProjectRequest::Remove:
              { 
                  if (!deleteMode) {
                      if (spaceMode == TupProject::FRAMES_EDITION) {
                          guiScene->drawCurrentPhotogram();
                      } else {
                          guiScene->cleanWorkSpace();
                          guiScene->drawSceneBackground(guiScene->currentFrameIndex());
                      }

                      viewport()->update(scene()->sceneRect().toRect());
                  } 
              }
            break;
            case TupProjectRequest::Move:
            case TupProjectRequest::UpdateTweenPath:
              {
                  // Do nothing
              }
            break;
            default:
              {
                  if (spaceMode == TupProject::FRAMES_EDITION) {
                      guiScene->drawCurrentPhotogram();
                  } else {
                      guiScene->cleanWorkSpace();
                      guiScene->drawSceneBackground(guiScene->currentFrameIndex());
                  }

                  viewport()->update(scene()->sceneRect().toRect());

                  if (guiScene->currentTool()->toolType() != TupToolInterface::Tweener
                      && currentTool.compare(tr("PolyLine")) != 0)
                      guiScene->resetCurrentTool();
              }
            break;
        }
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupPaintArea::itemResponse() - isDrawing() == true! - No action taken!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tFatal() << msg;
            #endif
        #endif
    }

    guiScene->itemResponse(response);
}

void TupPaintArea::projectResponse(TupProjectResponse *)
{
}

void TupPaintArea::libraryResponse(TupLibraryResponse *request)
{
    #ifdef TUP_DEBUG
        QString msg = "TupPaintArea::libraryResponse() - Request Action: " + QString::number(request->action());
        #ifdef Q_OS_WIN
            qDebug() << msg;
        #else
            tDebug() << msg;
        #endif
    #endif

    TupGraphicsScene *guiScene = graphicsScene();

    if (!guiScene->currentScene())
        return;

    if (!guiScene->userIsDrawing()) {
        int frameIndex = guiScene->currentFrameIndex();
        switch (request->action()) {
            case TupProjectRequest::InsertSymbolIntoFrame:
              {
                  if (spaceMode == TupProject::FRAMES_EDITION) {
                      guiScene->drawCurrentPhotogram();
                  } else {
                      guiScene->cleanWorkSpace();
                      guiScene->drawSceneBackground(frameIndex);
                  }

                  viewport()->update(scene()->sceneRect().toRect());

                  if (currentTool.compare(tr("Object Selection")) == 0)
                      emit itemAddedOnSelection(guiScene);
              }
            break;
            case TupProjectRequest::Remove:
            case TupProjectRequest::RemoveSymbolFromFrame:
              {
                  if (spaceMode == TupProject::FRAMES_EDITION) {
                      guiScene->drawCurrentPhotogram();
                  } else {
                      guiScene->cleanWorkSpace();
                      guiScene->drawSceneBackground(frameIndex);
                  }

                  viewport()->update(scene()->sceneRect().toRect());
              }
            break;
        } 
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupPaintArea::libraryResponse() - isDrawing() == true! - No action taken!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tFatal() << msg;
            #endif
        #endif
    }
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
        #ifdef Q_OS_WIN
            qDebug() << "[TupPaintArea::deleteItems()]";
        #else
            T_FUNCINFOX("paintarea");
        #endif
    #endif

    if (currentTool.compare(tr("Object Selection")) != 0 && currentTool.compare(tr("Nodes Selection")) != 0)
        return;

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

                     TupLibraryObject::Type type = TupLibraryObject::Svg;
                     TupSvgItem *svg = qgraphicsitem_cast<TupSvgItem *>(item);
                     int itemIndex = -1;
                     int frameIndex = -1;
                     int layerIndex = -1;

                     if (spaceMode == TupProject::FRAMES_EDITION) {
                         frameIndex = currentScene->currentFrameIndex();
                         layerIndex = currentScene->currentLayerIndex();
                         if (svg) {
                             itemIndex = currentScene->currentFrame()->indexOf(svg);
                         } else {
                             type = TupLibraryObject::Item;
                             itemIndex = currentScene->currentFrame()->indexOf(item);
                         }
                     } else {
                         TupBackground *bg = currentScene->currentScene()->background();
                         if (bg) {
                             TupFrame *frame;
                             if (spaceMode == TupProject::STATIC_BACKGROUND_EDITION)
                                 frame = bg->staticFrame();
                             else
                                 frame = bg->dynamicFrame();

                             if (frame) {
                                 if (svg) {
                                     itemIndex = frame->indexOf(svg);
                                 } else {
                                     type = TupLibraryObject::Item;
                                     itemIndex = frame->indexOf(item);
                                 }
                             } else {
                                 #ifdef TUP_DEBUG
                                     QString msg = "TupPaintArea::deleteItems() - Fatal Error: Background frame is NULL!";
                                     #ifdef Q_OS_WIN
                                         qDebug() << msg;
                                     #else
                                         tError() << msg;
                                     #endif
                                 #endif
                             } 
                         } else {
                             #ifdef TUP_DEBUG
                                 QString msg = "TupPaintArea::deleteItems() - Fatal Error: Scene has no background element!";
                                 #ifdef Q_OS_WIN
                                     qDebug() << msg;
                                 #else
                                     tError() << msg;
                                 #endif
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
                             QString msg = "TupPaintArea::deleteItems() - Fatal Error: Invalid item index -> " + QString::number(itemIndex);
                             #ifdef Q_OS_WIN
                                 qDebug() << msg;
                             #else
                                 tFatal() << msg;
                             #endif
                         #endif
                     }

                     counter++;
            }

        }
    }
}

/*
void TupPaintArea::ungroupItems()
{
    QList<QGraphicsItem *> selected = scene()->selectedItems();
    if (!selected.isEmpty()) {
        TupGraphicsScene* currentScene = graphicsScene();
        if (currentScene) {
            foreach (QGraphicsItem *item, selected) {
                     TupProjectRequest event = TupRequestBuilder::createItemRequest( 
                                              currentScene->currentSceneIndex(),
                                              currentScene->currentLayerIndex(), 
                                              currentScene->currentFrameIndex(), 
                                              currentScene->currentFrame()->indexOf(item), QPointF(), 
                                              spaceMode, TupLibraryObject::Item,
                                              TupProjectRequest::Ungroup);
                     emit requestTriggered(&event);
            }
        }
    }
}
*/

void TupPaintArea::copyItems()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupPaintArea::copyItems()]";
        #else
            T_FUNCINFOX("paintarea");
        #endif
    #endif

    copiesXml.clear();
    QList<QGraphicsItem *> selected = scene()->selectedItems();

    if (!selected.isEmpty()) {
        foreach (QGraphicsItem *item, selected) {
             if (qgraphicsitem_cast<TControlNode *> (item))
                 selected.removeOne(item);
        }

        TupGraphicsScene* currentScene = graphicsScene();

        if (currentScene) {
            oldPosition = selected.at(0)->boundingRect().topLeft();
            foreach (QGraphicsItem *item, selected) {
                QDomDocument dom;
                dom.appendChild(dynamic_cast<TupAbstractSerializable *>(item)->toXml(dom));
                copiesXml << dom.toString();

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
                opt.matrix = item->sceneMatrix();
                opt.palette = palette();

                item->paint(&painter, &opt, this);
                painter.end();

                QApplication::clipboard()->setPixmap(toPixmap);
            }
        }
    } else {
        copyCurrentFrame();
    }
}

void TupPaintArea::pasteItems()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupPaintArea::pasteItems()]";
        #else
            T_FUNCINFOX("paintarea");
        #endif
    #endif

    if (!copiesXml.isEmpty()) {
        TupGraphicsScene* currentScene = graphicsScene();

        if (!menuOn)
            position = viewPosition();
    
        foreach (QString xml, copiesXml) {
            TupFrame *frame = currentScene->currentFrame();
            if (frame) {
                int total = frame->graphicItemsCount();
                TupLibraryObject::Type type = TupLibraryObject::Item;

                if (xml.startsWith("<svg")) {
                    type = TupLibraryObject::Svg;
                    total = frame->svgItemsCount();
                } 

                TupProjectRequest event = TupRequestBuilder::createItemRequest(currentScene->currentSceneIndex(),
                                          currentScene->currentLayerIndex(),
                                          currentScene->currentFrameIndex(),
                                          total, itemPoint(xml), spaceMode, type,
                                          TupProjectRequest::Add, xml);
                emit requestTriggered(&event);
            } else {
                #ifdef TUP_DEBUG
                    QString msg = "TupPaintArea::pasteItems() - Fatal Error: Frame is NULL!";
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
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
        #ifdef Q_OS_WIN
            qDebug() << "[TupPaintArea::multipasteObject()]";
        #else
            T_FUNCINFOX("paintarea");
        #endif
    #endif

    TupGraphicsScene* currentScene = graphicsScene();

    if (!menuOn)
        position = viewPosition();
   
    int layerIndex = currentScene->currentLayerIndex();
    int limit = 0;
    foreach (QString xml, copiesXml) {
        TupLibraryObject::Type type = TupLibraryObject::Item;
        int total = currentScene->currentFrame()->graphicItemsCount();

        TupScene *scene = project->sceneAt(currentScene->currentSceneIndex());
        if (scene) {
            TupLayer *layer = scene->layerAt(currentScene->currentLayerIndex());
            if (layer) {
                int framesCount = layer->framesCount();
                int currentFrame = currentScene->currentFrameIndex();
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

                limit = currentFrame + pasteTotal;
                for (int i=currentFrame+1; i<=limit; i++) {
                    TupProjectRequest event = TupRequestBuilder::createItemRequest(globalSceneIndex,
                                                                 layerIndex, i, total, itemPoint(xml), spaceMode, type,
                                                                 TupProjectRequest::Add, xml);
                    emit requestTriggered(&event);
                }
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

QPoint TupPaintArea::itemPoint(const QString &xml)
{
    int init = xml.indexOf("pos=") + 6;
    int end = xml.indexOf(")", init);
    int n = end - init;
    QString string = xml.mid(init, n);
    QStringList list = string.split(",");
    int x = static_cast<int> (list.at(0).toFloat());
    int y = static_cast<int> (list.at(1).toFloat());

    return QPoint(x, y);
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
        #ifdef Q_OS_WIN
            qDebug() << "[TupPaintArea::cutItems()]";
        #else
            T_FUNCINFOX("paintarea");
        #endif
    #endif

    copyItems();
    deleteItems();
}

void TupPaintArea::setNextFramesOnionSkinCount(int n)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupPaintArea::setNextFramesOnionSkinCount()]";
        #else
            T_FUNCINFOX("paintarea");
        #endif
    #endif

    if (TupGraphicsScene* currentScene = graphicsScene())
        currentScene->setNextOnionSkinCount(n);
}

void TupPaintArea::setPreviousFramesOnionSkinCount(int n)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupPaintArea::setPreviousFramesOnionSkinCount()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (TupGraphicsScene* currentScene = graphicsScene())
        currentScene->setPreviousOnionSkinCount(n);
}

void TupPaintArea::addSelectedItemsToLibrary()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupPaintArea::addSelectedItemsToLibrary()]";
        #else
            tDebug("paintarea") << "TupPaintArea::addSelectedItemsToLibrary()";
        #endif
    #endif

    QList<QGraphicsItem *> selected = scene()->selectedItems();

    if (selected.isEmpty()) {
        TOsd::self()->display(tr("Error"), tr("No items selected"), TOsd::Error);
        return;
    }

    TupLibraryDialog dialog(project->library());
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
        #ifdef Q_OS_WIN
            qDebug() << "[TupPaintArea::requestItemMovement()]";
        #else
            T_FUNCINFOX("paintarea");
        #endif
    #endif

    QList<QGraphicsItem *> selected = scene()->selectedItems();

    if (selected.isEmpty()) {
        TOsd::self()->display(tr("Error"), tr("No items selected"), TOsd::Error);
        return;
    }
	
    TupGraphicsScene *currentScene = graphicsScene();
    if (!currentScene)
        return;

    TupFrame *currentFrame = currentScene->currentFrame();

    foreach (QGraphicsItem *item, selected) {
             TupLibraryObject::Type type = TupLibraryObject::Item; 
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
                         QString msg = "TupPaintArea::requestItemMovement() - Fatal error: Invalid action [ " + QString::number(moveType) + " ]";
                         #ifdef Q_OS_WIN
                             qDebug() << msg;
                         #else
                             tError() << msg;
                         #endif
                     #endif
                 }
             } else {
                 #ifdef TUP_DEBUG
                     QString msg = "TupPaintArea::requestItemMovement() - Fatal error: Invalid object index [ " + QString::number(index) + " ]";
                     #ifdef Q_OS_WIN
                         qDebug() << msg;
                     #else
                         tError() << msg;
                     #endif
                 #endif
             }
    }
}

void TupPaintArea::updatePaintArea() 
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupPaintArea::updatePaintArea()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (spaceMode == TupProject::FRAMES_EDITION) {
        TupGraphicsScene* currentScene = graphicsScene();
        currentScene->drawCurrentPhotogram();
    } else {
        paintBackground();
    }
}

void TupPaintArea::paintBackground()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupPaintArea::paintBackground()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    TupGraphicsScene* currentScene = graphicsScene();
    currentScene->cleanWorkSpace();
    currentScene->drawSceneBackground(currentScene->currentFrameIndex());
}

void TupPaintArea::setCurrentTool(QString tool) 
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupPaintArea::setCurrentTool()]";
            qDebug() << "SHOW_VAR : " << tool;
        #else
            T_FUNCINFO;
            SHOW_VAR(tool);
        #endif
    #endif

    currentTool = tool;
    canvasEnabled = true;
}

void TupPaintArea::updateSpaceContext()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupPaintArea::updateSpaceContext()]";
        #else
            T_FUNCINFO;
        #endif
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
        QString msg1 = "TupPaintArea::keyPressEvent() - Current tool: " + currentTool;
        QString msg2 = "TupPaintArea::keyPressEvent() - Key: " + QString::number(event->key());
        QString msg3 = "TupPaintArea::keyPressEvent() - Key: " + event->text(); 
        #ifdef Q_OS_WIN
            qDebug() << "[TupPaintArea::keyPressEvent()]";
            qDebug() << msg1;
            qDebug() << msg2;
            qDebug() << msg3;
        #else
            T_FUNCINFO;
            tDebug() << msg1;
            tDebug() << msg2;
            tDebug() << msg3;
        #endif
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

    if (event->key() == Qt::Key_Plus) {
        if (event->modifiers() == Qt::NoModifier) {
            emit zoomIn();
            return;
        }
    }

    if (event->key() == Qt::Key_Minus) {
        if (event->modifiers() == Qt::NoModifier) {
            emit zoomOut();
            return;
        }
    }

    if (currentTool.compare(tr("PolyLine")) == 0) {
        if (event->key() == Qt::Key_X)
            emit closePolyLine();
        return;
    }

    /*
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
    */

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
                copyFrameName = frame->frameName();
                TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, frameIndex, TupProjectRequest::Copy);
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

        TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, frameIndex, TupProjectRequest::Paste);
        emit localRequestTriggered(&request);

        request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, frameIndex, TupProjectRequest::Rename, copyFrameName);
        emit requestTriggered(&request);
    }
}

void TupPaintArea::copyFrameForward()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupPaintArea::copyFrameForward()]";
        #else
            T_FUNCINFO;
        #endif
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
        TOptionalDialog dialog(tr("Do you want to remove this frame?"), tr("Confirmation"), this);
        dialog.setModal(true);
        QDesktopWidget desktop;
        dialog.move(static_cast<int> ((desktop.screenGeometry().width() - dialog.sizeHint().width())/2),
                    static_cast<int> ((desktop.screenGeometry().height() - dialog.sizeHint().height())/2));

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
    TupGraphicsScene *gScene = graphicsScene();
    int sceneIndex = gScene->currentSceneIndex();
    int layerIndex = gScene->currentLayerIndex();
    int frameIndex = gScene->currentFrameIndex();

    if (layerIndex > 0)
        goToFrame(frameIndex, layerIndex-1, sceneIndex);
}

void TupPaintArea::goOneLayerForward()
{
    TupGraphicsScene *gScene = graphicsScene();
    int sceneIndex = gScene->currentSceneIndex();
    int layerIndex = gScene->currentLayerIndex();
    int frameIndex = gScene->currentFrameIndex();

    layerIndex++;
    TupScene *scene = gScene->currentScene();
    if (layerIndex < scene->layersCount())
        goToFrame(frameIndex, layerIndex, sceneIndex);
}

void TupPaintArea::goToScene(int sceneIndex)
{
    TupProjectRequest request = TupRequestBuilder::createSceneRequest(sceneIndex, TupProjectRequest::Select);
    emit localRequestTriggered(&request);
}

/*
void TupPaintArea::tabletEvent(QTabletEvent *event)
{
    tError() << "TupPaintArea::tabletEvent() - Pressure: " << event->pressure();
    tError() << "TupPaintArea::tabletEvent() - xTilt: " << event->xTilt();
    tError() << "TupPaintArea::tabletEvent() - yTilt: " << event->yTilt();

    TupPaintAreaBase::tabletEvent(event);
}
*/

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
    #ifdef Q_OS_WIN
        qDebug() << "[TupPaintArea::resetWorkSpaceCenter()]";
    #else
        T_FUNCINFO;
    #endif
#endif

    int centerX = projectSize.width()/2;
    int centerY = projectSize.height()/2;

    foreach (QGraphicsView *view, graphicsScene()->views()) {
             view->centerOn(QPointF(centerX, centerY));
             view->setSceneRect(0, 0, projectSize.width(), projectSize.height());
    }
}

void TupPaintArea::updateLoadingFlag(bool flag)
{
    graphicsScene()->updateLoadingFlag(flag);
}
