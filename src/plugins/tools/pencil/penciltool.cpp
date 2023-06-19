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

#include "penciltool.h"
#include "tuppaintareaevent.h"
#include "tuprequestbuilder.h"
#include "tupprojectrequest.h"
#include "tupgraphicalgorithm.h"

#include <QGraphicsView>
#include <QPolygonF>
#include <cmath>

PencilTool::PencilTool(): TupToolPlugin()
{
    settings = nullptr;
    item = nullptr;

    setupActions();
}

PencilTool::~PencilTool()
{
}

void PencilTool::setupActions()
{
    penCursor = QCursor(QPixmap(kAppProp->themeDir() + "cursors/target.png"), 4, 4);

    TAction *pencil = new TAction(QPixmap(kAppProp->themeDir() + "icons/pencil.png"), tr("Pencil"), this);
    pencil->setShortcut(QKeySequence(tr("P")));
    pencil->setToolTip(tr("Pencil") + " - " + tr("P"));
    pencil->setCursor(penCursor);
    pencil->setActionId(TAction::Pencil);

    penActions.insert(TAction::Pencil, pencil);
}

void PencilTool::init(TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PencilTool::init()]";
    #endif

    currentTool = PencilMode;
    settings->enablePencilMode();

    scene = gScene;
    brushManager = scene->getBrushManager();
    input = scene->inputDeviceInformation();
    resizeMode = false;

    setZValueReferences();
    circleZValue = ZLAYER_BASE + (scene->layersCount() * ZLAYER_LIMIT);

    TCONFIG->beginGroup("BrushParameters");
    penWidth = TCONFIG->value("Thickness", 3).toInt();
    eraserSize = TCONFIG->value("EraserSize", 10).toInt();

    #ifdef TUP_DEBUG
        qDebug() << "[PencilTool::init()] - eraserSize ->" << eraserSize;
    #endif

    qreal radius = eraserSize/2;
    qDebug() << "*** RADIUS ->" << radius;
    eraserDistance = QPointF(radius + 2, radius + 2);
    eraserPen = QPen(Qt::red, 3, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin);
    eraserCircle = new QGraphicsEllipseItem(0, 0, eraserSize, eraserSize);
    eraserCircle->setPen(eraserPen);

    foreach (QGraphicsView *view, scene->views())
        view->setDragMode(QGraphicsView::NoDrag);
}

void PencilTool::setZValueReferences()
{
    baseZValue = ((BG_LAYERS + scene->currentLayerIndex()) * ZLAYER_LIMIT);
    topZValue = ((BG_LAYERS + scene->currentLayerIndex()) * ZLAYER_LIMIT) + 100;
}

QList<TAction::ActionId> PencilTool::keys() const
{    
    return QList<TAction::ActionId>() << TAction::Pencil;
}

void PencilTool::press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PencilTool::press()] - currentTool ->" << currentTool;
    #endif

    firstPoint = input->pos();

    if (currentTool == PencilMode) {
        if (!resizeMode) {
            path = QPainterPath();
            path.moveTo(firstPoint);

            previousPos = input->pos();

            item = new TupPathItem();
            if (brushManager->pen().color().alpha() == 0) {
                QPen pen;
                pen.setWidth(1);
                pen.setBrush(QBrush(Qt::black));
                item->setPen(pen);
            } else {
                item->setPen(brushManager->pen());
            }
            gScene->includeObject(item);
        }
    } else { // EraserMode
        eraserCircle->setPos(firstPoint - eraserDistance);
        gScene->includeObject(eraserCircle);
        if (!lineItems.isEmpty())
            activeEraser(firstPoint);
    }
}

