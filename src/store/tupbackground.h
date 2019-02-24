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

typedef QList<TupFrame *> Frames;
typedef QList<QImage> Images;

class TUPITUBE_EXPORT TupBackground : public QObject, public TupAbstractSerializable
{
    Q_OBJECT

    public:
        enum Direction { Right = 0, Left = 1, Top, Bottom };
        TupBackground(TupScene *parent, const QSize dimension, const QColor bgColor);
        ~TupBackground();

        TupFrame* staticFrame(int layer);
        TupFrame* dynamicFrame(int layer);

        TupFrame * getCurrentDynamicFrame();
        void setCurrentDynamicLayer(int layer);
        int getCurrentDynamicLayer();
        TupFrame * getCurrentStaticFrame();
        void setCurrentStaticLayer(int layer);

        void setBgColor(const QColor color);
        void clear();

        void renderDynamicViews();
        QPixmap dynamicView(int layer, int frameIndex);
        void setDynamicDirection(int layer, int direction);
        void setDynamicShift(int layer, int shift);
        Direction dynamicDirection(int layer);
        int dynamicShift(int layer);
        QImage dynamicRaster(int layer);
        bool dynamicBgIsEmpty();
        bool dynamicLayerIsEmpty(int layer);

        bool rasterRenderIsPending();
        void scheduleRender(bool status);
        void setDynamicOpacity(int layer, double opacity);
        double dynamicOpacity(int layer);

        bool staticBgIsEmpty();
        void setStaticOpacity(int layer, double opacity);
        double staticOpacity(int layer);
        TupScene * scene();
        TupProject * project();

        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;

    private:
        void loadBgLayers(const QString &xml);
        void loadLegacyBgLayers(const QString &xml);

        QSize dimension;
        QColor bgColor;
        bool noRender;

        Frames dynamicFrames;
        Frames staticFrames;
        Images rasterBg;
        int currentDynamicLayer;
        int currentStaticLayer;
};

#endif
