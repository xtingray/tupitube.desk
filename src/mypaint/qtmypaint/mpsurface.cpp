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

#include "mpsurface.h"
#include <QDebug>

static void freeTiledSurface(MyPaintSurface *surface)
{
    MPSurface *self = (MPSurface *) surface;
    mypaint_tiled_surface_destroy(self);

    free(self->tile_buffer);
    free(self->null_tile);
    free(self);
}

static void defaultUpdateFunction(MPSurface *surface, MPTile *tile)
{
    Q_UNUSED(surface)
    Q_UNUSED(tile)
    // Things to do if no update callback has been affected
}

static void onTileRequestStart(MyPaintTiledSurface *tiled_surface, MyPaintTileRequest *request)
{
    MPSurface *self = (MPSurface *) tiled_surface;

    const int tx = request->tx;
    const int ty = request->ty;
    uint16_t *tile_pointer = nullptr;

    if (tx >= self->getTilesWidth() || ty >= self->getTilesHeight() || tx < 0 || ty < 0) {
        // Give it a tile which we will ignore writes to
        tile_pointer = self->null_tile;
    } else {
        MPTile* tile = self->getTileFromIdx(QPoint(tx, ty));
        tile_pointer = tile ? tile->bits(false) : nullptr;
    }

    request->buffer = tile_pointer;
}

static void onTileRequestEnd(MyPaintTiledSurface *tiled_surface, MyPaintTileRequest *request)
{
    MPSurface *self = (MPSurface *) tiled_surface;

    const int tx = request->tx;
    const int ty = request->ty;

    MPTile* tile = self->getTileFromIdx(QPoint(tx, ty));
    if (tile)
        tile->updateCache();

    self->onUpdateTileFunction(self, tile);
}

MPSurface::MPSurface(QSize size)
{
    #ifdef TUP_DEBUG
        qDebug() << "MPSurface::MPSurface() - size: " << size;
        qDebug() << "MPSurface::MPSurface() - calling resetSurface()";
    #endif

    // Init callbacks
    this->onUpdateTileFunction = defaultUpdateFunction;
    this->onNewTileFunction = defaultUpdateFunction;

    // MPSurface vfuncs
    this->parent.destroy = freeTiledSurface;

    resetSurface(size);

    mypaint_tiled_surface_init((MyPaintTiledSurface *) this, onTileRequestStart, onTileRequestEnd);
}

MPSurface::~MPSurface()
{
}

void MPSurface::setOnUpdateTile(MPOnUpdateTileFunction onUpdateFunction)
{
    this->onUpdateTileFunction = onUpdateFunction;
}

void MPSurface::setOnNewTile(MPOnUpdateTileFunction onNewTileFunction)
{
    this->onNewTileFunction = onNewTileFunction;
}

void MPSurface::setOnClearedSurface(MPOnUpdateSurfaceFunction onClearedSurfaceFunction)
{
    this->onClearedSurfaceFunction = onClearedSurfaceFunction;
}

void MPSurface::setSize(QSize size)
{
    #ifdef TUP_DEBUG
        qDebug() << "MPSurface::setSize() - size: " << size;
    #endif

    free(this->tile_buffer);
    free(this->null_tile);

    resetSurface(size);
}

QSize MPSurface::size()
{
    #ifdef TUP_DEBUG
        qDebug() << "MPSurface::size() - width: " << width;
        qDebug() << "MPSurface::size() - height: " << height;
    #endif

    return QSize(width, height);
}

void MPSurface::clear()
{
    QHashIterator<QPoint, MPTile*> i(tilesHash);
    while (i.hasNext()) {
        i.next();
        MPTile *tile = i.value();
        if (tile) {
            // Clear the content of the tile
            tile->clear();

            // Removes blank tile from the scene for output optimization
            //
            // A tile without a scene is not re-created but onNewTile is
            // called when this tile is to be shown again.
            QGraphicsScene* scene = tile->scene();
            if (scene)
                scene->removeItem(tile);

            /*
            if (gScene)
                gScene->removeItem(tile);
            */
        }
    }
    undoHash = tilesHash;
    tilesHash.clear();

    this->onClearedSurfaceFunction(this);
}

