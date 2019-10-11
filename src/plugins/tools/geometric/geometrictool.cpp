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
    path = nullptr;
    setupActions();
}

GeometricTool::~GeometricTool()
{
}

QStringList GeometricTool::keys() const
{
    return QStringList() << tr("Rectangle") << tr("Ellipse") << tr("Line");
}

void GeometricTool::init(TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[GeometricTool::init()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    scene = gScene;
    delete path;
    path = nullptr;
    proportion = false;
    side = false;

    foreach (QGraphicsView *view, scene->views())
        view->setDragMode(QGraphicsView::NoDrag);
}

void GeometricTool::setupActions()
{
    TAction *action1 = new TAction(QIcon(kAppProp->themeDir() + "icons/square.png"), tr("Rectangle"), this);
    action1->setShortcut(QKeySequence(tr("R")));
    squareCursor = QCursor(kAppProp->themeDir() + "cursors/square.png", 0, 0);
    action1->setCursor(squareCursor);
    
    geoActions.insert(tr("Rectangle"), action1);
    
    TAction *action2 = new TAction(QIcon(kAppProp->themeDir() + "icons/ellipse.png"), tr("Ellipse"), this);
    action2->setShortcut(QKeySequence(tr("C")));
    circleCursor = QCursor(kAppProp->themeDir() + "cursors/circle.png", 2, 2);
    action2->setCursor(circleCursor);
    
    geoActions.insert(tr("Ellipse"), action2);
    
    TAction *action3 = new TAction(QIcon(kAppProp->themeDir() + "icons/line.png"), tr("Line"), this);
    action3->setShortcut(QKeySequence(tr("L")));
    lineCursor = QCursor(kAppProp->themeDir() + "cursors/line.png", 0, 15);
    action3->setCursor(lineCursor);

    geoActions.insert(tr("Line"), action3);
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
        #ifdef Q_OS_WIN
            qDebug() << "[GeometricTool::press()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    Q_UNUSED(input);
    Q_UNUSED(brushManager);

    if (input->buttons() == Qt::LeftButton) {
        fillBrush = brushManager->brush();
        if (name() == tr("Rectangle")) {
            added = false;
            rect = new TupRectItem(QRectF(input->pos(), QSize(0,0)));
            rect->setPen(brushManager->pen());
            if (brushManager->brush().color().alpha() > 0)
                rect->setBrush(setLiteBrush(brushManager->brush().color(), brushManager->brush().style()));
            else
                rect->setBrush(brushManager->brush());

            currentPoint = input->pos();
        } else if (name() == tr("Ellipse")) {
            added = false;
            ellipse = new TupEllipseItem(QRectF(input->pos(), QSize(0,0)));
            ellipse->setPen(brushManager->pen());
            if (brushManager->brush().color().alpha() > 0)
                ellipse->setBrush(setLiteBrush(brushManager->brush().color(), brushManager->brush().style()));
            else
                ellipse->setBrush(brushManager->brush());

            currentPoint = input->pos();
        } else if (name() == tr("Line")) {
            currentPoint = input->pos();

            if (path) {
                QPainterPath painterPath = path->path();
                painterPath.cubicTo(lastPoint, lastPoint, lastPoint);
                path->setPath(painterPath);
            } else {
                path = new TupPathItem;
                path->setPen(brushManager->pen());
                if (brushManager->brush().color().alpha() > 0)
                   path->setBrush(setLiteBrush(brushManager->brush().color(), brushManager->brush().style()));
                else
                   path->setBrush(brushManager->brush());

                QPainterPath painterPath;
                painterPath.moveTo(currentPoint);
                path->setPath(painterPath);
                gScene->includeObject(path);

                line = new TupLineItem();
                if (brushManager->pen().color().alpha() == 0) {
                    QPen pen;
                    pen.setWidth(1);
                    pen.setBrush(QBrush(Qt::black));
                    line->setPen(pen);
                } else {
                    line->setPen(brushManager->pen());
                }

                line->setLine(QLineF(input->pos().x(), input->pos().y(), input->pos().x(), input->pos().y()));
                gScene->includeObject(line);
            }
        }
    }
}

void GeometricTool::move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    /*
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[GeometricTool::move()]";
        #else
            T_FUNCINFO;
        #endif
    #endif
    */

    Q_UNUSED(brushManager);
    Q_UNUSED(gScene);
    
    if (name() == tr("Rectangle") || name() == tr("Ellipse")) {
        if (!added) {
            if (name() == tr("Rectangle"))
                gScene->includeObject(rect);
            else
                gScene->includeObject(ellipse);
            added = true;
        }

        int xMouse = static_cast<int> (input->pos().x());
        int yMouse = static_cast<int> (input->pos().y());
        int xInit = static_cast<int> (currentPoint.x());
        int yInit = static_cast<int> (currentPoint.y());

        QRectF rectVar;
        if (name() == tr("Rectangle"))
            rectVar = rect->rect();
        else
            rectVar = ellipse->rect();

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
        } else {
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

        if (name() == tr("Rectangle"))
            rect->setRect(rectVar);
        else
            ellipse->setRect(rectVar);
    } 
}

