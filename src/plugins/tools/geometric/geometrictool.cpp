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
#include "tuppathitem.h"
#include "tuplibraryobject.h"
#include "tupinputdeviceinformation.h"
#include "tupgraphicsscene.h"
#include "tupscene.h"
#include "tupframe.h"
#include "tuprequestbuilder.h"
#include "tupprojectrequest.h"
#include "tupbrushmanager.h"

struct GeometricTool::Private
{
    TupRectItem *rect;
    TupEllipseItem *ellipse;
    TupLineItem *line;
    TupPathItem *path;
    TupGraphicsScene *scene;
    Settings *configurator;
    bool added;
    QPointF currentPoint;
    QPointF lastPoint;
    QMap<QString, TAction *> actions;

    bool proportion;
    bool side;
    int xSideLength;
    int ySideLength;

    QGraphicsItem *item;
    QCursor squareCursor;
    QCursor circleCursor;
    QCursor lineCursor;
};

GeometricTool::GeometricTool() : k(new Private)
{
    k->scene = nullptr;
    k->path = nullptr;
    setupActions();
}

GeometricTool::~GeometricTool()
{
}

QStringList GeometricTool::keys() const
{
    return QStringList() << tr("Rectangle") << tr("Ellipse") << tr("Line");
}

void GeometricTool::init(TupGraphicsScene *scene)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[GeometricTool::init()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    k->scene = scene;
    delete k->path;
    k->path = nullptr;
    k->proportion = false;
    k->side = false;

    foreach (QGraphicsView *view, scene->views())
        view->setDragMode(QGraphicsView::NoDrag);
}

void GeometricTool::setupActions()
{
    TAction *action1 = new TAction(QIcon(kAppProp->themeDir() + "icons/square.png"), tr("Rectangle"), this);
    action1->setShortcut(QKeySequence(tr("R")));
    k->squareCursor = QCursor(kAppProp->themeDir() + "cursors/square.png", 0, 0);
    action1->setCursor(k->squareCursor);
    
    k->actions.insert(tr("Rectangle"), action1);
    
    TAction *action2 = new TAction(QIcon(kAppProp->themeDir() + "icons/ellipse.png"), tr("Ellipse"), this);
    action2->setShortcut(QKeySequence(tr("C")));
    k->circleCursor = QCursor(kAppProp->themeDir() + "cursors/circle.png", 2, 2);
    action2->setCursor(k->circleCursor);
    
    k->actions.insert(tr("Ellipse"), action2);
    
    TAction *action3 = new TAction(QIcon(kAppProp->themeDir() + "icons/line.png"), tr("Line"), this);
    action3->setShortcut(QKeySequence(tr("L")));
    k->lineCursor = QCursor(kAppProp->themeDir() + "cursors/line.png", 0, 15);
    action3->setCursor(k->lineCursor);

    k->actions.insert(tr("Line"), action3);
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

void GeometricTool::press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
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
            k->added = false;
            k->rect = new TupRectItem(QRectF(input->pos(), QSize(0,0)));
            k->rect->setPen(brushManager->pen());
            if (brushManager->brush().color().alpha() > 0)
                k->rect->setBrush(setLiteBrush(brushManager->brush().color(), brushManager->brush().style()));
            else
                k->rect->setBrush(brushManager->brush());

            k->currentPoint = input->pos();
        } else if (name() == tr("Ellipse")) {
            k->added = false;
            k->ellipse = new TupEllipseItem(QRectF(input->pos(), QSize(0,0)));
            k->ellipse->setPen(brushManager->pen());
            if (brushManager->brush().color().alpha() > 0)
                k->ellipse->setBrush(setLiteBrush(brushManager->brush().color(), brushManager->brush().style()));
            else
                k->ellipse->setBrush(brushManager->brush());

            k->currentPoint = input->pos();
        } else if (name() == tr("Line")) {
            k->currentPoint = input->pos();

            if (k->path) {
                QPainterPath path = k->path->path();
                path.cubicTo(k->lastPoint, k->lastPoint, k->lastPoint);
                k->path->setPath(path);
            } else {
                k->path = new TupPathItem;
                k->path->setPen(brushManager->pen());
                if (brushManager->brush().color().alpha() > 0)
                   k->path->setBrush(setLiteBrush(brushManager->brush().color(), brushManager->brush().style()));
                else
                   k->path->setBrush(brushManager->brush());

                QPainterPath path;
                path.moveTo(k->currentPoint);
                k->path->setPath(path);
                scene->includeObject(k->path);

                k->line = new TupLineItem();
                if (brushManager->pen().color().alpha() == 0) {
                    QPen pen;
                    pen.setWidth(1);
                    pen.setBrush(QBrush(Qt::black));
                    k->line->setPen(pen);
                } else {
                    k->line->setPen(brushManager->pen());
                }

                k->line->setLine(QLineF(input->pos().x(), input->pos().y(), input->pos().x(), input->pos().y()));
                scene->includeObject(k->line);
            }
        }
    }
}

