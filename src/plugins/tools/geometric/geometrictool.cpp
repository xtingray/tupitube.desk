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

#include "geometrictool.h"
#include "tconfig.h"
#include "tuprectitem.h"
#include "tupellipseitem.h"
#include "tuplineitem.h"
#include "tuplibraryobject.h"
#include "tupinputdeviceinformation.h"
#include "tupgraphicsscene.h"
#include "tupscene.h"
#include "tupframe.h"
#include "tuprequestbuilder.h"
#include "tupprojectrequest.h"
#include "tupbrushmanager.h"

GeometricTool::GeometricTool()
{
    scene = nullptr;
    linePath = nullptr;
    setupActions();
}

GeometricTool::~GeometricTool()
{
}

QList<TAction::ActionId> GeometricTool::keys() const
{    
    return QList<TAction::ActionId>() << TAction::Rectangle
                                      << TAction::Ellipse
                                      << TAction::Line
                                      << TAction::Triangle;
}

void GeometricTool::init(TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[GeometricTool::init()]";
    #endif

    scene = gScene;
    delete linePath;
    linePath = nullptr;
    proportion = false;
    side = false;

    straightMode = false;
    TCONFIG->beginGroup("GeometricTool");
    int type = TCONFIG->value("LineType", 0).toInt();
    if (type)
        straightMode = true;

    if (configPanel && toolId() == TAction::Line)
        configPanel->updateLineType(type);

    foreach (QGraphicsView *view, scene->views())
        view->setDragMode(QGraphicsView::NoDrag);
}

void GeometricTool::setupActions()
{
    TAction *action1 = new TAction(QIcon(kAppProp->themeDir() + "icons/square.png"), tr("Rectangle"), this);
    action1->setShortcut(QKeySequence(tr("R")));
    action1->setToolTip(tr("Rectangle") + " - " + tr("R"));
    squareCursor = QCursor(kAppProp->themeDir() + "cursors/square.png", 0, 0);
    action1->setCursor(squareCursor);
    action1->setActionId(TAction::Rectangle);
    
    geoActions.insert(TAction::Rectangle, action1);
    
    TAction *action2 = new TAction(QIcon(kAppProp->themeDir() + "icons/ellipse.png"), tr("Ellipse"), this);
    action2->setShortcut(QKeySequence(tr("C")));
    action2->setToolTip(tr("Ellipse") + " - " + tr("C"));
    circleCursor = QCursor(kAppProp->themeDir() + "cursors/circle.png", 2, 2);
    action2->setCursor(circleCursor);
    action2->setActionId(TAction::Ellipse);

    geoActions.insert(TAction::Ellipse, action2);
    
    TAction *action3 = new TAction(QIcon(kAppProp->themeDir() + "icons/line.png"), tr("Line"), this);
    action3->setShortcut(QKeySequence(tr("L")));
    action3->setToolTip(tr("Line") + " - " + tr("L"));
    lineCursor = QCursor(kAppProp->themeDir() + "cursors/line.png", 0, 15);
    action3->setCursor(lineCursor);
    action3->setActionId(TAction::Line);

    geoActions.insert(TAction::Line, action3);

    TAction *action4 = new TAction(QIcon(kAppProp->themeDir() + "icons/triangle.png"), tr("Triangle"), this);
    action4->setShortcut(QKeySequence(tr("Ctrl+T")));
    action4->setToolTip(tr("Triangle") + " - " + tr("Ctrl+T"));
    triangleCursor = QCursor(kAppProp->themeDir() + "cursors/triangle.png", 2, 2);
    action4->setCursor(triangleCursor);
    action4->setActionId(TAction::Triangle);

    geoActions.insert(TAction::Triangle, action4);
}

QBrush GeometricTool::setLiteBrush(QColor c, Qt::BrushStyle style)
{
    QBrush brush;
    QColor color = c;
    color.setAlpha(50);
    brush.setColor(color);
    brush.setStyle(style);

    return brush;
}

