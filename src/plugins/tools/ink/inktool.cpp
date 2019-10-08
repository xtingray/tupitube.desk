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

#include "inktool.h"
#include "tupinputdeviceinformation.h"
#include "tupbrushmanager.h"
#include "tupgraphicalgorithm.h"
#include "tupgraphicsscene.h"
#include "tuprequestbuilder.h"
#include "tupprojectrequest.h"
#include "tupellipseitem.h"
#include "tuplineitem.h"
#include "tuptextitem.h"
#include "taction.h"
#include "talgorithm.h"
#include "tconfig.h"

InkTool::InkTool()
{
    configPanel = nullptr;
    guidePath = nullptr;
    inkCursor = QCursor(kAppProp->themeDir() + "cursors/ink.png", 0, 16);

    setupActions();
}

InkTool::~InkTool()
{
}

void InkTool::init(TupGraphicsScene *gScene)
{
    Q_UNUSED(gScene)

    spacing = 1;
    tolerance = 0;
    smoothness = 2;
}

QStringList InkTool::keys() const
{
    return QStringList() << tr("Ink");
}

void InkTool::press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    firstHalfOnTop = true;
    previousDirection = None;
    oldSlope = 0;
    penWidth = brushManager->pen().widthF() / 2;

    dotsCounter = 1;
    firstPoint = input->pos();

    guidePainterPath = QPainterPath();
    guidePainterPath.moveTo(firstPoint);

    inkPath = QPainterPath();
    inkPath.setFillRule(Qt::WindingFill);
    inkPath.moveTo(firstPoint);

    shapePoints.clear();
    shapePoints << firstPoint;

    oldPos = input->pos();
    firstHalfPrevious = input->pos();
    secondHalfPrevious = input->pos();
    previewPoint = input->pos();

    // Guide line
    guidePath = new TupPathItem();
    QColor color(55, 155, 55, 200);
    QPen pen(QBrush(color), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    guidePath->setPen(pen);

    gScene->includeObject(guidePath);

    firstArrow = rand() % 10 + 1;
    arrowSize = -1;
}

