/***************************************************************************
 *   Project TUPITUBE DESK                                                *
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

#include "tupgraphicsscene.h"
#include "tupitemgroup.h"
#include "tupprojectloader.h"
#include "tupitemfactory.h"
#include "tupinputdeviceinformation.h"
#include "tupitemtweener.h"
#include "tupgraphiclibraryitem.h"
#include "tuppathitem.h"
#include "tuplineitem.h"
#include "tuprectitem.h"
#include "tupellipseitem.h"
#include "tupwatermark.h"
#include "tuptextitem.h"

#include <QSvgRenderer>
#include <QGraphicsView>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMouseEvent>

TupGraphicsScene::TupGraphicsScene() : QGraphicsScene()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene()] - Creating instance...";
    #endif

    loadingProject = true;
    setItemIndexMethod(QGraphicsScene::NoIndex);

    framePosition.layer = -1;
    framePosition.frame = -1;
    spaceContext = TupProject::FRAMES_MODE;

    onionSkin.next = 0;
    onionSkin.previous = 0;
    gTool = nullptr;
    isDrawing = false;
    showWaterMark = false;

    inputInformation = new TupInputDeviceInformation(this);
    brushManager = new TupBrushManager(this);

    vectorDynamicBg = new QGraphicsPixmapItem;
    rasterStaticBg = new QGraphicsPixmapItem;
    rasterDynamicBg = new QGraphicsPixmapItem;
}

TupGraphicsScene::~TupGraphicsScene()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupGraphicsScene()]";
    #endif

    clearFocus();
    clearSelection();

    // SQA: Check if these instructions are actually required
    foreach (QGraphicsItem *item, items()) {
        removeItem(item);
        item = nullptr;
        delete item;
    }
}

void TupGraphicsScene::updateLayerVisibility(int layerIndex, bool visible)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::updateLayerVisibility()]";
    #endif

    if (!tupScene)
        return;

    if (TupLayer *layer = tupScene->layerAt(layerIndex))
        layer->setLayerVisibility(visible);
}

void TupGraphicsScene::setCurrentFrame(int layer, int frame)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::setCurrentFrame()]";
    #endif

    if ((frame != framePosition.frame && framePosition.frame >= 0) ||
        (layer != framePosition.layer && framePosition.layer >= 0)) {
        if (gTool) {
            if (gTool->toolId() == TAction::Polyline || gTool->toolType() == TupToolInterface::Tweener)
                gTool->aboutToChangeScene(this);
        }
    }

    framePosition.layer = layer;
    framePosition.frame = frame;

    foreach (QGraphicsView *view, views())
        view->setDragMode(QGraphicsView::NoDrag);
}

void TupGraphicsScene::drawCurrentPhotogram()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::drawCurrentPhotogram()] - layer index -> " << framePosition.layer;
    #endif

    if (loadingProject)
        return;

    TupLayer *layer = tupScene->layerAt(framePosition.layer);
    if (layer) {
        int frames = layer->framesCount();

        if (framePosition.frame >= frames)
            framePosition.frame = frames - 1;

        if (spaceContext == TupProject::FRAMES_MODE) {
            drawPhotogram(framePosition.frame, true);
        } else if (spaceContext == TupProject::VECTOR_FG_MODE) {
            cleanWorkSpace();
            drawVectorFg();
        } else {
            cleanWorkSpace();
            drawSceneBackground(framePosition.frame);
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupGraphicsScene::drawCurrentPhotogram()] - Fatal error: Invalid layer index -> "
                        + QString::number(framePosition.layer);
        #endif
    }
}

void TupGraphicsScene::drawPhotogram(int photogram, bool drawContext)
{ 
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::drawPhotogram()] - photogram -> " << photogram;
    #endif

    if (photogram < 0 || !tupScene) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupGraphicsScene::drawPhotogram()] - Error: Invalid photogram -> " << photogram;
        #endif
        return;
    }

    cleanWorkSpace();
    // Painting the background
    drawSceneBackground(photogram);

    // Drawing frames from every layer
    int layersTotal = tupScene->layersCount();

    TupLayer *layer;
    TupFrame *mainFrame;
    TupFrame *frame;
    for (int layerIndex=0; layerIndex < layersTotal; layerIndex++) {
         layer = tupScene->layerAt(layerIndex);
         if (layer) {
             layerOnProcess = layerIndex;
             layerOnProcessOpacity = layer->getOpacity();
             int framesCount = layer->framesCount();
             zLevel = (BG_LAYERS + layerIndex) * ZLAYER_LIMIT;

             if (photogram < framesCount) {
                 mainFrame = layer->frameAt(photogram);
                 QString currentFrame = "";

                 if (mainFrame) {
                     if (layer->isLayerVisible()) {
                         int maximum = qMax(onionSkin.previous, onionSkin.next);
                         double opacityFactor = gOpacity / static_cast<double>(maximum);
                         double opacity = gOpacity + ((maximum - onionSkin.previous) * opacityFactor);
                         if (drawContext) {
                             // Painting previous frames
                             if (onionSkin.previous > 0 && photogram > 0) {
                                 int limit = photogram - onionSkin.previous;
                                 if (limit < 0) 
                                     limit = 0;

                                 for (int frameIndex = limit; frameIndex < photogram; frameIndex++) {
                                     frame = layer->frameAt(frameIndex);
                                     if (frame) {
                                         frameOnProcess = frameIndex;
                                         addFrame(frame, opacity, Previous);
                                         // addTweeningObjects(layerIndex, frameIndex, opacity);
                                         // addSvgTweeningObjects(layerIndex, frameIndex, opacity);
                                     }
                                     opacity += opacityFactor;
                                 }
                             }
                         }

                         // Painting current frame
                         frameOnProcess = photogram;
                         addFrame(mainFrame);
                         addTweeningObjects(layerIndex, photogram);
                         addSvgTweeningObjects(layerIndex, photogram);

                         if (drawContext) {
                             // Painting next frames
                             if (onionSkin.next > 0 && framesCount > photogram + 1) {
                                 opacity = gOpacity + (opacityFactor*(maximum - 1));

                                 int limit = photogram + onionSkin.next;
                                 if (limit >= framesCount)
                                     limit = framesCount - 1;

                                 for (int frameIndex = photogram+1; frameIndex <= limit; frameIndex++) {
                                     frame = layer->frameAt(frameIndex);
                                     if (frame) {
                                         frameOnProcess = frameIndex;
                                         addFrame(frame, opacity, Next);
                                         // addTweeningObjects(layerIndex, frameIndex, opacity, false);
                                         // addSvgTweeningObjects(layerIndex, frameIndex, opacity, false);
                                     }
                                     opacity -= opacityFactor;
                                 }
                             }
                         }

                         addLipSyncObjects(layer, photogram, zLevel);
                     }
                 }
             }
         } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupGraphicsScene::drawPhotogram()] - Error: Invalid layer at index -> " << layerIndex;
            #endif
            return;
         }
    }

    if (background->isLayerVisible(TupBackground::VectorForeground))
        drawVectorFg();

    if (showWaterMark) {
        TupWaterMark *mark = new TupWaterMark;
        addItem(mark->generateWaterMark(background->getBgColor(), background->getProjectSize(), zLevel));
    }

    if (gTool)
        gTool->updateScene(this);
}

void TupGraphicsScene::drawSceneBackground(int photogram)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::drawSceneBackground()] - photogram -> " << photogram;
        qDebug() << "*** spaceContext -> " << spaceContext;
    #endif

    if (!tupScene) {
        #ifdef TUP_DEBUG
            qWarning() << "TupGraphicsScene::drawSceneBackground() - Warning: gScene is nullptr!";
        #endif
        return;
    }

    if (background) {
        // Dynamic Bg
        if (spaceContext == TupProject::VECTOR_DYNAMIC_BG_MODE) {
            // SQA: Include option "show" raster dynamic bg as option
            // drawRasterDynamicBg();
            drawVectorDynamicBg();
            return;
        } else if (spaceContext == TupProject::VECTOR_STATIC_BG_MODE) {
            // SQA: Include option "show" raster static bg as option
            // drawRasterStaticBg();
            drawVectorStaticBg(0);
        } else { // Frames Mode
            QList<TupBackground::BgType> bgLayerIndex = background->layerIndexes();
            for (int i=0; i < bgLayerIndex.count(); i++) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupGraphicsScene::drawSceneBackground()] - Processing BG index -> " << bgLayerIndex.at(i);
                #endif

                switch(bgLayerIndex.at(i)) {
                    case TupBackground::VectorStatic:
                    {
                        if (background->isLayerVisible(TupBackground::VectorStatic))
                            drawVectorStaticBg(i);
                    }
                    break;
                    case TupBackground::VectorDynamic:
                    {
                        if (background->isLayerVisible(TupBackground::VectorDynamic))
                            drawVectorDynamicBgOnMovement(i, photogram);
                    }
                    break;
                    case TupBackground::RasterStatic:
                    {
                        if (background->isLayerVisible(TupBackground::RasterStatic))
                            drawRasterStaticBg(i);
                    }
                    break;
                    case TupBackground::RasterDynamic:
                    {
                        if (background->isLayerVisible(TupBackground::RasterDynamic))
                            drawRasterDynamicBgOnMovement(i, photogram);
                    }
                    break;
                    default:
                    {
                    }
                }
            }
        }
    }
}

void TupGraphicsScene::drawVectorStaticBg(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::drawVectorStaticBg()] - index -> " << index;
    #endif

    // Vector Static Bg
    if (!background->vectorStaticBgIsEmpty()) {
        TupFrame *frame = background->vectorStaticFrame();
        if (frame) {
            zLevel = index * ZLAYER_LIMIT;
            addFrame(frame, frame->frameOpacity());
        }
        frame = nullptr;
        delete frame;
        return;
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupGraphicsScene::drawVectorStaticBg()] - Vector static bg frame is empty";
        #endif
    }

    #ifdef TUP_DEBUG
        qDebug() << "---";
    #endif
}

// Shows dynamic background on background mode
void TupGraphicsScene::drawVectorDynamicBg()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::drawVectorDynamicBg()]";
    #endif

    // Vector
    if (!background->vectorDynamicBgIsEmpty()) {
        TupFrame *frame = background->vectorDynamicFrame();
        if (frame) {
            zLevel = 0;
            addFrame(frame, frame->frameOpacity());
        }
        frame = nullptr;
        delete frame;
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupGraphicsScene::drawVectorDynamicBg()] - Vector dynamic bg frame is empty";
        #endif
    }

    #ifdef TUP_DEBUG
        qDebug() << "---";
    #endif
}

// Shows dynamic background as part of the scene
void TupGraphicsScene::drawVectorDynamicBgOnMovement(int index, int photogram)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::drawVectorDynamicBgOnMovement()] - photogram: " << photogram;
        qDebug() << "[TupGraphicsScene::drawVectorDynamicBgOnMovement()] - index -> " << index;
    #endif

    // Vector Dynamic Bg on movement
    if (!background->vectorDynamicBgIsEmpty()) {
        if (background->vectorRenderIsPending())
            background->renderVectorDynamicView();

        vectorDynamicBg = new QGraphicsPixmapItem(background->vectorDynamicExpandedImage());
        vectorDynamicBg->setZValue(index * ZLAYER_LIMIT);
        vectorDynamicBg->setPos(background->vectorDynamicPos(photogram));
        addItem(vectorDynamicBg);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupGraphicsScene::drawVectorDynamicBgOnMovement()] - Vector dynamic bg frame is empty";
        #endif
    }

    #ifdef TUP_DEBUG
        qDebug() << "---";
    #endif
}

void TupGraphicsScene::drawRasterStaticBg(int index)
{
    // Raster Static Bg
    if (!background->rasterStaticBgIsNull()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupGraphicsScene::drawRasterStaticBg()] - Adding RASTER STATIC image!";
        #endif

        if (background->rasterStaticUpdateIsPending()) {
            rasterStaticBg->setPixmap(background->rasterStaticBackground());
            background->updateRasterStaticStatus(false);
        }

        rasterStaticBg->setZValue(index * ZLAYER_LIMIT);
        addItem(rasterStaticBg);
        // rasterStaticBg = nullptr;
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupGraphicsScene::drawRasterStaticBg()] - RASTER STATIC image is NULL!";
        #endif
    }

    #ifdef TUP_DEBUG
        qDebug() << "---";
    #endif
}

void TupGraphicsScene::drawRasterDynamicBgOnMovement(int index, int photogram)
{
    // Raster Dynamic Bg on movement
    if (!background->rasterDynamicBgIsNull()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupGraphicsScene::drawRasterDynamicBgOnMovement()] - Adding RASTER DYNAMIC image! "
                        "- photogram -> " << photogram;
        #endif
        // Calculate current raster dynamic view
        if (background->rasterDynamicRenderIsPending()) {
            background->renderRasterDynamicView();
            rasterDynamicBg->setPixmap(background->rasterDynamicExpandedImage());
        }

        rasterDynamicBg->setZValue(index * ZLAYER_LIMIT);
        rasterDynamicBg->setPos(background->rasterDynamicPos(photogram));
        addItem(rasterDynamicBg);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupGraphicsScene::drawRasterDynamicBgOnMovement()] - RASTER DYNAMIC image is NULL!";
        #endif
    }

    #ifdef TUP_DEBUG
        qDebug() << "---";
    #endif
}

void TupGraphicsScene::drawVectorFg()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::drawVectorFg()]";
    #endif

    // Vector Foreground
    if (!background->vectorFgIsEmpty()) {
        TupFrame *frame = background->vectorForegroundFrame();
        if (frame) {
            zLevel = (tupScene->layersCount() + BG_LAYERS + 1) * ZLAYER_LIMIT;
            #ifdef TUP_DEBUG
                qDebug() << "[TupGraphicsScene::drawVectorFg()] - zLevel -> " << zLevel;
            #endif

            addFrame(frame, frame->frameOpacity());
        }

        frame = nullptr;
        delete frame;
        return;
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupGraphicsScene::drawVectorFg()] - Vector foreground frame is empty";
        #endif
    }

    #ifdef TUP_DEBUG
        qDebug() << "---";
    #endif
}

void TupGraphicsScene::addFrame(TupFrame *frame, double opacityFactor, Context mode)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::addFrame()]";
    #endif
    */

    TupFrame::FrameType frameType = frame->frameType();
    QList<TupGraphicObject *> objects = frame->graphicItems(); 
    QList<TupSvgItem *> svgItems = frame->svgItems();

    int objectsCount = objects.count();
    int svgCount = svgItems.count();

    if (objectsCount == 0 && svgCount == 0)
        return;

    // Adding only native objects
    if (objectsCount > 0 && svgCount == 0) {
        foreach (TupGraphicObject *object, objects)
            processNativeObject(object, frameType, opacityFactor, mode);
        return;
    }

    // Adding only SVG objects
    if (svgCount > 0 && objectsCount == 0) {
        foreach (TupSvgItem *svg, svgItems)
            processSVGObject(svg, frameType, opacityFactor, mode);
        return;
    }

    // Adding native AND SVG objects
    TupGraphicObject *object;
    TupSvgItem *svg;
    do {
        int objectZValue = objects.at(0)->itemZValue();  
        int svgZValue = static_cast<int> (svgItems.at(0)->zValue());

        if (objectZValue < svgZValue) {
            object = objects.takeFirst();
            processNativeObject(object, frameType, opacityFactor, mode);
        } else {  
            svg = svgItems.takeFirst();
            processSVGObject(svg, frameType, opacityFactor, mode);
        }

        if (objects.isEmpty()) {
            foreach (TupSvgItem *svg, svgItems)
                processSVGObject(svg, frameType, opacityFactor, mode);
            return;
        } else {
            if (svgItems.isEmpty()) {
                foreach (TupGraphicObject *object, objects)
                    processNativeObject(object, frameType, opacityFactor, mode);
                return;
            }
        }
    } while (true);
}