void GeometricTool::press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[GeometricTool::press()]";
    #endif

    Q_UNUSED(input)
    Q_UNUSED(brushManager)

    if (input->buttons() == Qt::LeftButton) {
        fillBrush = brushManager->brush();
        if (toolId() == TAction::Rectangle) {
            added = false;
            rect = new TupRectItem(QRectF(input->pos(), QSize(0,0)));
            rect->setPen(brushManager->pen());
            if (brushManager->brush().color().alpha() > 0)
                rect->setBrush(setLiteBrush(brushManager->brush().color(), brushManager->brush().style()));
            else
                rect->setBrush(brushManager->brush());

            currentPoint = input->pos();
        } else if (toolId() == TAction::Ellipse) {
            added = false;
            ellipse = new TupEllipseItem(QRectF(input->pos(), QSize(0,0)));
            ellipse->setPen(brushManager->pen());
            if (brushManager->brush().color().alpha() > 0)
                ellipse->setBrush(setLiteBrush(brushManager->brush().color(), brushManager->brush().style()));
            else
                ellipse->setBrush(brushManager->brush());

            currentPoint = input->pos();
        } else if (toolId() == TAction::Line) {
            currentPoint = input->pos();

            if (linePath) {
                QPainterPath painterPath = linePath->path();
                if (straightMode)
                    painterPath.lineTo(lastPoint);
                else
                    painterPath.cubicTo(lastPoint, lastPoint, lastPoint);

                linePath->setPath(painterPath);
            } else {
                linePath = new TupPathItem;
                linePath->setPen(brushManager->pen());
                if (brushManager->brush().color().alpha() > 0)
                   linePath->setBrush(setLiteBrush(brushManager->brush().color(), brushManager->brush().style()));
                else
                   linePath->setBrush(brushManager->brush());

                QPainterPath painterPath;
                painterPath.moveTo(currentPoint);
                linePath->setPath(painterPath);
                gScene->includeObject(linePath);

                guideLine = new TupLineItem();
                if (brushManager->pen().color().alpha() == 0) { // Show border guide line
                    QPen pen;
                    pen.setWidth(1);
                    pen.setBrush(QBrush(Qt::black));
                    guideLine->setPen(pen);
                } else {
                    guideLine->setPen(brushManager->pen());
                }

                guideLine->setLine(QLineF(input->pos().x(), input->pos().y(), input->pos().x(), input->pos().y()));
                gScene->includeObject(guideLine);
            }
        } else if (toolId() == TAction::Triangle) {
            added = false;
            triangle = new TupPathItem();
            triangle->setPen(brushManager->pen());
            if (brushManager->brush().color().alpha() > 0)
                triangle->setBrush(setLiteBrush(brushManager->brush().color(), brushManager->brush().style()));
            else
                triangle->setBrush(brushManager->brush());

            currentPoint = input->pos();
        }
    }
}

void GeometricTool::move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[GeometricTool::move()]";
    #endif
    */

    Q_UNUSED(brushManager)
    Q_UNUSED(gScene)
    
    if (toolId() == TAction::Rectangle || toolId() == TAction::Ellipse ||
        toolId() == TAction::Triangle) {
        if (!added) {
            if (toolId() == TAction::Rectangle)
                gScene->includeObject(rect);
            else if (toolId() == TAction::Ellipse)
                gScene->includeObject(ellipse);
            else if (toolId() == TAction::Triangle)
                gScene->includeObject(triangle);
            added = true;
        }

        int xMouse = static_cast<int> (input->pos().x());
        int yMouse = static_cast<int> (input->pos().y());
        int xInit = static_cast<int> (currentPoint.x());
        int yInit = static_cast<int> (currentPoint.y());

        QRectF rectVar;
        if (toolId() == TAction::Rectangle) {
            rectVar = rect->rect();
        } else if (toolId() == TAction::Ellipse) {
            rectVar = ellipse->rect();
        } else if (toolId() == TAction::Triangle) {
            rectVar = QRectF(currentPoint, QSize(5, 5));
            // rectVar.setBottomLeft(input->pos());
        }

        int width = abs(xMouse - xInit);
        int height = abs(yMouse - yInit);
        bool xWins = false;
        if (width <= height)
            xWins = true;

        if (proportion) {
            QPointF target;
            if (xMouse >= xInit) {
                if (yMouse >= yInit) {
                    if (xWins)
                        target = QPointF(xInit + width, yInit + width);      
                    else
                        target = QPointF(xInit + height, yInit + height);

                    rectVar.setBottomRight(target);
                } else {
                    if (xWins)
                        target = QPointF(xInit + width, yInit - width);
                    else
                        target = QPointF(xInit + height, yInit - height);

                    rectVar.setTopRight(target);
                }
            } else {
                if (yMouse >= yInit) {
                    if (xWins)
                        target = QPointF(xInit - width, yInit + width);
                    else
                        target = QPointF(xInit - height, yInit + height);

                    rectVar.setBottomLeft(target);
                } else {
                    if (xWins)
                        target = QPointF(xInit - width, yInit - width);
                    else
                        target = QPointF(xInit - height, yInit - height);

                    rectVar.setTopLeft(target);
                }
            }            
        } else { // No proportion
            if (side) {
                QPointF target;
                if (xMouse >= xInit) {
                    if (yMouse >= yInit) { // Right-Down
                        if (xWins) // Y > X
                            target = QPointF(xInit + xSideLength, yInit + height);
                        else // X > Y
                            target = QPointF(xInit + width, yInit + ySideLength);
                        rectVar.setBottomRight(target);
                    } else { // Right-Up
                        if (xWins) // Y > X
                            target = QPointF(xInit + xSideLength, yInit - height);
                        else // X > Y
                            target = QPointF(xInit + width, yInit - ySideLength);
                        rectVar.setTopRight(target);
                    }
                } else {
                    if (yMouse >= yInit) { // Left-Down
                        if (xWins) // Y > X
                            target = QPointF(xInit - xSideLength, yInit + height);
                        else // X > Y
                            target = QPointF(xInit - width, yInit + ySideLength);
                        rectVar.setBottomLeft(target);
                    } else { // Left-Up
                        if (xWins) // Y > X
                            target = QPointF(xInit - xSideLength, yInit - height);
                        else // X > Y
                            target = QPointF(xInit - width, yInit - ySideLength);
                        rectVar.setTopLeft(target);
                    }
                }
            } else {
                xSideLength = static_cast<int> (abs(xInit - input->pos().x()));
                ySideLength = static_cast<int> (abs(yInit - input->pos().y()));

                if (xMouse >= xInit) {
                    if (yMouse >= yInit)
                        rectVar.setBottomRight(input->pos());
                    else
                        rectVar.setTopRight(input->pos());
                } else {
                    if (yMouse >= yInit)
                        rectVar.setBottomLeft(input->pos());
                    else
                        rectVar.setTopLeft(input->pos());
                }
            }
        }

        if (toolId() == TAction::Rectangle) {
            rect->setRect(rectVar);
        } else if (toolId() == TAction::Ellipse) {
            ellipse->setRect(rectVar);
        } else if (toolId() == TAction::Triangle) {
            QPointF leftTopCorner = rectVar.topLeft();
            QPointF rightBottomCorner = rectVar.bottomRight();

            double topCornerX = leftTopCorner.x() + ((rightBottomCorner.x() - leftTopCorner.x())/2);
            double topCornerY = leftTopCorner.y();
            QPointF topCorner(topCornerX, topCornerY);
            double leftCornerX = leftTopCorner.x();
            double leftCornerY = rightBottomCorner.y();
            QPointF leftCorner(leftCornerX, leftCornerY);

            trianglePath = QPainterPath();
            trianglePath.moveTo(topCorner);
            trianglePath.cubicTo(rightBottomCorner, rightBottomCorner, rightBottomCorner);
            trianglePath.cubicTo(leftCorner, leftCorner, leftCorner);
            trianglePath.cubicTo(topCorner, topCorner, topCorner);

            triangle->setPath(trianglePath);
        }
    }
}