void GeometricTool::release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[GeometricTool::release()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    Q_UNUSED(input);
    Q_UNUSED(brushManager);

    QDomDocument doc;
    QPointF point;

    if (name() == tr("Rectangle")) {
        rect->setBrush(fillBrush);
        doc.appendChild(dynamic_cast<TupAbstractSerializable *>(rect)->toXml(doc));
        point = rect->pos();
    } else if (name() == tr("Ellipse")) {
        ellipse->setBrush(fillBrush);
        doc.appendChild(dynamic_cast<TupAbstractSerializable *>(ellipse)->toXml(doc));
        QRectF rect = ellipse->rect();
        point = rect.topLeft();
    } else if (name() == tr("Line")) {
        return;
    }

    TupProjectRequest event = TupRequestBuilder::createItemRequest(gScene->currentSceneIndex(), gScene->currentLayerIndex(),
                              gScene->currentFrameIndex(), 0, point, gScene->getSpaceContext(), TupLibraryObject::Item,
                              TupProjectRequest::Add, doc.toString());
    emit requested(&event);
}

QMap<QString, TAction *> GeometricTool::actions() const
{
    return geoActions;
}

int GeometricTool::toolType() const
{
    return Brush;
}
        
QWidget *GeometricTool::configurator()
{
    PenSettings::ToolType toolType = PenSettings::Line;

    if (name() == tr("Rectangle"))
        toolType = PenSettings::Rectangle;
    else if (name() == tr("Ellipse"))
        toolType = PenSettings::Ellipse;

    configPanel = new PenSettings(toolType);
    return configPanel;
}

void GeometricTool::aboutToChangeScene(TupGraphicsScene *scene)
{
    Q_UNUSED(scene);

    endItem();
}

void GeometricTool::aboutToChangeTool() 
{
    endItem();
}

void GeometricTool::saveConfig()
{
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
        if (name() == tr("Line"))
            endItem();
    } else {
        QPair<int, int> flags = TupToolPlugin::setKeyAction(event->key(), event->modifiers());
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

QCursor GeometricTool::polyCursor() const
{
    if (name() == tr("Rectangle")) {
        return squareCursor;
    } else if (name() == tr("Ellipse")) {
        return circleCursor;
    } else if (name() == tr("Line")) {
        return lineCursor;
    }

    return QCursor(Qt::ArrowCursor);
}

void GeometricTool::endItem()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[GeometricTool::endItem()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (path) {
        path->setBrush(fillBrush);
        QDomDocument doc;
        doc.appendChild(dynamic_cast<TupAbstractSerializable *>(path)->toXml(doc));
        QPointF point = QPointF(0, 0);

        TupProjectRequest event = TupRequestBuilder::createItemRequest(scene->currentSceneIndex(), scene->currentLayerIndex(),
                                  scene->currentFrameIndex(), 0, point, scene->getSpaceContext(), TupLibraryObject::Item,
                                  TupProjectRequest::Add, doc.toString());

        emit requested(&event);
        path = nullptr;
    }
}

void GeometricTool::updatePos(QPointF pos)
{
    if (path) {
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
        if (line)
            line->setLine(lineVar);
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
    Q_UNUSED(event);
    if (name() == tr("Line")) 
        init(scene);
}

void GeometricTool::layerResponse(const TupLayerResponse *event)
{
    Q_UNUSED(event);
    if (name() == tr("Line")) 
        init(scene);
}

void GeometricTool::frameResponse(const TupFrameResponse *event)
{
    Q_UNUSED(event);
    if (name() == tr("Line")) 
        init(scene);
}
