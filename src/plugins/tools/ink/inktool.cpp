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
    item = nullptr;
    inkCursor = QCursor(kAppProp->themeDir() + "cursors/ink.png", 0, 16);

    setupActions();
}

InkTool::~InkTool()
{
}

void InkTool::init(TupGraphicsScene *gScene)
{
    Q_UNUSED(gScene)

    /*
    spacing = configPanel->spacingValue();
    tolerance = configPanel->sizeToleranceValue()/(qreal)100;
    smoothness = 3;
    */

    spacing = 1;
    tolerance = 0;
    smoothness = 3;

    TCONFIG->beginGroup("BrushParameters");
    int thickness = TCONFIG->value("Thickness", 3).toInt();

    widthVar = tolerance*thickness;
    if (widthVar < 1)
        widthVar = 1;
		
    /*
    qDebug() << "InkTool::init() - thickness: " << thickness;
    qDebug() << "InkTool::init() - tolerance: " << tolerance;
    qDebug() << "InkTool::init() - widthVar: " << widthVar;
    */
}

QStringList InkTool::keys() const
{
    return QStringList() << tr("Ink");
}

void InkTool::press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    oldSlope = 0;
    penWidth = brushManager->pen().widthF()/2;

    dotsCounter = 1;
    firstPoint = input->pos();
    connector = firstPoint;

    path = QPainterPath();
    path.moveTo(firstPoint);

    inkPath = QPainterPath();
    inkPath.setFillRule(Qt::WindingFill);
    inkPath.moveTo(firstPoint);

    leftPoints.clear();
    leftPoints << firstPoint;

    oldPos = input->pos();
    oldPosRight = input->pos();
    oldPosLeft = input->pos();
    previewPoint = input->pos();

    item = new TupPathItem();
    QColor color(55, 155, 55, 200);
    QPen pen(QBrush(color), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    item->setPen(pen);

    gScene->includeObject(item);

    firstArrow = rand() % 10 + 1;
    arrowSize = -1;
}