void GeometricTool::release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[GeometricTool::release()]";
    #endif

    Q_UNUSED(input)
    Q_UNUSED(brushManager)

    QDomDocument doc;
    QPointF point;

    if (toolId() == TAction::Rectangle) {
        rect->setBrush(fillBrush);
        doc.appendChild(dynamic_cast<TupAbstractSerializable *>(rect)->toXml(doc));
        point = rect->pos();
    } else if (toolId() == TAction::Ellipse) {
        ellipse->setBrush(fillBrush);
        doc.appendChild(dynamic_cast<TupAbstractSerializable *>(ellipse)->toXml(doc));
        point = QPoint(0, 0);
    } else if (toolId() == TAction::Triangle) {
        triangle->setBrush(fillBrush);
        doc.appendChild(dynamic_cast<TupAbstractSerializable *>(triangle)->toXml(doc));
        point = triangle->pos();
    } else if (toolId() == TAction::Line) {
        return;
    }

    TupProjectRequest event = TupRequestBuilder::createItemRequest(gScene->currentSceneIndex(), gScene->currentLayerIndex(),
                              gScene->currentFrameIndex(), 0, point, gScene->getSpaceContext(), TupLibraryObject::Item,
                              TupProjectRequest::Add, doc.toString());
    emit requested(&event);
}

QMap<TAction::ActionId, TAction *> GeometricTool::actions() const
{
    return geoActions;
}

TAction * GeometricTool::getAction(TAction::ActionId toolId)
{
    return geoActions[toolId];
}

int GeometricTool::toolType() const
{
    return Brush;
}
        
QWidget *GeometricTool::configurator()
{
    GeometricSettings::ToolType toolType = GeometricSettings::Line;

    if (toolId() == TAction::Rectangle)
        toolType = GeometricSettings::Rectangle;
    else if (toolId() == TAction::Ellipse)
        toolType = GeometricSettings::Ellipse;
    else if (toolId() == TAction::Triangle)
        toolType = GeometricSettings::Triangle;

    configPanel = new GeometricSettings(toolType);
    connect(configPanel, SIGNAL(lineTypeChanged(GeometricSettings::LineType)),
            this, SLOT(updateLineMode(GeometricSettings::LineType)));

    return configPanel;
}