void MPSurface::loadImage(const QImage &image)
{
    QSize tileSize = QSize(MYPAINT_TILE_SIZE, MYPAINT_TILE_SIZE);
    int nbTilesOnWidth = static_cast<int>(ceil(this->width / tileSize.width())) + 1;
    int nbTilesOnHeight = static_cast<int>(ceil(this->height / tileSize.height())) + 1;

    /*
    #ifdef TUP_DEBUG
        qDebug() << "RasterCanvas::loadImage() - width: " << this->width;
        qDebug() << "RasterCanvas::loadImage() - height: " << this->height;
        qDebug() << "RasterCanvas::loadImage() - tileSize width: " << tileSize.width();
        qDebug() << "RasterCanvas::loadImage() - tileSize height: " << tileSize.height();
        qDebug() << "RasterCanvas::loadImage() - nbTilesOnWidth: " << nbTilesOnWidth;
        qDebug() << "RasterCanvas::loadImage() - nbTilesOnHeight: " << nbTilesOnHeight;
    #endif
    */

    for (int h=0; h < nbTilesOnHeight; h++) {
        for (int w=0; w < nbTilesOnWidth; w++) {
            MPTile *tile = getTileFromIdx(QPoint(w, h));
            QRect tileRect = QRect(tile->pos().toPoint(), tileSize);

            // QImage tileImage = image.copy(tileRect);
            tile->setImage(image.copy(tileRect));
            // tile->setImage(tileImage);
            this->onUpdateTileFunction(this, tile);
        }
    }
}

QImage MPSurface::renderImage(const QSize canvasSize)
{
    #ifdef TUP_DEBUG
        qDebug() << "MPSurface::renderImage() - size: " << size();
        qDebug() << "MPSurface::renderImage() - canvasSize: " << canvasSize;
    #endif

    QPixmap renderedImage = QPixmap(canvasSize);
    renderedImage.fill(QColor(Qt::transparent));

    QGraphicsScene scene;
    scene.setSceneRect(QRect(QPoint(0, 0), canvasSize));

    QHashIterator<QPoint, MPTile*> i(tilesHash);
    while (i.hasNext()) {
        i.next();
        MPTile *tile = i.value();
        if (tile) {
            QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(tile->image()));
            item->setPos(tile->pos());
            scene.addItem(item);
        }
    }

    QPainter painter;
    painter.begin(&renderedImage);
    scene.render(&painter);
    painter.end();

    scene.clear();

    return renderedImage.toImage();
}

int MPSurface::getTilesWidth()
{
    return this->tiles_width;
}

int MPSurface::getTilesHeight()
{
    return this->tiles_height;
}

int MPSurface::getWidth()
{
    return this->width;
}

int MPSurface::getHeight()
{
    return this->height;
}

void MPSurface::resetNullTile()
{
    memset(this->null_tile, 0, static_cast<size_t>(this->tile_size));
}