void InkTool::move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    Q_UNUSED(brushManager)

    dotsCounter++;
    gScene->views().at(0)->setDragMode(QGraphicsView::NoDrag);
    QPointF currentPoint = input->pos();

    if (currentPoint != previewPoint) {
        qreal my = currentPoint.y() - previewPoint.y();
        qreal mx = currentPoint.x() - previewPoint.x();
        qreal m;

        if (static_cast<int>(mx) != 0) // Calculating slope
            m = my / mx;
        else
            m = 100; // mx = 0 -> path is vertical | 100 == infinite

        // guidePath is the guideline to calculate the real QGraphicsPathItem
        guidePainterPath.lineTo(currentPoint);
        guidePath->setPath(guidePainterPath);

        // Calculating distance between current point and previous
        qreal distance = sqrt(pow(std::abs(currentPoint.x() - oldPos.x()), 2) + pow(std::abs(currentPoint.y() - oldPos.y()), 2));

        // Time to calculate a new point of the QGraphicsPathItem (stroke)
        if (distance > 5) {
            /*
            qreal pm; // Perpendicular slope

            if (static_cast<int>(m) == 0) // path is horizontal
                pm = 100; 
            else
                pm = (-1) * (1/m);
            */

            #ifdef TUP_DEBUG
                /*
                qDebug() << "";
                qDebug() << "InkTool::move() - old slope: " << oldSlope;
                qDebug() << "InkTool::move() - slope: " << m;

                bool isNAN = false;
                if (static_cast<int>(m) == 0) // path is horizontal
                    isNAN = true;

                if (static_cast<int>(m) == 100) // path is vertical | 100 == infinite
                    qDebug() << "InkTool::move() - M: NAN";
                else
                    qDebug() << "InkTool::move() - M: " + QString::number(m);

                if (isNAN)
                    qDebug() << "InkTool::move() - M(inv): NAN";
                else
                    qDebug() << "InkTool::move() - M(inv): " + QString::number(pm);
                */
            #endif

            QPointF firstHalfPoint;
            QPointF secondHalfPoint;

            double lineAngle = atan(m);
            // qDebug() << "InkTool::move() - inv tan: " << lineAngle;
            // double degrees = lineAngle * 180 / PI;
            // qDebug() << "InkTool::move() - degrees: " << degrees;
            double angle = 1.5708 - lineAngle;
            // qDebug() << "InkTool::move() - angle: " << angle;
            double deltaX = fabs(cos(angle) * penWidth);
            // qDebug() << "InkTool::move() - deltaX: " << deltaX;
            double deltaY = fabs(sin(angle) * penWidth);
            // qDebug() << "InkTool::move() - deltaY: " << deltaY;

            qreal x0 = 0;
            qreal y0 = 0;
            qreal x1 = 0;
            qreal y1 = 0;
            if (previewPoint.x() < currentPoint.x()) {
                if (previewPoint.y() < currentPoint.y()) {
                    #ifdef TUP_DEBUG
                        qDebug() << "    -> InkTool::move() - Going right-down";
                    #endif

                    x0 = currentPoint.x() + deltaX;
                    x1 = currentPoint.x() - deltaX;
                    y0 = currentPoint.y() - deltaY;
                    y1 = currentPoint.y() + deltaY;

                    if (previousDirection == None)
                        previousDirection = RightDown;

                    switch(previousDirection) {
                        case Up:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case Down:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case Right:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case RightUp:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case RightDown:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case Left:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);

                                double xFix = firstHalfPrevious.x() - penWidth;
                                double yFix = firstHalfPrevious.y();
                                QPointF fixPoint = QPointF(xFix, yFix);

                                inkPath.lineTo(fixPoint);
                                inkPath.lineTo(fixPoint + QPointF(-penWidth, penWidth));
                                shapePoints.removeAt(shapePoints.size() - 1);
                                shapePoints.removeAt(shapePoints.size() - 1);

                                firstHalfOnTop = false;
                            } else {
                                shapePoints.removeAt(shapePoints.size() - 1);
                                shapePoints.removeAt(shapePoints.size() - 1);

                                firstHalfOnTop = true;
                            }
                        break;
                        case LeftUp:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case LeftDown:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);

                                double xFix = firstHalfPrevious.x() - penWidth;
                                double yFix = firstHalfPoint.y() - fabs(firstHalfPoint.y() - firstHalfPrevious.y()) / 2;
                                QPointF fixPoint = QPointF(xFix, yFix);

                                inkPath.lineTo(fixPoint);
                                shapePoints[shapePoints.size() - 1] = QPointF(secondHalfPrevious.x() + penWidth, secondHalfPrevious.y() - penWidth);

                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);

                                double yFix = secondHalfPoint.y() - fabs(secondHalfPoint.y() - secondHalfPrevious.y()) / 2;
                                double xFix = secondHalfPrevious.x() - penWidth;
                                QPointF fixPoint = QPointF(xFix, yFix);

                                inkPath.lineTo(fixPoint);
                                shapePoints.removeAt(shapePoints.size() - 1);
                                shapePoints.removeAt(shapePoints.size() - 1);

                                firstHalfOnTop = true;
                            }
                        break;
                        case None:
                        break;
                    }
                    // qDebug() << "InkTool::move() - firstHalfOnTop: " << firstHalfOnTop;
                    previousDirection = RightDown;
                } else if (previewPoint.y() > currentPoint.y()) {
                    #ifdef TUP_DEBUG
                        qDebug() << "    -> InkTool::move() - Going right-up";
                    #endif

                    x0 = currentPoint.x() - deltaX;
                    x1 = currentPoint.x() + deltaX;
                    y0 = currentPoint.y() - deltaY;
                    y1 = currentPoint.y() + deltaY;

                    if (previousDirection == None)
                        previousDirection = RightUp;

                    switch(previousDirection) {
                        case Up:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case Down:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = true;
                            }
                        break;
                        case Right:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case RightUp:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case RightDown:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case Left:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case LeftUp:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case LeftDown:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case None:
                        break;
                    }
                    // qDebug() << "InkTool::move() - firstHalfOnTop: " << firstHalfOnTop;
                    previousDirection = RightUp;
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "    -> InkTool::move() - Going right";
                    #endif

                    x0 = currentPoint.x();
                    y0 = currentPoint.y() - penWidth;
                    x1 = currentPoint.x();
                    y1 = currentPoint.y() + penWidth;

                    if (previousDirection == None)
                        previousDirection = Right;

                    switch(previousDirection) {
                        case Up:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case Down:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case Right:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case RightUp:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case RightDown:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case Left:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case LeftUp:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case LeftDown:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case None:
                        break;
                    }

                    // qDebug() << "InkTool::move() - firstHalfOnTop: " << firstHalfOnTop;
                    previousDirection = Right;
                }
            } else if (previewPoint.x() > currentPoint.x()) {
                if (previewPoint.y() < currentPoint.y()) {
                    #ifdef TUP_DEBUG
                        qDebug() << "    -> InkTool::move() - Going left-down";
                    #endif

                    x0 = currentPoint.x() - deltaX;
                    x1 = currentPoint.x() + deltaX;
                    y0 = currentPoint.y() - deltaY;
                    y1 = currentPoint.y() + deltaY;

                    if (previousDirection == None)
                        previousDirection = LeftDown;

                    switch(previousDirection) {
                        case Up:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case Down:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case Right:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case RightUp:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case RightDown:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case Left:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case LeftUp:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case LeftDown:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case None:
                        break;
                    }

                    // qDebug() << "InkTool::move() - firstHalfOnTop: " << firstHalfOnTop;
                    previousDirection = LeftDown;
                } else if (previewPoint.y() > currentPoint.y()) {
                    #ifdef TUP_DEBUG
                        qDebug() << "    -> InkTool::move() - Going left-up";
                    #endif

                    x0 = currentPoint.x() + deltaX;
                    x1 = currentPoint.x() - deltaX;
                    y0 = currentPoint.y() - deltaY;
                    y1 = currentPoint.y() + deltaY;

                    if (previousDirection == None)
                        previousDirection = LeftUp;

                    switch(previousDirection) {
                        case Up:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case Down:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case Right:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case RightUp:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case RightDown:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case Left:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case LeftUp:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case LeftDown:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case None:
                        break;
                    }

                    // qDebug() << "InkTool::move() - firstHalfOnTop: " << firstHalfOnTop;
                    previousDirection = LeftUp;
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "    -> InkTool::move() - Going left";
                    #endif
                    x0 = currentPoint.x();
                    y0 = currentPoint.y() - penWidth;
                    x1 = currentPoint.x();
                    y1 = currentPoint.y() + penWidth;                    

                    if (previousDirection == None)
                        previousDirection = Left;

                    switch(previousDirection) {
                        case Up:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case Down:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case Right:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case RightUp:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case RightDown:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case Left:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case LeftUp:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case LeftDown:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case None:
                        break;
                    }

                    // qDebug() << "InkTool::move() - firstHalfOnTop: " << firstHalfOnTop;
                    previousDirection = Left;
                }
            } else if (static_cast<int>(previewPoint.x()) == static_cast<int>(currentPoint.x())) {
                if (previewPoint.y() > currentPoint.y()) {
                    #ifdef TUP_DEBUG
                        qDebug() << "    -> InkTool::move() - Going up";
                    #endif

                    x0 = currentPoint.x() - penWidth;
                    y0 = currentPoint.y();
                    x1 = currentPoint.x() + penWidth;
                    y1 = currentPoint.y();

                    if (previousDirection == None)
                        previousDirection = Up;

                    switch(previousDirection) {
                        case Up:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case Down:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case Right:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case RightUp:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case RightDown:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case Left:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case LeftUp:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case LeftDown:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case None:
                        break;
                    }

                    // qDebug() << "InkTool::move() - firstHalfOnTop: " << firstHalfOnTop;
                    previousDirection = Up;
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "    -> InkTool::move() - Going down";
                    #endif

                    x0 = currentPoint.x() - penWidth;
                    y0 = currentPoint.y();
                    x1 = currentPoint.x() + penWidth;
                    y1 = currentPoint.y();

                    if (previousDirection == None)
                        previousDirection = Down;

                    switch(previousDirection) {
                        case Up:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case Down:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case Right:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case RightUp:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case RightDown:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                                firstHalfOnTop = false;
                            } else {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                                firstHalfOnTop = true;
                            }
                        break;
                        case Left:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case LeftUp:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case LeftDown:
                            if (firstHalfOnTop) {
                                firstHalfPoint = QPointF(x0, y0);
                                secondHalfPoint = QPointF(x1, y1);
                            } else {
                                firstHalfPoint = QPointF(x1, y1);
                                secondHalfPoint = QPointF(x0, y0);
                            }
                        break;
                        case None:
                        break;
                    }

                    // qDebug() << "InkTool::move() - firstHalfOnTop: " << firstHalfOnTop;
                    previousDirection = Down;
                }
             }
             /*
             qDebug() << "";
             qDebug() << "First Half Point: " << firstHalfPoint;
             qDebug() << "Second Half Point: " << secondHalfPoint;
             qDebug() << "deltaX: " << deltaX;
             qDebug() << "deltaY: " << deltaY;

             qDebug() << "Old Right: " << firstHalfPrevious;
             distance = sqrt(pow(std::abs(firstHalfPoint.x() - firstHalfPrevious.x()), 2) + pow(std::abs(firstHalfPoint.y() - firstHalfPrevious.y()), 2));
             qDebug() << "distance: " << distance;
             qDebug() << "";

             qreal firstHalfM = (y0 - firstHalfPrevious.y()) / (x0 - firstHalfPrevious.x());
             qreal secondHalfM = (y1 - secondHalfPrevious.y()) / (x1 - secondHalfPrevious.x());
             qDebug() << "*** firstHalfM: " << firstHalfM;
             qDebug() << "*** secondHalfM: " << secondHalfM;

             if ((firstHalfM < 0 && secondHalfM > 0) || (firstHalfM > 0 && secondHalfM < 0)) {
                 qDebug() << "*** Exchanging sides!";
                 firstHalfPoint = QPointF(x1, y1);
                 secondHalfPoint = QPointF(x0, y0);
             } else {
                 qDebug() << "No exchanging call!";
                 firstHalfPoint = QPointF(x0, y0);
                 secondHalfPoint = QPointF(x1, y1);
             }
             */

             inkPath.lineTo(firstHalfPoint);
             firstHalfPrevious = firstHalfPoint;

             secondHalfPrevious = secondHalfPoint;
             shapePoints << secondHalfPoint;

             oldPos = currentPoint;
             oldSlope = m;
        }
    }

    previewPoint = currentPoint;
}