void TupGraphicsScene::processNativeObject(TupGraphicObject *object, TupFrame::FrameType frameType,
                                           double opacityFactor, Context mode)
{
    if (mode != TupGraphicsScene::Current) {
        if (!object->hasTweens())
            addGraphicObject(object, frameType, opacityFactor);
        else
            zLevel++; // Saving space for a tween
    } else {
        addGraphicObject(object, frameType,  opacityFactor);
    }
}

void TupGraphicsScene::processSVGObject(TupSvgItem *svg, TupFrame::FrameType frameType,
                                        double opacityFactor, Context mode)
{
    if (mode != TupGraphicsScene::Current) {
        if (!svg->hasTweens())
            addSvgObject(svg, frameType, opacityFactor);
        else
            zLevel++; // Saving space for an SVG tween
    } else {
        addSvgObject(svg, frameType, opacityFactor);
    }
}

void TupGraphicsScene::addGraphicObject(TupGraphicObject *object, TupFrame::FrameType frameType,
                                        double opacityFactor, bool tweenInAdvance)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::addGraphicObject()]";
    #endif
    */

    QGraphicsItem *item = object->item();
    if (item) {
        /* SQA: Code for debugging purposes 
        #ifdef TUP_DEBUG
            qWarning() << "Object XML:";
            qWarning() << object->toString();
        #endif
        */

        if (frameType == TupFrame::Regular) {
            if (framePosition.layer == layerOnProcess && framePosition.frame == frameOnProcess)
                onionSkin.accessMap.insert(item, true);
            else
                onionSkin.accessMap.insert(item, false);
        } else {
            if (spaceContext == TupProject::VECTOR_STATIC_BG_MODE || spaceContext == TupProject::VECTOR_DYNAMIC_BG_MODE
                || spaceContext == TupProject::VECTOR_FG_MODE)
                onionSkin.accessMap.insert(item, true);
            else
                onionSkin.accessMap.insert(item, false);
        }

        if (TupItemGroup *group = qgraphicsitem_cast<TupItemGroup *>(item))
            group->recoverChilds();

        item->setSelected(false);
        if (frameType == TupFrame::Regular)
            item->setOpacity(opacityFactor * layerOnProcessOpacity);
        else
            item->setOpacity(opacityFactor);

        if (!object->hasTweens() || !tweenInAdvance) {
            item->setZValue(zLevel);
            zLevel++;
        }

        addItem(item);
    }
}

