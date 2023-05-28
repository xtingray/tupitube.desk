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
#include "talgorithm.h"

#include <QBrush>
#include <QMimeData>
#include <QPainterPath>

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

QString TupPathItem::refactoringPath(NodeLocation policy, int nodesTotal)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::refactoringPath()] - policy ->" << policy;
        qDebug() << "[TupPathItem::refactoringPath()] - Tracking nodesTotal ->" << nodesTotal;
    #endif

    if (!pathCollection.contains(nodesTotal)) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPathItem::refactoringPath()] - Storing path for nodesTotal ->" << nodesTotal;
            qDebug() << "[TupPathItem::refactoringPath()] - path ->" << pathToString();
        #endif
        pathCollection[nodesTotal] = pathToString();
    }

    if (pathCollection.contains(nodesTotal-1)) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPathItem::refactoringPath()] - Returning saved path at ->" << (nodesTotal-1);
        #endif

        return pathCollection[nodesTotal-1];
    }

    QChar t;
    QPainterPath route = path();
    int elementsTotal = route.elementCount();
    QString pathStr = "";
    QList<int> curveIndexes;

    bool lookingData = false;
    int dataCounter = 0;
    int curvesCounter = 0;
    for(int i=0; i<elementsTotal; i++) {
        QPainterPath::Element e = route.elementAt(i);
        if (e.type == QPainterPath::CurveToElement || e.type == QPainterPath::MoveToElement
            || e.type == QPainterPath::LineToElement) {
            curveIndexes << curvesCounter;
            curvesCounter++;
        }
    }

    int mark = curveIndexes.at(0); // FirstNode
    if (curvesCounter > 1) {
        if (policy == MiddleNode) {
            if (curvesCounter % 2 == 0) { // Remove the middle node
                mark = curveIndexes.at(curvesCounter/2);
            } else {
                if (curvesCounter == 3)
                    mark = curveIndexes.at(1);
                else
                    mark = curveIndexes.at((curvesCounter/2) + 1);
            }
        } else if (policy == RandomNode) {
            int index = TAlgorithm::randomNumber(curvesCounter);
            if (index == 0)
                index = 1;
            if (index == (curvesCounter - 1))
                index = curvesCounter - 2;

            mark = curveIndexes.at(index);
        } else if (policy == LastNode) {
            mark = curveIndexes.at(curvesCounter - 1);
        }
    }

    bool replace = false;
    curvesCounter = 0;
    for(int i=0; i<elementsTotal; i++) {
        QPainterPath::Element e = route.elementAt(i);

        switch (e.type) {
            case QPainterPath::MoveToElement:
            {
                if (policy == FirstNode && curvesCounter == mark) {
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
                if (replace) {
                    if (t != 'M') {
                        t = 'M';
                        pathStr += "M " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                    } else {
                        pathStr += QString::number(e.x) + " " + QString::number(e.y) + " ";
                    }
                    replace = false;
                } else if (curvesCounter != mark) {
                    if (t != 'L') {
                        t = 'L';
                        pathStr += " L " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                    } else {
                        pathStr += QString::number(e.x) + " " + QString::number(e.y) + " ";
                    }
                }

                curvesCounter++;
            }
            break;
            case QPainterPath::CurveToElement:
            {
                if (!replace && curvesCounter != mark) {
                    if (t != 'C') {
                        t = 'C';
                        pathStr += " C " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                    } else {
                        pathStr += "  " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                    }
                } else {
                    lookingData = true;
                }
                curvesCounter++;
            }
            break;
            case QPainterPath::CurveToDataElement:
            {
                if (!lookingData) {
                    if (t == 'C')
                        pathStr +=  " " + QString::number(e.x) + "  " + QString::number(e.y) + " ";
                } else {
                    dataCounter++;

                    if (dataCounter == 2) {
                        lookingData = false;
                        dataCounter = 0;

                        if (replace) {
                            replace = false;
                            pathStr += "M " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                        }
                    }
                }
            }
            break;
        }
    }

    pathCollection[nodesTotal-1] = pathStr;

    return pathStr;
}

QString TupPathItem::pathRestored(int nodesTotal) const
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::pathRestored()] - nodesTotal ->" << nodesTotal;
    #endif

    if (pathCollection.contains(nodesTotal))
        return pathCollection[nodesTotal];

    return "";
}

void TupPathItem::resetPathHistory()
{
    pathCollection.clear();
}

QString TupPathItem::removeNodeFromPath(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::removeNodeFromPath()] - index ->" << index;
    #endif

    QChar t;
    QPainterPath route = path();
    int elementsTotal = route.elementCount();
    QString pathStr = "";

    // bool lookingData = false;
    int dataCounter = 0;
    int nodesCounter = 0;
    bool found = false;

    QStringList parts;
    bool replace = false;
    for(int i=0; i<elementsTotal; i++) {
        QPainterPath::Element e = route.elementAt(i);
        switch (e.type) {
            case QPainterPath::MoveToElement:
            {
                nodesCounter++;
                /*
                qDebug() << "---";
                qDebug() << "index ->" << index;
                qDebug() << "MoveToElement - nodesCounter ->" << nodesCounter;
                */

                if (nodesCounter != index) {
                    // qDebug() << "Adding MoveToElement record...";
                    if (t != 'M') {
                        t = 'M';
                        parts << "M " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                    } else {
                        parts << QString::number(e.x) + " " + QString::number(e.y) + " ";
                    }
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupPathItem::removeNodeFromPath()] - MoveToElement - Node was FOUND!";
                    #endif

                    replace = true;
                    found = true;
                }
            }
            break;
            case QPainterPath::LineToElement:
            {
                nodesCounter++;
                /*
                qDebug() << "---";
                qDebug() << "index ->" << index;
                qDebug() << "LineToElement - nodesCounter ->" << nodesCounter;
                */

                if (replace) {
                    // qDebug() << "Adding LineToElement record...";
                    parts << "M " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                    replace = false;
                } else {
                    if (nodesCounter != index) {
                        if (t != 'L') {
                            t = 'L';
                            parts << " L " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                        } else {
                            parts << QString::number(e.x) + " " + QString::number(e.y) + " ";
                        }
                    } else {
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupPathItem::removeNodeFromPath()] - LineToElement - Node was FOUND!";
                        #endif
                    }
                }
            }
            break;
            case QPainterPath::CurveToElement:
            {
                /*
                qDebug() << "---";
                qDebug() << "index ->" << index;
                qDebug() << "CurveToElement - nodesCounter ->" << nodesCounter;
                qDebug() << "Adding CurveToElement record...";
                */

                if (t != 'C') {
                    t = 'C';
                    parts << " C " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                } else {
                    parts << "  " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                }

                dataCounter = 0;
            }
            break;
            case QPainterPath::CurveToDataElement:
            {
                /*
                qDebug() << "---";
                qDebug() << "index ->" << index;
                qDebug() << "CurveToDataElement - nodesCounter ->" << nodesCounter;
                */

                dataCounter++;
                if (dataCounter == 2)
                    nodesCounter++;

                // qDebug() << "dataCounter ->" << dataCounter;

                if ((index == nodesCounter && dataCounter == 2) || (replace && dataCounter == 2)) {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupPathItem::removeNodeFromPath()] - *** Node was FOUND!";
                    #endif

                    found = true;

                    // qDebug() << "Removing record ->" << parts.last();
                    parts.removeLast();
                    // qDebug() << "Removing record ->" << parts.last();
                    parts.removeLast();

                    if (replace) {
                        t = 'M';
                        parts << "M " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                        replace = false;
                    } else {
                        t = ' ';
                    }
                } else {
                    /*
                    qDebug() << "dataCounter->" << dataCounter;
                    qDebug() << "Adding CurveToDataElement 1!";
                    qDebug() << "t ->" << t;
                    */

                    parts <<  " " + QString::number(e.x) + "  " + QString::number(e.y) + " ";
                }
            }
            break;
        }
    }

    if (!found) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPathItem::removeNodeFromPath()] - Warning: Node was NOT found! ->" << index;
        #endif
    }

    foreach(QString line, parts)
        pathStr += line;

    // qDebug() << "pathStr ->" << pathStr;

    return pathStr;
}