void PencilTool::move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PencilTool::move()]";
    #endif

    Q_UNUSED(brushManager)
    Q_UNUSED(gScene)

    QPointF currentPoint = input->pos();
    if (currentTool == PencilMode) {
        if (resizeMode) {
            QPointF result = penCirclePos - currentPoint;
            penWidth = static_cast<int>(sqrt(pow(result.x(), 2) + pow(result.y(), 2)));

            QPointF topLeft(penCirclePos.x() - (penWidth/2), penCirclePos.y() - (penWidth/2));
            QSize size(penWidth, penWidth);
            QRectF rect(topLeft, size);
            penCircle->setRect(rect);
        } else {
            if (!item)
                return;

            QPointF lastPoint = input->pos();

            path.moveTo(previousPos);
            path.lineTo(lastPoint);

            item->setPath(path);
            previousPos = lastPoint;
        }
    } else {
        eraserCircle->setPos(currentPoint - eraserDistance);
        if (!lineItems.isEmpty())
            activeEraser(currentPoint);
    }
}

void PencilTool::release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PencilTool::release()] - currentTool ->" << currentTool;
    #endif

    Q_UNUSED(brushManager)

    QPointF currentPoint = input->pos();
    if (currentTool == PencilMode) {
        if (!resizeMode) {
            if (!item)
                return;

            // Drawing a point
            if (firstPoint == input->pos() && path.elementCount() == 1) {
                gScene->removeItem(item);

                qreal radius = brushManager->pen().width();
                QPointF distance((radius + 2)/2, (radius + 2)/2);
                QPen inkPen(brushManager->penColor(), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                TupEllipseItem *blackEllipse = new TupEllipseItem(QRectF(currentPoint - distance,
                                                                  QSize(static_cast<int>(radius + 2), static_cast<int>(radius + 2))));
                blackEllipse->setPen(inkPen);
                blackEllipse->setBrush(inkPen.brush());
                gScene->includeObject(blackEllipse);

                QDomDocument doc;
                doc.appendChild(blackEllipse->toXml(doc));
                TupProjectRequest request = TupRequestBuilder::createItemRequest(gScene->currentSceneIndex(), gScene->currentLayerIndex(), gScene->currentFrameIndex(),
                                                                                 0, QPointF(0, 0), gScene->getSpaceContext(), TupLibraryObject::Item, TupProjectRequest::Add,
                                                                                 doc.toString());
                emit requested(&request);

                return;
            }

            TupGraphicalAlgorithm::smoothPath(path, smoothness);

            item->setPen(brushManager->pen());
            item->setBrush(brushManager->brush());
            item->setPath(path);

            QDomDocument doc;
            doc.appendChild(item->toXml(doc));

            TupProjectRequest request = TupRequestBuilder::createItemRequest(gScene->currentSceneIndex(), gScene->currentLayerIndex(), gScene->currentFrameIndex(),
                                                                             0, QPoint(), gScene->getSpaceContext(), TupLibraryObject::Item, TupProjectRequest::Add,
                                                                             doc.toString());
            emit requested(&request);
        }
    } else { // Eraser Mode
        gScene->removeItem(eraserCircle);
        if (!lineItems.isEmpty())
            activeEraser(currentPoint);
    }
}

QMap<TAction::ActionId, TAction *> PencilTool::actions() const
{
    return penActions;
}

TAction * PencilTool::getAction(TAction::ActionId toolId)
{
    return penActions[toolId];
}

int PencilTool::toolType() const
{
    return TupToolInterface::Brush;
}

QWidget *PencilTool::configurator()
{
    #ifdef TUP_DEBUG
        qDebug() << "[PencilTool::configurator()]";
    #endif

    if (!settings) {
        settings = new PenSettings;
        connect(settings, SIGNAL(toolEnabled(PenTool)), this, SLOT(updatePenTool(PenTool)));
        connect(settings, SIGNAL(smoothnessUpdated(double)), this, SLOT(updateSmoothness(double)));
        connect(settings, SIGNAL(eraserSizeChanged(int)), this, SLOT(updateEraserSize(int)));

        TCONFIG->beginGroup("PencilTool");
        smoothness = TCONFIG->value("Smoothness", 4.0).toDouble();
        if (smoothness == 0.0)
            smoothness = 4.0;
        settings->updateSmoothness(smoothness);
    }

    return settings;
}

void PencilTool::updatePenTool(PenTool tool)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PencilTool::updatePenTool()] - tool -> " << tool;
    #endif

    currentTool = tool;

    if (tool == EraserMode)
        storePathItems();
}

