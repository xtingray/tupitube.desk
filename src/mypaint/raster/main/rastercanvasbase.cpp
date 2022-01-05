/***************************************************************************
 *   Project TUPITUBE DESK                                                *
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

#include "rastercanvasbase.h"
#include "tuptextitem.h"
#include "tupgraphicalgorithm.h"

// TupiTube Framework 
#include "tupscene.h"
#include "tconfig.h"
#include "tapplication.h"
#include "tosd.h"

#include <QStyleOptionGraphicsItem>
#include <QClipboard>
#include <QTimer>
#include <QPolygon>
#include <QApplication>
#include <cmath>
#include <QGraphicsSceneMouseEvent>
#include <QScreen>

RasterCanvasBase::RasterCanvasBase(QSize dimension, QWidget *parent) : QGraphicsView(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "RasterCanvasBase::RasterCanvasBase()";
    #endif

    gScene = new QGraphicsScene(this);

    grid = nullptr;
    updateGridParameters();

    greenThickPen = QPen(QColor(0, 135, 0, 255), 2);

    grayPen = QPen(QColor(150, 150, 150, 255), 1);
    greenBoldPen = QPen(QColor(0, 135, 0, 255), 3);
    greenThinPen = QPen(QColor(0, 135, 0, 255), 1);
    blackPen = QPen(QColor(0, 0, 0, 180), 2);
    dotPen = QPen(QColor(150, 150, 150, 255), 1, Qt::DashLine);

    gridEnabled = false;
    safeAreaEnabled = false;
    angle = 0;
    spaceBar = false;

    drawingRect = QRectF(QPointF(0, 0), dimension);
    centerPoint = drawingRect.center().toPoint();
    target = static_cast<int> (drawingRect.width() * (0.02));

    gScene->setSceneRect(drawingRect);
    setScene(gScene);
    centerDrawingArea();
    setMouseTracking(true);

    setRenderHints(QPainter::RenderHints(QPainter::Antialiasing));

    dial = new TupRotationDial(parent);
    connect(dial, SIGNAL(valueChanged(int)), this, SLOT(updateAngle(int)));
}

RasterCanvasBase::~RasterCanvasBase()
{
}

void RasterCanvasBase::setBgColor(const QColor color)
{
    bgColor = color;
    viewport()->update();
}

void RasterCanvasBase::setAntialiasing(bool use)
{
    setRenderHint(QPainter::Antialiasing, use);
    setRenderHint(QPainter::TextAntialiasing, use);
}

void RasterCanvasBase::drawGrid(bool draw)
{
    gridEnabled = draw;
    viewport()->update();
}

void RasterCanvasBase::drawActionSafeArea(bool draw)
{
    safeAreaEnabled = draw;
    viewport()->update();
}

bool RasterCanvasBase::getGridState() const
{
    return gridEnabled;
}

bool RasterCanvasBase::getSafeAreaState() const
{
    return safeAreaEnabled;
}

void RasterCanvasBase::mouseMoveEvent(QMouseEvent *event)
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

void RasterCanvasBase::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space) {
        spaceBar = true;
        return;
    }

    if (event->modifiers () == (Qt::AltModifier | Qt::ControlModifier)) {
        QDesktopWidget desktop;
        dial->setAngle(static_cast<int>(angle));
        dial->show();

        QScreen *screen = QGuiApplication::screens().at(0);
        dial->move(static_cast<int> ((screen->geometry().width() - dial->sizeHint().width()) / 2),
                   static_cast<int> ((screen->geometry().height() - dial->sizeHint().height()) / 2));

        return;
    }

    QGraphicsView::keyPressEvent(event);
}

void RasterCanvasBase::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space) {
        spaceBar = false;
        return;
    }

    QGraphicsView::keyReleaseEvent(event);
}

void RasterCanvasBase::enterEvent(QEvent *event)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "RasterCanvasBase::enterEvent(QEvent)";
    #endif
    */
    if (!hasFocus())
        setFocus();

    QGraphicsView::enterEvent(event);
}

void RasterCanvasBase::leaveEvent(QEvent *event)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "RasterCanvasBase::leaveEvent(QEvent)";
    #endif
    */
    if (hasFocus())
        clearFocus();

    QGraphicsView::leaveEvent(event);
}

