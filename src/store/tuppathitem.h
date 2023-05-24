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

#ifndef TUPPATHITEM_H
#define TUPPATHITEM_H

#include "tupabstractserializable.h"

#include <QGraphicsPathItem>
#include <QGraphicsSceneDragDropEvent>
#include <QPainter>

class TUPITUBE_EXPORT TupPathItem : public TupAbstractSerializable, public QGraphicsPathItem
{
    public:
        TupPathItem(QGraphicsItem *parent = nullptr);
        ~TupPathItem();
        
        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
        bool contains(const QPointF &point) const;
        bool isNotEdited();
        void saveOriginalPath();
        QString pathToString() const;
        void setPathFromString(const QString &route);
        void undoPath();
        void redoPath();

        QString refactoringPath(NodeLocation policy, int nodesTotal);
        QString pathRestored(int nodesTotal) const;
        void resetPathHistory();

        QString removeNodeFromPath(int index);
        QString changeNodeTypeFromPath(int index);
        bool containsOnPath(QPointF pos, float tolerance);
        QString addInnerNode(int tolerance, NodeType node);
        bool pointIsContainedBetweenRange(const QPointF &point1, const QPointF &point2, const QPointF &newPoint, float tolerance);
        QString appendNode(const QPointF &pos);
        QPair<QPointF,QPointF> calculateEndPathCPoints(const QPointF &pos);

        QList<QPointF> keyNodes();
        QList<QColor> nodeColors();
        QList<QString> nodeTips();
        QList<QPair<QPointF,QPointF>> getCPoints();

        QPainterPath clearPath(int tolerance);
        static QPainterPath brushPath(const QPainterPath &route, int tolerance);
        int nodesCount();
 
    protected:
        virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
        virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
        virtual void dropEvent(QGraphicsSceneDragDropEvent *event);
        
    private:
        QPair<QPointF, QPointF> getCurveElements(QPointF initPos, QPointF endPos);
        bool pointIsContainedBetweenNodes(const QPointF &node1, const QPointF &node2, const QPointF &point, float tolerance);
        bool pointIsContainedBetweenFlatNodes(const QPointF &node1, const QPointF &node2, const QPointF &point, int tolerance);
        QPair<QPointF,QPointF> calculateCPoints(const QPointF &pos1, const QPointF &pos2);
        QPair<QPointF,QPointF> calculatePlainCPoints(const QPointF &pos1, const QPointF &pos2);
        bool pointIsPartOfLine(const QPainterPath &route, const QPointF &point, int tolerance);
        bool findPointAtLine(const QPointF &point1, const QPointF &point2, const QPointF &target, int tolerance);

        bool dragOver;
        QList<QString> undoList;
        QList<QString> doList;
        QHash<int, QString> pathCollection;

        bool straightLineFlag;
        bool flatCurveFlag;
        QPointF newNode;

        QList<QColor> colors;
        QList<QString> tips;
        QList<QPair<QPointF,QPointF>> curvePoints;
};

#endif
