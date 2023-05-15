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

QString TupPathItem::refactoringPath(NodePosition policy, int nodesTotal)
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

    // Remove the middle node
    int mark = curveIndexes.at(0); // FirstNode
    if (curvesCounter > 1) {
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
                if (curvesCounter != mark) {
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
                if (policy == FirstNode && replace) {
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
                        lookingData = true;
                    }
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

QString TupPathItem::removeNodeFromPath(QPointF pos)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::removeNodeFromPath()] - pos ->" << pos;
    #endif

    QChar t;
    QPainterPath route = path();
    int elementsTotal = route.elementCount();
    QString pathStr = "";

    bool lookingData = false;
    int dataCounter = 0;

    QStringList parts;
    bool replace = false;
    int curvesCounter = 0;
    for(int i=0; i<elementsTotal; i++) {
        QPainterPath::Element e = route.elementAt(i);

        switch (e.type) {
            case QPainterPath::MoveToElement:
            {
                if (pos != QPointF(e.x, e.y)) {
                    if (t != 'M') {
                        t = 'M';
                        parts << "M " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                    } else {
                        parts << QString::number(e.x) + " " + QString::number(e.y) + " ";
                    }
                } else {
                    replace = true;
                }
            }
            break;
            case QPainterPath::LineToElement:
            {
                if (pos != QPointF(e.x, e.y)) {
                    if (t != 'L') {
                        t = 'L';
                        parts << " L " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                    } else {
                        parts << QString::number(e.x) + " " + QString::number(e.y) + " ";
                    }
                }
            }
            break;
            case QPainterPath::CurveToElement:
            {
                if (replace) {
                    parts << "M " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                    replace = false;
                    lookingData = true;
                } else {
                    if (pos != QPointF(e.x, e.y)) {
                        if (t != 'C') {
                            t = 'C';
                            parts << " C " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                        } else {
                            parts << "  " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                        }
                        curvesCounter++;
                    } else {
                        lookingData = true;
                    }
                }
            }
            break;
            case QPainterPath::CurveToDataElement:
            {
                if (pos == QPointF(e.x, e.y)) {
                    parts.removeLast();
                    parts.removeLast();

                    parts << "C";
                    lookingData = false;
                } else {
                    if (!lookingData) {
                        if (t == 'C')
                            parts <<  " " + QString::number(e.x) + "  " + QString::number(e.y) + " ";
                    } else {
                        dataCounter++;

                        if (dataCounter == 2) {
                            lookingData = false;
                            dataCounter = 0;
                        }
                    }
                }
            }
            break;
        }
    }

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

    bool contained = (topLeft.x() <= pos.x() && topLeft.y() <= pos.y()) && (pos.x() <= bottomRight.x() && pos.y() <= bottomRight.y());
    if (!contained) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPathItem::containsOnPath()] - Warning: pos is out of scope! ->" << pos;
            qDebug() << "[TupPathItem::containsOnPath()] - boundingRect() - top left ->" << boundingRect().topLeft();
            qDebug() << "[TupPathItem::containsOnPath()] - boundingRect() - bottom right ->" << boundingRect().bottomRight();
        #endif

        return false;
    }

    QPainterPath route = path();
    QPolygonF points = route.toFillPolygon();
    float minimum;
    float farMin;

    QPointF beforePoint;
    QPointF middlePoint;
    QPointF afterPoint;

    int beforeKey = 0;
    int middleKey = 0;
    int afterKey = 0;

    float found = false;
    int total = points.size();
    for(int i=0; i<total-1; i++) {
        float distance = TAlgorithm::distance(pos, points.at(i));
        if (i == 0) {
            minimum = distance;
            farMin = distance;
        }

        #ifdef TUP_DEBUG
            qDebug() << "[TupPathItem::containsOnPath()] - pos ->" << pos;
            qDebug() << "[TupPathItem::containsOnPath()] - points.at(i) ->" << points.at(i);
            qDebug() << "[TupPathItem::containsOnPath()] - distance ->" << distance;
            qDebug() << "---";
        #endif

        if (distance <= tolerance) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::containsOnPath()] - Point is part of the path!";
            #endif
            if (distance < minimum) {
                if (i >= 2) {
                    c1 = points.at(i - 2);
                    c2 = points.at(i - 1);
                } else {
                    c1 = points.at(0);
                    c2 = points.at(0);
                }
                newNode = points.at(i);
                minimum = distance;
                found = true;
            }
        } else { // Looking for the closest points greater than tolerance
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::containsOnPath()] - Looking for the closest points greater than tolerance...";
            #endif
            if (distance < farMin) {
                farMin = distance;

                beforeKey = i - 1;
                middleKey = i;
                afterKey = i + 1;

                beforePoint = points.at(beforeKey);
                middlePoint = points.at(middleKey);
                afterPoint = points.at(afterKey);
            }
        }
    }

    if (found) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPathItem::containsOnPath()] - Point was found!";
        #endif

        return true;
    } else { // Calculating node point and C1/C2
        QList<QPointF> beforeList;
        float beforeSegX = middlePoint.x() - beforePoint.x();
        float beforeSegY = middlePoint.y() - beforePoint.y();
        float beforeSeg = sqrt(pow(beforeSegX, 2) + pow(beforeSegY, 2));
        float step = beforeSeg/(tolerance/2);

        if (step < 1) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::containsOnPath()] - First segment is too short!";
            #endif

            return false;
        }
        float stepX = beforeSegX/step;
        float stepY = beforeSegY/step;
        for(int i=0; i < (step-1); i++) // Storing points from the first segment
            beforeList << (beforePoint + QPointF(stepX*i, stepY*i));

        if (beforeList.size() < 2) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::containsOnPath()] - First segment doesn't contain points!";
            #endif

            return false;
        }

        QPointF bestPoint = newNode;
        float bestDistance = beforeSeg;
        int index = 0;
        foreach(QPointF point, beforeList) {
            float distance = TAlgorithm::distance(pos, point); // Calculating distances
            if (distance <= bestDistance) { // Selecting the minimum distance
                bestPoint = point;
                bestDistance = distance;
                if (index > 1) {
                    c1 = beforeList.at(index - 2);
                    c2 = beforeList.at(index - 1);
                } else {
                    c1 = beforePoint;
                    c2 = beforePoint;
                }
            }
            index++;
        }

        QList<QPointF> afterList;
        float afterSegX = afterPoint.x() - middlePoint.x();
        float afterSegY = afterPoint.y() - middlePoint.y();
        float afterSeg = sqrt(pow(afterSegX, 2) + pow(afterSegY, 2));
        step = afterSeg/(tolerance/2);
        if (step < 1) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::containsOnPath()] - Second segment is too short!";
            #endif

            return false;
        }
        stepX = afterSegX/step;
        stepY = afterSegY/step;
        for(int i=0; i < (step-1); i++) // Storing distances from the second segment
            afterList << (middlePoint + QPointF(stepX*i, stepY*i));

        if (afterList.size() < 2) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::containsOnPath()] - Second segment doesn't contain points!";
            #endif

            return false;
        }

        index = 0;
        foreach(QPointF point, afterList) {
            float distance = TAlgorithm::distance(pos, point); // Calculating distances
            if (distance <= bestDistance) { // Selecting the minimum distance
                bestPoint = point;
                bestDistance = distance;
                if (index > 1) {
                    c1 = afterList.at(index - 2);
                    c2 = afterList.at(index - 1);
                } else {
                    c1 = middlePoint;
                    c2 = middlePoint;
                }
            }
            index++;
        }

        if (bestDistance > tolerance) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPathItem::containsOnPath()] - Best distance is greater than tolerance!";
            #endif

            return false;
        }

        newNode = bestPoint;

        return true;
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::containsOnPath()] - Input point is out of the path!";
    #endif

    return false;
}