void PencilTool::storePathItems()
{
    #ifdef TUP_DEBUG
        qDebug() << "[PencilTool::storePathItems()]";
    #endif

    // Store all the path items of the current frame in a list
    lineItems.clear();
    foreach (QGraphicsItem *item, scene->items()) {
        if (TupPathItem *line = qgraphicsitem_cast<TupPathItem *> (item)) {
            qDebug() << "";
            qDebug() << "BASE Z Value -> " << baseZValue;
            qDebug() << "line Z Value -> " << line->zValue();
            qDebug() << "item Z Value -> " << item->zValue();
            qDebug() << "TOP Z Value -> " << topZValue;

            int zVal = line->zValue();
            if (baseZValue <= zVal && zVal < topZValue) {
                qDebug() << "Storing line!";
                lineItems << line;
            } else {
                qDebug() << "Line doesn't classify!";
            }
        }
    }
}

void PencilTool::updateSmoothness(double value)
{
    smoothness = value;

    TCONFIG->beginGroup("PencilTool");
    TCONFIG->setValue("Smoothness", QString::number(smoothness, 'f', 2));
}

void PencilTool::aboutToChangeTool() 
{
}

void PencilTool::saveConfig()
{
    #ifdef TUP_DEBUG
        qDebug() << "[PencilTool::saveConfig()]";
    #endif

    TCONFIG->beginGroup("PencilTool");
    TCONFIG->setValue("Smoothness", QString::number(smoothness, 'f', 2));

    TCONFIG->beginGroup("BrushParameters");
    TCONFIG->setValue("EraserSize", eraserSize);

    /*
    if (settings) {
        settings = new PenSettings;
        connect(settings, SIGNAL(toolEnabled(PenTool)), this, SLOT(updatePenTool(PenTool)));
        connect(settings, SIGNAL(smoothnessUpdated(double)), this, SLOT(updateSmoothness(double)));
        connect(settings, SIGNAL(eraserSizeChanged(int)), this, SLOT(updateEraserSize(int)));

        settings->updateSmoothness(smoothness);
    }
    */
}

void PencilTool::keyPressEvent(QKeyEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PencilTool::keyPressEvent()]";
    #endif

    if (event->modifiers() == Qt::ShiftModifier && currentTool == PencilMode) {
        resizeMode = true;
        input = scene->inputDeviceInformation();
        int diameter = brushManager->penWidth();
        int radius = diameter/2;
        penCirclePos = input->pos();

        penCircle = new QGraphicsEllipseItem(penCirclePos.x() - radius, penCirclePos.y() - radius, diameter, diameter);
        penCircle->setZValue(circleZValue);
        scene->addItem(penCircle);

        return;
    }

    if (event->key() == Qt::Key_F11 || event->key() == Qt::Key_Escape) {
        emit closeHugeCanvas();

        return;
    }

    QPair<int, int> flags = TAction::setKeyAction(event->key(), event->modifiers());
    if (flags.first != -1 && flags.second != -1)
        emit callForPlugin(flags.first, flags.second);
}

void PencilTool::keyReleaseEvent(QKeyEvent *event) 
{
    #ifdef TUP_DEBUG
        qDebug() << "[PencilTool::keyReleaseEvent()]";
    #endif

    Q_UNUSED(event)

    if (resizeMode) {
        resizeMode = false;
        scene->removeItem(penCircle);

        TCONFIG->beginGroup("BrushParameters");
        TCONFIG->setValue("Thickness", penWidth);

        emit penWidthChanged(penWidth);
    }
}

QCursor PencilTool::toolCursor() // const
{
    return penCursor;
}

void PencilTool::sceneResponse(const TupSceneResponse *event)
{
    Q_UNUSED(event)
}

void PencilTool::updateEraserSize(int value)
{
    eraserSize = value;
}

void PencilTool::frameResponse(const TupFrameResponse *event)
{
    Q_UNUSED(event)

    setZValueReferences();
}