void InkTool::release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    gScene->removeItem(guidePath);
    QPointF currentPoint = input->pos();

    // Drawing a point
    if (inkPath.elementCount() == 1) {
        qreal radius = brushManager->pen().width(); // * 2;
        QPointF distance((radius + 2)/2, (radius + 2)/2);
        QPen inkPen(brushManager->penColor(), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        TupEllipseItem *blackEllipse = new TupEllipseItem(QRectF(currentPoint - distance,
                                                                 QSize(static_cast<int>(radius), static_cast<int>(radius))));
        blackEllipse->setPen(inkPen);
        blackEllipse->setBrush(inkPen.brush());
        gScene->includeObject(blackEllipse);

        QDomDocument doc;
        doc.appendChild(blackEllipse->toXml(doc));
        TupProjectRequest request = TupRequestBuilder::createItemRequest(gScene->currentSceneIndex(), gScene->currentLayerIndex(), gScene->currentFrameIndex(),
                                                                         0, currentPoint, gScene->getSpaceContext(), TupLibraryObject::Item, TupProjectRequest::Add,
                                                                         doc.toString());
        emit requested(&request);
        return;
    }

    inkPath.lineTo(currentPoint);

    // Creating the whole shape of the line (QPainterPath)
    for (int i = shapePoints.size()-1; i > 0; i--)
        inkPath.lineTo(shapePoints.at(i-1));

    // smoothPath(inkPath, smoothness);

    TupPathItem *stroke = new TupPathItem();
    stroke->setPath(inkPath);
    // Set border color for shape
    stroke->setPen(QPen(brushManager->penColor()));
    // Set fill color for shape
    stroke->setBrush(brushManager->penColor());
    // stroke->setPath(inkPath);
    gScene->includeObject(stroke);

    QDomDocument doc;
    doc.appendChild(stroke->toXml(doc));
    TupProjectRequest request = TupRequestBuilder::createItemRequest(gScene->currentSceneIndex(), gScene->currentLayerIndex(), gScene->currentFrameIndex(),
                                                                     0, QPointF(), gScene->getSpaceContext(), TupLibraryObject::Item, TupProjectRequest::Add,
                                                                     doc.toString());
    emit requested(&request);

    /*
    TupPathItem *greenLine = new TupPathItem();
    QColor color(55, 155, 55, 200);
    QPen pen(QBrush(color), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    greenLine->setPen(pen);
    greenLine->setPath(guidePainterPath);
    gScene->includeObject(greenLine);
    QDomDocument data;
    data.appendChild(greenLine->toXml(data));
    request = TupRequestBuilder::createItemRequest(gScene->currentSceneIndex(), gScene->currentLayerIndex(), gScene->currentFrameIndex(),
                                                   0, QPointF(), gScene->getSpaceContext(), TupLibraryObject::Item, TupProjectRequest::Add,
                                                   data.toString());
    emit requested(&request);
    */
}