bool RasterCanvasBase::viewportEvent(QEvent *event)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "RasterCanvasBase::viewportEvent(QEvent)";
    #endif
    */

    return QGraphicsView::viewportEvent(event);
}

void RasterCanvasBase::wheelEvent(QWheelEvent *event)
{
    // SQA: Evaluate this replacement
    // scaleView(pow(2.0, event->delta() / 520.0));
    scaleView(pow(2.0, event->angleDelta().y() / 520.0));
}

void RasterCanvasBase::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter, rect);
    painter->save();

    bool hasAntialiasing = painter->renderHints() & QPainter::Antialiasing;

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(blackPen);
    painter->fillRect(drawingRect, bgColor);
    painter->drawRect(drawingRect);

    emit changedZero(painter->worldTransform().map(QPointF(0, 0)));

    painter->setRenderHint(QPainter::Antialiasing, hasAntialiasing);
    painter->restore();
}

void RasterCanvasBase::drawForeground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect)

    // if enabled draw grid
    if (gridEnabled) {
        painter->setPen(gridPen);
        int maxX = static_cast<int> (drawingRect.width() + 100);
        int maxY = static_cast<int> (drawingRect.height() + 100);
        for (int i = -100; i <= maxX; i += gridSeparation)
             painter->drawLine(i, -100, i, maxY);
        for (int i = -100; i <= maxY; i += gridSeparation)
             painter->drawLine(-100, i, maxX, i);
    }
    // if enabled action safe area
    if (safeAreaEnabled) {
        painter->setPen(greenThickPen);
        painter->drawRect(drawingRect);

        int w = static_cast<int> (drawingRect.width());
        int h = static_cast<int> (drawingRect.height());
        int outerBorder = w / 19;
        int innerBorder = w / 6;

        int hSpace = w / 3;
        int vSpace = static_cast<int> (drawingRect.height() / 3);

        QPointF left = drawingRect.topLeft() + QPointF(outerBorder, outerBorder);
        QPointF right = drawingRect.bottomRight() - QPointF(outerBorder, outerBorder);
        int leftX = static_cast<int> (left.x());
        int leftY = static_cast<int> (left.y());
        int rightX = static_cast<int> (right.x());
        int rightY = static_cast<int> (right.y());

        QRectF outerRect(left, right);

        painter->setPen(grayPen);
        painter->drawRect(outerRect);

        painter->setPen(greenBoldPen);
        painter->drawLine(QPoint(hSpace, leftY - 8), QPoint(hSpace, leftY + 8));
        painter->drawLine(QPoint(hSpace - 5, leftY), QPoint(hSpace + 5, leftY));
        painter->drawLine(QPoint(hSpace*2, leftY - 8), QPoint(hSpace*2, leftY + 8));
        painter->drawLine(QPoint(hSpace*2 - 5, leftY), QPoint(hSpace*2 + 5, leftY));

        painter->drawLine(QPoint(hSpace, rightY - 8), QPoint(hSpace, rightY + 8));
        painter->drawLine(QPoint(hSpace - 5, rightY), QPoint(hSpace + 5, rightY));
        painter->drawLine(QPoint(hSpace*2, rightY - 8), QPoint(hSpace*2, rightY + 8));
        painter->drawLine(QPoint(hSpace*2 - 5, rightY), QPoint(hSpace*2 + 5, rightY));

        painter->drawLine(QPoint(leftX - 8, vSpace), QPoint(leftX + 8, vSpace));
        painter->drawLine(QPoint(leftX, vSpace - 5), QPoint(leftX, vSpace + 5));
        painter->drawLine(QPoint(leftX - 8, vSpace*2), QPoint(leftX + 8, vSpace*2));
        painter->drawLine(QPoint(leftX, vSpace*2 - 5), QPoint(leftX, vSpace*2 + 5));

        painter->drawLine(QPoint(rightX - 8, vSpace), QPoint(rightX + 8, vSpace));
        painter->drawLine(QPoint(rightX, vSpace - 5), QPoint(rightX, vSpace + 5));
        painter->drawLine(QPoint(rightX - 8, vSpace*2), QPoint(rightX + 8, vSpace*2));
        painter->drawLine(QPoint(rightX, vSpace*2 - 5), QPoint(rightX, vSpace*2 + 5));

        painter->setPen(greenThinPen);

        left = drawingRect.topLeft() + QPointF(innerBorder, innerBorder);
        right = drawingRect.bottomRight() - QPointF(innerBorder, innerBorder);
        QRectF innerRect(left, right);
        painter->drawRect(innerRect);

        painter->setPen(dotPen);
        int middleX = w/2;
        int middleY = h/2;
        painter->drawLine(QPoint(0, middleY), QPoint(w, middleY));
        painter->drawLine(QPoint(middleX, 0), QPoint(middleX, h));
        QRect rect(QPoint(middleX - target, middleY - target),
                   QPoint(middleX + target, middleY + target));
        painter->drawRect(rect);
    }
}