QString TupPathItem::changeNodeTypeFromPath(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::changeNodeTypeFromPath()] - index ->" << index;
    #endif

    QChar t;
    QPainterPath route = path();
    int elementsTotal = route.elementCount();

    QStringList parts;
    bool replace = false;
    int dataCounter = 0;
    bool found = false;
    int nodesCounter = 0;
    for(int i=0; i<elementsTotal; i++) {
        QPainterPath::Element e = route.elementAt(i);
        QPointF pathPoint = QPointF(e.x, e.y);

        switch (e.type) {
            case QPainterPath::MoveToElement:
            {
                nodesCounter++;
                if (nodesCounter == index)
                    replace = true;

                if (t != 'M') {
                    t = 'M';
                    parts << "M " + QString::number(pathPoint.x()) + " " + QString::number(pathPoint.y()) + " ";
                } else {
                    parts << QString::number(pathPoint.x()) + " " + QString::number(pathPoint.y()) + " ";
                }
            }
            break;
            case QPainterPath::LineToElement:
            {
                nodesCounter++;
                /*
                qDebug() << "nodesCounter ->" << nodesCounter;
                qDebug() << "index ->" << index;
                */

                if ((nodesCounter == index) || replace) {
                    // Replacing a line node for a curve node
                    if (t != 'C') {
                        t = 'C';
                        parts << " C " + QString::number(pathPoint.x()) + " " + QString::number(pathPoint.y()) + " ";
                    } else {
                        parts << "  " + QString::number(pathPoint.x()) + " " + QString::number(pathPoint.y()) + " ";
                    }
                    parts <<  " " + QString::number(pathPoint.x()) + "  " + QString::number(pathPoint.y()) + " ";
                    parts <<  " " + QString::number(pathPoint.x()) + "  " + QString::number(pathPoint.y()) + " ";

                    if (replace)
                        replace = false;

                    found = true;
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupPathItem::changeNodeTypeFromPath()] - LineToElement - Node was FOUND!";
                    #endif
                } else {
                    if (t != 'L') {
                        t = 'L';
                        parts << " L " + QString::number(pathPoint.x()) + " " + QString::number(pathPoint.y()) + " ";
                    } else {
                        parts << QString::number(pathPoint.x()) + " " + QString::number(pathPoint.y()) + " ";
                    }
                }
            }
            break;
            case QPainterPath::CurveToElement:
            {
                if (t != 'C') {
                    t = 'C';
                    parts << " C " + QString::number(pathPoint.x()) + " " + QString::number(pathPoint.y()) + " ";
                } else {
                    parts << "  " + QString::number(pathPoint.x()) + " " + QString::number(pathPoint.y()) + " ";
                }

                dataCounter = 0;
            }
            break;
            case QPainterPath::CurveToDataElement:
            {
                dataCounter++;
                if (dataCounter == 2)
                    nodesCounter++;

                /*
                qDebug() << "nodesCounter ->" << nodesCounter;
                qDebug() << "index + 1 ->" << (index + 1);
                */

                if ((nodesCounter == index) || replace) {
                    if (dataCounter == 2) {
                        parts.removeLast(); // Removing first CurveToDataElement item
                        parts.removeLast(); // Removing CurveToElement item

                        // Replacing a curve node for a line node
                        t = 'L';
                        parts << " L " + QString::number(pathPoint.x()) + " " + QString::number(pathPoint.y()) + " ";

                        if (replace)
                            replace = false;

                        found = true;
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupPathItem::changeNodeTypeFromPath()] - CurveToDataElement - Node was FOUND!";
                        #endif
                    } else {
                        if (t == 'C')
                            parts <<  " " + QString::number(pathPoint.x()) + "  " + QString::number(pathPoint.y()) + " ";
                    }
                } else {
                    if (t == 'C')
                        parts <<  " " + QString::number(pathPoint.x()) + "  " + QString::number(pathPoint.y()) + " ";
                }
            }

            break;
        }
    }

    if (!found) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPathItem::changeNodeTypeFromPath()] - Warning: Node was NOT found ->" << (index +1);
        #endif
    }

    QString pathStr = "";
    foreach(QString line, parts)
        pathStr += line;

    return pathStr;
}

bool TupPathItem::containsOnPath(QPointF pos, float tolerance)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::containsOnPath()] - pos ->" << pos;
        qDebug() << "[TupPathItem::containsOnPath()] - tolerance ->" << tolerance;
    #endif

    QRectF rect = boundingRect();
    QPointF topLeft = rect.topLeft();
    QPointF bottomRight = rect.bottomRight();

    /*
    qDebug() << "---";
    qDebug() << "3 topLeft ->" << topLeft;
    qDebug() << "3 mapToScene(coords) ->" << mapToScene(topLeft);
    qDebug() << "3 mapToItem(coords) ->" << mapToItem(this, topLeft);
    qDebug() << "3 mapToParent(coords) ->" << mapToParent(topLeft);
    qDebug() << "---";
    */

    // Checking if point is part of the path area (rect)
    bool contained = (topLeft.x() <= pos.x() && topLeft.y() <= pos.y()) && (pos.x() <= bottomRight.x() && pos.y() <= bottomRight.y());
    if (!contained) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPathItem::containsOnPath()] - Warning: pos is out of scope! ->" << pos;
            qDebug() << "[TupPathItem::containsOnPath()] - boundingRect() - top left ->" << boundingRect().topLeft();
            qDebug() << "[TupPathItem::containsOnPath()] - boundingRect() - bottom right ->" << boundingRect().bottomRight();
        #endif

        return false;
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::containsOnPath()] - Point is part of the item rect!";
        qDebug() << "---";
    #endif

    float minimum;
    straightLineFlag = false;
    flatCurveFlag = false;

    QPainterPath route = path();
    QPolygonF dots = route.toFillPolygon();
    QPolygonF points;

    // Checking if point is part of a straight line segment
    if (pointIsPartOfLine(route, pos, tolerance)) {
        // qDebug() << "Point is either part of a flat curve or a line!";
        return true;
    }

    int total = dots.size() - 1;
    for(int i=0; i<total; i++) {
        float distance = TAlgorithm::distance(dots.at(i), dots.at(i+1));
        points << dots.at(i);

        if (distance > tolerance) {
            float x = dots.at(i+1).x() - dots.at(i).x();
            float y = dots.at(i+1).y() - dots.at(i).y();
            float segment = sqrt(pow(x, 2) + pow(y, 2));
            float step = (segment/(tolerance/2)) - 1;

            float stepX = x/step;
            float stepY = y/step;
            for(int j=1; j<step; j++)
                points << (dots.at(i) + QPointF(stepX*j, stepY*j));
        }

        points << dots.at(i+1);
    }

    float found = false;
    total = points.size();
    for(int i=0; i<total-1; i++) {
        float distance = TAlgorithm::distance(pos, points.at(i));
        if (i == 0) {
            minimum = distance;
            // farMin = distance;
        }

        #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::containsOnPath()] - pos ->" << pos;
                qDebug() << "[TupPathItem::containsOnPath()] - points.at(i) ->" << points.at(i);
                qDebug() << "[TupPathItem::containsOnPath()] - distance ->" << distance;
                qDebug() << "---";
        #endif

        // If point is closer than tolerance value, store it
        if (distance <= tolerance) {
            #ifdef TUP_DEBUG
                        qDebug() << "[TupPathItem::containsOnPath()] - Point is part of the path!";
            #endif
            if (distance < minimum) {
                newNode = points.at(i);
                minimum = distance;
                found = true;
            }
        }
    }

    if (found) {
        #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::containsOnPath()] - Point was found!";
                qDebug() << "---";
        #endif

        return true;
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::containsOnPath()] - Warning: Input point is out of the path!";
        qDebug() << "[TupPathItem::containsOnPath()] - pos ->" << pos;
    #endif

    return false;
}

