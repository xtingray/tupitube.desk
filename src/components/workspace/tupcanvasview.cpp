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

#include "tupcanvasview.h"
#include <cmath>

TupCanvasView::TupCanvasView(QWidget *parent, TupGraphicsScene *gScene, const QSize &sSize, const QSize &pSize,
                             const QColor &bgColor) : QGraphicsView(parent)
{
    setAccessibleName("FULL_SCREEN");
    screenSize = sSize;
    projectSize = pSize;
    bg = bgColor;
    scene = gScene;
    spaceBar = false;

    drawingRect = scene->sceneRect();
    centerPoint = drawingRect.center().toPoint();

    setScene(scene);
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::TextAntialiasing, true);
    setBackgroundBrush(QBrush(bg, Qt::SolidPattern));
    setMouseTracking(true);
    setInteractive(true);
}

TupCanvasView::~TupCanvasView()
{
}

void TupCanvasView::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter, rect);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(Qt::NoPen));

    double w = (double) projectSize.width() / (double) 2;
    double h = (double) projectSize.height() / (double) 2;

    painter->setPen(QPen(QColor(230, 230, 230, 255), 6));
    QPointF topLeft = drawingRect.center() - QPointF(w, h);
    QPointF bottomRight = drawingRect.center() + QPointF(w, h);

    painter->drawLine(topLeft - QPoint(0, 30), topLeft + QPoint(0, 30));  
    painter->drawLine(topLeft - QPoint(30, 0), topLeft + QPoint(30, 0)); 

    painter->drawLine(bottomRight - QPoint(0, 30), bottomRight + QPoint(0, 30));          
    painter->drawLine(bottomRight - QPoint(30, 0), bottomRight + QPoint(30, 0));  

    painter->restore();
}

void TupCanvasView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        emit rightClick();
        return;
    }

    scene->setSelectionRange();
    QGraphicsView::mousePressEvent(event);
}

void TupCanvasView::mouseMoveEvent(QMouseEvent *event)
{
    QPoint point = mapToScene(event->pos()).toPoint();
    if (spaceBar) {
        updateCenter(point);
        return;
    } else {
        initPoint = point;
    }

    QGraphicsView::mouseMoveEvent(event);
}

void TupCanvasView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space)
        spaceBar = true;

    if (event->key() == Qt::Key_Space)
        spaceBar = true;

    if (event->key() == Qt::Key_1) {
        if (event->modifiers() == Qt::NoModifier) {
            emit zoomIn();
            return;
        }
    }

    if (event->key() == Qt::Key_2) {
        if (event->modifiers() == Qt::NoModifier) {
            emit zoomOut();
            return;
        }
    }

    if (event->key() == Qt::Key_PageUp) {
        emit frameBackward();
        return;
    }

    if (event->key() == Qt::Key_PageDown) {
        emit frameForward();
        return;
    }

    QGraphicsView::keyPressEvent(event);
}

void TupCanvasView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space)
        spaceBar = false;

    QGraphicsView::keyReleaseEvent(event);
}

void TupCanvasView::updateCenter(const QPoint point)
{
    int x = point.x();
    int y = point.y();

    int cx = centerPoint.x();
    int cy = centerPoint.y();

    int x0 = initPoint.x();
    int y0 = initPoint.y();

    // int b = fabs(x0 - x);
    // int h = fabs(y0 - y);
    int b = abs(x0 - x);
    int h = abs(y0 - y);
    if (x0 > x)
        cx += b;
    else
        cx -= b;

    if (y0 > y)
        cy += h;
    else
        cy -= h;

    centerPoint = QPoint(cx, cy);
    centerOn(centerPoint);
    setSceneRect(cx - (drawingRect.width()/2), cy - (drawingRect.height()/2), drawingRect.width(), drawingRect.height());
}
