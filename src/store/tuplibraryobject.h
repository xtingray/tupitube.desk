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

#ifndef TUPLIBRARYOBJECT_H
#define TUPLIBRARYOBJECT_H

#include "tglobal.h"
#include "tupabstractserializable.h"

#include <QVariant>
#include <QGraphicsItem>
#include <QTemporaryFile>

class TupLibraryObject;

class TUPITUBE_EXPORT TupLibraryObject : public QObject, public TupAbstractSerializable
{
    public:
        enum ObjectType
        {
            None = -1,
            Item = 1,
            Image,
            Audio,
            Svg,
            Video,
            Folder
        };

        enum ItemType
        {
            Path = 0x01,
            Rect,
            Ellipse,
            Text,
            Group
        };

        TupLibraryObject(QObject *parent = nullptr);
        TupLibraryObject(const QString &name, const QString &folder,
                         TupLibraryObject::ObjectType type, QObject *parent = nullptr);
        ~TupLibraryObject();

        void setObjectType(TupLibraryObject::ObjectType type);
        TupLibraryObject::ObjectType getObjectType() const;

        void setItemType(TupLibraryObject::ItemType type);
        TupLibraryObject::ItemType getItemType() const;

        void setData(const QVariant &data);
        QVariant getData() const;

        void setDataPath(const QString &path);
        QString getDataPath() const;

        void setSymbolName(const QString &name);
        QString getSymbolName() const;

        void setFolder(const QString &folder);
        QString getFolder() const;

        void enableMute(bool flag);
        bool isMuted();

        void updateFolder(const QString &projectPath, const QString &folder = QString());

        int frameToPlay();
        void updateFrameToPlay(int frame);

        SoundType getSoundType();
        void setSoundType(SoundType type);
        SoundResource getSoundResourceParams();

        QString getShortId() const;
        QString getExtension() const;
        
        bool loadRawData(const QByteArray &data);
        bool loadDataFromPath(const QString &dataDir);
        bool loadData(const QString &path);
        
        bool saveData(const QString &dataDir);

        QString toString() const;

        static QPixmap renderImage(const QString &xml, int width);
        static QPixmap generateImage(QGraphicsItem *item, int width);
        static QPixmap generateImage(const QString &xml, int width);

        TupLibraryObject * clone();

    public:
        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;
        
    private:
        TupLibraryObject::ObjectType objectType;
        TupLibraryObject::ItemType itemType;
        QVariant data;
        QString dataPath;
        QString symbolName;
        QString folder;
        QString shortId;
        QString extension;
        QByteArray rawData;
        QString xmlString;

        SoundType soundType;
        bool mute;
        int playAt;
};

#endif
