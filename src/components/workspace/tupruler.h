/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez                                                     *
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

#ifndef TUPRULER_H
#define TUPRULER_H

#include "tglobal.h"

#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QRectF>
#include <QPoint>
#include <QSize>
#include <cmath>

#define RULER_BREADTH 20

class TUPITUBE_EXPORT TupRuler : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal getOrigin READ getOrigin WRITE setOrigin)
    Q_PROPERTY(qreal getRulerZoom READ getRulerZoom WRITE setRulerZoom)

    public:
        enum Transformation { None = 1, Rotation, Scale };
        TupRuler(Qt::Orientation getRulerType, QWidget *parent);
        ~TupRuler();

        QSize minimumSizeHint() const;
        Qt::Orientation getRulerType() const;
        qreal getOrigin() const;
        qreal getRulerZoom() const;
        void updateCurrentTransformation(Transformation current);

    public slots:
        void setOrigin(const qreal getOrigin);
        void setRulerZoom(const qreal getRulerZoom);
        void movePointers(const QPointF pos);

    protected:
        void paintEvent(QPaintEvent* event);

    private:
        void translateArrow(double dx, double dy);
        void drawAScaleMeter(QPainter *painter, QRectF rulerRectr);
        void drawFromOriginTo(QPainter *painter, QRectF rulerRect, qreal startMark, qreal endMark, qreal step);
        void drawMaximizedRuler(QPainter *painter, QRectF rulerRect, qreal startMark, qreal endMark, qreal step);
        void drawSimpleRuler(QPainter *painter, QRectF rulerRect, qreal startMark, qreal endMark, qreal step);

    private:
        Qt::Orientation rulerType;
        qreal origin;
        qreal oldPos;
        qreal rulerZoom;
        qreal currentZoomFactor;
        QPointF cursorPos;
        QPolygonF arrow;
        Transformation currentTransformation;
};
#endif
