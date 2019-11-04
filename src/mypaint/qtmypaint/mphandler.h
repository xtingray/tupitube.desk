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

#include <QColor>

#include "mypaint-brush.h"
#include "mypaint-surface.h"

#include "mpbrush.h"
#include "mpsurface.h"

#ifndef QTMYPAINT_SURFACE_WIDTH
#define QTMYPAINT_SURFACE_WIDTH 100
#endif

#ifndef QTMYPAINT_SURFACE_HEIGHT
#define QTMYPAINT_SURFACE_HEIGHT 100
#endif

class MPHandler : public QObject
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

        void setBrushColor(QColor newColor);
        void setBrushValue(MyPaintBrushSetting setting, float value);

        void requestUpdateTile(MPSurface *surface, MPTile *tile);
        void hasNewTile(MPSurface *surface, MPTile *tile);
        void hasClearedSurface(MPSurface *surface);

        void setSurfaceSize(QSize size);
        QSize surfaceSize();

        void clearSurface();
        QImage renderImage();

        void loadImage(const QImage &image);

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
