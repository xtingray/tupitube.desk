/***************************************************************************
 *   Project TUPITUBE DESK                                                 *
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

#include "tupprojectresponse.h"

TupProjectResponse::TupProjectResponse(int p, int a) : part(p), action(a)
{
}

TupProjectResponse::~TupProjectResponse()
{
}

int TupProjectResponse::getPart() const
{
    return part;
}

int TupProjectResponse::getAction() const
{
    if (mode == Undo) {
        switch (action) {
            case TupProjectRequest::Add:
                 {
                    return TupProjectRequest::Remove;
                 }
            break;
            case TupProjectRequest::Remove:
                 {
                    return TupProjectRequest::Add;
                 }
            break;
            case TupProjectRequest::InsertSymbolIntoFrame:
                 {
                    return TupProjectRequest::RemoveSymbolFromFrame;
                 }
            break;
            case TupProjectRequest::Ungroup:
                 {
                    return TupProjectRequest::Group;
                 }
            break;
            case TupProjectRequest::Group:
                 {
                    return TupProjectRequest::Ungroup;
                 }
            break;
            default:
                 {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupProjectResponse::action()] - Returning same action as UNDO respoonse ->" << action;
                    #endif
                 }
            break;
        }
    }

    return action;
}

int TupProjectResponse::originalAction() const
{
    return action;
}

void TupProjectResponse::setMode(Mode value)
{
    mode = value;
}

void TupProjectResponse::setExternal(bool ext)
{
    isExternal = ext;
}

bool TupProjectResponse::external() const
{
    return isExternal;
}

TupProjectResponse::Mode TupProjectResponse::getMode() const
{
    return mode;
}

void TupProjectResponse::setArg(const QString &value)
{
    arg = value;
}

void TupProjectResponse::setData(const QByteArray &input)
{
    data = input;
}

TupProjectRequestArgument TupProjectResponse::getArg() const
{
    return arg;
}

QByteArray TupProjectResponse::getData() const
{
    return data;
}

void TupProjectResponse::toString()
{
    qDebug() << "---";
    qDebug() << "  Project Response";
    qDebug() << "  *** Action: " + QString::number(getAction());
    qDebug() << "  *** Mode: " + QString::number(getMode());
    qDebug() << "  *** Args: " + getArg().toString();
    qDebug() << "  *** Part: " + QString::number(part);
    qDebug() << "  *** isExternal: " + QString::number(isExternal);
    qDebug() << "  *** Data: " + QString(data);
    qDebug() << "---";
}

// SCENE

TupSceneResponse::TupSceneResponse(int part, int action) : TupProjectResponse(part, action), m_sceneIndex(-1)
{
}

TupSceneResponse::~TupSceneResponse()
{
}

int TupSceneResponse::getSceneIndex() const
{
    return m_sceneIndex;
}

void TupSceneResponse::setSceneIndex(int index)
{
    m_sceneIndex = index;
}

void TupSceneResponse::setState(const QString &state)
{
    m_state = state;
}

QString TupSceneResponse::getState() const
{
    return m_state;
}

void TupSceneResponse::setScenes(Scenes scenes) 
{
    m_scenes = scenes;
}

// LAYER

TupLayerResponse::TupLayerResponse(int part, int action) : TupSceneResponse(part, action), m_layerIndex(-1)
{
}

TupLayerResponse::~TupLayerResponse()
{
}

int TupLayerResponse::getLayerIndex() const
{
    return m_layerIndex;
}

void TupLayerResponse::setLayerIndex(int index)
{
    m_layerIndex = index;
}

// FRAME

TupFrameResponse::TupFrameResponse(int part, int action) : TupLayerResponse(part, action), m_frameIndex(-1)
{
}

TupFrameResponse::~TupFrameResponse()
{
}

int TupFrameResponse::getFrameIndex() const
{
    return m_frameIndex;
}

void TupFrameResponse::setFrameIndex(int index)
{
    m_frameIndex = index;
}

bool TupFrameResponse::frameIsEmpty()
{
    return empty;
}

void TupFrameResponse::setFrameState(bool state)
{
    empty = state;
}

// ITEM

TupItemResponse::TupItemResponse(int part, int action) : TupFrameResponse(part, action), m_itemIndex(-1)
{
}

TupItemResponse::~TupItemResponse()
{
}

int TupItemResponse::getItemIndex() const
{
    return m_itemIndex;
}

void TupItemResponse::setItemIndex(int index)
{
    m_itemIndex = index;
}

TupLibraryObject::ObjectType TupItemResponse::getItemType() const
{
    return m_itemType;
}

void TupItemResponse::setItemType(TupLibraryObject::ObjectType type)
{
    m_itemType = type;
}

QPointF TupItemResponse::position() const
{
    return QPointF(m_x, m_y);
}

void TupItemResponse::setPosX(double coord)
{
    m_x = coord;
}

void TupItemResponse::setPosY(double coord)
{
    m_y = coord;
}

TupProject::Mode TupItemResponse::spaceMode()
{
    return m_mode;
}

void TupItemResponse::setSpaceMode(TupProject::Mode mode)
{
    m_mode = mode;
}

bool TupItemResponse::frameIsEmpty()
{
    return empty;
}

void TupItemResponse::setFrameState(bool state)
{
    empty = state;
}

TupLibraryResponse::TupLibraryResponse(int part, int action) : TupFrameResponse(part, action)
{
}

TupLibraryResponse::~TupLibraryResponse()
{
}

void TupLibraryResponse::setSymbolType(TupLibraryObject::ObjectType symtype)
{
    m_symbolType = symtype;
}

TupLibraryObject::ObjectType TupLibraryResponse::symbolType() const
{
    return m_symbolType;
}

void TupLibraryResponse::setParent(const QString top)
{
    parentNode = top;
}

QString TupLibraryResponse::getParent() const
{
    return parentNode;
}

TupProject::Mode TupLibraryResponse::getSpaceMode()
{
    return m_mode;
}

void TupLibraryResponse::setSpaceMode(TupProject::Mode mode)
{
    m_mode = mode;
}

bool TupLibraryResponse::frameIsEmpty()
{
    return empty;
}

void TupLibraryResponse::setFrameState(bool state)
{
    empty = state;
}

TupProjectResponseFactory::TupProjectResponseFactory()
{
}

TupProjectResponseFactory::~TupProjectResponseFactory()
{
}

TupProjectResponse *TupProjectResponseFactory::create(int part, int action)
{
    switch (part) {
        case TupProjectRequest::Scene:
        {
            return new TupSceneResponse(part, action);
        }
        break;
        case TupProjectRequest::Layer:
        {
            return new TupLayerResponse(part, action);
        }
        break;
        case TupProjectRequest::Frame:
        {
            return new TupFrameResponse(part, action);
        }
        break;
        case TupProjectRequest::Item:
        {
            return new TupItemResponse(part, action);
        }
        break;
        case TupProjectRequest::Library:
        {
            return new TupLibraryResponse(part, action);
        }
        break;
        default:
        {
            #ifdef TUP_DEBUG
                qDebug() << "[TupProjectResponseFactory::create()] - Error: Unknown/Unhandled element -> " << part;
            #endif
        }
        break;
    }
    
    return new TupProjectResponse(part, action);
}