void RasterCanvasBase::drawPadLock(QPainter *painter, const QRectF &rect, QString text)
{
    QFont font;
    font.setPointSize(30);
    font.setBold(true);
    QFontMetricsF fm(font);

    painter->setFont(font);
    painter->fillRect(rect, QColor(201, 201, 201, 200));

    QRectF shore = fm.boundingRect(text);

    int middleX = static_cast<int> (gScene->sceneRect().topRight().x() - gScene->sceneRect().topLeft().x());
    int middleY = static_cast<int> (gScene->sceneRect().bottomLeft().y() - gScene->sceneRect().topLeft().y());

    int x = static_cast<int> ((middleX - shore.width()) / 2);
    int y = static_cast<int> ((middleY - shore.height()) / 2);
    painter->drawText(x, y, text);

    x = (middleX - 20) / 2;
    y = (middleY - 20) / 2;
    painter->setPen(QPen(QColor(100, 100, 100), 4, Qt::SolidLine));
    painter->drawRoundedRect(x, y + 18, 20, 20, 1, 1, Qt::AbsoluteSize);

    x = (middleX - 30) / 2;
    painter->fillRect(x, y + 30, 30, 20, QColor(100, 100, 100));
}

void RasterCanvasBase::centerDrawingArea()
{
    centerOn(drawingRect.center());
}

QPointF RasterCanvasBase::getCenterPoint() const
{
    return drawingRect.center();
}

void RasterCanvasBase::scaleView(qreal scaleFactor)
{
    // SQA: Evaluate this replacement
    // qreal factor = matrix().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();

    if (factor < 0.07 || factor > 100)
        return;
    scale(scaleFactor, scaleFactor);

    emit scaled(scaleFactor);
}

void RasterCanvasBase::setRotationAngle(int degree)
{
    int degrees = degree - static_cast<int>(angle);
    rotate(degrees);
    angle = degree;
}

void RasterCanvasBase::setZoom(qreal scaleFactor)
{
    scale(scaleFactor, scaleFactor);
}

QRectF RasterCanvasBase::getDrawingRect() const
{
    return drawingRect;
}

QGraphicsScene *RasterCanvasBase::graphicsScene() const
{
    return gScene;
}

QPointF RasterCanvasBase::viewPosition()
{
    return position;
}

void RasterCanvasBase::updateDimension(const QSize dimension)
{
    drawingRect = QRectF(QPointF(0, 0), dimension);
    gScene->setSceneRect(drawingRect);

    update();
}

void RasterCanvasBase::updateCenter(const QPoint point)
{
    int x = point.x();
    int y = point.y();

    int cx = centerPoint.x();
    int cy = centerPoint.y();

    int x0 = initPoint.x();
    int y0 = initPoint.y();

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
    setSceneRect(cx - (drawingRect.width()/2), cy - (drawingRect.height()/2),
                 drawingRect.width(), drawingRect.height());
}

void RasterCanvasBase::updateGridParameters()
{
    TCONFIG->beginGroup("PaintArea");
    QString colorName = TCONFIG->value("GridColor").toString();
    QColor gridColor(colorName);
    gridColor.setAlpha(50);
    gridPen = QPen(gridColor, 1);
    gridSeparation = TCONFIG->value("GridSeparation").toInt();
}

void RasterCanvasBase::updateAngle(int degree)
{
    setRotationAngle(degree);
    emit rotated(degree);
}