void InkTool::move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    Q_UNUSED(brushManager)

    dotsCounter++;

    gScene->views().at(0)->setDragMode(QGraphicsView::NoDrag);

    QPointF currentPoint = input->pos();
    qreal my = currentPoint.y() - previewPoint.y();
    qreal mx = currentPoint.x() - previewPoint.x();
    qreal m;

    if (currentPoint != previewPoint) {
        if (static_cast<int>(mx) != 0)
            m = my / mx;
        else
            m = 100; // mx = 0 -> path is vertical | 100 == infinite

        // path is the guideline to calculate the real QGraphicsPathItem
        path.moveTo(previewPoint);
        path.lineTo(currentPoint);
        item->setPath(path);

        qreal slopeVar = std::abs(oldSlope - m);
        qreal distance = sqrt(pow(std::abs(currentPoint.x() - oldPos.x()), 2) + pow(std::abs(currentPoint.y() - oldPos.y()), 2));

        // Time to calculate a new point of the QGraphicsPathItem
        if ((dotsCounter > firstArrow) && ((dotsCounter % spacing == 0) || ((slopeVar >= 1) && (distance > 10)))) {
            // Calculating the begining of the line (vertex "<")
            if (arrowSize == -1) {
                qreal pow1 = pow(currentPoint.x() - firstPoint.x(), 2);
                qreal pow2 = pow(currentPoint.y() - firstPoint.y(), 2);
                arrowSize = static_cast<int> (sqrt(pow1 + pow2));
                if (arrowSize > 0)
                    arrowSize = (rand() % arrowSize) + 1;
                else
                    arrowSize = 5;
            }

            oldSlope = m;

            qreal pm;  
            qreal x0 = 0;
            qreal y0 = 0;
            qreal x1 = 0;
            qreal y1 = 0;

            if (static_cast<int>(m) == 0) // path is horizontal
                pm = 100; 
            else
                pm = (-1) * (1/m);

            #ifdef TUP_DEBUG
                bool isNAN = false;
                if (static_cast<int>(m) == 0) // path is horizontal
                    isNAN = true;

                if (static_cast<int>(m) == 100) { // path is vertical | 100 == infinite
                    qDebug() << "InkTool::move() - M: NAN";
                } else {
                    qDebug() << "InkTool::move() - M: " + QString::number(m);
                }

                if (isNAN) {
                    qDebug() << "InkTool::move() - M(inv): NAN";
                } else {
                    qDebug() << "InkTool::move() - M(inv): " + QString::number(pm);
                }
            #endif

            qreal hypotenuse;

            if (fabs(pm) < 5) { // path's slope is close to 0
                int cutter = static_cast<int>(penWidth);
                bool found = false;
                qreal limit = 0;
                int iterations = 0;

                if (tolerance < 1) { // tolerance == decimal percent of tolerance [0.0 -> 1.0]
                    while (!found) {
                       iterations++;
                       x0 = currentPoint.x() - cutter;
                       y0 = (pm*(x0 - currentPoint.x())) + currentPoint.y();

                       x1 = currentPoint.x() + cutter;
                       y1 = (pm*(x1 - currentPoint.x())) + currentPoint.y();
                       hypotenuse = sqrt(pow(x1 - x0, 2) + pow(y1 - y0, 2));

                       limit = hypotenuse - brushManager->pen().widthF();

                       if (fabs(limit) > widthVar) {
                           if (limit > 0) {
                               cutter -= static_cast<int>(0.2);
                               if (cutter == 0)
                                   found = true;
                           } else {
                               cutter += static_cast<int>(0.2);
                           }
                       } else {
                           found = true;
                       }

                       if (iterations >10)
                           found = true;
                    }
                } else {
                    int random = rand() % 101;
                    qreal plus = static_cast<qreal>(random) / static_cast<qreal>(100 * (penWidth*tolerance));

                    x0 = currentPoint.x() - plus;
                    y0 = (pm*(x0 - currentPoint.x())) + currentPoint.y();

                    x1 = currentPoint.x() + plus;
                    y1 = (pm*(x1 - currentPoint.x())) + currentPoint.y();
                    hypotenuse = sqrt(pow(x1 - x0, 2) + pow(y1 - y0, 2));
                }
            } else { // Line's slope is 0 or closer to
                qreal delta;
                qreal plus;
                int random = rand() % 101;

                if (static_cast<int>(tolerance) == 0) {
                    plus = 0;
                } else if (tolerance < 1) {
                    if (widthVar > 0)
                        plus = rand() % static_cast<int>(widthVar);
                    else
                        plus = rand() % 5;
                } else {
                    plus = static_cast<qreal>(random) / static_cast<qreal>(100 * (penWidth*tolerance));
                }

                delta = penWidth + plus;

                x0 = currentPoint.x();
                y0 = currentPoint.y() - delta;

                x1 = currentPoint.x();
                y1 = currentPoint.y() + delta;

                hypotenuse = fabs(y1 - y0);
            }

            QPointF right;
            QPointF left;

            if (previewPoint.x() < currentPoint.x()) {
                if (previewPoint.y() < currentPoint.y()) {
                    #ifdef TUP_DEBUG
                        qDebug() << "    -> InkTool::move() - Going down-right";
                    #endif

                    if (y0 > y1) {
                        left = QPointF(x0, y0);
                        right = QPointF(x1, y1);
                    } else {
                        left = QPointF(x1, y1);
                        right = QPointF(x0, y0);
                    }

                    qreal endX = currentPoint.x() + arrowSize;
                    qreal endY = (m*(endX - currentPoint.x())) + currentPoint.y();
                    connector = QPoint(static_cast<int>(endX), static_cast<int>(endY));
                } else if (previewPoint.y() > currentPoint.y()) {
                    #ifdef TUP_DEBUG
                        qDebug() << "    -> InkTool::move() - Going up-right";
                    #endif

                    if (x0 > x1) {
                        left = QPointF(x0, y0);
                        right = QPointF(x1, y1);
                    } else {
                        left = QPointF(x1, y1);
                        right = QPointF(x0, y0);
                    }

                    qreal endX = currentPoint.x() + arrowSize;
                    qreal endY = (m*(endX - currentPoint.x())) + currentPoint.y();
                    connector = QPoint(static_cast<int>(endX), static_cast<int>(endY));
                } else {
                     #ifdef TUP_DEBUG
                         qDebug() << "    -> InkTool::move() - Going right";
                     #endif

                     if (y0 > y1) {
                         left = QPointF(x0, y0);
                         right = QPointF(x1, y1);
                     } else {
                         left = QPointF(x1, y1);
                         right = QPointF(x0, y0);
                     }

                     qreal endX = currentPoint.x() + arrowSize;
                     qreal endY = (m*(endX - currentPoint.x())) + currentPoint.y();
                     connector = QPoint(static_cast<int>(endX), static_cast<int>(endY));
                }
            } else if (previewPoint.x() > currentPoint.x()) {
                if (previewPoint.y() < currentPoint.y()) {
                    #ifdef TUP_DEBUG
                        qDebug() << "    -> InkTool::move() - Going down-left";
                    #endif

                    if (y0 > y1) {
                        right = QPointF(x0, y0);
                        left = QPointF(x1, y1);
                    } else {
                        right = QPointF(x1, y1);
                        left = QPointF(x0, y0);
                    }

                    qreal endX = currentPoint.x() - arrowSize;
                    qreal endY = (m*(endX - currentPoint.x())) + currentPoint.y();
                    connector = QPoint(static_cast<int>(endX), static_cast<int>(endY));
                } else if (previewPoint.y() > currentPoint.y()) {
                    #ifdef TUP_DEBUG
                        qDebug() << "    -> InkTool::move() - Going up-left";
                    #endif

                    if (x0 > x1) {
                        left = QPointF(x0, y0);
                        right = QPointF(x1, y1);
                    } else {
                        if (x0 < x1) {
                            left = QPointF(x1, y1);
                            right = QPointF(x0, y0);
                        } else { // x0 == x1
                            if (y0 > y1) {
                                left = QPointF(x1, y1);
                                right = QPointF(x0, y0);
                            } else {
                                left = QPointF(x0, y0);
                                right = QPointF(x1, y1);
                            }
                        }
                    }

                    qreal endX = currentPoint.x() - arrowSize;
                    qreal endY = (m*(endX - currentPoint.x())) + currentPoint.y();
                    connector = QPoint(static_cast<int>(endX), static_cast<int>(endY));
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "    -> InkTool::move() - Going left";
                    #endif
                    if (y0 > y1) {
                        right = QPointF(x0, y0);
                        left = QPointF(x1, y1);
                    } else {
                        right = QPointF(x1, y1);
                        left = QPointF(x0, y0);
                    }

                    qreal endX = currentPoint.x() - arrowSize;
                    qreal endY = (m*(endX - currentPoint.x())) + currentPoint.y();
                    connector = QPoint(static_cast<int>(endX), static_cast<int>(endY));
                }
            } else if (static_cast<int>(previewPoint.x()) == static_cast<int>(currentPoint.x())) {
                       if (previewPoint.y() > currentPoint.y()) {
                           #ifdef TUP_DEBUG
                               qDebug() << "    -> InkTool::move() - Going up";
                           #endif
                           if (x0 > x1) {
                               left = QPointF(x0, y0);
                               right = QPointF(x1, y1);
                           } else {
                               left = QPointF(x1, y1);
                               right = QPointF(x0, y0);
                           }

                           qreal endX = currentPoint.x();
                           qreal endY = currentPoint.y() - arrowSize;
                           connector = QPoint(static_cast<int>(endX), static_cast<int>(endY));
                       } else {
                           #ifdef TUP_DEBUG
                               qDebug() << "    -> InkTool::move() - Going down";
                           #endif
                           if (x0 > x1) {
                               right = QPointF(x0, y0);
                               left = QPointF(x1, y1);
                           } else {
                               right = QPointF(x1, y1);
                               left = QPointF(x0, y0);
                           }

                           qreal endX = currentPoint.x();
                           qreal endY = currentPoint.y() + arrowSize;
                           connector = QPoint(static_cast<int>(endX), static_cast<int>(endY));
                       }
            }

            inkPath.moveTo(oldPosRight);
            inkPath.lineTo(right);
            oldPosRight = right;

            oldPosLeft = left;
            leftPoints << left;

            oldPos = currentPoint;
        }
    }

    previewPoint = currentPoint;
}