void TupGraphicsScene::addSvgObject(TupSvgItem *svgItem, TupFrame::FrameType frameType,
                                    double opacityFactor, bool tweenInAdvance)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::addSvgObject()]";
    #endif
    */

    if (svgItem) {
        svgItem->setSelected(false);
        if (frameType == TupFrame::Regular) {
            if (framePosition.layer == layerOnProcess && framePosition.frame == frameOnProcess)
                onionSkin.accessMap.insert(svgItem, true);
            else
                onionSkin.accessMap.insert(svgItem, false);
        } else {
            if (spaceContext == TupProject::VECTOR_STATIC_BG_MODE || spaceContext == TupProject::VECTOR_DYNAMIC_BG_MODE
                || spaceContext == TupProject::VECTOR_FG_MODE)
                onionSkin.accessMap.insert(svgItem, true);
            else
                onionSkin.accessMap.insert(svgItem, false);
        }

        TupLayer *layer = tupScene->layerAt(framePosition.layer);
        if (layer) {
            TupFrame *frame = layer->frameAt(framePosition.frame);
            if (frame) {
                if (frameType == TupFrame::Regular)
                    svgItem->setOpacity(opacityFactor * layerOnProcessOpacity);
                else
                    svgItem->setOpacity(opacityFactor);

                // SQA: Experimental code related to interactive features
                // if (svgItem->symbolName().compare("dollar.svg")==0)
                //     connect(svgItem, SIGNAL(enabledChanged()), this, SIGNAL(showInfoWidget()));

                if (!svgItem->hasTweens() || !tweenInAdvance) {
                    svgItem->setZValue(zLevel);
                    zLevel++;
                }

                // SQA: Pending to implement SVG group support

                addItem(svgItem);
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupGraphicsScene::addSvgObject()] - Error: Frame #"
                                + QString::number(framePosition.frame) + " NO available!";
                #endif
            }
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupGraphicsScene::addSvgObject()] - Error: Layer #"
                            + QString::number(framePosition.layer) + " NO available!";
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupGraphicsScene::addSvgObject()] - Error: No SVG item!";
        #endif
    } 
} 

