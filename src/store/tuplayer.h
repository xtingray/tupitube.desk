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

#ifndef TUPLAYER_H
#define TUPLAYER_H

#include "tglobal.h"
#include "tupabstractserializable.h"
#include "tupframe.h"
#include "tupgraphicobject.h"
#include "tuplipsync.h"

#include <QDomDocument>
#include <QDomElement>
#include <QList>
#include <QTextStream>

typedef QList<TupFrame *> Frames;
typedef QList<TupLipSync *> Mouths;

class TupScene;
class TupProject;

class TUPITUBE_EXPORT TupLayer : public QObject, public TupAbstractSerializable
{
    Q_OBJECT

    public:
        TupLayer();
        TupLayer(TupScene *parentScene, int index = 0);
        ~TupLayer();
        
        Frames getFrames();
        void setFrames(const Frames &getFrames);
        void setFrame(int index, TupFrame *frame);
        
        void setLayerName(const QString &name);
        QString getLayerName() const;
        
        void setLocked(bool isLayerLocked);
        bool isLayerLocked() const;

        void setLayerVisibility(bool isLayerVisible);
        bool isLayerVisible() const;

        void setOpacity(qreal getOpacity);
        qreal getOpacity();
        
        TupFrame *createFrame(QString name, int position, bool loaded = false);
        bool restoreFrame(int index);
        bool removeFrame(int position);
        bool resetFrame(int position);
        bool restoreResettedFrame(int pos);
        int resettedFramesCount();
        void clear();

        bool moveFrame(int from, int to);
        bool exchangeFrame(int from, int to);
        bool extendFrame(int pos, int times);
        
        TupFrame *frameAt(int position) const;

        TupLipSync *createLipSync(const QString &name, const QString &soundFile, int initFrame);
        void addLipSync(TupLipSync *lipsync);
        int lipSyncCount();
        Mouths getLipSyncList();
        bool removeLipSync(const QString &name);
        
        TupScene *parentScene() const;
        TupProject *parentProject() const;

        void updateLayerIndex(int index);
        int layerIndex();
        int visualIndexOf(TupFrame *frame) const;
        int objectIndex() const;
        int framesCount() const;

        void addTweenObject(TupGraphicObject *object);
        void addTweenObject(TupSvgItem *object);
        void updateTweenObject(int index, TupGraphicObject *object);
        void updateTweenObject(int index, TupSvgItem *object);
        void removeTweenObject(TupGraphicObject *object);
        void removeTweenObject(TupSvgItem *object);
        QList<TupGraphicObject *> getTweeningGraphicObjects() const;
        QList<TupSvgItem *> getTweeningSvgObjects() const;
        bool tweenExists(const QString &name, TupItemTweener::Type type);
        bool removeTween(const QString &name, TupItemTweener::Type type);
        void removeAllTweens();
        void removeTweensFromFrame(int frameIndex);
        // int tweensCount();

    public:
        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;
        
    private:
        TupScene *scene;
        Frames frames;
        Frames undoFrames;
        Frames resettedFrames;
        Mouths lipsyncList;
        bool isVisible;
        QString layerName;
        int framesCounter;
        bool isLocked;
        int index;
        qreal opacity;

        QList<TupGraphicObject *> tweeningGraphicObjects;
        QList<TupSvgItem *> tweeningSvgObjects;
};

#endif
