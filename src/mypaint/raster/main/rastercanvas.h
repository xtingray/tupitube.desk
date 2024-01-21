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
        RasterCanvas(TupProject *project, double brushSize, const QColor contourColor, QWidget *parent = nullptr);
        ~RasterCanvas();

        void setTabletDevice(QTabletEvent *event);

        void saveToFile(QString filePath);
        void loadFromFile(QString filePath);

        void resetWorkSpaceCenter(const QSize projectSize);
        bool canvasIsEmpty();

        void resetMem();
        void clearCanvas();

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
        void rasterStrokeMade();

    public slots:
        void undo();
        void redo();
        void updateBrushColor(const QColor color);
        void setBrushSize(double size);

        void onNewTile(MPSurface *surface, MPTile *tile);
        void onUpdateTile(MPSurface *surface, MPTile *tile);
        void onClearedSurface(MPSurface *surface);

        void loadBrush(const QByteArray &content);
        void loadBrush(const QByteArray &content, double brushSize);

    private:
        void centerDrawingArea();
        void updateCursor(const QTabletEvent *event);
        QByteArray getBrushData(QJsonObject brush, double size);

        QGraphicsScene *gScene;
        QRectF drawingRect;

        bool pressed;
        bool spaceBar;
        QColor color;
        bool tableInUse;
        MPHandler *myPaintCanvas;

        QSize canvasSize;
        double brushSize;
};

#endif // RASTERCANVAS_H
