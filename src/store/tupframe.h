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

#ifndef TUPFRAME_H
#define TUPFRAME_H

#include "tglobal.h"
#include "tupabstractserializable.h"
#include "tupsvgitem.h"
#include "tupbackground.h"
#include "tupgraphiclibraryitem.h"

#include <QGraphicsScene>
#include <QDomDocument>
#include <QDomElement>
#include <QList>

class TupFrame;
class TupLayer;
class TupBackground;
class TupGraphicObject;
class TupSvgItem;
class TupProject;
class TupScene;

typedef QList<TupGraphicObject *> GraphicObjects;
typedef QList<TupSvgItem *> SvgObjects;

class TUPITUBE_EXPORT TupFrame : public QObject, public TupAbstractSerializable
{
    Q_OBJECT

    public:
        enum FrameType { DynamicBg = 0, StaticBg, Regular };
        enum MoveItemType { MoveBack, MoveToFront, MoveOneLevelBack, MoveOneLevelToFront };

        TupFrame();
        TupFrame(TupLayer *parent);
        TupFrame(TupBackground *bg, const QString &label);
       
        ~TupFrame();
       
        void setFrameName(const QString &name);
        QString getFrameName() const;

        void setRasterBgImage(QImage *image, int index);
        QImage * getBgRasterImage();
        int getBgRasterImageIndex();
        void setRasterIndex(int index);

        void setDynamicDirection(const QString &direction);
        void setDynamicShift(const QString &shift);
        TupBackground::Direction dynamicDirection() const;
        int dynamicShift() const;

        void setLocked(bool isFrameLocked);
        bool isFrameLocked() const;
       
        void setVisible(bool isFrameVisible);
        bool isFrameVisible() const;

        void setFrameOpacity(double frameOpacity);
        double frameOpacity();

        TupFrame::FrameType frameType();
      
        void addLibraryItem(const QString &id, TupGraphicLibraryItem *libraryItem);
        void addItem(const QString &id, QGraphicsItem *item);
        bool removeImageItemFromFrame(const QString &id);
        void updateIdFromFrame(const QString &oldId, const QString &newId);

        void addSvgItem(const QString &id, TupSvgItem *item);
        bool removeSvgItemFromFrame(const QString &id);
        void updateSvgIdFromFrame(const QString &oldId, const QString &newId);

        void replaceItem(int position, QGraphicsItem *item);
        bool moveItem(TupLibraryObject::Type frameType, int currentPosition, int action);
      
        bool removeGraphic(int position);
        bool removeGraphicAt(int position);
        void restoreGraphic();

        bool removeSvg(int position);
        bool removeSvgAt(int position);
        void restoreSvg();

        QGraphicsItem *createItem(QPointF coords, const QString &xml, bool loaded = false);
        TupSvgItem *createSvgItem(QPointF coords, const QString &xml, bool loaded = false);

        void setGraphics(GraphicObjects objects);       
        void setSvgObjects(SvgObjects objects);
        GraphicObjects graphicItems() const;
        SvgObjects svgItems() const; 
       
        TupGraphicObject *graphicAt(int position) const;
        TupSvgItem *svgAt(int position) const; 
        QGraphicsItem *item(int position) const;
       
        int createItemGroup(int position, QList<int> group);
        QList<QGraphicsItem *> splitGroup(int position);
             
        TupLayer *parentLayer() const;
        TupScene *parentScene() const;
        TupProject *parentProject() const;
       
        int indexOf(TupGraphicObject *object) const;
        int indexOf(QGraphicsItem *item) const;
        int indexOf(TupSvgItem *item) const;
       
        int index() const;
       
        void reset();
        void clear();
        int graphicsCount();
        int svgItemsCount();
        int itemsTotalCount();

        int getTopZLevel();

        bool isEmpty();

        void reloadGraphicItem(const QString &id, const QString &path);
        void reloadSVGItem(const QString &id, TupLibraryObject *object);
        void updateZLevel(int zLevelIndex);

        void checkTransformationStatus(TupLibraryObject::Type itemType, int index);
        void storeItemTransformation(TupLibraryObject::Type itemType, int index, const QString &properties);
        void undoTransformation(TupLibraryObject::Type itemType, int index);
        void redoTransformation(TupLibraryObject::Type itemType, int index);

       void checkBrushStatus(int itemIndex);
       void setBrushAtItem(int itemIndex, const QString &xml);
       void redoBrushAction(int itemIndex);
       void undoBrushAction(int itemIndex);

       void checkPenStatus(int itemIndex);
       void setPenAtItem(int itemIndex, const QString &xml);
       void redoPenAction(int itemIndex);
       void undoPenAction(int itemIndex);
       
    public:
       virtual void fromXml(const QString &xml);
       virtual QDomElement toXml(QDomDocument &doc) const;
       
    private:
       void insertItem(int position, QGraphicsItem *item, const QString &label);
       void insertObject(int position, TupGraphicObject *object, const QString &label);
       void insertSvg(int position, TupSvgItem *item, const QString &label);

       TupLayer *layer;
       QString frameName;
       FrameType type;
       bool isLocked;
       bool isVisible;

       QString direction;
       QString shift;

       GraphicObjects graphics;
       QList<QString> objectIndexes;

       GraphicObjects itemsUndoList;
       QList<QString> objectUndoIndexes;
       QList<int> objectUndoPos;

       SvgObjects svg;
       QList<QString> svgIndexes;

       SvgObjects svgUndoList;
       QList<QString> svgUndoIndexes;
       QList<int> svgUndoPos;

       int zLevelIndex;
       double opacity;

       QImage *bgRasterImage;
       int bgRasterImageIndex;
};

#endif