void TupGraphicsScene::addTweeningObjects(int layerIndex, int photogram, double opacity, bool onProcess)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::addTweeningObjects()]";
    #endif

    QList<TupGraphicObject *> tweenList = tupScene->getTweeningGraphicObjects(layerIndex);
    int total = tweenList.count();

    #ifdef TUP_DEBUG
        qWarning() << "*** Objects with tweens: " << total;
    #endif

    TupGraphicObject *object;
    TupTweenerStep *stepItem;
    for (int i=0; i < total; i++) {
         object = tweenList.at(i);
         int origin = object->frameIndex();

         QList<TupItemTweener *> list = object->tweensList(); 
         foreach(TupItemTweener *tween, list) {
             int adjustX = static_cast<int> (object->item()->boundingRect().width()) / 2;
             int adjustY = static_cast<int> (object->item()->boundingRect().height()) / 2;

             if (origin == photogram) {
                 #ifdef TUP_DEBUG
                     qWarning() << "Tween -> " << tween->getTweenName();
                     qWarning() << "Type -> " << tween->getType();
                     qWarning() << "Adding FIRST tween transformation - photogram -> " << photogram;
                 #endif

                 stepItem = tween->stepAt(0);
                 if (stepItem->has(TupTweenerStep::Motion)) {
                     QPointF point = QPoint(-adjustX, -adjustY);
                     object->setLastTweenPos(stepItem->getPosition() + point);
                     object->item()->setPos(tween->transformOriginPoint());
                 } else if (stepItem->has(TupTweenerStep::Rotation)) {
                     double angle = stepItem->getRotation();
                     QPointF pos = tween->transformOriginPoint();
                     if (qgraphicsitem_cast<QGraphicsItemGroup *>(object->item())) {
                         QPointF newTarget = pos - (object->item()->boundingRect().center() + object->item()->pos());
                         pos = object->item()->boundingRect().center() + newTarget;
                     }

                     object->item()->setTransformOriginPoint(pos);
                     object->item()->setRotation(angle);
                 } else if (stepItem->has(TupTweenerStep::Scale)) {
                     QPointF point = tween->transformOriginPoint();
                     double scaleX = tween->initXScaleValue();
                     double scaleY = tween->initYScaleValue();
                     QTransform transform;
                     transform.translate(point.x(), point.y());
                     transform.scale(scaleX, scaleY);
                     transform.translate(-point.x(), -point.y());

                     object->item()->setTransform(transform);
                 } else if (stepItem->has(TupTweenerStep::Shear)) {
                     QTransform transform;
                     transform.shear(0, 0);
                     object->item()->setTransform(transform);
                 } else if (stepItem->has(TupTweenerStep::Coloring)) {
                     if (TupTextItem *text = qgraphicsitem_cast<TupTextItem *>(object->item())) {
                         QColor itemColor = stepItem->getColor();
                         text->setDefaultTextColor(itemColor);
                     } else {
                         if (tween->tweenColorFillType() == TupItemTweener::Line || tween->tweenColorFillType() == TupItemTweener::FillAll) {
                             QColor itemColor = stepItem->getColor();
                             if (TupPathItem *path = qgraphicsitem_cast<TupPathItem *>(object->item())) {
                                 QPen pen = path->pen();
                                 pen.setColor(itemColor);
                                 path->setPen(pen);
                             } else if (TupEllipseItem *ellipse = qgraphicsitem_cast<TupEllipseItem *>(object->item())) {
                                 QPen pen = ellipse->pen();
                                 pen.setColor(itemColor);
                                 ellipse->setPen(pen);
                             } else if (TupLineItem *line = qgraphicsitem_cast<TupLineItem *>(object->item())) {
                                 QPen pen = line->pen();
                                 pen.setColor(itemColor);
                                 line->setPen(pen);
                             } else if (TupRectItem *rect = qgraphicsitem_cast<TupRectItem *>(object->item())) {
                                 QPen pen = rect->pen();
                                 pen.setColor(itemColor);
                                 rect->setPen(pen);
                             }
                         }

                         if (tween->tweenColorFillType() == TupItemTweener::Internal || tween->tweenColorFillType() == TupItemTweener::FillAll) {
                             QColor itemColor = stepItem->getColor();
                             if (TupPathItem *path = qgraphicsitem_cast<TupPathItem *>(object->item())) {
                                 QBrush brush = path->brush();
                                 brush.setColor(itemColor);
                                 path->setBrush(brush);
                             } else if (TupEllipseItem *ellipse = qgraphicsitem_cast<TupEllipseItem *>(object->item())) {
                                 QBrush brush = ellipse->brush();
                                 brush.setColor(itemColor);
                                 ellipse->setBrush(brush);
                             } else if (TupRectItem *rect = qgraphicsitem_cast<TupRectItem *>(object->item())) {
                                 QBrush brush = rect->brush();
                                 brush.setColor(itemColor);
                                 rect->setBrush(brush);
                             }
                         }
                     }
                 } else if (stepItem->has(TupTweenerStep::Opacity)) {
                     object->item()->setOpacity(stepItem->getOpacity());
                 }

                 if (!onProcess && opacity < 1) {
                     object->item()->setZValue(((BG_LAYERS + layerIndex) * ZLAYER_LIMIT) + tween->getZLevel());
                     // Including object into the scene
                     addGraphicObject(object, TupFrame::Regular, 1.0, true);
                     object->item()->setOpacity(opacity);
                 }
             // Processing photograms > tween init frame
             } else if ((origin < photogram) && (photogram < (origin + tween->getFrames()))) {
                 if (!onProcess)
                     return;

                 #ifdef TUP_DEBUG
                     qWarning() << "Adding tween transformation - photogram -> " << photogram;
                 #endif
                 int step = photogram - origin;
                 stepItem = tween->stepAt(step);
                 if (stepItem->has(TupTweenerStep::Motion)) {
                     qreal dx = stepItem->getPosition().x() - (object->lastTweenPos().x() + adjustX);
                     qreal dy = stepItem->getPosition().y() - (object->lastTweenPos().y() + adjustY);
                     object->item()->moveBy(dx, dy);
                     QPointF point = QPoint(-adjustX, -adjustY);
                     object->setLastTweenPos(stepItem->getPosition() + point);
                 } else if (stepItem->has(TupTweenerStep::Rotation)) {
                     double angle = stepItem->getRotation();
                     object->item()->setRotation(angle);
                 } else if (stepItem->has(TupTweenerStep::Scale)) {
                     QPointF point = tween->transformOriginPoint();

                     double scaleX = stepItem->horizontalScale();
                     double scaleY = stepItem->verticalScale();
                     QTransform transform;
                     transform.translate(point.x(), point.y());
                     transform.scale(scaleX, scaleY);
                     transform.translate(-point.x(), -point.y());

                     object->item()->setTransform(transform);
                 } else if (stepItem->has(TupTweenerStep::Shear)) {
                     QPointF point = tween->transformOriginPoint();

                     double shearX = stepItem->horizontalShear();
                     double shearY = stepItem->verticalShear();
                     QTransform transform;
                     transform.translate(point.x(), point.y());
                     transform.shear(shearX, shearY);
                     transform.translate(-point.x(), -point.y());

                     object->item()->setTransform(transform);
                 } else if (stepItem->has(TupTweenerStep::Coloring)) {                     
                     if (TupTextItem *text = qgraphicsitem_cast<TupTextItem *>(object->item())) {
                         QColor itemColor = stepItem->getColor();
                         text->setDefaultTextColor(itemColor);
                     } else {
                         if (tween->tweenColorFillType() == TupItemTweener::Line || tween->tweenColorFillType() == TupItemTweener::FillAll) {
                             QColor itemColor = stepItem->getColor();
                             if (TupPathItem *path = qgraphicsitem_cast<TupPathItem *>(object->item())) {
                                 QPen pen = path->pen();
                                 pen.setColor(itemColor);
                                 path->setPen(pen);
                             } else if (TupEllipseItem *ellipse = qgraphicsitem_cast<TupEllipseItem *>(object->item())) {
                                 QPen pen = ellipse->pen();
                                 pen.setColor(itemColor);
                                 ellipse->setPen(pen);
                             } else if (TupLineItem *line = qgraphicsitem_cast<TupLineItem *>(object->item())) {
                                 QPen pen = line->pen();
                                 pen.setColor(itemColor);
                                 line->setPen(pen);
                             } else if (TupRectItem *rect = qgraphicsitem_cast<TupRectItem *>(object->item())) {
                                 QPen pen = rect->pen();
                                 pen.setColor(itemColor);
                                 rect->setPen(pen);
                             }
                         }

                         if (tween->tweenColorFillType() == TupItemTweener::Internal || tween->tweenColorFillType() == TupItemTweener::FillAll) {
                             QColor itemColor = stepItem->getColor();
                             if (TupPathItem *path = qgraphicsitem_cast<TupPathItem *>(object->item())) {
                                 QBrush brush = path->brush();
                                 brush.setColor(itemColor);
                                 path->setBrush(brush);
                             } else if (TupEllipseItem *ellipse = qgraphicsitem_cast<TupEllipseItem *>(object->item())) {
                                 QBrush brush = ellipse->brush();
                                 brush.setColor(itemColor);
                                 ellipse->setBrush(brush);
                             } else if (TupRectItem *rect = qgraphicsitem_cast<TupRectItem *>(object->item())) {
                                 QBrush brush = rect->brush();
                                 brush.setColor(itemColor);
                                 rect->setBrush(brush);
                             }
                         }
                     }
                 }

                 int level = tween->getZLevel();
                 if (level > 0)
                     level--;
                 object->item()->setZValue(((BG_LAYERS + layerIndex) * ZLAYER_LIMIT) + level);
                 // Including object into the scene
                 addGraphicObject(object, TupFrame::Regular, 1.0, true);

                 if (stepItem->has(TupTweenerStep::Opacity)) {
                     double transp = stepItem->getOpacity();
                     if (transp == 0)
                         object->item()->setOpacity(0);
                     else
                         if (opacity < transp)
                             object->item()->setOpacity(opacity);
                         else
                             object->item()->setOpacity(transp);

                     if (opacity < 1)
                         return;
                 } else if (opacity < 1) {
                     object->item()->setOpacity(opacity);
                     return;
                 }
             }

             QString tip = object->item()->toolTip();
             if (tip.length() == 0) {
                 object->item()->setToolTip("Tweens: " + tween->tweenTypeToString());
             } else if (!tip.contains(tween->tweenTypeToString())) {
                 tip += "," + tween->tweenTypeToString();
                 object->item()->setToolTip(tip);
             }
         }
    }
}

