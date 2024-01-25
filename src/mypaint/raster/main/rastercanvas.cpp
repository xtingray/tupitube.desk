/*
    Copyright © 2015 by The QTMyPaint Project

    This file is part of QTMyPaint, a Qt-based interface for MyPaint C++ library.

    QTMyPaint is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QTMyPaint is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QTMyPaint. If not, see <http://www.gnu.org/licenses/>.
*/

#include "rastercanvas.h"
#include "tapplicationproperties.h"

#include <QGraphicsRectItem>
#include <QBrush>
#include <QPixmap>
#include <QCursor>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

RasterCanvas::RasterCanvas(TupProject *project, double brushWidth, const QColor contourColor, QWidget *parent):
                           RasterCanvasBase(project->getDimension(), parent)
{
    pressed = false;
    setBgColor(project->getCurrentBgColor());
    tableInUse = false;
    spaceBar = false;
    brushSize = brushWidth;

    #ifdef TUP_DEBUG
        qDebug() << "[RasterCanvas()] - brushSize ->" << brushSize;
    #endif

    // Set scene
    canvasSize = project->getDimension();
    drawingRect = QRectF(QPointF(0, 0), canvasSize);
    gScene = new QGraphicsScene(this);
    gScene->setSceneRect(drawingRect);
    setScene(gScene);

    myPaintCanvas = MPHandler::handler();
    myPaintCanvas->setBrushColor(contourColor);
    myPaintCanvas->clearSurface();

    connect(myPaintCanvas, SIGNAL(newTile(MPSurface*,MPTile*)), this, SLOT(onNewTile(MPSurface*,MPTile*)));
    connect(myPaintCanvas, SIGNAL(updateTile(MPSurface*,MPTile*)), this, SLOT(onUpdateTile(MPSurface*,MPTile*)));
    connect(myPaintCanvas, SIGNAL(clearedSurface(MPSurface*)), this, SLOT(onClearedSurface(MPSurface*)));

    QCursor cursor = QCursor(QPixmap(kAppProp->themeDir() + "cursors/target.png"), 4, 4);
    viewport()->setCursor(cursor);

    centerDrawingArea();
}

RasterCanvas::~RasterCanvas()
{
}

void RasterCanvas::resetMem()
{
    myPaintCanvas->resetMem();

    myPaintCanvas = nullptr;
    delete myPaintCanvas;

    gScene = nullptr;
    delete gScene;
}

void RasterCanvas::centerDrawingArea()
{
    centerOn(drawingRect.center());
}

void RasterCanvas::resetWorkSpaceCenter(const QSize projectSize)
{
    #ifdef TUP_DEBUG
        qDebug() << "[RasterCanvas::resetWorkSpaceCenter()]";
    #endif

    int centerX = projectSize.width() / 2;
    int centerY = projectSize.height() / 2;

    centerOn(QPointF(centerX, centerY));
    setSceneRect(0, 0, projectSize.width(), projectSize.height());
}

void RasterCanvas::setTabletDevice(QTabletEvent* event)
{
    if (event->type() == QEvent::TabletEnterProximity) {
        tableInUse = true;
    } else if (event->type() == QEvent::TabletLeaveProximity) {
        tableInUse = false;
    }

    updateCursor(event);
}

void RasterCanvas::onNewTile(MPSurface *surface, MPTile *tile)
{
    Q_UNUSED(surface)
    gScene->addItem(tile);
}

void RasterCanvas::onUpdateTile(MPSurface *surface, MPTile *tile)
{
    Q_UNUSED(surface)
    tile->update();
}

void RasterCanvas::onClearedSurface(MPSurface *surface)
{
    Q_UNUSED(surface)
}