QString TupPathItem::addInnerNode(int tolerance)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::addInnerNode()] - newNode ->" << newNode;
        qDebug() << "[TupPathItem::addInnerNode()] - tolerance ->" << tolerance;
    #endif

    QChar t;
    QPainterPath route = path();
    int elementsTotal = route.elementCount();
    QPointF previewPoint;

    QList<QPointF> nodePoints;
    int dataCounter = 0;
    int nodesCounter = 0;
    QPointF initPoint;
    QPointF endPoint;
    bool ignoreData = false;
    bool isEarlyNode = false;
    bool found = false;

    QStringList parts;
    QString pathStr = "";

    // Storing nodes from path
    for(int i=0; i<elementsTotal; i++) {
        QPainterPath::Element e = route.elementAt(i);
        switch (e.type) {
            case QPainterPath::MoveToElement:
            {
                nodePoints << QPointF(e.x, e.y);
                nodesCounter++;

                previewPoint = QPointF(e.x, e.y);
            }
            break;
            case QPainterPath::CurveToElement:
            {
                dataCounter = 0;
            }
            break;
            case QPainterPath::CurveToDataElement:
            {
                if (dataCounter == 1) {
                    nodePoints << QPointF(e.x, e.y);
                    nodesCounter++;

                    if (pointIsContainedBetweenNodes(previewPoint, QPointF(e.x, e.y), newNode, tolerance)) {
                        found = true;
                        initPoint = previewPoint;
                        endPoint = QPointF(e.x, e.y);
                        if ((nodesCounter - 1) == 1)
                            isEarlyNode = true;
                        goto next;
                    }

                    previewPoint = QPointF(e.x, e.y);
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

    nodesCounter = 0;

    for(int i=0; i<elementsTotal; i++) {
        QPainterPath::Element e = route.elementAt(i);
        switch (e.type) {
            case QPainterPath::MoveToElement:
            {
                nodesCounter++;

                if (t != 'M') {
                    t = 'M';
                    parts << "M " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                } else {
                    parts << QString::number(e.x) + " " + QString::number(e.y) + " ";
                }

                if (isEarlyNode) {
                    ignoreData = true;

                    // Adding the previews node before the new one
                    if (t == 'C')
                        parts << " " + QString::number(e.x) + "  " + QString::number(e.y) + " ";

                    // Calculating new segment and insert it here
                    QPair<QPointF, QPointF> elements = getCurveElements(initPoint, newNode);
                    // Adding CurveToElement
                    QPointF curveToElement = elements.first;
                    if (t != 'C') {
                        t = 'C';
                        parts << " C " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
                    } else {
                        parts << "  " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
                    }

                    // Adding CurveToDataElement
                    QPointF dataToElement = elements.second;
                    parts << " " + QString::number(dataToElement.x()) + "  " + QString::number(dataToElement.y()) + " ";

                    // Adding NewNode element (CurveToDataElement)
                    parts << " " + QString::number(newNode.x()) + "  " + QString::number(newNode.y()) + " ";

                    // Calculating curve positions of the new node
                    elements = getCurveElements(newNode, endPoint);
                    curveToElement = elements.first;
                    // Adding CurveToElement
                    parts << "  " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";

                    // Creating CurveToDataElement
                    dataToElement = elements.second;
                    parts << " " + QString::number(dataToElement.x()) + "  " + QString::number(dataToElement.y()) + " ";
                }
            }
            break;
            case QPainterPath::LineToElement:
            {
                if (t != 'L') {
                    t = 'L';
                    parts << " L " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                } else {
                    parts << QString::number(e.x) + " " + QString::number(e.y) + " ";
                }
            }
            break;
            case QPainterPath::CurveToElement:
            {
                dataCounter = 0;

                if (!ignoreData) {
                    if (t != 'C') {
                        t = 'C';
                        parts << " C " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                    } else {
                        parts << "  " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                    }
                }
            }
            break;
            case QPainterPath::CurveToDataElement:
            {
                QPointF dataPoint = QPointF(e.x, e.y);
                if (dataPoint == initPoint) {
                    ignoreData = true;

                    // Adding the previews node before the new one
                    if (t == 'C')
                        parts << " " + QString::number(e.x) + "  " + QString::number(e.y) + " ";

                    // Calculating new segment and insert it here
                    QPair<QPointF, QPointF> elements = getCurveElements(initPoint, newNode);
                    // Adding CurveToElement
                    QPointF curveToElement = elements.first;
                    if (t != 'C') {
                        t = 'C';
                        parts << " C " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
                    } else {
                        parts << "  " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";
                    }

                    // Adding CurveToDataElement
                    QPointF dataToElement = elements.second;
                    parts << " " + QString::number(dataToElement.x()) + "  " + QString::number(dataToElement.y()) + " ";

                    // Adding NewNode element (CurveToDataElement)
                    parts << " " + QString::number(newNode.x()) + "  " + QString::number(newNode.y()) + " ";

                    elements = getCurveElements(newNode, endPoint);
                    curveToElement = elements.first;
                    // Adding CurveToElement
                    parts << "  " + QString::number(curveToElement.x()) + " " + QString::number(curveToElement.y()) + " ";

                    // Creating CurveToDataElement
                    dataToElement = elements.second;
                    parts << " " + QString::number(dataToElement.x()) + "  " + QString::number(dataToElement.y()) + " ";
                } else if (dataPoint == endPoint) { // Adding the node after the new node
                    if (t == 'C')
                        parts << " " + QString::number(e.x) + "  " + QString::number(e.y) + " ";

                    ignoreData = false;
                } else {
                    if (!ignoreData) {
                        if (t == 'C')
                            parts << " " + QString::number(e.x) + "  " + QString::number(e.y) + " ";
                    }
                }

                dataCounter++;
            }
            break;
        }
    }

    foreach(QString line, parts)
        pathStr += line;

    setPathFromString(pathStr);

    return pathStr;
}

QPair<QPointF, QPointF> TupPathItem::getCurveElements(QPointF initPos, QPointF endPos)
{
    #ifdef TUP_DEBUG
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
            startSaving = true;
        } else {
            if (pointIsContainedBetweenRange(points.at(i), points.at(i+1), endPos, 0)) {
                startSaving = false;
            } else {
                if (startSaving)
                    range << points.at(i);
            }
        }
    }

    if (!range.isEmpty()) {
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

    return QPair<QPointF, QPointF>(firstPoint, secondPoint);
}

// Method for short distances

bool TupPathItem::pointIsContainedBetweenRange(const QPointF &point1, const QPointF &point2, const QPointF &newPoint, float tolerance)
{
    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupPathItem::pointIsContainedBetweenRange()]";
        qDebug() << "[TupPathItem::pointIsContainedBetweenRange()] - point1 ->" << point1;
        qDebug() << "[TupPathItem::pointIsContainedBetweenRange()] - newPoint ->" << newPoint;
        qDebug() << "[TupPathItem::pointIsContainedBetweenRange()] - point2 ->" << point2;
    #endif

    if (point1.x() <= newPoint.x() && point2.x() >= newPoint.x()) {
        if (point1.y() <= newPoint.y() && point2.y() >= newPoint.y())
            return true;

        if (point2.y() <= newPoint.y() && point1.y() >= newPoint.y())
            return true;

        // Measuring distance between newPoint and path
        qreal distance = TAlgorithm::distanceFromLine(point1, point2, newPoint);
        if (distance <= tolerance)
            return true;
    }

    if (point2.x() <= newPoint.x() && point1.x() >= newPoint.x()) {
        if (point1.y() <= newPoint.y() && point2.y() >= newPoint.y())
            return true;

        if (point2.y() <= newPoint.y() && point1.y() >= newPoint.y())
            return true;

        // Measuring distance between newPoint and path
        qreal distance = TAlgorithm::distanceFromLine(point1, point2, newPoint);
        if (distance <= tolerance)
            return true;
    }

    // Line is vertical
    if (point1.y() <= newPoint.y() && point2.y() >= newPoint.y()) {
        if (point1.x() <= newPoint.x() && point2.x() >= newPoint.x())
            return true;

        if (point2.x() <= newPoint.x() && point1.x() >= newPoint.x())
            return true;

        // Measuring distance between newPoint and path
        qreal distance = TAlgorithm::distanceFromLine(point1, point2, newPoint);
        if (distance <= tolerance)
            return true;
    }

    if (point2.y() <= newPoint.y() && point1.y() >= newPoint.y()) {
        if (point1.x() <= newPoint.x() && point2.x() >= newPoint.x())
            return true;

        if (point2.x() <= newPoint.x() && point1.x() >= newPoint.x())
            return true;

        // Measuring distance between newPoint and path
        qreal distance = TAlgorithm::distanceFromLine(point1, point2, newPoint);
        if (distance <= tolerance)
            return true;
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::pointIsContainedBetweenRange()] - Warning: Point is out of scope!";
    #endif

    return false;
}

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
    for(int i=0; i<total-1; i++) {
        if (node1 == points.at(i)) {
            qDebug() << "startSaving => true";
            startSaving = true;
        } else {
            if (node2 == points.at(i)) {
                qDebug() << "startSaving => false";
                startSaving = false;
            } else {
                if (startSaving) {
                    if (i>0) {
                        float innerDistance = TAlgorithm::distance(points.at(i-1), points.at(i));
                        if (innerDistance > tolerance) {
                            int times = innerDistance / tolerance;
                            float deltaX = (points.at(i).x() - points.at(i-1).x()) / times;
                            float deltaY = (points.at(i).y() - points.at(i-1).y()) / times;
                            for (int k=1; k<=times; k++) {
                                QPointF middlePoint(points.at(i-1).x() + (deltaX * k), points.at(i-1).y() + (deltaY * k));
                                range << middlePoint;
                            }
                        }
                    }

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
        if (distance < tolerance)
            return true;
    }

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

    return QPair(c1, c2);
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

    return QPair(c1, c2);
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

    qDebug() << "PATH step ->" << step;
    qDebug() << "elementsTotal ->" << elementsTotal;

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
                qDebug() << "QPainterPath::MoveToElement ->" << i;
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
                qDebug() << "QPainterPath::MoveToElement ->" << i;
            }
            break;
            case QPainterPath::LineToElement:
            {
                slope = TAlgorithm::slope(previewPoint, currentPoint);
                newSlope = TAlgorithm::slope(currentPoint, nextPoint);
                diff = fabs(newSlope - slope);

                qDebug() << "";
                qDebug() << "- QPainterPath::LineToElement ->" << i;
                qDebug() << "slope ->" << slope;
                qDebug() << "newSlope ->" << newSlope;
                qDebug() << "diff ->" << diff;

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
                qDebug() << "QPainterPath::CurveToElement ->" << i;
            }
            break;
            case QPainterPath::CurveToDataElement:
            {
                qDebug() << "QPainterPath::CurveToDataElement -> " << i;
            }
            break;
        }

        previewPoint = currentPoint;
    }

    qDebug() << "NEW ELEMENTS COUNT ->" << qPath.elementCount();

    return qPath;
}