void TupGraphicsScene::addSvgTweeningObjects(int layerIndex, int photogram, double opacity, bool onProcess)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::addSvgTweeningObjects()]";
    #endif

    QList<TupSvgItem *> svgList = tupScene->getTweeningSvgObjects(layerIndex);
    int total = svgList.count();

    #ifdef TUP_DEBUG
        qWarning() << "*** Objects with SVG tweens: " << total;
    #endif

    TupSvgItem *object;
    TupTweenerStep *stepItem;
    for (int i=0; i < total; i++) {
         object = svgList.at(i);
         int origin = object->frameIndex();

         QList<TupItemTweener *> list = object->tweensList();
         foreach(TupItemTweener *tween, list) {
             int adjustX = static_cast<int> (object->boundingRect().width()/2);
             int adjustY = static_cast<int> (object->boundingRect().height()/2);

             if (origin == photogram) {
                 #ifdef TUP_DEBUG
                     qWarning() << "Tween -> " << tween->getTweenName();
                     qWarning() << "Type -> " << tween->getType();
                     qWarning() << "Adding FIRST SVG tween transformation - photogram -> " << photogram;
                 #endif

                 stepItem = tween->stepAt(0);
                 if (stepItem->has(TupTweenerStep::Motion)) {
                     object->setPos(tween->transformOriginPoint());
                     QPointF offset = QPoint(-adjustX, -adjustY);
                     object->setLastTweenPos(stepItem->getPosition() + offset);
                 } else if (stepItem->has(TupTweenerStep::Rotation)) {
                     double angle = stepItem->getRotation();
                     object->setTransformOriginPoint(tween->transformOriginPoint());
                     object->setRotation(angle);
                 } else if (stepItem->has(TupTweenerStep::Scale)) {
                     QPointF point = tween->transformOriginPoint();
                     double scaleX = tween->initXScaleValue();
                     double scaleY = tween->initYScaleValue();
                     QTransform transform;
                     transform.translate(point.x(), point.y());
                     transform.scale(scaleX, scaleY);
                     transform.translate(-point.x(), -point.y());

                     object->setTransform(transform);
                 } else if (stepItem->has(TupTweenerStep::Shear)) {
                     QTransform transform;
                     transform.shear(0, 0);
                     object->setTransform(transform);
                 } else if (stepItem->has(TupTweenerStep::Opacity)) {
                     object->setOpacity(stepItem->getOpacity());
                 }

                 if (!onProcess && opacity < 1) {
                     // Including object into the scene
                     addSvgObject(object, TupFrame::Regular, 1.0, true);
                     object->setOpacity(opacity);
                 }
             // Processing photograms > tween init frame
             } else if ((origin < photogram) && (photogram < origin + tween->getFrames())) {
                 if (!onProcess)
                     return;

                 #ifdef TUP_DEBUG
                     qDebug() << "Adding SVG tween transformation - photogram -> " << photogram;
                 #endif

                 int step = photogram - origin;
                 stepItem = tween->stepAt(step);

                 if (stepItem->has(TupTweenerStep::Motion)) {
                     qreal dx = stepItem->getPosition().x() - (object->lastTweenPos().x() + adjustX);
                     qreal dy = stepItem->getPosition().y() - (object->lastTweenPos().y() + adjustY);
                     object->moveBy(dx, dy);
                     QPointF offset = QPoint(-adjustX, -adjustY);
                     object->setLastTweenPos(stepItem->getPosition() + offset);
                 } else if (stepItem->has(TupTweenerStep::Rotation)) {
                     double angle = stepItem->getRotation();
                     object->setRotation(angle);
                 } else if (stepItem->has(TupTweenerStep::Scale)) {
                     QPointF point = tween->transformOriginPoint();

                     double scaleX = stepItem->horizontalScale();
                     double scaleY = stepItem->verticalScale();
                     QTransform transform;
                     transform.translate(point.x(), point.y());
                     transform.scale(scaleX, scaleY);
                     transform.translate(-point.x(), -point.y());

                     object->setTransform(transform);
                 } else if (stepItem->has(TupTweenerStep::Shear)) {
                     QPointF point = tween->transformOriginPoint();

                     double shearX = stepItem->horizontalShear();
                     double shearY = stepItem->verticalShear();
                     QTransform transform;
                     transform.translate(point.x(), point.y());
                     transform.shear(shearX, shearY);
                     transform.translate(-point.x(), -point.y());

                     object->setTransform(transform);
                 }

                 int level = tween->getZLevel();
                 if (level > 0)
                     level--;

                 object->setZValue(((BG_LAYERS + layerIndex) * ZLAYER_LIMIT) + level);
                 // Including SVG object into the scene
                 addSvgObject(object, TupFrame::Regular, 1.0, true);

                 if (stepItem->has(TupTweenerStep::Opacity)) {
                     double transp = stepItem->getOpacity();
                     if (transp == 0)
                         object->setOpacity(0);
                     else
                         if (opacity < transp)
                             object->setOpacity(opacity);
                         else
                             object->setOpacity(transp);

                     if (opacity < 1)
                         return;
                 } else if (opacity < 1) {
                     object->setOpacity(opacity);
                     return;
                 }
             }

             QString tip = object->toolTip();
             if (tip.length() == 0) {
                 object->setToolTip("Tweens: " + tween->tweenTypeToString());
             } else if (!tip.contains(tween->tweenTypeToString())) {
                 tip += "," + tween->tweenTypeToString();
                 object->setToolTip(tip);
             }
         }
    }
}