void RasterCanvas::loadBrush(const QByteArray &content)
{
    QString input = QString(content);
    input.replace("\n","");
    input.replace("\\","");
    QJsonDocument json = QJsonDocument::fromJson(input.toUtf8());
    QJsonObject brush = json.object();

    #ifdef TUP_DEBUG
        QJsonValue value = brush.value(QString("comment"));
        qDebug() << "[RasterCanvas::loadBrush()] - Brush Name ->" << value.toString();
        qDebug() << "[RasterCanvas::loadBrush()] - Brush Size ->" << brushSize;
    #endif

    MPHandler::handler()->loadBrush(getBrushData(brush, brushSize));
}

void RasterCanvas::loadBrush(const QByteArray &content, double size)
{
    QString input = QString(content);
    input.replace("\n","");
    input.replace("\\","");
    QJsonDocument json = QJsonDocument::fromJson(input.toUtf8());
    QJsonObject brush = json.object();

    MPHandler::handler()->loadBrush(getBrushData(brush, size));
}

QByteArray RasterCanvas::getBrushData(QJsonObject brush, double size)
{
   QString targetKey = "settings";
   if (brush.contains(targetKey)) {
       QJsonValue jsonValue = brush.value(targetKey);
       QJsonObject nestedObject1 = jsonValue.toObject();
       QString targetKey1 = "radius_logarithmic";
       if (nestedObject1.contains(targetKey1)) {
           QJsonValue jsonValue1 = nestedObject1.value(targetKey1);
           QJsonObject nestedObject2 = jsonValue1.toObject();
           QString targetKey2 = "base_value";
           if (nestedObject2.contains(targetKey2)) {
               nestedObject2[targetKey2] = size;
               nestedObject1[targetKey1] = nestedObject2;
               brush[targetKey] = nestedObject1;
           }
       }
   }

   QJsonDocument jsonDocument(brush);
   return jsonDocument.toJson(QJsonDocument::Indented);
}

void RasterCanvas::tabletEvent(QTabletEvent *event)
{
    #ifdef TUP_DEBUG
       qDebug() << "[RasterCanvas::tabletEvent()]";
    #endif

    tableInUse = true;

    switch (event->type()) {
        case QEvent::TabletPress:
            if (event->pointerType() == QTabletEvent::Pen) {
                MPHandler::handler()->startStroke();
                event->accept();
            }
        break;
        case QEvent::TabletRelease:
            if (event->pointerType() == QTabletEvent::Pen) {
                // Finalize the stroke sequence.                
                myPaintCanvas->saveTiles();
                pressed = false;
                emit rasterStrokeMade();

                event->accept();
            }
        break;
        case QEvent::TabletMove:
            if (event->pointerType() == QTabletEvent::Pen) {
                QPointF pt(mapToScene(event->pos()));
                MPHandler::handler()->strokeTo(static_cast<float>(pt.x()), static_cast<float>(pt.y()),
                                               static_cast<float>(event->pressure()), event->xTilt(), event->yTilt());
                event->accept();
            }
        break;
        default:
        break;
    }
}

void RasterCanvas::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    #ifdef TUP_DEBUG
        qDebug() << "[RasterCanvas::mousePressEvent()]";
    #endif

    tableInUse = false;
    pressed = true;
    MPHandler::handler()->startStroke();
}

void RasterCanvas::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    #ifdef TUP_DEBUG
        qDebug() << "[RasterCanvas::mouseMoveEvent()]";
    #endif

    if (pressed) {
        if (!tableInUse) {
            QPointF pt = mapToScene(event->pos());
            MPHandler::handler()->strokeTo(static_cast<float>(pt.x()), static_cast<float>(pt.y()));
        }
    }
}

void RasterCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    #ifdef TUP_DEBUG
        qDebug() << "[RasterCanvas::mouseReleaseEvent()]";
    #endif

    myPaintCanvas->saveTiles();
    pressed = false;
    emit rasterStrokeMade();
}

