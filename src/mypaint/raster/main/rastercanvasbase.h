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

#ifndef RASTERCANVASBASE_H
#define RASTERCANVASBASE_H

#include "tglobal.h"
#include "tuprotationdial.h"
#include "tupproject.h"

#include <QGraphicsView>
#include <QGraphicsRectItem>

class TUPITUBE_EXPORT RasterCanvasBase : public QGraphicsView
{
    Q_OBJECT

    public:
        RasterCanvasBase(QSize dimension, QWidget *parent = nullptr);
        ~RasterCanvasBase();

        void setAntialiasing(bool use);
        void drawGrid(bool draw);
        void drawActionSafeArea(bool draw);

        bool getGridState() const;
        bool getSafeAreaState() const;

        void scaleView(qreal scaleFactor);
        void setRotationAngle(int angle);
        void setZoom(qreal factor);

        QRectF getDrawingRect() const;
        QGraphicsScene *graphicsScene() const;
        QPointF viewPosition();
        QPointF getCenterPoint() const;

        void updateDimension(const QSize dimension);
        void updateGridParameters();

    private:
        void drawPadLock(QPainter *painter, const QRectF &rect, QString text);
        void updateCenter(const QPoint point);

    protected:
        void keyPressEvent(QKeyEvent *event);
        void keyReleaseEvent(QKeyEvent *event);

        virtual void mousePressEvent(QMouseEvent *event);
        virtual void mouseMoveEvent(QMouseEvent *event);
        virtual void mouseReleaseEvent(QMouseEvent *event);

        void enterEvent(QEvent *event);
        void leaveEvent(QEvent *event);

        bool viewportEvent(QEvent *event);
        void wheelEvent(QWheelEvent *event);

    signals:
        void requestTriggered(const TupProjectRequest *event);
        void changedZero(const QPointF &zero);
        void scaled(qreal scaleFactor);
        void rotated(int angle);

    private slots:
        void updateAngle(int angle);

    public slots:
        void setBgColor(const QColor color);
        void centerDrawingArea();

    protected:
        virtual void drawBackground(QPainter *painter, const QRectF &rect);
        virtual void drawForeground(QPainter *painter, const QRectF &rect);

    private:
        QGraphicsRectItem *grid;
        QRectF drawingRect;
        QPointF position;
        QColor bgColor;

        bool gridEnabled;
        bool safeAreaEnabled;
        double angle;

        QStringList copiesXml;
        QGraphicsScene *gScene;

        QPen greenThickPen;
        QPen grayPen;
        QPen greenBoldPen;
        QPen greenThinPen;
        QPen blackPen;
        bool spaceBar;
        QPen gridPen;
        QPen dotPen;
        int target;
        int gridSeparation;

        QPoint initPoint;
        QPoint centerPoint;

        TupRotationDial *dial;
};

#endif