void TupGraphicsScene::addLipSyncObjects(TupLayer *layer, int photogram, int zValue)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::addLipSyncObjects()]";
    #endif

    if (layer->lipSyncCount() > 0) {
        Mouths mouths = layer->getLipSyncList();
        int total = mouths.count();

        TupLipSync *lipSync;
        TupLibraryFolder *folder;
        TupVoice *voice;
        TupPhoneme *phoneme;
        TupLibraryObject *mouthImg;
        TupGraphicLibraryItem *item;
        // Looking for through all the mouths
        for (int i=0; i<total; i++) {
             lipSync = mouths.at(i);
             int initFrame = lipSync->getInitFrame();
             // If photogram is in the range of the lip-sync object
             if ((photogram >= initFrame) && (photogram <= initFrame + lipSync->getFramesCount())) {
                 QString name = lipSync->getLipSyncName();
                 folder = library->getFolder(name);
                 if (folder) {
                     QList<TupVoice *> voices = lipSync->getVoices();
                     int voicesTotal = voices.count();
                     // Looking for through all the voices
                     for(int j=0; j<voicesTotal; j++) {
                         voice = voices.at(j);
                         int index = photogram - initFrame;
                         // Voice contains a phoneme for this frame
                         if (voice->contains(index)) {
                             // Adding phoneme image
                             phoneme = voice->getPhonemeAt(index);
                             if (phoneme) {
                                 QString imgName = phoneme->value() + lipSync->getPicExtension();
                                 mouthImg = folder->getObject(imgName);
                                 if (mouthImg) {
                                     item = new TupGraphicLibraryItem(mouthImg);
                                     // Adding image of the mouth phoneme
                                     if (item) {
                                         QPointF pos = phoneme->position();
                                         if (lipSync->getPicExtension().compare(".tobj") == 0) {
                                             pos -= item->boundingRect().center();
                                             item->setPos(pos);
                                         } else {
                                             int wDelta = static_cast<int> (item->boundingRect().width()/2);
                                             int hDelta = static_cast<int> (item->boundingRect().height()/2);
                                             item->setPos(pos.x() - wDelta, pos.y() - hDelta);
                                         }
                                         item->setToolTip(tr("lipsync:") + name + ":" + QString::number(j));
                                         item->setZValue(zValue);
                                         addItem(item);
                                     }
                                 } else {
                                     #ifdef TUP_DEBUG
                                         qDebug() << "[TupGraphicsScene::addLipSyncObjects()] - Warning: Can't find phoneme image -> "
                                                  << imgName;
                                     #endif
                                 } 
                             } else {
                                 #ifdef TUP_DEBUG
                                     qDebug() << "[TupGraphicsScene::addLipSyncObjects()] - Warning: No lipsync phoneme at frame "
                                              << photogram << " - index: " << index;
                                 #endif

                                 // Adding rest phoneme to cover empty frame
                                 QString imgName = "rest" + lipSync->getPicExtension();
                                 mouthImg = folder->getObject(imgName);
                                 if (mouthImg) {
                                     item = new TupGraphicLibraryItem(mouthImg);
                                     if (item) {
                                         QPointF pos = voice->mouthPos();
                                         int wDelta = static_cast<int> (item->boundingRect().width()/2);
                                         int hDelta = static_cast<int> (item->boundingRect().height()/2);
                                         item->setPos(pos.x()-wDelta, pos.y()-hDelta);
                                         item->setToolTip(tr("lipsync:") + name + ":" + QString::number(j));
                                         item->setZValue(zValue);
                                         addItem(item);
                                     }
                                 } else {
                                     #ifdef TUP_DEBUG
                                         qDebug() << "[TupGraphicsScene::addLipSyncObjects()] - Warning: Can't find phoneme image -> "
                                                  << imgName;
                                     #endif
                                 }
                             }
                         } else {
                             #ifdef TUP_DEBUG
                                 qDebug() << "[TupGraphicsScene::addLipSyncObjects()] - No lipsync phoneme in voice at position: "
                                          << j << " - looking for index: " << index;
                             #endif
                         }
                     }
                 } else {
                     #ifdef TUP_DEBUG
                         qDebug() << "[TupGraphicsScene::addLipSyncObjects()] - Folder with lipsync mouths is not available -> "
                                  << name;
                     #endif
                 } 
             }
        }
    }
}

void TupGraphicsScene::cleanWorkSpace()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::cleanWorkSpace()]";
    #endif

    if (vectorDynamicBg) {
        vectorDynamicBg = nullptr;
        delete vectorDynamicBg;
    }

    onionSkin.accessMap.clear();

    foreach (QGraphicsItem *item, items()) {
        if (item->scene() == this)
            removeItem(item);
    }

    foreach (TupLineGuide *line, lines)
        addItem(line);
}

int TupGraphicsScene::currentFrameIndex() const
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::currentFrameIndex()]";
    #endif
    */

    return framePosition.frame;
}

int TupGraphicsScene::currentLayerIndex() const
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::currentLayerIndex()]";
    #endif
    */

    return framePosition.layer;
}

int TupGraphicsScene::currentSceneIndex() const
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::currentSceneIndex()]";
    #endif
    */

    if (!tupScene) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupGraphicsScene::currentSceneIndex()] - Error: Scene index is -1";
        #endif
        return -1;
    }

    return tupScene->objectIndex();
}

void TupGraphicsScene::setNextOnionSkinCount(int n)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::setNextOnionSkinCount()]";
    #endif
    */

    onionSkin.next = n;

    if (spaceContext == TupProject::FRAMES_MODE)
        drawCurrentPhotogram();
}

void TupGraphicsScene::setPreviousOnionSkinCount(int n)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::setPreviousOnionSkinCount()]";
    #endif
    */

    onionSkin.previous = n;

    if (spaceContext == TupProject::FRAMES_MODE)
        drawCurrentPhotogram();
}