bool TupPathItem::pointIsPartOfLine(const QPainterPath &route, const QPointF &target, int tolerance)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::pointIsPartOfLine()] - point ->" << target;
    #endif

    QList<QPointF> previewPoint;
    QPointF curvePoint;
    QPair<QPointF, QPointF> dataPoints;
    int dataCounter = 0;
    int elementsTotal = route.elementCount();
    for(int i=0; i<elementsTotal; i++) {
        QPainterPath::Element e = route.elementAt(i);
        QPointF pathPoint = QPointF(e.x, e.y);

        switch (e.type) {
            case QPainterPath::MoveToElement:
            {
                previewPoint << pathPoint;
            }
            break;
            case QPainterPath::LineToElement:
            {
                // qDebug() << "LineToElement - Element Pos ->" << pathPoint.x() << ", " << pathPoint.y();

                if (findPointAtLine(previewPoint.last(), pathPoint, target, tolerance)) {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupPathItem::pointIsPartOfLine()] - Point was found in line!";
                    #endif
                    straightLineFlag = true;

                    return true;
                }

                previewPoint << pathPoint;
            }
            break;
            case QPainterPath::CurveToElement:
            {
                dataCounter = 0;
                curvePoint = pathPoint;
            }
            break;
            case QPainterPath::CurveToDataElement:
            {
                dataCounter++;
                if (dataCounter == 1)
                    dataPoints.first = pathPoint;
                if (dataCounter == 2) {
                    dataPoints.second = pathPoint;
                    previewPoint << pathPoint;
                }

                /*
                qDebug() << "CurveToDataElement - curvePoint ->" << curvePoint;
                qDebug() << "CurveToDataElement - dataPoints.first ->" << dataPoints.first;
                qDebug() << "CurveToDataElement - dataPoints.second ->" << dataPoints.second;
                qDebug() << "CurveToDataElement - pathPoint  ->" << pathPoint;
                qDebug() << "---";
                */

                // if ((dataCounter == 2) && (dataPoints.first == dataPoints.second)) {
                if ((dataCounter == 2) && (curvePoint == dataPoints.first && curvePoint == dataPoints.second)) {
                    // qDebug() << "[TupPathItem::pointIsPartOfLine()] - Processing flat curve!";
                    int index = previewPoint.size() - 2;
                    if (findPointAtLine(previewPoint.at(index), pathPoint, target, tolerance)) {
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupPathItem::pointIsPartOfLine()] - Point was found in flat curve!";
                        #endif
                        flatCurveFlag = true;

                        return true;
                    }
                }

            }
            break;
            default:
               //  qDebug() << "[TupPathItem::pointIsPartOfLine()] - e.type ->" << e.type;
            break;
        }
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::pointIsPartOfLine()] - Undetermined!";
    #endif

    return false;
}

bool TupPathItem::findPointAtLine(const QPointF &point1, const QPointF &point2, const QPointF &target, int tolerance)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::findPointAtLine()] - point1 ->" << point1;
        qDebug() << "[TupPathItem::findPointAtLine()] - point2 ->" << point2;
        qDebug() << "[TupPathItem::findPointAtLine()] - target ->" << target;
    #endif

    QPointF bestPoint;
    float distance = TAlgorithm::distance(point1, point2);
    if (distance > tolerance) {
        float min = distance;
        int times = distance / tolerance;
        float deltaX = (point2.x() - point1.x()) / times;
        float deltaY = (point2.y() - point1.y()) / times;
        for (int k=1; k<=times; k++) {
            QPointF point(point1.x() + (deltaX * k), point1.y() + (deltaY * k));
            distance = TAlgorithm::distance(point, target);
            if (distance < min) {
                min = distance;
                bestPoint = point;
            }
        }
        if (min < tolerance) {
            newNode = bestPoint;
            // qDebug() << "*** 1 Good! Point was found!";

            return true;
        }
    } else { // Segment is too short
        float deltaX = (point2.x() - point1.x()) / 2;
        float deltaY = (point2.y() - point1.y()) / 2;
        QPointF point(point1.x() + deltaX, point1.y() + deltaY);
        distance = TAlgorithm::distance(point, target);
        if (distance < tolerance) {
            newNode = bestPoint;
            // qDebug() << "*** 2 Good! Point was found!";

            return true;
        }
    }

    // qDebug() << "[TupPathItem::findPointAtLine()] - Warning: Point was NOT found!";

    return false;
}

