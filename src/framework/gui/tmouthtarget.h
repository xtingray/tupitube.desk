/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustav Gonzalez / xtingray                                           *
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

#ifndef TMOUTHTARGET_H
#define TMOUTHTARGET_H

#include "tglobal.h"
// #include "tupgraphicalgorithm.h"
// #include "tupgraphicobject.h"

#include <QGraphicsItem>
#include <QObject>
#include <QPointF>
#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QStyleOptionButton>
#include <QApplication>

/**
 * @author Gustav Gonzalez 
*/

class TUPITUBE_PLUGIN TMouthTarget : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    
    public: 
        TMouthTarget(const QPointF & pos = QPoint(0,0), int zLevel = 0);
        ~TMouthTarget();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *);
        QRectF boundingRect() const;

        QPointF currentPos();
        void resize(qreal factor);

    signals:
        void initPos(const QPointF &point);
        void positionUpdated(const QPointF &point);

    protected:
        // QVariant itemChange(GraphicsItemChange change, const QVariant &value);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    private:
        QSizeF size;
};

#endif