TupFrame *TupGraphicsScene::currentFrame()
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::currentFrame()]";
    #endif
    */

    if (tupScene) {
        if (tupScene->layersCount() > 0) {
            if (framePosition.layer < tupScene->layersCount()) {
                TupLayer *layer = tupScene->layerAt(framePosition.layer);
                // Q_CHECK_PTR(layer);
                if (layer) {
                    if (!layer->getFrames().isEmpty())
                        return layer->frameAt(framePosition.frame);
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupGraphicsScene::currentFrame()] - No layer available at -> "
                                    + QString::number(framePosition.frame);
                    #endif
                }
            } else {
                TupLayer *layer = tupScene->layerAt(tupScene->layersCount() - 1);
                if (layer) {
                    if (!layer->getFrames().isEmpty())
                        return layer->frameAt(framePosition.frame);
                }
            }
        }
    }

    return nullptr;
}

void TupGraphicsScene::setCurrentScene(TupScene *scene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::setCurrentScene()]";
    #endif

    if (scene) {
        setCurrentFrame(0, 0);
        if (gTool)
            gTool->aboutToChangeScene(this);
        qDeleteAll(lines);
        lines.clear();

        cleanWorkSpace();
        tupScene = scene;

        background = tupScene->sceneBackground();

        if (!background->rasterStaticBgIsNull())
            rasterStaticBg->setPixmap(background->rasterStaticBackground());

        if (!background->rasterDynamicBgIsNull())
            rasterDynamicBg->setPixmap(background->rasterDynamicExpandedImage());

        if (spaceContext == TupProject::FRAMES_MODE) {
            drawCurrentPhotogram();
        } else if (spaceContext == TupProject::VECTOR_FG_MODE) {
            cleanWorkSpace();
            drawVectorFg();
        } else {
            cleanWorkSpace();
            drawSceneBackground(framePosition.frame);
        }
    }
}

TupScene *TupGraphicsScene::currentScene() const
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::scene()]";
    #endif
    */

    if (tupScene)
        return tupScene;

    return nullptr;
}

void TupGraphicsScene::setTool(TupToolPlugin *plugin)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::setTool()]";
    #endif

    if (spaceContext == TupProject::FRAMES_MODE) {
        drawCurrentPhotogram();
    } else if (spaceContext == TupProject::VECTOR_FG_MODE) {
        cleanWorkSpace();
        drawVectorFg();
    } else {
        cleanWorkSpace();
        drawSceneBackground(framePosition.frame);
    }

    if (gTool)
        gTool->aboutToChangeTool();

    gTool = plugin;
    gTool->init(this);
}

TupToolPlugin *TupGraphicsScene::currentTool() const
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::currentTool()]";
    #endif
    */

    return gTool;
}

void TupGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::mousePressEvent()]";
    #endif

    QGraphicsScene::mousePressEvent(event);
    inputInformation->updateFromMouseEvent(event);
    isDrawing = false;

    // This condition locks all the tools while workspace is rotated 
    if ((event->buttons() != Qt::LeftButton) || (event->modifiers () != (Qt::ShiftModifier | Qt::ControlModifier))) {
        if (gTool) {
            if (gTool->toolType() == TupToolPlugin::Brush && event->isAccepted())
                return;

            if (gTool->toolType() == TupToolPlugin::Tweener && event->isAccepted()) {
                if (gTool->currentEditMode() == TupToolPlugin::Properties)
                    return;
            } 

            // If there's no frame... the tool is disabled 
            if (currentFrame()) {
                if (event->buttons() == Qt::LeftButton) {
                    gTool->begin();
                    isDrawing = true;
                    gTool->press(inputInformation, brushManager, this);
                }
            }
        }
    }
}

void TupGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::mouseMoveEvent()]";
    #endif
    */

    QGraphicsScene::mouseMoveEvent(event);
    mouseMoved(event);

    if (gTool) {
        if (gTool->toolId() == TAction::Line || gTool->toolId() == TAction::Polyline)
            gTool->updatePos(event->scenePos());
    }
}

void TupGraphicsScene::mouseMoved(QGraphicsSceneMouseEvent *event)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::mouseMoved()]";
    #endif
    */

    inputInformation->updateFromMouseEvent(event);

    if (gTool && isDrawing)
        gTool->move(inputInformation, brushManager,  this);
}

void TupGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::mouseReleaseEvent()]";
    #endif
    */

    // SQA: Temporal solution for cases when there's no current frame defined
    if (!currentFrame())
        return;

    QGraphicsScene::mouseReleaseEvent(event);
    mouseReleased(event);
}

void TupGraphicsScene::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::mouseReleased()]";
    #endif

    if (gTool) {
        if (gTool->toolType() == TupToolInterface::Brush) {
            if (event->button() == Qt::RightButton) 
                return;
        }
    }

    if (currentFrame()) {
        if (currentFrame()->isFrameLocked()) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupGraphicsScene::mouseReleased()] - Frame is locked!";
            #endif
            return;
        }
    }

    inputInformation->updateFromMouseEvent(event);

    if (isDrawing) {
        if (gTool) {
            gTool->release(inputInformation, brushManager, this);
            gTool->end();
        }
    }

    isDrawing = false;
}

void TupGraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::mouseDoubleClickEvent()]";
    #endif

    QGraphicsScene::mouseDoubleClickEvent(event);
    inputInformation->updateFromMouseEvent(event);

    if (gTool)
        gTool->doubleClick(inputInformation, this);
}

void TupGraphicsScene::keyPressEvent(QKeyEvent *event)
{
    if (gTool) {
        gTool->keyPressEvent(event);
        
        if (event->isAccepted())
            return;
    }
    
    QGraphicsScene::keyPressEvent(event);
}

void TupGraphicsScene::keyReleaseEvent(QKeyEvent *event)
{
    if (gTool) {
        gTool->keyReleaseEvent(event);

        if (event->isAccepted())
            return;
    }
   
    QGraphicsScene::keyReleaseEvent(event);
}

/*
// SQA: Check this code, not sure whether it does something or it's handy :S
void TupGraphicsScene::dragEnterEvent(QGraphicsSceneDragDropEvent * event)
{
    if (event->mimeData()->hasFormat("tupi-ruler"))
        event->acceptProposedAction();

    TupLineGuide *line = 0;
    if (event->mimeData()->data("tupi-ruler") == "verticalLine") {
        line  = new TupLineGuide(Qt::Vertical, this);
        line->setPos(event->scenePos());
    } else {
        line = new TupLineGuide(Qt::Horizontal, this);
        line->setPos(event->scenePos());
    }

    if (line)
        lines << line;
}

// TODO: Check this code, not sure whether it does something or it is helping :S

void  TupGraphicsScene::dragLeaveEvent(QGraphicsSceneDragDropEvent * event)
{
    Q_UNUSED(event);

    removeItem(lines.last());
    delete lines.takeLast();
}

// TODO: Check this code, not sure whether it does something or it is helping :S

void TupGraphicsScene::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
    if (!lines.isEmpty())
        lines.last()->setPos(event->scenePos());
}

// TODO: Check this code, not sure whether it does something or it is helping :S

void TupGraphicsScene::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    Q_UNUSED(event);

    if (gTool) {
        if (gTool->toolType() == TupToolPlugin::ObjectsTool) {
            lines.last()->setEnabledSyncCursor(false);
            lines.last()->setFlag(QGraphicsItem::ItemIsMovable, true);
        }
    }
}
*/

bool TupGraphicsScene::event(QEvent *event)
{
    return QGraphicsScene::event(event);
}

