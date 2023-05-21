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

#ifndef NODE_H
#define NODE_H

#include "tglobal.h"

#include <QGraphicsItem>
#include <QPointF>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QCursor>

class NodeManager;

class TUPITUBE_PLUGIN Node : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    
    public:
        Node(NodeContext tool, NodePosition node, NodeAction action, const QPointF &pos=QPoint(0,0), NodeManager *manager=nullptr,
             QGraphicsItem *parent=nullptr, int zValue=0);
        ~Node();
        
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *);
        QRectF boundingRect() const;
        void resize(qreal factor);
        
        void setAction(NodeAction action);
        int nodeAction();

        int nodeType() const;

    protected:
        QVariant itemChange(GraphicsItemChange change, const QVariant &value);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
        void keyReleaseEvent(QKeyEvent *event);

    signals:
        void positionUpdated(const QPointF &point);
        void transformationUpdated();

    private:
        NodeContext context;
        NodePosition node;
        NodeAction action;
        NodeAction generalState;
        QGraphicsItem *parent;
        NodeManager *manager;
        QSizeF size;
        QPointF oldPoint;
};

#endif