void InkTool::setupActions()
{
    TAction *inkPen = new TAction(QPixmap(kAppProp->themeDir() + "icons/ink.png"), tr("Ink"), this);
    inkPen->setShortcut(QKeySequence(tr("K")));
    inkPen->setToolTip(tr("Ink") + " - " + "K");
    inkPen->setCursor(inkCursor);

    inkActions.insert(tr("Ink"), inkPen);
}

QMap<QString, TAction *> InkTool::actions() const
{
    return inkActions;
}

int InkTool::toolType() const
{
    return TupToolInterface::Brush;
}

QWidget *InkTool::configurator()
{
    /*
    if (!configPanel) {
        configPanel = new configPanel;
        connect(configPanel, SIGNAL(updateSpacing(int)), this, SLOT(updateSpacingVar(int)));
        connect(configPanel, SIGNAL(updateSizeTolerance(int)), this, SLOT(updateSizeToleranceVar(int)));
    }
    */

    return configPanel;
}

void InkTool::aboutToChangeTool() 
{
}

void InkTool::saveConfig()
{
    /*
    if (configPanel) {
        TCONFIG->beginGroup("InkTool");
        TCONFIG->setValue("DotsSpacing", configPanel->spacingValue());
        TCONFIG->setValue("Tolerance", configPanel->sizeToleranceValue());
        TCONFIG->setValue("Smoothness", configPanel->smoothness());
        TCONFIG->setValue("ShowBorder", configPanel->showBorder());
        TCONFIG->setValue("BorderSize", configPanel->borderSizeValue());
    }
    */
}