void GeometricTool::move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
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
    Q_UNUSED(scene);
    
    if (name() == tr("Rectangle") || name() == tr("Ellipse")) {
        if (!k->added) {
            if (name() == tr("Rectangle"))
                scene->includeObject(k->rect);
            else
                scene->includeObject(k->ellipse);
            k->added = true;
        }

        int xMouse = static_cast<int> (input->pos().x());
        int yMouse = static_cast<int> (input->pos().y());
        int xInit = static_cast<int> (k->currentPoint.x());
        int yInit = static_cast<int> (k->currentPoint.y());

        QRectF rect;
        if (name() == tr("Rectangle"))
            rect = k->rect->rect();
        else
            rect = k->ellipse->rect();

        int width = abs(xMouse - xInit);
        int height = abs(yMouse - yInit);
        bool xWins = false;
        if (width <= height)
            xWins = true;

        if (k->proportion) {
            QPointF target;
            if (xMouse >= xInit) {
                if (yMouse >= yInit) {
                    if (xWins)
                        target = QPointF(xInit + width, yInit + width);      
                    else
                        target = QPointF(xInit + height, yInit + height);

                    rect.setBottomRight(target);
                } else {
                    if (xWins)
                        target = QPointF(xInit + width, yInit - width);
                    else
                        target = QPointF(xInit + height, yInit - height);

                    rect.setTopRight(target);
                }
            } else {
                if (yMouse >= yInit) {
                    if (xWins)
                        target = QPointF(xInit - width, yInit + width);
                    else
                        target = QPointF(xInit - height, yInit + height);

                    rect.setBottomLeft(target);
                } else {
                    if (xWins)
                        target = QPointF(xInit - width, yInit - width);
                    else
                        target = QPointF(xInit - height, yInit - height);

                    rect.setTopLeft(target);
                }
            }            
        } else {
            if (k->side) {
                QPointF target;
                if (xMouse >= xInit) {
                    if (yMouse >= yInit) { // Right-Down
                        if (xWins) // Y > X
                            target = QPointF(xInit + k->xSideLength, yInit + height);
                        else // X > Y
                            target = QPointF(xInit + width, yInit + k->ySideLength);
                        rect.setBottomRight(target);
                    } else { // Right-Up
                        if (xWins) // Y > X
                            target = QPointF(xInit + k->xSideLength, yInit - height);
                        else // X > Y
                            target = QPointF(xInit + width, yInit - k->ySideLength);
                        rect.setTopRight(target);
                    }
                } else {
                    if (yMouse >= yInit) { // Left-Down
                        if (xWins) // Y > X
                            target = QPointF(xInit - k->xSideLength, yInit + height);
                        else // X > Y
                            target = QPointF(xInit - width, yInit + k->ySideLength);
                        rect.setBottomLeft(target);
                    } else { // Left-Up
                        if (xWins) // Y > X
                            target = QPointF(xInit - k->xSideLength, yInit - height);
                        else // X > Y
                            target = QPointF(xInit - width, yInit - k->ySideLength);
                        rect.setTopLeft(target);
                    }
                }
            } else {
                k->xSideLength = static_cast<int> (abs(xInit - input->pos().x()));
                k->ySideLength = static_cast<int> (abs(yInit - input->pos().y()));

                if (xMouse >= xInit) {
                    if (yMouse >= yInit)
                        rect.setBottomRight(input->pos());
                    else
                        rect.setTopRight(input->pos());
                } else {
                    if (yMouse >= yInit)
                        rect.setBottomLeft(input->pos());
                    else
                        rect.setTopLeft(input->pos());
                }
            }
        }

        if (name() == tr("Rectangle"))
            k->rect->setRect(rect);
        else
            k->ellipse->setRect(rect);
    } 
}

