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

#ifndef TUPLINEGUIDE_H
#define TUPLINEGUIDE_H

#include "tglobal.h"

#include <QGraphicsItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

class TUPITUBE_EXPORT TupLineGuide : public QGraphicsItem
{
    public:
        TupLineGuide(Qt::Orientation orientation);
        ~TupLineGuide();
        
        QRectF boundingRect() const;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
        void setEnabledSyncCursor(bool enabled);
        
    protected:
        QVariant itemChange(GraphicsItemChange change, const QVariant &value);
        // void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        bool sceneEvent(QEvent *event);

    private:
        void syncCursor();

        Qt::Orientation orientation;
        bool enabled;
};

#endif