/*
void InkTool::updateSpacingVar(int value)
{
    spacing = value;
}
*/

/*
void InkTool::updateSizeToleranceVar(int value)
{
    tolerance = static_cast<int>(value) / static_cast<int>(100);
}
*/

void InkTool::smoothPath(QPainterPath &path, double smoothness, int from, int to)
{
    QPolygonF pol;
    QList<QPolygonF> polygons = path.toSubpathPolygons();
    QList<QPolygonF>::iterator it = polygons.begin();
    QPolygonF::iterator pointIt;

    while (it != polygons.end()) {
        pointIt = (*it).begin();
        while (pointIt <= (*it).end()-2) {
            pol << (*pointIt);
            pointIt += 2;
        }
        ++it;
    }

    if (smoothness > 0) {
        path = TupGraphicalAlgorithm::bezierFit(pol, static_cast<float>(smoothness), from, to);
    } else {
        path = QPainterPath();
        path.addPolygon(pol);
    }
}

void InkTool::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F11 || event->key() == Qt::Key_Escape) {
        emit closeHugeCanvas();
    } else {
        QPair<int, int> flags = TupToolPlugin::setKeyAction(event->key(), event->modifiers());
        if (flags.first != -1 && flags.second != -1)
            emit callForPlugin(flags.first, flags.second);
    }
}

QCursor InkTool::polyCursor() const
{
    return inkCursor;
}