void InkTool::release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    gScene->removeItem(item);
    QPointF currentPoint = input->pos();
    // qreal radius = brushManager->pen().width();
    // int size = configPanel->borderSizeValue();
    // QPen inkPen(brushManager->penColor(), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    // Drawing a point
    if (firstPoint == currentPoint && inkPath.elementCount() == 1) {
        qreal radius = brushManager->pen().width();
        QPointF distance((radius + 2)/2, (radius + 2)/2);
        QPen inkPen(brushManager->penColor(), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        TupEllipseItem *blackEllipse = new TupEllipseItem(QRectF(connector - distance,
                                                                 QSize(static_cast<int>(radius + 2), static_cast<int>(radius + 2))));
        blackEllipse->setPen(inkPen);
        blackEllipse->setBrush(inkPen.brush());
        // blackEllipse->setBrush(brushManager->brush());
        gScene->includeObject(blackEllipse);

        QDomDocument doc;
        doc.appendChild(blackEllipse->toXml(doc));
        TupProjectRequest request = TupRequestBuilder::createItemRequest(gScene->currentSceneIndex(), gScene->currentLayerIndex(), gScene->currentFrameIndex(),
                                                                         0, currentPoint, gScene->getSpaceContext(), TupLibraryObject::Item, TupProjectRequest::Add,
                                                                         doc.toString());
        emit requested(&request);
        return;
    }

    path.moveTo(oldPos);
    path.lineTo(connector);
    inkPath.moveTo(oldPosRight);
    inkPath.lineTo(connector);

    leftPoints << connector;

    // Creating the whole shape of the line
    for (int i = leftPoints.size()-1; i > 0; i--) {
         inkPath.moveTo(leftPoints.at(i));
         inkPath.lineTo(leftPoints.at(i-1));
    }

    inkPath.moveTo(leftPoints.at(0));
    inkPath.lineTo(QPointF(0, 0));

    // smoothPath(inkPath, configPanel->smoothness());
    smoothPath(inkPath, smoothness);

    TupPathItem *stroke = new TupPathItem();
    // stroke->setPen(QPen(Qt::NoPen));
    stroke->setPen(QPen(brushManager->penColor()));

    /*
    if (configPanel->showBorder())
        stroke->setPen(inkPen);
    else
        stroke->setPen(QPen(Qt::NoPen));
    */

    stroke->setBrush(brushManager->penColor());
    stroke->setPath(inkPath);
    gScene->includeObject(stroke);

    QDomDocument doc;
    doc.appendChild(stroke->toXml(doc));
    TupProjectRequest request = TupRequestBuilder::createItemRequest(gScene->currentSceneIndex(), gScene->currentLayerIndex(), gScene->currentFrameIndex(),
                                                                     0, QPointF(), gScene->getSpaceContext(), TupLibraryObject::Item, TupProjectRequest::Add,
                                                                     doc.toString());
    emit requested(&request);
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

void InkTool::updateSpacingVar(int value)
{
    spacing = value;
}

void InkTool::updateSizeToleranceVar(int value)
{
    tolerance = static_cast<int>(value) / static_cast<int>(100);
}

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
