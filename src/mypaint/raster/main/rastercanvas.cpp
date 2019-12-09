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

RasterCanvas::RasterCanvas(TupProject *project, const QColor contourColor, QWidget *parent):
                           RasterCanvasBase(project->getDimension(), parent)
{
    pressed = false;
    setBgColor(project->getBgColor());
    tableInUse = false;
    spaceBar = false;
    // counter = 0;

    // Set scene
    canvasSize = project->getDimension();
    drawingRect = QRectF(QPointF(0, 0), canvasSize);
    gScene = new QGraphicsScene(this);
    gScene->setSceneRect(drawingRect);
    setScene(gScene);

    myPaintCanvas = MPHandler::handler();
    myPaintCanvas->setSurfaceSize(canvasSize);
    // myPaintCanvas->setScene(gScene);
    myPaintCanvas->setBrushColor(contourColor);
    myPaintCanvas->clearSurface();

    connect(myPaintCanvas, SIGNAL(newTile(MPSurface*, MPTile*)), this, SLOT(onNewTile(MPSurface*, MPTile*)));
    connect(myPaintCanvas, SIGNAL(updateTile(MPSurface*, MPTile*)), this, SLOT(onUpdateTile(MPSurface*, MPTile*)));
    connect(myPaintCanvas, SIGNAL(clearedSurface(MPSurface*)), this, SLOT(onClearedSurface(MPSurface*)));

    QCursor cursor = QCursor(THEME_DIR + "cursors/pencil.png", 0, 11);
    viewport()->setCursor(cursor);

    centerDrawingArea();
}

RasterCanvas::~RasterCanvas()
{
}

void RasterCanvas::centerDrawingArea()
{
    centerOn(drawingRect.center());
}

void RasterCanvas::resetWorkSpaceCenter(const QSize projectSize)
{
    #ifdef TUP_DEBUG
        qDebug() << "RasterCanvas::resetWorkSpaceCenter()";
    #endif

    int centerX = projectSize.width() / 2;
    int centerY = projectSize.height() / 2;

    centerOn(QPointF(centerX, centerY));
    setSceneRect(0, 0, projectSize.width(), projectSize.height());
}

/*
void RasterCanvas::setSize(QSize size)
{
    myPaintCanvas->setSurfaceSize(size);
}
*/

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
    // counter++;
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
    MPHandler::handler()->loadBrush(content);
}

void RasterCanvas::tabletEvent(QTabletEvent *event)
{
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
    pressed = true;
    MPHandler::handler()->startStroke();
}

void RasterCanvas::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    if (pressed) {
        if (!tableInUse) {
            QPointF pt = mapToScene(event->pos());
            MPHandler::handler()->strokeTo(static_cast<float>(pt.x()), static_cast<float>(pt.y()));
        }
    }

    RasterCanvasBase::mouseMoveEvent(event);
}

void RasterCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    /*
    qDebug() << "RasterCanvas::mouseReleaseEvent() - Releasing mouse...";
    qDebug() << "Tiles Count: " << myPaintCanvas->getTilesCounter();
    qDebug() << "Tile parts: " << counter;
    myPaintCanvas->saveScreen();
    tileSets << counter;
    counter = 0;
    */

    pressed = false;
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

void RasterCanvas::clearCanvas()
{
    myPaintCanvas->clearSurface();
}

void RasterCanvas::saveToFile(QString filePath)
{
    #ifdef TUP_DEBUG
        qDebug() << "RasterCanvas::saveToFile() - filePath: " << filePath;
    #endif

    /*
    QImage image = myPaintCanvas->renderImage(QSize(static_cast<int>(drawingRect.size().width()),
                                                    static_cast<int>(drawingRect.size().height())));
    */

    QImage image = myPaintCanvas->renderImage(canvasSize);
    image.save(filePath);
}

void RasterCanvas::loadFromFile(QString filePath)
{
    #ifdef TUP_DEBUG
        qDebug() << "RasterCanvas::loadFromFile() - Tracing...";
    #endif

    // Clear the surface
    myPaintCanvas->clearSurface();

    // Laod the new image
    QImage image = QImage(filePath);
    if (!image.isNull()) {
        myPaintCanvas->loadImage(image);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "RasterCanvas::loadFromFile() - Fatal Error: Can't load image at path: " << filePath;
        #endif
    }
}

void RasterCanvas::updateCursor(const QTabletEvent *event)
{
    QCursor cursor;
    if (event->type() != QEvent::TabletLeaveProximity) {
        if (event->pointerType() == QTabletEvent::Eraser) {
            cursor = QCursor(QPixmap(RASTER_RESOURCES_DIR + "resources/cursor-eraser.png"), 3, 28);
        } else {
            switch (event->device()) {
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

/*
void RasterCanvas::undo()
{
    myPaintCanvas->undo();
}

void RasterCanvas::redo()
{
    myPaintCanvas->redo();
}
*/
