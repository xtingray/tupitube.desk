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

#ifndef TUPPROJECTREQUEST_H
#define TUPPROJECTREQUEST_H

#include "tglobal.h"

#include <QObject>
#include <QString>
#include <QVariant>

class TupProjectRequest;
class TupProjectResponse;

class TUPITUBE_EXPORT TupProjectRequestArgument
{
    public:
        TupProjectRequestArgument();
        TupProjectRequestArgument(const QString &v);
        ~TupProjectRequestArgument();
        
        void operator = (const QString &value);
        void setValue(const QString &value);
        
        bool toBool();
        int toInt();
        double toReal();
        QString toString();
        
    private:
        QString m_value;
};

class TUPITUBE_EXPORT TupProjectRequest
{
    public:
        enum Action
        {
            None = 0,
            Add = 1,
            Remove = -Add,
            RemoveSelection = 2, 
            Reset = 3,
            Exchange = 4,
            Move = 5,
            Lock = 6,
            Rename = 7,
            Select = 8,
            View = 9,
            
            Group = 10,
            Ungroup = -Group,
            
            // Items
            Transform = 11,
            Convert = 12,
            EditNodes = 13,
            Pen = 14,
            Brush = 15,
            InsertSymbolIntoFrame = 16,
            RemoveSymbolFromFrame = 17,

            SetTween = 18,
            UpdateTween = 19,
            RemoveTween = 20,
            UpdateTweenPath = 21,
            
            // frames
            Update = 22,
            Extend = 23,
            Copy = 24,
            CopySelection = 25,
            Paste = 26,
            PasteSelection = 27,
            ReverseSelection = 28,

            // scenes
            GetInfo = 29,
            BgColor = 30,

            // layer
            AddLipSync = 31,
            UpdateLipSync = 32,
            RemoveLipSync = 33,
            UpdateOpacity = 34,

            // Raster
            AddRasterItem,
            RemoveRasterItem,
            ClearRasterCanvas
        };
        
        enum Part
        {
            Project = 1000,
            Scene,
            Layer,
            Frame,
            Item,
            Library
        };
        
        TupProjectRequest(const QString &data = nullptr);
        virtual ~TupProjectRequest();
        
        void setId(int getId);
        virtual int getId() const;
        virtual bool isValid() const;
        
        QString getXml() const;
        
        void setExternal(bool b);
        bool isRequestExternal() const;
        
        TupProjectRequest &operator = (const TupProjectRequest &other);

    private:
        QString xml;
        int id;
        bool isExternal;
};

#endif
