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

#ifndef TUPSCENE_H
#define TUPSCENE_H

#include "tglobal.h"
#include "tupabstractserializable.h"
#include "tupproject.h"
#include "tupstoryboard.h"
#include "tupbackground.h"
#include "tupitemtweener.h"
#include "tuplipsync.h"

#include <QDomDocument>
#include <QDomElement>
#include <QGraphicsScene>
#include <QMap>
#include <QList>
#include <QPainter>
#include <QGraphicsItem>
#include <QStyleOptionGraphicsItem>
#include <QDir>
#include <QGraphicsView>

class TupLayer;
class TupSoundLayer;
class TupGraphicObject;
class TupSvgItem;
class TupBackground;
class TupItemTweener;

typedef QList<TupFrame *> Frames;
typedef QList<TupLayer *> Layers;
typedef QList<TupSoundLayer *> SoundLayers;
typedef QList<TupLipSync *> Mouths;

class TUPITUBE_EXPORT TupScene : public QObject, public TupAbstractSerializable
{
    Q_OBJECT

    public:
        TupScene(TupProject *parent, int index, const QSize dimension, const QColor bgColor);
        ~TupScene();

        void setSceneName(const QString &name);
        QString getSceneName() const;

        void setBgColor(const QColor bgColor);

        void setFPS(const int value);
        int getFPS();

        void setSceneLocked(bool isSceneLocked);
        bool isSceneLocked() const;

        void setVisibility(bool isSceneVisible);
        bool isSceneVisible() const;

        Layers getLayers() const;
        int layersCount() const;
        SoundLayers getSoundLayers() const;

        void setBasicStructure();

        TupLayer *layerAt(int position) const;
        TupSoundLayer *soundLayer(int position) const;

        void setLayers(const Layers &);

        bool removeLayer(int index);

        TupLayer *createLayer(QString name, int position, bool loaded = false);
        void addLayer(const QString &xml);

        TupSoundLayer *createSoundLayer(int position, bool loaded = false);

        bool restoreLayer(int index);

        bool moveLayer(int from, int to);

        int objectIndex() const;
        int visualIndexOf(TupLayer *layer) const;

        TupProject *project() const;
        void addTweenObject(int layerIndex, TupGraphicObject *object);
        void addTweenObject(int layerIndex, TupSvgItem *object);

        void updateTweenObject(int layerIndex, int objectIndex, TupGraphicObject *object);
        void updateTweenObject(int layerIndex, int objectIndex, TupSvgItem *object);

        void removeTweenObject(int layerIndex, TupGraphicObject *object);
        void removeTweenObject(int layerIndex, TupSvgItem *object);

        bool tweenExists(const QString &name, TupItemTweener::Type type);
        bool removeTween(const QString &name, TupItemTweener::Type type);

        TupItemTweener * tween(const QString &name, TupItemTweener::Type type);

        QList<QString> getTweenNames(TupItemTweener::Type type);
        QList<QGraphicsItem *> getItemsFromTween(const QString &name, TupItemTweener::Type type);

        // int getTotalTweens();

        // QList<TupGraphicObject *> tweeningGraphicObjects() const;
        QList<TupGraphicObject *> getTweeningGraphicObjects(int layerIndex) const;

        // QList<TupSvgItem *> tweeningSvgObjects() const;
        QList<TupSvgItem *> getTweeningSvgObjects(int layerIndex) const;

        int framesCount();

        TupBackground *sceneBackground();
        void setSceneBackground(TupBackground *bg);

        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;

        void removeTweensFromFrame(int layerIndex, int frameIndex);
        void reset(const QString &name);
        void clear();

        void setStoryboard(TupStoryboard *storyboardStructure);
        TupStoryboard * storyboardStructure();

        void insertStoryBoardScene(int index);
        void appendStoryBoardScene();
        void moveStoryBoardScene(int oldIndex, int newIndex);
        void resetStoryBoardScene(int index);
        void removeStoryBoardScene(int index);

        QList<QString> getLipSyncNames();
        bool lipSyncExists(const QString &name);
        int getLipSyncLayerIndex(const QString &name);
        TupLipSync * getLipSync(const QString &name);
        bool updateLipSync(TupLipSync *lipsync);
        bool removeLipSync(const QString &name);
        int lipSyncTotal();
        Mouths getLipSyncList();
        int totalPhotograms();

        void updateRasterBackground(TupProject::Mode spaceContext, const QString &imgPath);

    private:
        void removeTweensFromLayer(int layerIndex);

        int sceneIndex;
        QSize dimension;
        QColor bgColor;
        int fps;
        TupStoryboard *storyboard;
        TupBackground *background;
        Layers layers;
        Layers undoLayers;
        SoundLayers soundLayers;
        QString sceneName;
        bool isLocked;
        int layerCount;
        bool isVisible;

        QList<TupGraphicObject *> tweeningGraphicObjects;
        QList<TupSvgItem *> tweeningSvgObjects;
};

#endif
