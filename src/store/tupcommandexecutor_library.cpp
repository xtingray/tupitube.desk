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

#include "tupcommandexecutor.h"
#include "tupproject.h"
#include "tupscene.h"
#include "tuplayer.h"
#include "tupframe.h"
#include "tupprojectrequest.h"
#include "tupprojectresponse.h"

// This method adds an object into the library

bool TupCommandExecutor::createSymbol(TupLibraryResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCommandExecutor::createSymbol()]";
        qDebug() << "*** Creating object: " << response->getArg().toString();
    #endif

    if (response->symbolType() == TupLibraryObject::Folder) {
        if (project->addFolder(response->getArg().toString())) {
            emit responsed(response);
            return true;
        }
    } else {
        if (response->getMode() == TupProjectResponse::Do) {
            if (project->createSymbol(response->symbolType(), response->getArg().toString(), response->getData(), response->getParent())) {
                emit responsed(response);
                return true;
            } 
        } else {
            // SQA: Undo/Redo actions must be implemented here 
        }
    }

    return false;
}

// This method removes an object from the library

bool TupCommandExecutor::removeSymbol(TupLibraryResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCommandExecutor::removeSymbol()]";
    #endif

    if (response->symbolType() == TupLibraryObject::Folder) {
        if (project->removeFolder(response->getArg().toString())) {
            emit responsed(response);
            return true;
        }
    } else {
        if (response->symbolType() == TupLibraryObject::Sound) {
            if (project->removeSound(response->getArg().toString())) {
                emit responsed(response);
                return true;
            }
        } else {
            if (project->removeSymbol(response->getArg().toString(), response->symbolType())) {
                emit responsed(response);
                return true;
            }
        }
    }

    return false;
}

// This method inserts an object into one frame 

bool TupCommandExecutor::insertSymbolIntoFrame(TupLibraryResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCommandExecutor::insertSymbolIntoFrame()] - Adding symbol to project -> " << response->getArg().toString();
    #endif

    if (project->scenesCount() > 0) {
        if (project->insertSymbolIntoFrame(response->getSpaceMode(), response->getArg().toString(), 
            response->getSceneIndex(), response->getLayerIndex(), response->getFrameIndex())) {
            emit responsed(response);
            return true;
        } 
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupCommandExecutor::insertSymbolIntoFrame()] - No scenes available!";
        #endif
    }

    return false;
}

// This method removes an object from one frame

bool TupCommandExecutor::removeSymbolFromFrame(TupLibraryResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCommandExecutor::removeSymbolFromFrame()]";
    #endif

    if (project->scenesCount() > 0) {
        if (project->removeSymbolFromFrame(response->getArg().toString(), response->symbolType())) {
            TupScene *scene = project->sceneAt(response->getSceneIndex());
            if (scene) {
                TupLayer *layer = scene->layerAt(response->getLayerIndex());
                if (layer) {
                    TupFrame *frame = layer->frameAt(response->getFrameIndex());
                    if (frame) 
                        response->setFrameState(frame->isEmpty());
                }
            }
            emit responsed(response);
            return true;
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "TupCommandExecutor::removeSymbolFromFrame() - Error: Symbol can't be removed from project!";
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "TupCommandExecutor::removeSymbolFromFrame() - No scenes available!";
        #endif
    }

    return false;
}
