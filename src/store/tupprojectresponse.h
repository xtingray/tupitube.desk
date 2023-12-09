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

#ifndef TUPPROJECTRESPONSE_H
#define TUPPROJECTRESPONSE_H

#include "tglobal.h"
#include "tupprojectrequest.h"
#include "tupscene.h"
#include "tuplibraryobject.h"

class TUPITUBE_EXPORT TupProjectResponse
{
    public:
        enum Mode
             {
               None = 0x00,
               Do,
               Redo,
               Undo
             };

        TupProjectResponse(int part, int action);
        virtual ~TupProjectResponse();

        int getPart() const;
        int getAction() const;

        int originalAction() const;

        void setArg(const QString &value);
        void setData(const QByteArray &data);
        void setMode(Mode mode);

        void setExternal(bool e);
        bool external() const;

        Mode getMode() const;

        TupProjectRequestArgument getArg() const;
        QByteArray getData() const;

        void toString();

    private:
        int part;
        int action;
        TupProjectRequestArgument arg;
        QByteArray data;
        Mode mode;
        bool isExternal;
};

class TUPITUBE_EXPORT TupSceneResponse : public TupProjectResponse
{
    public:
        TupSceneResponse(int part, int action);
        ~TupSceneResponse();

        int getSceneIndex() const;
        void setSceneIndex(int index);

        void setState(const QString &state);
        QString getState() const;
        void setScenes(Scenes scenes);

    private:
        int m_sceneIndex;
        QString m_state;
        Scenes m_scenes;
};

class TUPITUBE_EXPORT TupLayerResponse : public TupSceneResponse
{
    public:
        TupLayerResponse(int part, int action);
        ~TupLayerResponse();

        int getLayerIndex() const;
        void setLayerIndex(int index);

    private:
        int m_layerIndex;
};

class TUPITUBE_EXPORT TupFrameResponse : public TupLayerResponse
{
    public:
        TupFrameResponse(int part, int action);
        ~TupFrameResponse();

        int getFrameIndex() const;
        void setFrameIndex(int index);
        bool frameIsEmpty();
        void setFrameState(bool state);

    private:
        int m_frameIndex;
        bool empty;
};

class TUPITUBE_EXPORT TupItemResponse : public TupFrameResponse
{
    public:
        TupItemResponse(int part, int action);
        ~TupItemResponse();

        int getItemIndex() const;
        void setItemIndex(int index);
        TupLibraryObject::ObjectType getItemType() const;
        void setItemType(TupLibraryObject::ObjectType type);
        QPointF position() const;
        void setPosX(double coord);
        void setPosY(double coord);
        TupProject::Mode spaceMode();
        void setSpaceMode(TupProject::Mode mode);
        bool frameIsEmpty();
        void setFrameState(bool state);

    private:
        int m_itemIndex;
        TupLibraryObject::ObjectType m_itemType;
        double m_x;
        double m_y;
        TupProject::Mode m_mode;
        bool empty;
};

class TUPITUBE_EXPORT TupLibraryResponse : public TupFrameResponse
{
    public:
        TupLibraryResponse(int part, int action);
        ~TupLibraryResponse();

        void setSymbolType(TupLibraryObject::ObjectType symtype);
        TupLibraryObject::ObjectType symbolType() const;
        void setParent(const QString top);
        QString getParent() const;
        TupProject::Mode getSpaceMode();
        void setSpaceMode(TupProject::Mode mode);
        bool frameIsEmpty();
        void setFrameState(bool state);

    private:
        TupLibraryObject::ObjectType m_symbolType;
        QString parentNode;
        TupProject::Mode m_mode;
        bool empty;
};

class TUPITUBE_EXPORT TupProjectResponseFactory
{
    private:
        TupProjectResponseFactory();

    public:
        ~TupProjectResponseFactory();
        static TupProjectResponse *create(int part, int action);
};

#endif