void TupGraphicsScene::sceneResponse(TupSceneResponse *response)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::sceneResponse()]";
    #endif
    */

    if (gTool)
        gTool->sceneResponse(response);
}

void TupGraphicsScene::layerResponse(TupLayerResponse *response)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::layerResponse()]";
    #endif
    */

    if (gTool)
        gTool->layerResponse(response);
}

void TupGraphicsScene::frameResponse(TupFrameResponse *response)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::frameResponse()]";
    #endif
    */

    if (gTool)
        gTool->frameResponse(response);
}

void TupGraphicsScene::itemResponse(TupItemResponse *response)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::itemResponse()]";
    #endif
    */
   
    if (gTool)
        gTool->itemResponse(response);

    if (spaceContext == TupProject::VECTOR_DYNAMIC_BG_MODE) {
        if (response->getAction() == TupProjectRequest::Add || response->getAction() == TupProjectRequest::Remove)
            background->scheduleVectorRender(true);
    }
}

void TupGraphicsScene::libraryResponse(TupLibraryResponse *response)
{
    if (spaceContext == TupProject::VECTOR_DYNAMIC_BG_MODE) {
        if (response->getAction() == TupProjectRequest::Add || response->getAction() == TupProjectRequest::Remove)
            background->scheduleVectorRender(true);
    }
}

bool TupGraphicsScene::userIsDrawing() const
{
    return isDrawing;
}

TupBrushManager *TupGraphicsScene::getBrushManager() const
{
    return brushManager;
}

void TupGraphicsScene::setSelectionRange()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::setSelectionRange()]";
    #endif

    if (onionSkin.accessMap.empty() || gTool->toolType() == TupToolInterface::Tweener)
        return;

    QHash<QGraphicsItem *, bool>::iterator it = onionSkin.accessMap.begin();
    if (gTool->toolId() == TAction::ObjectSelection || gTool->toolId() == TAction::NodesEditor
        || gTool->toolId() == TAction::Text) {
        while (it != onionSkin.accessMap.end()) {
            // if item is a tween
            if (!it.value() || it.key()->toolTip().length() > 0) {
                it.key()->setAcceptedMouseButtons(Qt::NoButton);
                it.key()->setFlag(QGraphicsItem::ItemIsSelectable, false);
                it.key()->setFlag(QGraphicsItem::ItemIsMovable, false);
            } else {
                it.key()->setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MidButton | Qt::XButton1
                                                  | Qt::XButton2);
                if (gTool->toolId() == TAction::Text) {
                    if (qgraphicsitem_cast<TupTextItem *>(it.key())) {
                        it.key()->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
                    }
                } else if (gTool->toolId() == TAction::ObjectSelection) {
                    it.key()->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
                } else {
                    it.key()->setFlag(QGraphicsItem::ItemIsSelectable, true);
                    it.key()->setFlag(QGraphicsItem::ItemIsMovable, false);
                }
            }
            ++it;
        }
    } else {
        while (it != onionSkin.accessMap.end()) {
            it.key()->setAcceptedMouseButtons(Qt::NoButton);
            it.key()->setFlag(QGraphicsItem::ItemIsSelectable, false);
            it.key()->setFlag(QGraphicsItem::ItemIsMovable, false);
            ++it;
        }
    }
}

void TupGraphicsScene::enableItemsForSelection()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::enableItemsForSelection()]";
    #endif

    QHash<QGraphicsItem *, bool>::iterator it = onionSkin.accessMap.begin();
    while (it != onionSkin.accessMap.end()) {
        // if (it.value() && it.key()->toolTip().length() == 0)
        if (it.value())
            it.key()->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
        ++it;
    }
}

void TupGraphicsScene::includeObject(QGraphicsItem *object, bool isPolyLine) // SQA: Bool parameter is a hack for the Polyline tool. Must be fixed. 
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::includeObject()]";
    #endif
    */

    if (!object) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupGraphicsScene::includeObject()] - Fatal Error: Graphic item is nullptr!";
        #endif

        return;
    }

    if (spaceContext == TupProject::FRAMES_MODE) {
        TupLayer *layer = tupScene->layerAt(framePosition.layer);
        if (layer) {
            TupFrame *frame = layer->frameAt(framePosition.frame);
            if (frame) {
                // SQA: The constant 100 assumes than 100 items have been created per frame
                int zValue = (tupScene->framesCount()*100) + frame->getTopZLevel();
                if (isPolyLine) // SQA: Polyline hack
                    zValue--;

                qreal opacity = layer->getOpacity(); 
                if (opacity >= 0 && opacity <= 1) {
                    object->setOpacity(opacity);
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupGraphicsScene::includeObject()] - Fatal Error: Opacity value is invalid -> "
                                 << opacity;
                    #endif
                }

                #ifdef TUP_DEBUG
                    qDebug() << "[TupGraphicsScene::includeObject()] - zValue -> " << zValue;
                #endif

                object->setZValue(zValue);
                addItem(object);
                zLevel++;
            }
        }
    } else {
        if (background) {
            TupFrame *frame = new TupFrame;
            if (spaceContext == TupProject::VECTOR_STATIC_BG_MODE) {
                frame = background->vectorStaticFrame();
            } else if (spaceContext == TupProject::VECTOR_FG_MODE) {
                frame = background->vectorForegroundFrame();
            } else if (spaceContext == TupProject::VECTOR_DYNAMIC_BG_MODE) {
                frame = background->vectorDynamicFrame();
            }

            if (frame) {
                int zValue = frame->getTopZLevel();
                object->setZValue(zValue);
                addItem(object);
            }
        }
    }
}

void TupGraphicsScene::removeScene()
{
    cleanWorkSpace();
    tupScene = nullptr;
}

TupProject::Mode TupGraphicsScene::getSpaceContext()
{
    return spaceContext;
}

void TupGraphicsScene::setSpaceMode(TupProject::Mode mode)
{
    spaceContext = mode;
}

void TupGraphicsScene::setOnionFactor(double opacity)
{
    gOpacity = opacity;

    if (spaceContext == TupProject::FRAMES_MODE)
        drawCurrentPhotogram();
}

double TupGraphicsScene::getOpacity()
{
    return gOpacity;
}

int TupGraphicsScene::getFramesCount()
{
    TupLayer *layer = tupScene->layerAt(framePosition.layer);
    if (layer)
        return layer->framesCount();
    else
        return -1;
}

void TupGraphicsScene::setLibrary(TupLibrary *assets)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::setLibrary()]";
    #endif

    library = assets;
}

void TupGraphicsScene::resetCurrentTool() 
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupGraphicsScene::resetCurrentTool()]";
    #endif

    gTool->init(this);
}

TupInputDeviceInformation * TupGraphicsScene::inputDeviceInformation()
{
    return inputInformation;
}

void TupGraphicsScene::updateLoadingFlag(bool flag)
{
    loadingProject = flag;
}

void TupGraphicsScene::setWaterMarkFlag(bool enable)
{
    showWaterMark = enable;
}

QSize TupGraphicsScene::getSceneDimension()
{
    if (tupScene)
        return tupScene->getDimension();

    return QSize();
}
