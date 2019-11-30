/*
    Copyright © 2015 by The QTMyPaint Project

    This file is part of QTMyPaint, a Qt-based interface for MyPaint C++ library.

    QTMyPaint is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QTMyPaint is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QTMyPaint. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RASTERCANVAS_H
#define RASTERCANVAS_H

#include "rastercanvasbase.h"
#include "mphandler.h"
#include "mpbrush.h"
#include "mpsurface.h"
#include "mptile.h"

#include <QWidget>
#include <QMouseEvent>

class TUPITUBE_EXPORT RasterCanvas : public RasterCanvasBase
{
    Q_OBJECT

    public:
        RasterCanvas(TupProject *project, const QColor contourColor, QWidget *parent = nullptr);
        ~RasterCanvas();

        void setSize(QSize size);
        void setTabletDevice(QTabletEvent *event);

        void saveToFile(QString filePath);
        void loadFromFile(QString filePath);

        void resetWorkSpaceCenter(const QSize projectSize);
        bool canvasIsEmpty();

    protected:
        virtual void tabletEvent(QTabletEvent *event);
        virtual void mouseMoveEvent(QMouseEvent *event);
        virtual void mousePressEvent(QMouseEvent *event);
        virtual void mouseReleaseEvent(QMouseEvent *event);
        virtual void keyPressEvent(QKeyEvent *event);

    signals:
        void closeWindow();
        void zoomIn();
        void zoomOut();

    public slots:
        void updateBrushColor(const QColor color);
        void clearCanvas();

        void onNewTile(MPSurface *surface, MPTile *tile);
        void onUpdateTile(MPSurface *surface, MPTile *tile);
        void onClearedSurface(MPSurface *surface);

        void loadBrush(const QByteArray &content);

    private:
        void centerDrawingArea();
        void updateCursor(const QTabletEvent *event);

        QGraphicsScene *gScene;
        QRectF drawingRect;
        int globalSceneIndex;

        bool pressed;
        bool spaceBar;
        QColor color;
        bool tableInUse;
        MPHandler *myPaintCanvas;
};

#endif // RASTERCANVAS_H