void MPSurface::resetSurface(QSize size)
{
    #ifdef TUP_DEBUG
        qDebug() << "MPSurface::resetSurface() - Updating size: " << size;
    #endif

    width = size.width();
    height = size.height();

    assert(width > 0);
    assert(height > 0);

    const int tile_size_pixels = MYPAINT_TILE_SIZE;

    const int tiles_width = static_cast<int>(ceil(static_cast<float>(width) / tile_size_pixels));
    const int tiles_height = static_cast<int>(ceil(static_cast<float>(height) / tile_size_pixels));

    const size_t tile_size = tile_size_pixels * tile_size_pixels * 4 * sizeof(uint16_t);
    const size_t buffer_size = static_cast<unsigned long>(tiles_width * tiles_height) * tile_size;

    assert(tile_size_pixels*tiles_width >= width);
    assert(tile_size_pixels*tiles_height >= height);
    assert(buffer_size >= static_cast<unsigned long>(width * height * 4) * sizeof(uint16_t));

    uint16_t* buffer = static_cast<uint16_t *> (malloc(buffer_size));
    if (!buffer)
        fprintf(stderr, "CRITICAL: unable to allocate enough memory: %zu bytes", buffer_size);

    memset(buffer, 255, buffer_size);

    this->tile_buffer = buffer;
    this->tile_size = tile_size;
    this->null_tile = static_cast<uint16_t *> (malloc(tile_size));
    this->tiles_width = tiles_width;
    this->tiles_height = tiles_height;

    resetNullTile();
}

MPTile* MPSurface::getTileFromPos(const QPoint& pos)
{
    return getTileFromIdx(getTileIndex(pos));
}

MPTile* MPSurface::getTileFromIdx(const QPoint& idx)
{
    MPTile* selectedTile = nullptr;
    // Which tile index is it ?
    if (checkIndex(static_cast<uint>(idx.x())) && checkIndex(static_cast<uint>(idx.y()))) { // out of range ?
        // Ok, valid index. Does it exist already ?
        selectedTile = tilesHash.value(idx, nullptr);

        if (!selectedTile) {
            // Time to allocate it, update table and insert it as a QGraphicsItem in scene:
            selectedTile = new MPTile();
            tilesHash.insert(idx, selectedTile);
            tileIndexes << idx;

            QPoint tilePos(getTilePos(idx));
            selectedTile->setPos(tilePos);
        }

        if (!selectedTile->scene()) {
            this->onNewTileFunction(this, selectedTile);
        }
    }

    return selectedTile;
}

inline bool MPSurface::checkIndex(uint n)
{
    return (static_cast<int>(n) < k_max);
}

inline QPoint MPSurface::getTilePos(const QPoint& idx)
{
    return QPoint(MYPAINT_TILE_SIZE*idx.x(), MYPAINT_TILE_SIZE*idx.y());
}

inline QPoint MPSurface::getTileIndex(const QPoint& pos)
{
    return QPoint(pos.x()/MYPAINT_TILE_SIZE, pos.y()/MYPAINT_TILE_SIZE);
}

inline QPointF MPSurface::getTileFIndex(const QPoint& pos)
{
    return QPointF(static_cast<qreal>(pos.x() / MYPAINT_TILE_SIZE),
                   static_cast<qreal>(pos.y() / MYPAINT_TILE_SIZE));
}

bool MPSurface::isEmpty()
{
    return tilesHash.isEmpty();
}

void MPSurface::undo(int items)
{
    // qDebug() << "MPSurface::undo() - items: " << items;

    // clear();
    // tilesHash = undoHash;

    for (int i=0; i< items; i++) {
        QPoint point = tileIndexes.takeLast();
        MPTile *tile = tilesHash.take(point);
        if (gScene)
            gScene->removeItem(tile);

        undoList << tile;
    }

    QHashIterator<QPoint, MPTile*> i(tilesHash);
    while (i.hasNext()) {
        i.next();
        MPTile *tile = i.value();
        if (tile) {
            tile->update();
        }
    }

    /*
    QHashIterator<QPoint, MPTile*> i(tilesHash);
    while (i.hasNext()) {
        i.next();
        MPTile *tile = i.value();
        if (tile) {
            gScene->addItem(tile);
            // QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(tile->image()));
            // item->setPos(tile->pos());
            // gScene->addItem(item);
        }
    }
    */
}

void MPSurface::redo()
{
    // qDebug() << "MPSurface::redo() - Tracing...";
}

int MPSurface::getTilesCount()
{
    return tilesHash.count();
}

void MPSurface::setScene(QGraphicsScene *scene)
{
    gScene = scene;
}
