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

#include <QGraphicsRectItem>
#include <QBrush>
#include <QPixmap>
#include <QCursor>

#include "tapplicationproperties.h"
#include "rastercanvas.h"

RasterCanvas::RasterCanvas(TupProject *project, const QColor contourColor, QWidget *parent):
                           RasterCanvasBase(project->getDimension(), parent)
{
    pressed = false;
    setBgColor(project->getBgColor());
    tableInUse = false;
    spaceBar = false;

    myPaintCanvas = MPHandler::handler();
    myPaintCanvas->setBrushColor(contourColor);
    myPaintCanvas->clearSurface();

    connect(myPaintCanvas, SIGNAL(newTile(MPSurface*, MPTile*)), this, SLOT(onNewTile(MPSurface*, MPTile*)));
    connect(myPaintCanvas, SIGNAL(updateTile(MPSurface*, MPTile*)), this, SLOT(onUpdateTile(MPSurface*, MPTile*)));
    connect(myPaintCanvas, SIGNAL(clearedSurface(MPSurface*)), this, SLOT(onClearedSurface(MPSurface*)));

    // Set scene
    drawingRect = QRectF(QPointF(0, 0), project->getDimension());
    gScene = new QGraphicsScene(this);
    gScene->setSceneRect(drawingRect);
    setScene(gScene);

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

    int centerX = projectSize.width()/2;
    int centerY = projectSize.height()/2;

    centerOn(QPointF(centerX, centerY));
    setSceneRect(0, 0, projectSize.width(), projectSize.height());
}

void RasterCanvas::setSize(QSize size)
{
    myPaintCanvas->setSurfaceSize(size);
}

void RasterCanvas::setTabletDevice(QTabletEvent* event)
{
    if (event->type() == QEvent::TabletEnterProximity) {
        tableInUse = true;
    } else if(event->type() == QEvent::TabletLeaveProximity) {
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

    // RasterCanvasBase::mousePressEvent(event);
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
    pressed = false;
    // RasterCanvasBase::mouseReleaseEvent(event);
}

void RasterCanvas::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F11 || event->key() == Qt::Key_Escape) {
        emit closeWindow();
        return;
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
    QImage image = myPaintCanvas->renderImage(QSize(static_cast<int>(drawingRect.size().width()),
                                                    static_cast<int>(drawingRect.size().height())));
    image.save(filePath);
}

void RasterCanvas::loadFromFile(QString filePath)
{
    // Clear the surface
    myPaintCanvas->clearSurface();

    // Laod the new image
    QImage image = QImage(filePath);
    myPaintCanvas->loadImage(image);
}

void RasterCanvas::updateCursor(const QTabletEvent *event)
{
    QCursor cursor;
    if (event->type() != QEvent::TabletLeaveProximity) {
        if (event->pointerType() == QTabletEvent::Eraser) {
            cursor = QCursor(QPixmap(RASTER_DIR + "resources/cursor-eraser.png"), 3, 28);
        } else {
            switch (event->device()) {
            case QTabletEvent::Stylus:
                cursor = QCursor(QPixmap(RASTER_DIR + "resources/cursor-pencil.png"), 0, 0);
                break;
            case QTabletEvent::Airbrush:
                cursor = QCursor(QPixmap(RASTER_DIR + "resources/cursor-airbrush.png"), 3, 4);
                break;
            case QTabletEvent::RotationStylus: {
                QImage origImg(RASTER_DIR + "resources/cursor-felt-marker.png");
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