QPainterPath TupPathItem::brushPath(const QPainterPath &route, int tolerance)
{
    QPolygonF points = route.toFillPolygon();
    points.removeLast();
    qDebug() << "POINTS:";
    qDebug() << points;
    int total = points.size();

    QPainterPath qPath(points.at(0));
    qPath.lineTo(points.at(1));
    float slope = TAlgorithm::slope(points.at(0), points.at(1));

    for(int i=2; i<total-1; i++) {
        float newSlope = TAlgorithm::slope(points.at(i-1), points.at(i));
        float distance = TAlgorithm::distance(points.at(i-1), points.at(i));
        float diff = fabs(newSlope - slope);
        qDebug() << "previous slope ->" << slope;
        qDebug() << "newSlope ->" << newSlope;
        qDebug() << "diff ->" << diff;

        if ((diff > 0.2) && (distance > (tolerance*2))) {
            qDebug() << "";
            qDebug() << "*** Adding point to the line ->" << points.at(i);
            qDebug() << "";
            qPath.lineTo(points.at(i));
            slope = newSlope;
        }
    }

    qPath.lineTo(points.last());
    qDebug() << "NEW ELEMENTS COUNT ->" << qPath.elementCount();

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

        if (counter > 50)
            return 51;
    }

    return counter;
}

QList<QPair<QPointF,QPointF>> TupPathItem::getCPoints()
{
    return curvePoints;
}
