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

#include "tuppathitem.h"
#include "tupsvg2qt.h"
#include "tupgraphicalgorithm.h"
#include "tupserializer.h"

TupPathItem::TupPathItem(QGraphicsItem *parent) : QGraphicsPathItem(parent), dragOver(false)
{
    setAcceptDrops(true);
}

TupPathItem::~TupPathItem()
{
}

void TupPathItem::fromXml(const QString &xml)
{
    Q_UNUSED(xml)
}

QDomElement TupPathItem::toXml(QDomDocument &doc) const
{
    QDomElement root = doc.createElement("path");

    QString strPath = pathToString();
    root.setAttribute("coords", strPath);

    root.appendChild(TupSerializer::properties(this, doc));
    
    QBrush brush = this->brush();
    root.appendChild(TupSerializer::brush(&brush, doc));
    
    QPen pen = this->pen();
    root.appendChild(TupSerializer::pen(&pen, doc));

    return root;
}

void TupPathItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsPathItem::paint(painter, option, widget);
}

bool TupPathItem::contains(const QPointF &point) const
{
    return QGraphicsPathItem::contains(point);
}

void TupPathItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasColor()) {
        event->setAccepted(true);
        dragOver = true;
        update();
    } else {
        event->setAccepted(false);
    }
}

void TupPathItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event)

    dragOver = false;
    update();
}

void TupPathItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    dragOver = false;

    if (event->mimeData()->hasColor()) {
        QVariant color = event->mimeData()->colorData();
        setBrush(QBrush(color.value<QColor>()));
    } else if (event->mimeData()->hasImage()) {
        QVariant pixmap = event->mimeData()->imageData();
        setBrush(QBrush(pixmap.value<QPixmap>()));
    }

    update();
}

QString TupPathItem::pathToString() const
{
    QPainterPath route = path();
    QString strPath = "";
    QChar t;
    int total = route.elementCount();

    for(int i=0; i < total; i++) {
        QPainterPath::Element e = route.elementAt(i);
        switch (e.type) {
            case QPainterPath::MoveToElement:
            {
                if (t != 'M') {
                    t = 'M';
                    strPath += "M " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                } else {
                    strPath += QString::number(e.x) + " " + QString::number(e.y) + " ";
                }
            }
            break;
            case QPainterPath::LineToElement:
            {
                if (t != 'L') {
                    t = 'L';
                    strPath += " L " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                } else {
                    strPath += QString::number(e.x) + " " + QString::number(e.y) + " ";
                }
            }
            break;
            case QPainterPath::CurveToElement:
            {
                if (t != 'C') {
                    t = 'C';
                    strPath += " C " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                } else {
                    strPath += "  " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                }
            }
            break;
            case QPainterPath::CurveToDataElement:
            {
                if (t == 'C')
                    strPath +=  " " + QString::number(e.x) + "  " + QString::number(e.y) + " ";
            }
            break;
        }
    }

    return strPath;
}

bool TupPathItem::isNotEdited()
{
    return doList.isEmpty() && undoList.isEmpty();
}

void TupPathItem::saveOriginalPath()
{
    QString original = pathToString();
    doList << original;
}

void TupPathItem::setPathFromString(const QString &route)         
{
    QPainterPath qPath;
    TupSvg2Qt::svgpath2qtpath(route, qPath);
    setPath(qPath);
    doList << route;
}

void TupPathItem::undoPath()
{
    if (doList.count() > 1) {
        undoList << doList.takeLast();
        if (!doList.isEmpty()) {
            QString route = doList.last();
            QPainterPath qPath;
            TupSvg2Qt::svgpath2qtpath(route, qPath);
            setPath(qPath);
        }
    }
}

void TupPathItem::redoPath()
{
    if (!undoList.isEmpty()) {
        QString route = undoList.takeLast();
        doList << route;
        QPainterPath qPath;
        TupSvg2Qt::svgpath2qtpath(route, qPath);
        setPath(qPath);
    }
}