void RasterCanvas::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()) {
        case Qt::Key_F11:
        case Qt::Key_Escape:
        case Qt::Key_Return:
            emit closeWindow();
            return;
        case Qt::Key_Plus:
            if (event->modifiers() == Qt::NoModifier) {
                emit zoomIn();
                return;
            }
        break;
        case Qt::Key_Minus:
            if (event->modifiers() == Qt::NoModifier) {
                emit zoomOut();
                return;
            }
    }

    RasterCanvasBase::keyPressEvent(event);
}

void RasterCanvas::updateBrushColor(const QColor color)
{
    MPHandler *mypaint = MPHandler::handler();
    mypaint->setBrushColor(color);
}

void RasterCanvas::setBrushSize(double size)
{
    #ifdef TUP_DEBUG
        qDebug() << "[RasterCanvas::setBrushSize()] - size ->" << size;
    #endif

    brushSize = size;
    loadBrush(myPaintCanvas->currentBrushData(), size);
}

void RasterCanvas::clearCanvas()
{
    myPaintCanvas->clearSurface();
}

void RasterCanvas::saveToFile(QString filePath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[RasterCanvas::saveToFile()] - filePath ->" << filePath;
    #endif

    QImage image = myPaintCanvas->renderImage(canvasSize);
    image.save(filePath);
}

void RasterCanvas::loadFromFile(QString filePath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[RasterCanvas::loadFromFile()]";
    #endif

    // Clear the surface
    myPaintCanvas->clearSurface();

    // Laod the new image
    QImage image = QImage(filePath);
    if (!image.isNull()) {
        myPaintCanvas->loadImage(image);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[RasterCanvas::loadFromFile()] - Fatal Error: Can't load image at path ->" << filePath;
        #endif
    }
}

void RasterCanvas::updateCursor(const QTabletEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[RasterCanvas::updateCursor()]";
    #endif

    QCursor cursor;
    if (event->type() != QEvent::TabletLeaveProximity) {
        if (event->pointerType() == QTabletEvent::Eraser) {
            cursor = QCursor(QPixmap(RASTER_RESOURCES_DIR + "resources/cursor-eraser.png"), 3, 28);
        } else {
        #ifdef TUP_32BIT
            switch (event->device()) {
        #else
            switch (event->deviceType()) {
        #endif
                case QTabletEvent::Stylus:
                    cursor = QCursor(QPixmap(RASTER_RESOURCES_DIR + "resources/cursor-pencil.png"), 0, 0);
                    break;
                case QTabletEvent::Airbrush:
                    cursor = QCursor(QPixmap(RASTER_RESOURCES_DIR + "resources/cursor-airbrush.png"), 3, 4);
                    break;
                case QTabletEvent::RotationStylus: {
                    QImage origImg(RASTER_RESOURCES_DIR + "resources/cursor-felt-marker.png");
                    QImage img(32, 32, QImage::Format_ARGB32);
                    QColor solid = color;
                    solid.setAlpha(255);
                    img.fill(solid);

                    QPainter painter(&img);
                    QTransform transform = painter.transform();
                    transform.translate(16, 16);
                    transform.rotate(-event->rotation());
                    painter.setTransform(transform);
                    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                    painter.drawImage(-24, -24, origImg);
                    painter.setCompositionMode(QPainter::CompositionMode_HardLight);
                    painter.drawImage(-24, -24, origImg);
                    painter.end();
                    cursor = QCursor(QPixmap::fromImage(img), 16, 16);
                } break;
                default:
                    break;
            }
        }
    }
    setCursor(cursor);
}

bool RasterCanvas::canvasIsEmpty()
{
    return myPaintCanvas->isEmpty();
}

void RasterCanvas::undo()
{
    #ifdef TUP_DEBUG
        qDebug() << "[RasterCanvas::undo()]";
    #endif

    myPaintCanvas->undo();
}

void RasterCanvas::redo()
{
    #ifdef TUP_DEBUG
        qDebug() << "[RasterCanvas::redo()]";
    #endif

    myPaintCanvas->redo();
}