QString TupPathItem::addInnerNode(int tolerance, NodeType nodeType)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::addInnerNode()] - newNode ->" << newNode;
        qDebug() << "[TupPathItem::addInnerNode()] - tolerance ->" << tolerance;
        qDebug() << "[TupPathItem::addInnerNode()] - node type ->" << nodeType;
        qDebug() << "---";
    #endif

    QPainterPath route = path();
    int elementsTotal = route.elementCount();
    QPointF previewPoint;

    int dataCounter = 0;
    int nodesCounter = 0;
    QPointF initPoint;
    QPointF endPoint;
    bool ignoreData = false;
    bool isEarlyNode = false;
    bool found = false;
    int foundInLine = false;
    int lineIndex = 0;

    QStringList parts;
    QString pathStr = "";

    // Getting the key points for the new point (initPoint - endPoint)
    for(int i=0; i<elementsTotal; i++) {
        QPainterPath::Element e = route.elementAt(i);
        QPointF pathPoint = QPointF(e.x, e.y);
        switch (e.type) {
            case QPainterPath::MoveToElement:
            {
                nodesCounter++;
                previewPoint = pathPoint;
            }
            break;
            case QPainterPath::LineToElement:
            {
                // qDebug() << "LineToElement";
                nodesCounter++;
                lineIndex = nodesCounter;

                // Check if new node is contained in this straight segment
                if (pointIsContainedBetweenFlatNodes(previewPoint, pathPoint, newNode, tolerance)) {
                    // qDebug() << "[TupPathItem::addInnerNode()] - LineToElement - Point was found between nodes!";
                    found = true;
                    foundInLine = true;
                    initPoint = previewPoint;
                    endPoint = pathPoint;
                    goto next;
                }

                previewPoint = pathPoint;
            }
            break;
            case QPainterPath::CurveToElement:
            {
                // qDebug() << "CurveToElement";
                dataCounter = 0;
            }
            break;
            case QPainterPath::CurveToDataElement:
            {
                // qDebug() << "CurveToDataElement";

                if (dataCounter == 1) { // This is the node of the curve
                    nodesCounter++;

                    if (!straightLineFlag) {
                        if (flatCurveFlag) { // Flat curve segment
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupPathItem::addInnerNode()] - CurveToDataElement - Evaluating FLAT nodes segment";
                            #endif
                            if (pointIsContainedBetweenFlatNodes(previewPoint, pathPoint, newNode, tolerance)) {
                                found = true;
                                initPoint = previewPoint;
                                endPoint = pathPoint;
                                if ((nodesCounter - 1) == 1)
                                    isEarlyNode = true;
                                goto next;
                            }
                        } else { // Curve Segment
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupPathItem::addInnerNode()] - CurveToDataElement - Evaluating CURVE nodes segment";
                            #endif
                            if (pointIsContainedBetweenNodes(previewPoint, pathPoint, newNode, tolerance)) {
                                found = true;
                                initPoint = previewPoint;
                                endPoint = pathPoint;
                                if ((nodesCounter - 1) == 1)
                                    isEarlyNode = true;
                                goto next;
                            }
                        }
                    }

                    previewPoint = pathPoint;
                }

                dataCounter++;
            }
            break;
            default:
            break;
        }
    }

    next:

    if (!found) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPathItem::addInnerNode()] - Node is out of the path. Appending... ->" << newNode;
        #endif

        QString pathStr = appendNode(newNode);
        setPathFromString(pathStr);

        return pathStr;
    }

    // qDebug() << "";
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::addInnerNode()] - Segment was found. Time to generate the path string and insert the node...";
        qDebug() << "[TupPathItem::addInnerNode()] - initPoint ->" << initPoint;
        qDebug() << "[TupPathItem::addInnerNode()] - endPoint ->" << endPoint;
    #endif

    QChar t;
    nodesCounter = 0;
    bool nodeAdded = false;

    // Time to generate the path
    for(int i=0; i<elementsTotal; i++) {
        QPainterPath::Element e = route.elementAt(i);
        QPointF pathPoint = QPointF(e.x, e.y);
        switch (e.type) {
            case QPainterPath::MoveToElement:
            {
                // qDebug() << "2 MoveToElement - pathPoint ->" << pathPoint;
                nodesCounter++;

                if (t != 'M') {
                    t = 'M';
                    parts << "M " + QString::number(pathPoint.x()) + " " + QString::number(pathPoint.y()) + " ";
                } else {
                    parts << QString::number(pathPoint.x()) + " " + QString::number(pathPoint.y()) + " ";
                }

                if (isEarlyNode) {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupPathItem::addInnerNode()] - Adding node at the beginning of the path...";
                    #endif

                    if (flatCurveFlag) {
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupPathItem::addInnerNode()] - Adding node at FLAT CURVE segment";
                        #endif

                        if (nodeType == CurveNode) {
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupPathItem::addInnerNode()] - Adding CURVE node";
                            #endif

                            if (t != 'C') {
                                t = 'C';
                                parts << " C " + QString::number(newNode.x()) + " " + QString::number(newNode.y()) + " ";
                            } else {
                                parts << "  " + QString::number(newNode.x()) + " " + QString::number(newNode.y()) + " ";
                            }

                            parts << " " + QString::number(newNode.x()) + "  " + QString::number(newNode.y()) + " ";
                            // Adding NewNode element (CurveToDataElement)
                            parts << " " + QString::number(newNode.x()) + "  " + QString::number(newNode.y()) + " ";
                        } else { // Adding LineNode
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupPathItem::addInnerNode()] - Adding LINE node";
                            #endif

                            t = 'L';
                            parts << " L " + QString::number(newNode.x()) + " " + QString::number(newNode.y()) + " ";
                        }
                    } else { // Curve Segment
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupPathItem::addInnerNode()] - Adding node at CURVE segment";
                        #endif

                        ignoreData = true;

                        QPair<QPointF, QPointF> elements;
                        QPointF curveToElement;
                        QPointF dataToElement;
                        if (nodeType == CurveNode) {
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupPathItem::addInnerNode()] - Adding CURVE node";
                                qDebug() << "[TupPathItem::addInnerNode()] - Getting C1/C2 for newNode";
                            #endif

                            // Calculating new segment and insert it here
                            elements = getCurveElements(initPoint, newNode);
                            // Adding CurveToElement
                            curveToElement = elements.first;
                            if (t != 'C') {
                                t = 'C';
                                parts << " C " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
                            } else {
                                parts << "  " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
                            }

                            // Adding CurveToDataElement
                            dataToElement = elements.second;
                            parts << " " + QString::number(dataToElement.x()) + "  " + QString::number(dataToElement.y()) + " ";

                            // Adding NewNode element (CurveToDataElement)
                            parts << " " + QString::number(newNode.x()) + "  " + QString::number(newNode.y()) + " ";
                        } else { // Adding LineNode
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupPathItem::addInnerNode()] - Adding LINE node";
                            #endif

                            t = 'L';
                            parts << " L " + QString::number(newNode.x()) + " " + QString::number(newNode.y()) + " ";
                        }

                        // Calculating curve positions of the next node
                        #ifdef TUP_DEBUG
                            qDebug() << "---";
                            qDebug() << "[TupPathItem::addInnerNode()] - Getting C1/C2 for the node after the newNode";
                        #endif
                        elements = getCurveElements(newNode, endPoint);
                        curveToElement = elements.first;
                        // Adding CurveToElement
                        if (t != 'C') {
                            t = 'C';
                            parts << " C " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
                        } else {
                            parts << "  " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
                        }

                        // Creating CurveToDataElement
                        dataToElement = elements.second;
                        parts << " " + QString::number(dataToElement.x()) + "  " + QString::number(dataToElement.y()) + " ";
                    }
                    nodeAdded = true;
                }
            }
            break;
            case QPainterPath::LineToElement:
            {
                nodesCounter++;

                /*
                qDebug() << "2 LineToElement - foundInLine ->" << foundInLine;
                qDebug() << "2 LineToElement - nodesCounter ->" << nodesCounter;
                qDebug() << "2 LineToElement - lineIndex ->" << lineIndex;
                qDebug() << "2 LineToElement - pathPoint ->" << pathPoint;
                qDebug() << "2 LineToElement - initPoint ->" << initPoint;
                qDebug() << "---";
                */

                if (foundInLine && nodesCounter == lineIndex) {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupPathItem::addInnerNode()] - Adding node at line segment of the path...";
                    #endif

                    if (nodeType == CurveNode) {
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupPathItem::addInnerNode()] - Adding CURVE node";
                        #endif

                        QPointF segment = newNode - initPoint;
                        float stepX = segment.x()/3;
                        float stepY = segment.y()/3;

                        // Calculating new segment and insert it here

                        QPointF curveToElement = initPoint + QPointF(stepX, stepY);
                        QPointF dataToElement = initPoint + QPointF(stepX*2, stepY*2);

                        // Adding CurveToElement
                        if (t != 'C') {
                            t = 'C';
                            parts << " C " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
                        } else {
                            parts << " " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
                        }

                        // Adding CurveToDataElement
                        parts << " " + QString::number(dataToElement.x()) + "  " + QString::number(dataToElement.y()) + " ";

                        // Adding NewNode element (CurveToDataElement)
                        parts << " " + QString::number(newNode.x()) + "  " + QString::number(newNode.y()) + " ";
                    } else { // Adding LineNode
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupPathItem::addInnerNode()] - Adding LINE node";
                        #endif

                        if (t != 'L') {
                            t = 'L';
                            parts << " L " + QString::number(newNode.x()) + " " + QString::number(newNode.y()) + " ";
                        } else {
                            parts << QString::number(newNode.x()) + " " + QString::number(newNode.y()) + " ";
                        }
                    }
                    nodeAdded = true;
                }

                if (t != 'L') {
                    t = 'L';
                    parts << " L " + QString::number(pathPoint.x()) + " " + QString::number(pathPoint.y()) + " ";
                } else {
                    parts << QString::number(pathPoint.x()) + " " + QString::number(pathPoint.y()) + " ";
                }

                if (!foundInLine && pathPoint == initPoint) { // New node was added between LINE point and CURVE node
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupPathItem::addInnerNode()] - Adding CURVE node";
                        qDebug() << "[TupPathItem::addInnerNode()] - Getting C1/C2 for newNode";
                    #endif

                    ignoreData = true;

                    QPair<QPointF, QPointF> elements;
                    QPointF curveToElement;
                    QPointF dataToElement;
                    if (nodeType == CurveNode) { // Adding CURVE node
                        // Calculating new C1/C2 values for the new node
                        elements = getCurveElements(initPoint, newNode);
                        // Adding CurveToElement
                        curveToElement = elements.first;
                        if (t != 'C') {
                            t = 'C';
                            parts << " C " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
                        } else {
                            parts << "  " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
                        }

                        // Adding CurveToDataElement
                        dataToElement = elements.second;
                        parts << " " + QString::number(dataToElement.x()) + "  " + QString::number(dataToElement.y()) + " ";

                        // Adding NewNode element (CurveToDataElement)
                        parts << " " + QString::number(newNode.x()) + "  " + QString::number(newNode.y()) + " ";

                        // ---
                        // *** After Node
                        // Recalculating C1/C2 values for the node after the new one
                        #ifdef TUP_DEBUG
                            qDebug() << "---";
                            qDebug() << "[TupPathItem::addInnerNode()] - Adding CURVE node";
                            qDebug() << "[TupPathItem::addInnerNode()] - Getting C1/C2 for node after the newNode";
                        #endif

                        elements = getCurveElements(newNode, endPoint);
                        curveToElement = elements.first;
                        // Adding CurveToElement
                        if (t != 'C') {
                            t = 'C';
                            parts << " C " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
                        } else {
                            parts << "  " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
                        }

                        // Creating CurveToDataElement
                        dataToElement = elements.second;
                        parts << " " + QString::number(dataToElement.x()) + "  " + QString::number(dataToElement.y()) + " ";
                    } else { // Adding LineNode
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupPathItem::addInnerNode()] - Adding LINE node";
                        #endif

                        if (t != 'L') {
                            t = 'L';
                            parts << " L " + QString::number(newNode.x()) + " " + QString::number(newNode.y()) + " ";
                        } else {
                            parts << QString::number(newNode.x()) + " " + QString::number(newNode.y()) + " ";
                        }

                        // ---
                        // *** After Node
                        // Recalculating C1/C2 values for the node after the new one
                        elements = getCurveElements(newNode, endPoint);
                        curveToElement = elements.first;
                        // Adding CurveToElement
                        if (t != 'C') {
                            t = 'C';
                            parts << " C " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
                        } else {
                            parts << "  " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
                        }

                        // Creating CurveToDataElement
                        dataToElement = elements.second;
                        parts << " " + QString::number(dataToElement.x()) + "  " + QString::number(dataToElement.y()) + " ";

                    }
                    nodeAdded = true;
                }
            }
            break;
            case QPainterPath::CurveToElement:
            {
                /*
                qDebug() << "2 CurveToElement - pathPoint ->" << pathPoint;
                qDebug() << "---";
                */

                dataCounter = 0;

                if (!ignoreData) {
                    if (t != 'C') {
                        t = 'C';
                        parts << " C " + QString::number(pathPoint.x()) + " " + QString::number(pathPoint.y()) + " ";
                    } else {
                        parts << "  " + QString::number(pathPoint.x()) + " " + QString::number(pathPoint.y()) + " ";
                    }
                    previewPoint = pathPoint;
                }
            }
            break;
            case QPainterPath::CurveToDataElement:
            {
                /*
                qDebug() << "2 CurveToDataElement";
                qDebug() << "2 foundInLine   ->" << foundInLine;
                qDebug() << "2 flatCurveFlag ->" << flatCurveFlag;
                qDebug() << "2 initPoint     ->" << initPoint;
                qDebug() << "2 pathPoint     ->" << pathPoint;
                qDebug() << "---";
                */

                if (!foundInLine && !flatCurveFlag) {
                    if (pathPoint == initPoint && dataCounter == 1) { // Insert the new node in this segment
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupPathItem::addInnerNode()] - Adding node at CURVE segment of the path...";
                        #endif

                        ignoreData = true;

                        // *** Previous Node
                        // Adding the second CurveToDataElement component of the previous node
                        if (t == 'C')
                            parts << " " + QString::number(pathPoint.x()) + "  " + QString::number(pathPoint.y()) + " ";

                        QPair<QPointF, QPointF> elements;
                        QPointF curveToElement;
                        QPointF dataToElement;
                        if (nodeType == CurveNode) {
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupPathItem::addInnerNode()] - Adding CURVE node";
                                qDebug() << "[TupPathItem::addInnerNode()] - Getting C1/C2 for newNode";
                            #endif

                            // Calculating new C1/C2 values for the new node
                            elements = getCurveElements(initPoint, newNode);
                            // Adding CurveToElement
                            curveToElement = elements.first;
                            if (t != 'C') {
                                t = 'C';
                                parts << " C " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
                            } else {
                                parts << "  " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
                            }

                            // Adding CurveToDataElement
                            dataToElement = elements.second;
                            parts << " " + QString::number(dataToElement.x()) + "  " + QString::number(dataToElement.y()) + " ";

                            // Adding NewNode element (CurveToDataElement)
                            parts << " " + QString::number(newNode.x()) + "  " + QString::number(newNode.y()) + " ";
                        } else { // Adding LineNode
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupPathItem::addInnerNode()] - Adding LINE node";
                            #endif

                            if (t != 'L') {
                                t = 'L';
                                parts << " L " + QString::number(newNode.x()) + " " + QString::number(newNode.y()) + " ";
                            } else {
                                parts << QString::number(newNode.x()) + " " + QString::number(newNode.y()) + " ";
                            }
                        }
                        nodeAdded = true;

                        // ---
                        // *** After Node
                        // Recalculating C1/C2 values for the node after the new one
                        #ifdef TUP_DEBUG
                            qDebug() << "---";
                            qDebug() << "[TupPathItem::addInnerNode()] - Adding CURVE node";
                            qDebug() << "[TupPathItem::addInnerNode()] - Getting C1/C2 for node after the newNode";
                        #endif

                        elements = getCurveElements(newNode, endPoint);
                        curveToElement = elements.first;
                        // Adding CurveToElement
                        if (t != 'C') {
                            t = 'C';
                            parts << " C " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
                        } else {
                            parts << "  " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
                        }

                        // Creating CurveToDataElement
                        dataToElement = elements.second;
                        parts << " " + QString::number(dataToElement.x()) + "  " + QString::number(dataToElement.y()) + " ";
                    } else if (pathPoint == endPoint) { // Adding the node after the new node
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupPathItem::addInnerNode()] - Adding node after the new node";
                        #endif

                        if (t == 'C')
                            parts << " " + QString::number(e.x) + "  " + QString::number(e.y) + " ";

                        ignoreData = false;
                    } else {
                        if (!ignoreData) { // Saving data from other segments
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupPathItem::addInnerNode()] - Saving data from other segments";
                            #endif

                            if (t == 'C')
                                parts << " " + QString::number(e.x) + "  " + QString::number(e.y) + " ";
                            if (dataCounter == 1)
                                nodesCounter++;
                        }
                    }
                } else { // It's flat curve or it's part of other segment
                    if (flatCurveFlag && pathPoint == initPoint && dataCounter == 1) { // Flat curve
                        parts << "  " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupPathItem::addInnerNode()] - Adding node at FLAT CURVE segment of the path...";
                        #endif

                        if (t != 'C') {
                            t = 'C';
                            parts << " C " + QString::number(newNode.x()) + " " + QString::number(newNode.y()) + " ";
                        } else {
                            parts << "  " + QString::number(newNode.x()) + " " + QString::number(newNode.y()) + " ";
                        }

                        parts << " " + QString::number(newNode.x()) + "  " + QString::number(newNode.y()) + " ";
                        // Adding NewNode element (CurveToDataElement)
                        parts << " " + QString::number(newNode.x()) + "  " + QString::number(newNode.y()) + " ";

                        nodeAdded = true;
                    } else { // Saving data from other segments
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupPathItem::addInnerNode()] - Saving data from other segments";
                        #endif

                        if (t == 'C')
                            parts << " " + QString::number(e.x) + "  " + QString::number(e.y) + " ";
                        if (dataCounter == 1)
                            nodesCounter++;
                    }
                }

                dataCounter++;
            }
            break;
        }
    }

    if (!nodeAdded) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPathItem::addInnerNode()] - Warning: Node wasn't added at ->" << newNode;
        #endif
    }

    foreach(QString line, parts)
        pathStr += line;

    // qDebug() << "PATH STR ->" << pathStr;

    setPathFromString(pathStr);

    return pathStr;
}

