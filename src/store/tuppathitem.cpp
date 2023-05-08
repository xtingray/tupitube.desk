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
        qDebug() << "[TupPathItem::removeNodeFromPath()] - i ->" << i;
        qDebug() << "[TupPathItem::removeNodeFromPath()] - pos ->" << pos;
        qDebug() << "[TupPathItem::removeNodeFromPath()] - e.x ->" << e.x;
        qDebug() << "[TupPathItem::removeNodeFromPath()] - e.y ->" << e.y;

        switch (e.type) {
            case QPainterPath::MoveToElement:
            {
                qDebug() << "[TupPathItem::removeNodeFromPath()] - MoveToElement";

                if (pos != QPointF(e.x, e.y)) {
                    if (t != 'M') {
                        t = 'M';
                        parts << "M " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                    } else {
                        parts << QString::number(e.x) + " " + QString::number(e.y) + " ";
                    }
                    qDebug() << "PART ->" << parts.last();
                } else {
                    qDebug() << "FOUND!";
                    qDebug() << "*** Removing original first node!";
                    replace = true;
                }

                // curvesCounter++;
            }
            break;
            case QPainterPath::LineToElement:
            {
                qDebug() << "[TupPathItem::removeNodeFromPath()] - LineToElement";

                if (pos != QPointF(e.x, e.y)) {
                    if (t != 'L') {
                        t = 'L';
                        parts << " L " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                    } else {
                        parts << QString::number(e.x) + " " + QString::number(e.y) + " ";
                    }
                    qDebug() << "PART ->" << parts.last();
                } else {
                    qDebug() << "FOUND!";
                    qDebug() << "*** Removing original first node!";
                }
            }
            break;
            case QPainterPath::CurveToElement:
            {
                qDebug() << "[TupPathItem::removeNodeFromPath()] - CurveToElement";
                qDebug() << "[TupPathItem::removeNodeFromPath()] - curvesCounter ->" << curvesCounter;

                if (replace) {
                    qDebug() << "*** Adding new First Node to String!";
                    parts << "M " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                    replace = false;
                    lookingData = true;
                    qDebug() << "PART ->" << parts.last();
                } else {
                    if (pos != QPointF(e.x, e.y)) {
                        if (t != 'C') {
                            t = 'C';
                            parts << " C " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                        } else {
                            parts << "  " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                        }
                        qDebug() << "PART ->" << parts.last();
                        curvesCounter++;
                    } else {
                        qDebug() << "FOUND!";
                        qDebug() << "*** Removing curve node at index ->" << curvesCounter;
                        lookingData = true;
                    }
                }
            }
            break;
            case QPainterPath::CurveToDataElement:
            {
                qDebug() << "[TupPathItem::removeNodeFromPath()] - CurveToDataElement";

                if (pos == QPointF(e.x, e.y)) {
                    qDebug() << "FOUND! - Removing the last two parts...";
                    qDebug() << "last 1 -> " << parts.last();
                    parts.removeLast();
                    qDebug() << "last 2 -> " << parts.last();
                    parts.removeLast();

                    parts << "C";
                    lookingData = false;
                } else {
                    if (!lookingData) {
                        if (t == 'C')
                            parts <<  " " + QString::number(e.x) + "  " + QString::number(e.y) + " ";
                        qDebug() << "PART ->" << parts.last();
                    } else {
                        qDebug() << "*** Removing curve data node at index ->" << curvesCounter;
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

        qDebug() << "---";
    }

    foreach(QString line, parts)
        pathStr += line;

    qDebug() << "*** parts.size() ->" << parts.size();
    qDebug() << "*** parts ->" << parts;
    qDebug() << "*** pathStr ->" << pathStr;
    qDebug() << "";

    return pathStr;
}

bool TupPathItem::containsOnPath(QPointF pos, float tolerance)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::containsOnPath()] - pos ->" << pos;
        qDebug() << "[TupPathItem::containsOnPath()] - tolerance ->" << tolerance;
    #endif

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
    qDebug() << "points.size() ->" << points.size();

    int total = points.size();
    for(int i=0; i<total; i++) {
        float distance = TAlgorithm::distance(pos, points.at(i));
        if (i == 0) {
            minimum = distance;
            farMin = distance;
        }

        /*
        qDebug() << "points.at(i) ->" << points.at(i);
        qDebug() << "pos ->" << pos;
        qDebug() << "tolerance ->" << tolerance;
        qDebug() << "distance ->" << distance;
        qDebug() << "---";
        */

        if (distance <= tolerance) {
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
        qDebug() << "C1 ->" << c1;
        qDebug() << "C2 ->" << c2;
        qDebug() << "POINT WAS FOUND! - New Node ->" << newNode;
        qDebug() << "";

        return true;
    } else { // Calculating node point and C1/C2
        QList<QPointF> beforeList;
        float beforeSegX = middlePoint.x() - beforePoint.x();
        float beforeSegY = middlePoint.y() - beforePoint.y();
        float beforeSeg = sqrt(pow(beforeSegX, 2) + pow(beforeSegY, 2));
        float step = beforeSeg/(tolerance/2);
        float stepX = beforeSegX/step;
        float stepY = beforeSegY/step;
        for(int i=0; i < (step-1); i++)
            beforeList << (beforePoint + QPointF(stepX*i, stepY*i));

        QPointF bestPoint;
        float bestDistance = beforeSeg;
        int index = 0;
        foreach(QPointF point, beforeList) {
            float distance = TAlgorithm::distance(pos, point);
            if (distance <= bestDistance) {
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
        stepX = afterSegX/step;
        stepY = afterSegY/step;
        for(int i=0; i < (step-1); i++)
            afterList << (middlePoint + QPointF(stepX*i, stepY*i));

        index = 0;
        foreach(QPointF point, afterList) {
            float distance = TAlgorithm::distance(pos, point);
            if (distance <= bestDistance) {
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

        newNode = bestPoint;

        /*
        float xSpace = (minPoint1.x() - minPoint2.x());
        float ySpace = (minPoint1.y() - minPoint2.y());
        float stepX = xSpace / 2;
        float stepY = ySpace / 2;
        newNode = QPointF(minPoint1.x() + stepX, minPoint1.y() + stepY);
        if (key1 < key2) {
            c1 = points.at(key1 - 1);
            c2 = points.at(key1);
        } else {
            c1 = points.at(key2 - 1);
            c2 = points.at(key2);
        }
        */

        qDebug() << "C1 ->" << c1;
        qDebug() << "C2 ->" << c2;
        qDebug() << "POINT WAS MADE UP! - New Node ->" << newNode;
        qDebug() << "";

        return true;
    }

    qDebug() << "POINT NOT FOUND!";
    qDebug() << "";

    return false;
}

QString TupPathItem::addNewNode(int tolerance)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::addNewNode()] - newNode ->" << newNode;
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

    QStringList parts;
    QString pathStr = "";

    // Storing nodes
    for(int i=0; i<elementsTotal; i++) {
        QPainterPath::Element e = route.elementAt(i);
        switch (e.type) {
            case QPainterPath::MoveToElement:
            {
                // qDebug() << "*** Counting nodes - MoveToElement";
                nodePoints << QPointF(e.x, e.y);
                nodesCounter++;

                previewPoint = QPointF(e.x, e.y);
            }
            break;
            case QPainterPath::CurveToElement:
            {
                // qDebug() << "*** Counting nodes - CurveToElement";
                dataCounter = 0;
            }
            break;
            case QPainterPath::CurveToDataElement:
            {
                // qDebug() << "*** Counting nodes - CurveToDataElement";

                if (dataCounter == 1) {
                    // qDebug() << "Adding a new node to the list...";
                    nodePoints << QPointF(e.x, e.y);
                    nodesCounter++;

                    if (pointIsContained(previewPoint, QPointF(e.x, e.y), newNode, tolerance)) {
                        qDebug() << "";
                        qDebug() << "2 - Position of new node was FOUND!";
                        qDebug() << "nodesCounter ->" << nodesCounter;
                        qDebug() << "*** NEW NODE is located between node " << (nodesCounter-1) << " and " << nodesCounter;
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

        qDebug() << "---";
    }

    next:

    qDebug() << "nodePoints List ->" << nodePoints;
    nodesCounter = 0;

    for(int i=0; i<elementsTotal; i++) {
        QPainterPath::Element e = route.elementAt(i);
        switch (e.type) {
            case QPainterPath::MoveToElement:
            {
                qDebug() << "[TupPathItem::addNewNode()] - MoveToElement ->" << QPointF(e.x, e.y);
                nodesCounter++;

                if (t != 'M') {
                    t = 'M';
                    parts << "M " + QString::number(e.x) + " " + QString::number(e.y) + " ";
                } else {
                    parts << QString::number(e.x) + " " + QString::number(e.y) + " ";
                }

                if (isEarlyNode) {
                    ignoreData = true;

                    qDebug() << "*** Inserting NEW NODE after first node!!!";

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
                qDebug() << "[TupPathItem::addNewNode()] - LineToElement ->" << QPointF(e.x, e.y);

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
                qDebug() << "[TupPathItem::addNewNode()] - CurveToElement ->" << QPointF(e.x, e.y);
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
                qDebug() << "[TupPathItem::addNewNode()] - CurveToDataElement ->" << QPointF(e.x, e.y);
                qDebug() << "initPoint ->" << initPoint;
                qDebug() << "e.x, e.y ->" << dataPoint;

                if (dataPoint == initPoint) {
                    ignoreData = true;

                    qDebug() << "*** Inserting NEW NODE between nodes!!!";

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

        qDebug() << "---";
    }

    foreach(QString line, parts)
        pathStr += line;

    /*
    qDebug() << "*** parts.size() ->" << parts.size();
    qDebug() << "*** parts ->" << parts;
    qDebug() << "*** pathStr ->" << pathStr;
    qDebug() << "";
    */

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

    qDebug() << "points.size() ->" << points.size();
    qDebug() << "POINTS ->" << points;

    bool startSaving = false;
    QList<QPointF> range;
    int total = points.size();
    for(int i=0; i<total-1; i++) {
        if (pointIsContained(points.at(i), points.at(i+1), initPos, 0)) {
        // if (points.at(i) == initPos) {
            startSaving = true;
        } else {
            if (pointIsContained(points.at(i), points.at(i+1), endPos, 0)) {
            // if (points.at(i) == endPos) {
                startSaving = false;
            } else {
                if (startSaving)
                    range << points.at(i);
            }
        }
    }

    qDebug() << "range.size() ->" << range.size();

    if (!range.isEmpty()) {
        qDebug() << "Taking points from original list ->" << range.size();

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
            qDebug() << "RANGE -> " << range;
            qDebug() << "Dividing list in three parts!";
            qDebug() << "step ->" << step;
            qDebug() << "step*2 ->" << step*2;
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPathItem::getCurveElements()] - Warning: range.size() -> 0";
        #endif
        // Making up the missing points

        qDebug() << "Making up missing points...";

        QPointF segment = endPos - initPos;
        float stepX = segment.x()/3;
        float stepY = segment.y()/3;

        firstPoint = initPos + QPointF(stepX, stepY);
        secondPoint = initPos + QPointF(stepX*2, stepY*2);
    }

    qDebug() << "";
    qDebug() << "FIRST POINT ->" << firstPoint;
    qDebug() << "SECOND POINT ->" << secondPoint;
    qDebug() << "";

    return QPair<QPointF, QPointF>(firstPoint, secondPoint);
}

bool TupPathItem::pointIsContained(const QPointF &point1, const QPointF &point2, const QPointF &newPoint, int tolerance)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::pointIsContained()]";
        qDebug() << "[TupPathItem::pointIsContained()] - point1 ->" << point1;
        qDebug() << "[TupPathItem::pointIsContained()] - newPoint ->" << newPoint;
        qDebug() << "[TupPathItem::pointIsContained()] - point2 ->" << point2;
    #endif

    if (point1.x() <= newPoint.x() && point2.x() >= newPoint.x()) {
        if (point1.y() <= newPoint.y() && point2.y() >= newPoint.y())
            return true;

        if (point2.y() <= newPoint.y() && point1.y() >= newPoint.y())
            return true;

        // Measuring distance between newPoint and path
        qreal distance = TAlgorithm::distanceFromLine(point1, point2, newPoint);
        qDebug() << "DISTANCE TO LINE ->" << distance;
        qDebug() << "tolerance ->" << tolerance;

        if (distance <= tolerance) {
            qDebug() << "TOO CLOSE!!!";
            return true;
        }
    }

    if (point2.x() <= newPoint.x() && point1.x() >= newPoint.x()) {
        if (point1.y() <= newPoint.y() && point2.y() >= newPoint.y())
            return true;

        if (point2.y() <= newPoint.y() && point1.y() >= newPoint.y())
            return true;

        // Measuring distance between newPoint and path
        qreal distance = TAlgorithm::distanceFromLine(point1, point2, newPoint);
        qDebug() << "DISTANCE TO LINE ->" << distance;
        qDebug() << "tolerance ->" << tolerance;

        if (distance <= tolerance) {
            qDebug() << "TOO CLOSE!!!";
            return true;
        }
    }

    // Line is vertical

    if (point1.y() <= newPoint.y() && point2.y() >= newPoint.y()) {
        if (point1.x() <= newPoint.x() && point2.x() >= newPoint.x())
            return true;

        if (point2.x() <= newPoint.x() && point1.x() >= newPoint.x())
            return true;

        // Measuring distance between newPoint and path
        qreal distance = TAlgorithm::distanceFromLine(point1, point2, newPoint);
        qDebug() << "DISTANCE TO LINE ->" << distance;
        qDebug() << "tolerance ->" << tolerance;

        if (distance <= tolerance) {
            qDebug() << "TOO CLOSE!!!";
            return true;
        }
    }

    if (point2.y() <= newPoint.y() && point1.y() >= newPoint.y()) {
        if (point1.x() <= newPoint.x() && point2.x() >= newPoint.x())
            return true;

        if (point2.x() <= newPoint.x() && point1.x() >= newPoint.x())
            return true;

        // Measuring distance between newPoint and path
        qreal distance = TAlgorithm::distanceFromLine(point1, point2, newPoint);
        qDebug() << "DISTANCE TO LINE ->" << distance;
        qDebug() << "tolerance ->" << tolerance;

        if (distance <= tolerance) {
            qDebug() << "TOO CLOSE!!!";
            return true;
        }
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupPathItem::pointIsContained()] - Warning: Point is out of scope!";
    #endif

    return false;
}

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

QList<QColor> TupPathItem::nodeColors()
{
    return colors;
}

QList<QString> TupPathItem::nodeTips()
{
    return tips;
}
