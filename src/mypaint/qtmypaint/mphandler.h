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
#ifndef MPHANDLER_H
#define MPHANDLER_H

#include "tglobal.h"
#include "mypaint-brush.h"
#include "mypaint-surface.h"

#include "mpbrush.h"
#include "mpsurface.h"

#include <QColor>

#ifndef QTMYPAINT_SURFACE_WIDTH
#define QTMYPAINT_SURFACE_WIDTH 640
#endif

#ifndef QTMYPAINT_SURFACE_HEIGHT
#define QTMYPAINT_SURFACE_HEIGHT 480
#endif

class Q_DECL_EXPORT MPHandler : public QObject
{
    Q_OBJECT

    public:
        ~MPHandler();

        static MPHandler *handler();

        typedef void (*MPOnUpdateFunction) (MPHandler *handler, MPSurface *surface, MPTile *tile);

        void startStroke();
        void strokeTo(float x, float y, float pressure, float xtilt, float ytilt);
        void strokeTo(float x, float y);
        void endStroke();

        float getBrushValue(MyPaintBrushSetting setting);

        void setBrushColor(const QColor newColor);

        void setBrushValue(MyPaintBrushSetting setting, float value);

        void requestUpdateTile(MPSurface *surface, MPTile *tile);
        void hasNewTile(MPSurface *surface, MPTile *tile);
        void hasClearedSurface(MPSurface *surface);

        void setSurfaceSize(QSize size);
        QSize surfaceSize();

        void clearSurface();
        QImage renderImage(const QSize size);

        void loadImage(const QImage &image);
        bool isEmpty();

        int getTilesCounter();

        void resetMem();

        void undo();
        void redo();
        void saveTiles();

    public slots:
        void loadBrush(const QByteArray &content);

    signals:
        void updateTile(MPSurface *surface, MPTile *tile);
        void newTile(MPSurface *surface, MPTile *tile);
        void clearedSurface(MPSurface *surface);

    private:
        MPHandler();
        static bool instanceFlag;
        static MPHandler *currentHandler;

        MPBrush *m_brush;
        MPSurface *m_surface;
};

#endif // MPHANDLER_H
