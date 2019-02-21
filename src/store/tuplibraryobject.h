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
#include <QDir>

class TupLibraryObject;

class TUPITUBE_EXPORT TupLibraryObject : public QObject, public TupAbstractSerializable
{
    public:
        enum Type
        {
            // Item = 0x01,
            Item = 1,
            Image,
            Sound,
            Svg,
            Text,
            Folder 
        };
        
        TupLibraryObject(QObject *parent = 0);
        TupLibraryObject(const QString &name, const QString &getFolder, TupLibraryObject::Type getType, QObject *parent = 0);
        ~TupLibraryObject();
        
        void setType(TupLibraryObject::Type getType);
        TupLibraryObject::Type getType() const;
        
        void setData(const QVariant &getData);
        QVariant getData() const;
       
        void setDataPath(const QString &path);
        QString getDataPath() const;
        
        void setSymbolName(const QString &name);
        QString getSymbolName() const;

        void setFolder(const QString &getFolder);
        QString getFolder() const;

        void updateFolder(const QString &getFolder);

        int frameToPlay();
        void updateFrameToPlay(int frame);

        QString getSmallId() const;
        QString getExtension() const;
        
        bool loadRawData(const QByteArray &getData);
        bool loadDataFromPath(const QString &dataDir);
        bool loadData(const QString &path);
        
        bool saveData(const QString &dataDir);

        void setSoundEffectFlag(bool flag);
        bool isSoundEffect();

    public:
        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;
        
    private:
        TupLibraryObject::Type objectType;
        QVariant data;
        QString dataPath;
        QString symbolName;
        QString folder;
        QString smallId;
        QString extension;
        QByteArray rawData;

        bool objectHasSoundEffect;
        int playAt;
};

#endif