// This method calculates C1/C2 for a curve

QPair<QPointF, QPointF> TupPathItem::getCurveElements(QPointF initPos, QPointF endPos)
{
    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupPathItem::getCurveElements()] - initPos ->" << initPos;
        qDebug() << "[TupPathItem::getCurveElements()] - endPos ->" << endPos;
    #endif

    QPainterPath route = path();
    QPolygonF points = route.toFillPolygon();

    QPointF firstPoint;
    QPointF secondPoint;

    bool startSaving = false;
    QList<QPointF> range;
    int total = points.size();
    for(int i=0; i<total-1; i++) {
        if (pointIsContainedBetweenRange(points.at(i), points.at(i+1), initPos, 0)) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::getCurveElements()] - initPos found ->" << initPos;
            #endif
            startSaving = true;
        } else {
            if (pointIsContainedBetweenRange(points.at(i), points.at(i+1), endPos, 0)) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupPathItem::getCurveElements()] - endPos found ->" << endPos;
                #endif
                startSaving = false;
            } else {
                if (startSaving)
                    range << points.at(i);
            }
        }
    }

    if (!range.isEmpty()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPathItem::getCurveElements()] - range.size() ->" << range.size();
        #endif

        int size = range.size();
        if (size == 1) {
            firstPoint = range.at(0);
            secondPoint = range.at(0);
        } else if (size == 2) {
            firstPoint = range.at(0);
            secondPoint = range.at(1);
        } else {
            int step = size / 3;
            firstPoint = range.at(step);
            secondPoint = range.at(step*2);
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPathItem::getCurveElements()] - Warning: range.size() -> 0";
        #endif
        // Making up the missing points
        QPointF segment = endPos - initPos;
        float stepX = segment.x()/3;
        float stepY = segment.y()/3;

        firstPoint = initPos + QPointF(stepX, stepY);
        secondPoint = initPos + QPointF(stepX*2, stepY*2);
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::getCurveElements()] - range ->" << range;
        qDebug() << "[TupPathItem::getCurveElements()] - firstPoint ->" << firstPoint;
        qDebug() << "[TupPathItem::getCurveElements()] - secondPoint ->" << secondPoint;
    #endif

    return QPair<QPointF, QPointF>(firstPoint, secondPoint);
}

