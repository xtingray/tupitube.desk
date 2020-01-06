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

#ifndef TUPBACKGROUND_H
#define TUPBACKGROUND_H

#include "tglobal.h"
#include "tupabstractserializable.h"
#include "tupscene.h"

#include <QImage>
#include <QPixmap>

class TUPITUBE_EXPORT TupBackground : public QObject, public TupAbstractSerializable
{
    Q_OBJECT

    public:
        enum BgType { VectorDynamic = 0, RasterDynamic, VectorStatic, RasterStatic };
        enum Direction { Right = 0, Left = 1, Top, Bottom };
        TupBackground(TupScene *parent, int sceneIndex, const QSize dimension, const QColor bgColor);
        ~TupBackground();

        TupFrame* vectorStaticFrame();
        TupFrame* vectorDynamicFrame();

        void setBgColor(const QColor color);
        void clearBackground();

        void renderVectorDynamicView();
        QPixmap vectorDynamicView(int frameIndex);
        void setVectorDynamicDirection(int direction);
        void setRasterDynamicDirection(int direction);
        void setVectorDynamicShift(int shift);
        void setRasterDynamicShift(int shift);
        Direction vectorDynamicDirection();
        Direction rasterDynamicDirection();

        int vectorDynamicShift();
        int rasterDynamicShift();

        void setVectorDynamicViewImage(QImage bg);
        void setRasterDynamicViewImage(QImage bg);

        // QImage rasterDynamicImage();
        bool vectorDynamicBgIsEmpty();
        bool vectorRenderIsPending();
        void scheduleVectorRender(bool status);
        void setVectorDynamicOpacity(double opacity);
        double vectorDynamicOpacity();

        bool vectorStaticBgIsEmpty();
        void setVectorStaticOpacity(double opacity);
        double vectorStaticOpacity();
        TupScene * scene();
        TupProject * project();

        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;

        void updateRasterBgImage(TupProject::Mode spaceContext, const QString &imgPath);
        bool rasterDynamicBgIsNull();
        bool rasterStaticBgIsNull();
        QPixmap rasterDynamicBackground();
        QPixmap rasterStaticBackground();

        bool rasterRenderIsPending();
        void renderRasterDynamicView();
        QPixmap rasterDynamicView(int photogram);

        void setRasterDynamicOpacity(double opacity);
        double rasterDynamicOpacity();
        void setRasterStaticOpacity(double opacity);
        double rasterStaticOpacity();

        QList<TupBackground::BgType> layerIndexes();
        void updateLayerIndexes(QList<TupBackground::BgType> indexes);

        QList<bool> layersVisibility();
        void updateLayersVisibility(QList<bool> viewFlags);

    private:
        int sceneIndex;
        QSize dimension;
        QColor bgColor;

        TupFrame *vectorDynamicBgFrame;
        TupFrame *vectorStaticBgFrame;
        TupFrame *rasterDynamicBgFrame;
        TupFrame *rasterStaticBgFrame;

        bool noVectorRender;
        bool noRasterRender;

        // Vector base images
        QImage vectorDynamicBgImg;
        QImage vectorStaticBgImg;
        // Vector expanded image
        QImage vectorDynamicViewImg;

        // Raster base images
        QPixmap rasterDynamicBgPix;
        QPixmap rasterStaticBgPix;
        // Raster expanded image
        QImage rasterDynamicViewImg;

        QList<BgType> bgLayerIndexes;
        QList<bool> bgVisibilityList;
};

#endif
