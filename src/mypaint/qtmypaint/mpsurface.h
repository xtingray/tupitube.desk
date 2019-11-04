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

#ifndef MPSURFACE_H
#define MPSURFACE_H

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <QGraphicsScene>
#include <QImage>

#include "mypaint-glib-compat.h"
#include "mypaint-tiled-surface.h"
#include "mpbrush.h"
#include "mptile.h"

class MPSurface : public MyPaintTiledSurface
{
    public:
        MPSurface(QSize size);
        ~MPSurface();

        uint16_t *tile_buffer; // Stores tiles in a linear chunk of memory (16bpc RGBA)
        uint16_t *null_tile; // Single tile that we hand out and ignore writes to

        int getTilesWidth();
        int getTilesHeight();
        int getWidth();
        int getHeight();

        enum { k_center = 50, k_max = 2*k_center};

        MPTile* getTileFromPos(const QPoint &pos);
        MPTile* getTileFromIdx(const QPoint &idx);
        inline bool checkIndex(uint n);
        inline QPoint getTilePos(const QPoint &idx);
        inline QPoint getTileIndex(const QPoint &pos);
        inline QPointF getTileFIndex(const QPoint &pos);

        typedef void (*MPOnUpdateTileFunction) (MPSurface *surface, MPTile *tile);
        typedef void (*MPOnUpdateSurfaceFunction) (MPSurface *surface);

        void setOnUpdateTile(MPOnUpdateTileFunction onUpdateTileFunction);
        void setOnNewTile(MPOnUpdateTileFunction onNewTileFunction);
        void setOnClearedSurface(MPOnUpdateSurfaceFunction onNewTileFunction);

        MPOnUpdateTileFunction onUpdateTileFunction;
        MPOnUpdateTileFunction onNewTileFunction;
        MPOnUpdateSurfaceFunction onClearedSurfaceFunction;

        void setSize(QSize size);
        QSize size();

        void clear();
        QImage renderImage();

        void loadImage(const QImage &image);

    protected:
        QHash<QPoint, MPTile*> m_Tiles;

    private:
        void resetNullTile();
        void resetSurface(QSize size);
        std::string key;

        int tiles_width; // width in tiles
        int tiles_height; // height in tiles
        int width; // width in pixels
        int height; // height in pixels

        MPBrush *m_brush;
        QColor m_color;
};

inline uint qHash(const QPoint & key)
{
    return qHash(QPair<int,int>(key.x(), key.y()));
}

#endif // MPSURFACE_H