void GeometricTool::aboutToChangeScene(TupGraphicsScene *scene)
{
    Q_UNUSED(scene)

    endItem();
}

void GeometricTool::aboutToChangeTool() 
{
    #ifdef TUP_DEBUG
        qDebug() << "[GeometricTool::aboutToChangeTool()]";
    #endif

    endItem();
    saveLineSettings();
}

void GeometricTool::saveConfig()
{
    #ifdef TUP_DEBUG
        qDebug() << "[GeometricTool::saveConfig()]";
    #endif

    saveLineSettings();
}

void GeometricTool::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F11 || event->key() == Qt::Key_Escape) {
        emit closeHugeCanvas();
        return;
    } else if (event->key() == Qt::Key_Control) {
        proportion = true;
    } else if (event->key() == Qt::Key_Shift) {
        side = true;
    } else if (event->key() == Qt::Key_X) {
        if (toolId() == TAction::Line)
            endItem();
    } else {
        QPair<int, int> flags = TAction::setKeyAction(event->key(), event->modifiers());
        if (flags.first != -1 && flags.second != -1)
            emit callForPlugin(flags.first, flags.second);
    }
}

void GeometricTool::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Shift)
        side = false;
    else if (event->key() == Qt::Key_Control)
        proportion = false;
}

QCursor GeometricTool::toolCursor()
{
    if (toolId() == TAction::Rectangle) {
        return squareCursor;
    } else if (this->toolId() == TAction::Ellipse) {
        return circleCursor;
    } else if (this->toolId() == TAction::Line) {
        return lineCursor;
    }

    return QCursor(Qt::ArrowCursor);
}

void GeometricTool::endItem()
{
    #ifdef TUP_DEBUG
        qDebug() << "[GeometricTool::endItem()]";
    #endif

    if (linePath) {
        linePath->setBrush(fillBrush);
        QDomDocument doc;
        doc.appendChild(dynamic_cast<TupAbstractSerializable *>(linePath)->toXml(doc));
        QPointF point = QPointF(0, 0);

        TupProjectRequest event = TupRequestBuilder::createItemRequest(scene->currentSceneIndex(), scene->currentLayerIndex(),
                                  scene->currentFrameIndex(), 0, point, scene->getSpaceContext(), TupLibraryObject::Item,
                                  TupProjectRequest::Add, doc.toString());

        emit requested(&event);
        linePath = nullptr;
    }
}

void GeometricTool::updatePos(QPointF pos)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[GeometricTool::updatePos()] - pos -> " << pos;
    #endif
    */

    if (linePath) {
        QLineF lineVar;
        if (proportion) {
            qreal dx = pos.x() - currentPoint.x();
            qreal dy = pos.y() - currentPoint.y();
            qreal m = fabs(dx/dy);

            if (m > 1) {
                lineVar = QLineF(currentPoint.x(), currentPoint.y(), pos.x(), currentPoint.y());
                lastPoint = QPointF(pos.x(), currentPoint.y());
            } else {
                lineVar = QLineF(currentPoint.x(), currentPoint.y(), currentPoint.x(), pos.y());
                lastPoint = QPointF(currentPoint.x(), pos.y());
            }

        } else {
            lineVar = QLineF(currentPoint, pos);
            lastPoint = pos;
        }
        if (guideLine)
            guideLine->setLine(lineVar);
    }
}

/*
void GeometricTool::doubleClick(const TupInputDeviceInformation *input, TupGraphicsScene *gScene)
{
    Q_UNUSED(input);
    Q_UNUSED(gScene);

    endItem();
}
*/

void GeometricTool::sceneResponse(const TupSceneResponse *event)
{
    Q_UNUSED(event)

    if (toolId() == TAction::Line)
        init(scene);
}

void GeometricTool::layerResponse(const TupLayerResponse *event)
{
    Q_UNUSED(event)

    if (toolId() == TAction::Line)
        init(scene);
}

void GeometricTool::frameResponse(const TupFrameResponse *event)
{
    Q_UNUSED(event)

    if (toolId() == TAction::Line)
        init(scene);
}

void GeometricTool::updateLineMode(GeometricSettings::LineType type)
{
    #ifdef TUP_DEBUG
        qDebug() << "[GeometricTool::updateLineMode()] - type -> " << type;
    #endif

    if (type == GeometricSettings::Bendable)
        straightMode = false;
    else
        straightMode = true;
}

void GeometricTool::saveLineSettings()
{
    TCONFIG->beginGroup("GeometricTool");
    if (straightMode)
        TCONFIG->setValue("LineType", 1);
    else
        TCONFIG->setValue("LineType", 0);
}