// Method for short distances

bool TupPathItem::pointIsContainedBetweenRange(const QPointF &point1, const QPointF &point2, const QPointF &newPoint, float tolerance)
{
    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupPathItem::pointIsContainedBetweenRange()] - point1 ->" << point1;
        qDebug() << "[TupPathItem::pointIsContainedBetweenRange()] - newPoint ->" << newPoint;
        qDebug() << "[TupPathItem::pointIsContainedBetweenRange()] - point2 ->" << point2;
    #endif

    if (point1.x() <= newPoint.x() && newPoint.x() < point2.x()) {
        if (point1.y() <= newPoint.y() && newPoint.y() < point2.y()) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::pointIsContainedBetweenRange()] - MATCH!";
            #endif

            return true;
        }

        if (point2.y() < newPoint.y() && newPoint.y() <= point1.y()) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::pointIsContainedBetweenRange()] - MATCH!";
            #endif

            return true;
        }

        // Measuring distance between newPoint and path
        qreal distance = TAlgorithm::distanceFromLine(point1, point2, newPoint);
        if (distance <= tolerance) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::pointIsContainedBetweenRange()] - MATCH!";
            #endif

            return true;
        }
    }

    if (point2.x() < newPoint.x() && newPoint.x() <= point1.x()) {
        if (point1.y() <= newPoint.y() && newPoint.y() < point2.y()) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::pointIsContainedBetweenRange()] - MATCH!";
            #endif

            return true;
        }

        if (point2.y() < newPoint.y() && newPoint.y() <= point1.y()) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::pointIsContainedBetweenRange()] - MATCH!";
            #endif

            return true;
        }

        // Measuring distance between newPoint and path
        qreal distance = TAlgorithm::distanceFromLine(point1, point2, newPoint);
        if (distance <= tolerance) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::pointIsContainedBetweenRange()] - MATCH!";
            #endif

            return true;
        }
    }

    // Line is vertical
    if (point1.y() <= newPoint.y() && newPoint.y() < point2.y()) {
        if (point1.x() <= newPoint.x() && newPoint.x() < point2.x()) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::pointIsContainedBetweenRange()] - MATCH!";
            #endif

            return true;
        }

        if (point2.x() < newPoint.x() && newPoint.x() <= point1.x()) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::pointIsContainedBetweenRange()] - MATCH!";
            #endif

            return true;
        }

        // Measuring distance between newPoint and path
        qreal distance = TAlgorithm::distanceFromLine(point1, point2, newPoint);
        if (distance <= tolerance) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::pointIsContainedBetweenRange()] - MATCH!";
            #endif

            return true;
        }
    }

    if (point2.y() < newPoint.y() && newPoint.y() <= point1.y()) {
        if (point1.x() <= newPoint.x() && newPoint.x() < point2.x()) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::pointIsContainedBetweenRange()] - MATCH!";
            #endif

            return true;
        }

        if (point2.x() < newPoint.x() && newPoint.x() <= point1.x()) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::pointIsContainedBetweenRange()] - MATCH!";
            #endif

            return true;
        }

        // Measuring distance between newPoint and path
        qreal distance = TAlgorithm::distanceFromLine(point1, point2, newPoint);
        if (distance <= tolerance) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::pointIsContainedBetweenRange()] - MATCH!";
            #endif

            return true;
        }
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::pointIsContainedBetweenRange()] - Warning: Point is out of scope!";
    #endif

    return false;
}

// This method verify if point is between nodes of a curve