TupFrame* PencilTool::getCurrentFrame()
{
    TupFrame *frame = nullptr;
    if (scene->getSpaceContext() == TupProject::FRAMES_MODE) {
        frame = scene->currentFrame();
        currentLayer = scene->currentLayerIndex();
        currentFrame = scene->currentFrameIndex();
    } else {
        currentLayer = -1;
        currentFrame = -1;

        TupScene *tupScene = scene->currentScene();
        TupBackground *bg = tupScene->sceneBackground();
        if (tupScene && bg) {
            if (scene->getSpaceContext() == TupProject::VECTOR_STATIC_BG_MODE) {
                frame = bg->vectorStaticFrame();
            } else if (scene->getSpaceContext() == TupProject::VECTOR_FG_MODE) {
                frame = bg->vectorForegroundFrame();
            } else if (scene->getSpaceContext() == TupProject::VECTOR_DYNAMIC_BG_MODE) {
                frame = bg->vectorDynamicFrame();
            }
        }
    }

    return frame;
}

void PencilTool::activeEraser(const QPointF &point)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PencilTool::activeEraser()] - point ->" << point;
    #endif

    qDebug() << "PencilTool::activeEraser() - lineItems.size() ->" << lineItems.size();
    for (int i=0; i<lineItems.size(); i++) {
        TupPathItem *item = lineItems.at(i);
        if (item->pointMatchesPath(point, eraserSize/2, EraserMode)) {
            // Process item here
            qDebug() << "";
            qDebug() << "PencilTool::activeEraser() - MATCH!!!";
            qDebug() << "PencilTool::activeEraser() - eraser size ->" << eraserSize;
            QPair<QString, QString> segments = item->extractPathSegments(point, eraserSize/2);
            QString segment1 = segments.first;
            QString segment2 = segments.second;

            qDebug() << "  *** segment1 ->" << segment1;
            qDebug() << "  *** segment2 ->" << segment2;

            if (segment1.compare("-1") != 0) {
                TupFrame *frame = getCurrentFrame();
                int itemIndex = frame->indexOf(item);

                if (!segment1.isEmpty() && !segment2.isEmpty()) {
                    qDebug() << "PencilTool::activeEraser() - Adding TWO segments!";
                } else if (segment1.isEmpty() && !segment2.isEmpty()) {
                    qDebug() << "PencilTool::activeEraser() - Adding segment2";
                    // item->setPathFromString(segment2);
                    TupProjectRequest event = TupRequestBuilder::createItemRequest(scene->currentSceneIndex(),
                                                                                   currentLayer, currentFrame, itemIndex,
                                                                                   QPointF(), scene->getSpaceContext(), TupLibraryObject::Item,
                                                                                   TupProjectRequest::EditNodes, segment2);
                    emit requested(&event);
                } else if (segment2.isEmpty() && !segment1.isEmpty()) {
                    qDebug() << "PencilTool::activeEraser() - Adding segment1";
                    // item->setPathFromString(segment1);
                    TupProjectRequest event = TupRequestBuilder::createItemRequest(scene->currentSceneIndex(),
                                                                                   currentLayer, currentFrame, itemIndex,
                                                                                   QPointF(), scene->getSpaceContext(), TupLibraryObject::Item,
                                                                                   TupProjectRequest::EditNodes, segment1);
                    emit requested(&event);
                } else {
                    qDebug() << "PencilTool::activeEraser() - Removing item...";
                    qDebug() << "currentLayer ->" << currentLayer;
                    qDebug() << "currentFrame ->" << currentFrame;
                    qDebug() << "position ->" << itemIndex;

                    scene->removeItem(item);
                    lineItems.removeAt(i);

                    TupProjectRequest event = TupRequestBuilder::createItemRequest(scene->currentSceneIndex(),
                                                                                   currentLayer, currentFrame, itemIndex, QPointF(), scene->getSpaceContext(),
                                                                                   TupLibraryObject::Item, TupProjectRequest::Remove);
                    emit requested(&event);

                }
            } else {
                qDebug() << "PencilTool::activeEraser() - Warning: Eraser action failed!";
            }

            qDebug() << "---";
            qDebug() << "";
        } else {
            qDebug() << "PencilTool::activeEraser() - NO match!!!";
        }
    }
}