void GeometricTool::release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
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
        k->rect->setBrush(fillBrush);
        doc.appendChild(dynamic_cast<TupAbstractSerializable *>(k->rect)->toXml(doc));
        point = k->rect->pos();
    } else if (name() == tr("Ellipse")) {
        k->ellipse->setBrush(fillBrush);
        doc.appendChild(dynamic_cast<TupAbstractSerializable *>(k->ellipse)->toXml(doc));
        QRectF rect = k->ellipse->rect();
        point = rect.topLeft();
    } else if (name() == tr("Line")) {
        return;
    }

    TupProjectRequest event = TupRequestBuilder::createItemRequest(scene->currentSceneIndex(), scene->currentLayerIndex(), 
                              scene->currentFrameIndex(), 0, point, scene->getSpaceContext(), TupLibraryObject::Item, 
                              TupProjectRequest::Add, doc.toString());
    emit requested(&event);
}

QMap<QString, TAction *> GeometricTool::actions() const
{
    return k->actions;
}

int GeometricTool::toolType() const
{
    return Brush;
}
        
QWidget *GeometricTool::configurator()
{
    Settings::ToolType toolType = Settings::Line;

    if (name() == tr("Rectangle"))
        toolType = Settings::Rectangle;
    else if (name() == tr("Ellipse"))
             toolType = Settings::Ellipse;

    k->configurator = new Settings(toolType);
    return k->configurator;
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
        k->proportion = true;
    } else if (event->key() == Qt::Key_Shift) {
        k->side = true;
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
        k->side = false;
    else if (event->key() == Qt::Key_Control)
        k->proportion = false;
}

QCursor GeometricTool::cursor() const
{
    if (name() == tr("Rectangle")) {
        return k->squareCursor;
    } else if (name() == tr("Ellipse")) {
        return k->circleCursor;
    } else if (name() == tr("Line")) {
        return k->lineCursor;
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

    if (k->path) {
        k->path->setBrush(fillBrush);
        QDomDocument doc;
        doc.appendChild(dynamic_cast<TupAbstractSerializable *>(k->path)->toXml(doc));
        QPointF point = QPointF(0, 0);

        TupProjectRequest event = TupRequestBuilder::createItemRequest(k->scene->currentSceneIndex(), k->scene->currentLayerIndex(),
                                  k->scene->currentFrameIndex(), 0, point, k->scene->getSpaceContext(), TupLibraryObject::Item, 
                                  TupProjectRequest::Add, doc.toString());

        emit requested(&event);
        k->path = nullptr;
    }
}

void GeometricTool::updatePos(QPointF pos)
{
    if (k->path) {
        QLineF line;
        if (k->proportion) {
            qreal dx = pos.x() - k->currentPoint.x();
            qreal dy = pos.y() - k->currentPoint.y();
            qreal m = fabs(dx/dy);

            if (m > 1) {
                line = QLineF(k->currentPoint.x(), k->currentPoint.y(), pos.x(), k->currentPoint.y());
                k->lastPoint = QPointF(pos.x(), k->currentPoint.y());
            } else {
                line = QLineF(k->currentPoint.x(), k->currentPoint.y(), k->currentPoint.x(), pos.y());
                k->lastPoint = QPointF(k->currentPoint.x(), pos.y());
            }

        } else {
            line = QLineF(k->currentPoint, pos);
            k->lastPoint = pos;
        }
        if (k->line)
            k->line->setLine(line);
    }
}

/*
void GeometricTool::doubleClick(const TupInputDeviceInformation *input, TupGraphicsScene *scene)
{
    Q_UNUSED(input);
    Q_UNUSED(scene);

    endItem();
}
*/

void GeometricTool::sceneResponse(const TupSceneResponse *event)
{
    Q_UNUSED(event);
    if (name() == tr("Line")) 
        init(k->scene);
}

void GeometricTool::layerResponse(const TupLayerResponse *event)
{
    Q_UNUSED(event);
    if (name() == tr("Line")) 
        init(k->scene);
}

void GeometricTool::frameResponse(const TupFrameResponse *event)
{
    Q_UNUSED(event);
    if (name() == tr("Line")) 
        init(k->scene);
}