bool TupPathItem::pointIsContainedBetweenNodes(const QPointF &node1, const QPointF &node2,
                                               const QPointF &point, float tolerance)
{
    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupPathItem::pointIsContainedBetweenNodes()] - node1 ->" << node1;
        qDebug() << "[TupPathItem::pointIsContainedBetweenNodes()] - node2 ->" << node2;
        qDebug() << "[TupPathItem::pointIsContainedBetweenNodes()] - point ->" << point;
    #endif

    QPainterPath route = path();
    QPolygonF points = route.toFillPolygon();

    float distance;
    bool startSaving = false;
    QList<QPointF> range;
    int total = points.size();
    // qDebug() << "total-1 ->" << (total - 1);
    for(int i=0; i<total-1; i++) {
        // qDebug() << "i ->" << i;
        if (node1 == points.at(i)) {
            // qDebug() << "startSaving => true";
            startSaving = true;
        } else {
            if (node2 == points.at(i)) {
                // qDebug() << "startSaving => false";
                startSaving = false;
            } else {
                if (startSaving) {
                    if (i>0) {
                        float innerDistance = TAlgorithm::distance(points.at(i-1), points.at(i));
                        if (innerDistance > tolerance) {
                            /*
                            qDebug() << "innerDistance > tolerance!";
                            qDebug() << "innerDistance ->" << innerDistance;
                            qDebug() << "toleraance ->" << tolerance;
                            */
                            int times = innerDistance / tolerance;
                            float deltaX = (points.at(i).x() - points.at(i-1).x()) / times;
                            float deltaY = (points.at(i).y() - points.at(i-1).y()) / times;
                            for (int k=1; k<=times; k++) {
                                QPointF middlePoint(points.at(i-1).x() + (deltaX * k), points.at(i-1).y() + (deltaY * k));
                                range << middlePoint;
                                // qDebug() << "1 Storing point -> " << middlePoint;
                            }
                        }
                    }
                    // qDebug() << "2 Storing point -> " << points.at(i);
                    range << points.at(i);
                }
            }
        }
    }

    total = range.size();
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::pointIsContainedBetweenNodes()] - range.size() ->" << total;
    #endif
    if (total == 0) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPathItem::pointIsContainedBetweenNodes()] - Warning: range of points between nodes is empty!";
        #endif

        return false;
    }

    for(int i=0; i<total; i++) {
        distance = TAlgorithm::distance(range.at(i), point);
        if (distance <= tolerance) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::pointIsContainedBetweenNodes()] - Point was FOUND! ->" << point;
            #endif

            return true;
        }
    }

    return false;
}

bool TupPathItem::pointIsContainedBetweenFlatNodes(const QPointF &node1, const QPointF &node2,
                                                   const QPointF &point, int tolerance)
{
    if (node1.x() <= point.x() && point.x() <= node2.x()) {
        if (node1.y() <= point.y() && point.y() <= node2.y()) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::pointIsContainedBetweenFlatNodes()] - Point is contained!";
            #endif

            // Measuring distance between point and line (node1-node2)
            qreal distance = TAlgorithm::distanceFromLine(node1, node2, point);
            if (distance <= tolerance)
                return true;
        }

        if (node2.y() <= point.y() && point.y() <= node1.y()) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::pointIsContainedBetweenFlatNodes()] - Point is contained!";
            #endif

            // Measuring distance between point and line (node1-node2)
            qreal distance = TAlgorithm::distanceFromLine(node1, node2, point);
            if (distance <= tolerance)
                return true;
        }
    }

    if (node2.x() <= point.x() && point.x() <= node1.x()) {
        if (node1.y() <= point.y() && point.y() <= node2.y()) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::pointIsContainedBetweenFlatNodes()] - Point is contained!";
            #endif

            // Measuring distance between point and line (node1-node2)
            qreal distance = TAlgorithm::distanceFromLine(node1, node2, point);
            if (distance <= tolerance)
                return true;
        }

        if (node2.y() <= point.y() && point.y() <= node1.y()) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::pointIsContainedBetweenFlatNodes()] - Point is contained!";
            #endif

            // Measuring distance between point and line (node1-node2)
            qreal distance = TAlgorithm::distanceFromLine(node1, node2, point);
            if (distance <= tolerance)
                return true;
        }
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::pointIsContainedBetweenFlatNodes()] - Warning: Point is NOT contained!";
    #endif

    return false;
}

// This method is for debugging tasks
QList<QPointF> TupPathItem::keyNodes()
{
    QPainterPath route = path();
    int total = route.elementCount();
    QList<QPointF> points;

    for(int i=0; i < total; i++) {
        QPainterPath::Element e = route.elementAt(i);
        switch (e.type) {
            case QPainterPath::MoveToElement:
            {
                points << QPointF(e.x, e.y);
                colors << QColor(255, 0, 0);
                tips << "MoveToElement - Red";
            }
            break;
            case QPainterPath::LineToElement:
            {
                points << QPointF(e.x, e.y);
                colors << QColor(0, 0, 0);
                tips << "LineToElement - Black";
            }
            break;
            case QPainterPath::CurveToElement:
            {
                points << QPointF(e.x, e.y);
                colors << QColor(0, 255, 0);
                tips << "CurveToElement - Green";
            }
            break;
            case QPainterPath::CurveToDataElement:
            {
                points << QPointF(e.x, e.y);
                colors << QColor(144, 194, 231);
                tips << "CurveToDataElement - Blue";
            }
            break;
        }
    }

    return points;
}

// This method is for debugging tasks
QList<QColor> TupPathItem::nodeColors()
{
    return colors;
}

// This method is for debugging tasks
QList<QString> TupPathItem::nodeTips()
{
    return tips;
}

QString TupPathItem::appendNode(const QPointF &pos)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::appendNode()] - pos ->" << pos;
    #endif

    QPainterPath qPath = path();
    QPointF lastPoint = qPath.pointAtPercent(1);
    QString pathStr = pathToString();

    float stepX = (pos.x() - lastPoint.x()) / 3;
    float stepY = (pos.y() - lastPoint.y()) / 3;

    QPointF curveToElement = QPointF(lastPoint.x() + stepX, lastPoint.y() + stepY);
    QPointF c1 = QPointF(lastPoint.x() + (stepX*2), lastPoint.y() + (stepY*2));

    pathStr += " C " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
    pathStr += "  " + QString::number(c1.x()) + " " + QString::number(c1.y()) + " ";
    pathStr += "  " + QString::number(pos.x()) + " " + QString::number(pos.y()) + " ";

    setPathFromString(pathStr);

    return pathStr;
}

QPair<QPointF,QPointF> TupPathItem::calculateEndPathCPoints(const QPointF &pos)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::calculateEndPathCPoints()] - pos ->" << pos;
    #endif

    QPainterPath qPath = path();
    QPointF lastPoint = qPath.pointAtPercent(1);

    return calculateCPoints(lastPoint, pos);
}