QString TupPathItem::refactoringPath(NodePosition policy, int nodesTotal)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::refactoringPath()] - policy ->" << policy;
        qDebug() << "[TupPathItem::refactoringPath()] - Tracking nodesTotal ->" << nodesTotal;
    #endif

    if (!pathCollection.contains(nodesTotal)) {
        qDebug() << "STORING path for nodesTotal ->" << nodesTotal;
        qDebug() << "path ->" << pathToString();
        pathCollection[nodesTotal] = pathToString();
    }

    if (pathCollection.contains(nodesTotal-1)) {
        qDebug() << "Returning saved path at ->" << (nodesTotal-1);
        return pathCollection[nodesTotal-1];
    }

    QChar t;
    QPainterPath route = path();
    int elementsTotal = route.elementCount();
    QString pathStr = "";
    QList<int> curveIndexes;

    bool lookingData = false;
    int dataCounter = 0;

    qDebug() << "[TupPathItem::refactoringPath()] - remove policy ->" << policy;
    int curvesCounter = 0;
    for(int i=0; i<elementsTotal; i++) {
        QPainterPath::Element e = route.elementAt(i);
        if (e.type == QPainterPath::CurveToElement || e.type == QPainterPath::MoveToElement) {
            curveIndexes << curvesCounter;
            curvesCounter++;
        }
    }

    qDebug() << "*** Nodes Indexes ->" << curveIndexes;

    // Remove the middle node
    int mark = curveIndexes.at(0); // FirstNode
    if (policy == MiddleNode) {
        if (curvesCounter % 2 == 0) {
            mark = curveIndexes.at(curvesCounter/2);
        } else {
            if (curvesCounter == 3)
                mark = curveIndexes.at(1);
            else
                mark = curveIndexes.at((curvesCounter/2) + 1);
        }
    } else if (policy == LastNode) {
        mark = curveIndexes.at(curvesCounter - 1);
    }

    qDebug() << "*** mark ->" << mark;

    bool replace = false;
    curvesCounter = 0;
    for(int i=0; i<elementsTotal; i++) {
        QPainterPath::Element e = route.elementAt(i);
        qDebug() << "[TupPathItem::refactoringPath()] - i ->" << i;
        qDebug() << "[TupPathItem::refactoringPath()] - e.x ->"  << e.x;
        qDebug() << "[TupPathItem::refactoringPath()] - e.y ->"  << e.y;

        switch (e.type) {
            case QPainterPath::MoveToElement:
            {
                qDebug() << "[TupPathItem::refactoringPath()] - MoveToElement";

                if (policy == FirstNode && curvesCounter == mark) {
                    qDebug() << "*** Removing original first node!";
                    replace = true;
                } else {
                    if (t != 'M') {
                        t = 'M';
                        pathStr += "M " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                    } else {
                        pathStr += QString::number(e.x) + " " + QString::number(e.y) + " ";
                    }
                }

                curvesCounter++;
            }
            break;
            case QPainterPath::LineToElement:
            {
                qDebug() << "[TupPathItem::refactoringPath()] - LineToElement";

                if (t != 'L') {
                    t = 'L';
                    pathStr += " L " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                } else {
                    pathStr += QString::number(e.x) + " " + QString::number(e.y) + " ";
                }
            }
            break;
            case QPainterPath::CurveToElement:
            {
                qDebug() << "[TupPathItem::refactoringPath()] - CurveToElement";
                qDebug() << "[TupPathItem::refactoringPath()] - curvesCounter ->" << curvesCounter;
                qDebug() << "[TupPathItem::refactoringPath()] - mark ->" << mark;

                if (policy == FirstNode && replace) {
                    qDebug() << "*** Adding new First Node to String!";
                    pathStr += "M " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                    replace = false;
                    lookingData = true;
                } else {
                    if (curvesCounter != mark) {
                        if (t != 'C') {
                            t = 'C';
                            pathStr += " C " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                        } else {
                            pathStr += "  " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                        }
                    } else {
                        qDebug() << "*** Removing curve node at index ->" << curvesCounter;
                        lookingData = true;
                    }
                }
                curvesCounter++;
            }
            break;
            case QPainterPath::CurveToDataElement:
            {
                qDebug() << "[TupPathItem::refactoringPath()] - CurveToDataElement";
                if (!lookingData) {
                    if (t == 'C')
                        pathStr +=  " " + QString::number(e.x) + "  " + QString::number(e.y) + " ";
                } else {
                    qDebug() << "*** Removing curve data node at index ->" << curvesCounter;
                    dataCounter++;

                    if (dataCounter == 2) {
                        lookingData = false;
                        dataCounter = 0;
                    }
                }
            }
            break;
        }

        qDebug() << "---";
    }

    qDebug() << "STORING path for (nodesTotal - 1) ->" << (nodesTotal - 1);
    pathCollection[nodesTotal-1] = pathStr;

    qDebug() << "pathStr -> " << pathStr;

    return pathStr;
}

QString TupPathItem::pathRestored(int nodesTotal) const
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::pathRestored()] - nodesTotal ->" << nodesTotal;
    #endif

    if (pathCollection.contains(nodesTotal)) {
        qDebug() << "RETURNING path for nodesTotal ->" << nodesTotal;
        return pathCollection[nodesTotal];
    }

    return "";
}

void TupPathItem::resetPathHistory()
{
    pathCollection.clear();
}