QPair<QPointF,QPointF> TupPathItem::calculateCPoints(const QPointF &pos1, const QPointF &pos2)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::calculateCPoints()] - pos1 ->" << pos1;
        qDebug() << "[TupPathItem::calculateCPoints()] - pos2 ->" << pos2;
    #endif

    QPointF c1;
    QPointF c2;

    float distance = TAlgorithm::distance(pos1, pos2);
    float delta = distance * 0.2;

    float x = pos1.x() + ((pos2.x() - pos1.x()) / 2);
    float y = pos1.y() + ((pos2.y() - pos1.y()) / 2);

    QPointF middlePoint = QPointF(x, y);
    float slope = TAlgorithm::inverseSlope(pos1, pos2);

    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::calculateCPoints()] - delta ->" << delta;
        qDebug() << "[TupPathItem::calculateCPoints()] - slope ->" << slope;
    #endif

    if (fabs(slope) < 10) {
        float b = TAlgorithm::calculateBFromLine(middlePoint, slope);

        float x1 = middlePoint.x() - delta;
        float x2 = middlePoint.x() + delta;

        float y1 = TAlgorithm::calculateYFromLine(x1, slope, b);
        float y2 = TAlgorithm::calculateYFromLine(x2, slope, b);

        c1 = QPointF(x1, y1);
        c2 = QPointF(x2, y2);
    } else {
        float deltaX = (pos2.x() - pos1.x())/3;
        float y1 = pos1.y() - delta;
        float y2 = pos1.y() + delta;

        c1 = QPointF(pos1.x() + deltaX, y1);
        c2 = QPointF(pos1.x() + (deltaX*2), y2);
    }

    return QPair<QPointF, QPointF>(c1, c2);
}

QPair<QPointF,QPointF> TupPathItem::calculatePlainCPoints(const QPointF &pos1, const QPointF &pos2)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::calculatePlainCPoints()] - pos1 ->" << pos1;
        qDebug() << "[TupPathItem::calculatePlainCPoints()] - pos2 ->" << pos2;
    #endif

    float xDelta = (pos2.x() - pos1.x()) / 3;
    float yDelta = (pos2.x() - pos1.x()) / 3;

    float x1 = pos1.x() + xDelta;
    float y1 = pos1.y() + yDelta;

    float x2 = pos1.x() + (xDelta * 2);
    float y2 = pos1.y() + (yDelta * 2);

    QPointF c1(x1, y1);
    QPointF c2(x2, y2);

    return QPair<QPointF, QPointF>(c1, c2);
}

QPainterPath TupPathItem::clearPath(int tolerance)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::clearPath()] - tolerance ->" << tolerance;
    #endif

    QPainterPath route = path();
    int elementsTotal = route.elementCount();
    int step = elementsTotal/50;

    QPointF previewPoint;
    QPointF currentPoint;
    QPointF nextPoint;

    /*
    qDebug() << "PATH step ->" << step;
    qDebug() << "elementsTotal ->" << elementsTotal;
    */

    int curveCounter = 0;
    bool enableData = false;

    // First cleaning process
    QPainterPath::Element e = route.elementAt(0);
    QList<QPainterPath::Element> elements;
    elements << e;

    for(int i=2; i<elementsTotal-1; i++) {
        QPainterPath::Element e = route.elementAt(i);
        currentPoint = QPointF(e.x, e.y);

        switch (e.type) {
            case QPainterPath::MoveToElement:
            {
                // qDebug() << "QPainterPath::MoveToElement ->" << i;
            }
            break;
            case QPainterPath::LineToElement:
            {
                if (i % step == 0)
                    elements << e;
            }
            break;
            case QPainterPath::CurveToElement:
            {
                if (i % step == 0) {
                    elements << e;
                    enableData = true;
                    curveCounter = 0;
                }
            }
            break;
            case QPainterPath::CurveToDataElement:
            {
                if (enableData) {
                    elements << e;
                    curveCounter++;
                    if (curveCounter == 2)
                        enableData = false;
                }
            }
            break;
        }
    }

    elements << route.elementAt(route.elementCount()-1);

    // Second cleaning process

    QChar t;
    e = elements.at(0);
    QPainterPath qPath(QPointF(e.x, e.y));
    previewPoint = QPointF(e.x, e.y);

    float slope;
    float newSlope;
    float diff;

    elementsTotal = elements.size();
    for(int i=1; i<elementsTotal-1; i++) {
        e = elements.at(i);
        currentPoint = QPointF(e.x, e.y);
        e = elements.at(i+1);
        nextPoint = QPointF(e.x, e.y);

        switch (e.type) {
            case QPainterPath::MoveToElement:
            {
                // qDebug() << "QPainterPath::MoveToElement ->" << i;
            }
            break;
            case QPainterPath::LineToElement:
            {
                slope = TAlgorithm::slope(previewPoint, currentPoint);
                newSlope = TAlgorithm::slope(currentPoint, nextPoint);
                diff = fabs(newSlope - slope);

                /*
                qDebug() << "";
                qDebug() << "- QPainterPath::LineToElement ->" << i;
                qDebug() << "slope ->" << slope;
                qDebug() << "newSlope ->" << newSlope;
                qDebug() << "diff ->" << diff;
                */

                if (diff > 0.2) {
                    QPair<QPointF,QPointF> cPoints = calculateCPoints(previewPoint, currentPoint);
                    curvePoints << cPoints;

                    // QPair<QPointF,QPointF> cPoints = calculatePlainCPoints(previewPoint, currentPoint);
                    // qPath.cubicTo(cPoints.first, cPoints.second, currentPoint);
                    qPath.cubicTo(currentPoint, currentPoint, currentPoint);
                }
            }
            break;
            case QPainterPath::CurveToElement:
            {
                // qDebug() << "QPainterPath::CurveToElement ->" << i;
            }
            break;
            case QPainterPath::CurveToDataElement:
            {
                // qDebug() << "QPainterPath::CurveToDataElement -> " << i;
            }
            break;
        }

        previewPoint = currentPoint;
    }

    // qDebug() << "NEW ELEMENTS COUNT ->" << qPath.elementCount();

    return qPath;
}

QPainterPath TupPathItem::brushPath(const QPainterPath &route, int tolerance)
{
    QPolygonF points = route.toFillPolygon();
    points.removeLast();
    // qDebug() << "POINTS:";
    // qDebug() << points;
    int total = points.size();

    QPainterPath qPath(points.at(0));
    qPath.lineTo(points.at(1));
    float slope = TAlgorithm::slope(points.at(0), points.at(1));

    for(int i=2; i<total-1; i++) {
        float newSlope = TAlgorithm::slope(points.at(i-1), points.at(i));
        float distance = TAlgorithm::distance(points.at(i-1), points.at(i));
        float diff = fabs(newSlope - slope);
        /*
        qDebug() << "previous slope ->" << slope;
        qDebug() << "newSlope ->" << newSlope;
        qDebug() << "diff ->" << diff;
        */

        if ((diff > 0.2) && (distance > (tolerance*2))) {
            /*
            qDebug() << "";
            qDebug() << "*** Adding point to the line ->" << points.at(i);
            qDebug() << "";
            */
            qPath.lineTo(points.at(i));
            slope = newSlope;
        }
    }

    qPath.lineTo(points.last());
    // qDebug() << "NEW ELEMENTS COUNT ->" << qPath.elementCount();

    return qPath;
}

int TupPathItem::nodesCount()
{
    QPainterPath route = path();
    int elementsTotal = route.elementCount();
    int counter = 0;

    for(int i=0; i<elementsTotal; i++) {
        QPainterPath::Element e = route.elementAt(i);
        switch (e.type) {
            case QPainterPath::MoveToElement:
            {
                counter++;
            }
            break;
            case QPainterPath::LineToElement:
            {
                counter++;
            }
            break;
            case QPainterPath::CurveToElement:
            {
                counter++;
            }
            break;
            case QPainterPath::CurveToDataElement:
            {
            }
            break;
        }
    }

    return counter;
}

QList<QPair<QPointF,QPointF>> TupPathItem::getCPoints()
{
    return curvePoints;
}
