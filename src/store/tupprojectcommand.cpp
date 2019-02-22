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

#include "tupprojectcommand.h"
#include "tupcommandexecutor.h"
#include "tupprojectrequest.h"
#include "tuppaintareaevent.h"
#include "tuprequestparser.h"
#include "tupprojectresponse.h"
#include "tupsvg2qt.h"

#include <QVariant>

TupProjectCommand::TupProjectCommand(TupCommandExecutor *exec, const TupProjectRequest *request) : QUndoCommand()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProjectCommand()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    TupRequestParser parser;
    if (!parser.parse(request->getXml())) {
        #ifdef TUP_DEBUG
            QString msg = "TupProjectCommand::TupProjectCommand(): - Fatal error: request xml can't be parsed!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tFatal() << msg;
            #endif
        #endif
        return;
    }

    executor = exec;
    executed = false;
    response = parser.getResponse();
    response->setExternal(request->isRequestExternal());

    if (!response) {
        #ifdef TUP_DEBUG
            QString msg = "TupProjectCommand::TupProjectCommand() - Unparsed response!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tFatal() << msg;
            #endif
        #endif
    }

    initText();
}

TupProjectCommand::TupProjectCommand(TupCommandExecutor *exec, TupProjectResponse *res) : QUndoCommand()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProjectCommand()]";
        #else
            T_FUNCINFO;
        #endif
    #endif
	
    executor = exec;
    response = res;
    executed = false;

    initText();
}

void TupProjectCommand::initText()
{
    switch (response->getPart()) {
        case TupProjectRequest::Frame:
        {
            setText(actionString(response->getAction()) + " frame");
        }
        break;
        case TupProjectRequest::Layer:
        {
            setText(actionString(response->getAction()) + " layer");
        }
        break;
        case TupProjectRequest::Scene:
        {
            setText(actionString(response->getAction()) + " scene");
        }
        break;
        case TupProjectRequest::Item:
        {
            setText(actionString(response->getAction()) + " item");
        }
        break;
        case TupProjectRequest::Library:
        {
            setText(actionString(response->getAction()) + " symbol");
        }
        break;
        default:
        {				  
            #ifdef TUP_DEBUG
                QString msg = "TProjectCommand::initText() - Error: can't handle ID: " + QString::number(response->getPart());
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif
        }
        break;
    }
}

QString TupProjectCommand::actionString(int action)
{
    switch(action)
    {
        case TupProjectRequest::Add:
        {
            return QObject::tr("add");
        }
        break;
        case TupProjectRequest::Remove:
        {
            return QObject::tr("remove");
        }
        break;
        case TupProjectRequest::Move:
        {
            return QObject::tr("move");
        }
        break;
        case TupProjectRequest::ReverseSelection:
        {
            return QObject::tr("reverse");
        }
        break;
        case TupProjectRequest::Lock:
        {
            return QObject::tr("lock");
        }
        break;
        case TupProjectRequest::Rename:
        {
            return QObject::tr("rename");
        }
        break;
        case TupProjectRequest::Select:
        {
            return QObject::tr("select");
        }
        break;
        case TupProjectRequest::EditNodes:
        {
            return QObject::tr("edit node");
        }
        break;
        case TupProjectRequest::Pen:
        {
            return QObject::tr("pen");
        }
        break;
        case TupProjectRequest::Brush:
        {
            return QObject::tr("brush");
        }
        break;
        case TupProjectRequest::View:
        {
            return QObject::tr("view");
        }
        break;
        case TupProjectRequest::Transform:
        {
            return QObject::tr("transform");
        }
        break;
        case TupProjectRequest::Convert:
        {
            return QObject::tr("convert");
        }
        break;
    }
    
    return QString("Unknown");
}

TupProjectCommand::~TupProjectCommand()
{
    delete response;
}

void TupProjectCommand::redo()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProjectCommand::redo()] - Executing REDO action...";
            qDebug() << "[TupProjectCommand::redo()] - response->part(): " << response->part();
        #else
            T_FUNCINFO;
            tWarning() << "[TupProjectCommand::redo()] - Executing REDO action...";
            tWarning() << response->getPart();
        #endif
    #endif
	
    if (executed) {
        response->setMode(TupProjectResponse::Redo);
    } else {
        response->setMode(TupProjectResponse::Do);
        executed = true;
    }
    
    switch (response->getPart()) {
            case TupProjectRequest::Project:
            {
                #ifdef TUP_DEBUG
                     QString msg = "TupProjectCommand::redo() - Error: Project response isn't implemented";
                     #ifdef Q_OS_WIN
                         qWarning() << msg;
                     #else
                         tWarning() << msg;
                     #endif
                #endif
            }
            break;
            case TupProjectRequest::Frame:
            {
                 frameCommand();
            }
            break;
            case TupProjectRequest::Layer:
            {
                 layerCommand();
            }
            break;
            case TupProjectRequest::Scene:
            {
                 sceneCommand();
            }
            break;
            case TupProjectRequest::Item:
            {
                 itemCommand();
            }
            break;
            case TupProjectRequest::Library:
            {
                 libraryCommand();
            }
            break;
            default:
            {
                #ifdef TUP_DEBUG
                     QString msg = "TupProjectCommand::redo() - Error: Unknown project response";
                     #ifdef Q_OS_WIN
                         qDebug() << msg;
                     #else
                         tError() << msg;
                     #endif
                #endif
            }
            break;
    }
}

void TupProjectCommand::undo()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProjectCommand::undo()] - Executing UNDO action...";
        #else
            T_FUNCINFO << "[TupProjectCommand::undo()] - Executing UNDO action...";
        #endif
    #endif

    response->setMode(TupProjectResponse::Undo);
    switch (response->getPart()) {
            case TupProjectRequest::Project:
            {
                 #ifdef TUP_DEBUG
                     QString msg = "TupProjectCommand::undo() - Error: Project response isn't implemented";
                     #ifdef Q_OS_WIN
                         qDebug() << msg;
                     #else
                         tError() << msg;
                     #endif
                 #endif
            }
            break;
            case TupProjectRequest::Frame:
            {
                 frameCommand();
            }
            break;
            case TupProjectRequest::Layer:
            {
                 layerCommand();
            }
            break;
            case TupProjectRequest::Scene:
            {
                 sceneCommand();
            }
            break;
            case TupProjectRequest::Item:
            {
                 itemCommand();
            }
            break;
            case TupProjectRequest::Library:
            {
                 libraryCommand();
            }
            break;
            default:
            {
                 #ifdef TUP_DEBUG
                     QString msg = "TupProjectCommand::undo() - Error: Unknown project response";
                     #ifdef Q_OS_WIN
                         qDebug() << msg;
                     #else
                         tError() << msg;
                     #endif
                 #endif
            }
            break;
    }
}

void TupProjectCommand::frameCommand()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProjectCommand::frameCommand()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    TupFrameResponse *res = static_cast<TupFrameResponse *>(response);

    switch (res->getAction()) {
            case TupProjectRequest::Add:
            {
                 executor->createFrame(res);
            }
            break;
            case TupProjectRequest::Remove:
            {
                 executor->removeFrame(res);
            }
            break;
            case TupProjectRequest::RemoveSelection:
            {
                 executor->removeFrameSelection(res);
            }
            break;
            case TupProjectRequest::Reset:
            {
                 executor->resetFrame(res);
            }
            break;
            case TupProjectRequest::Exchange:
            {
                 executor->exchangeFrame(res);
            }
            break;
            case TupProjectRequest::Move:
            {
                 executor->moveFrame(res);
            }
            break;
            case TupProjectRequest::ReverseSelection:
            {
                 executor->reverseFrameSelection(res);
            }
            break;
            /*
            case TupProjectRequest::Lock:
            {
                 executor->lockFrame(res);
            }
            break;
            */
            case TupProjectRequest::Rename:
            {
                 executor->renameFrame(res);
            }
            break;
            case TupProjectRequest::Select:
            {
                 executor->selectFrame(res);
            }
            break;
            case TupProjectRequest::View:
            {
                 executor->setFrameVisibility(res);
            }
            break;
            case TupProjectRequest::Extend:
            {
                 executor->extendFrame(res);
            }
            break;
            /*
            case TupProjectRequest::Paste:
            {
                 executor->pasteFrame(res);
            }
            break;
            */
            case TupProjectRequest::CopySelection:
            {
                 executor->copyFrameSelection(res);
            }
            break;
            case TupProjectRequest::PasteSelection:
            {
                 executor->pasteFrameSelection(res);
            }
            break;
            default: 
            {
                 #ifdef TUP_DEBUG
                     QString msg = "TupProjectCommand::frameCommand() - Fatal Error: Unknown project request";
                     #ifdef Q_OS_WIN
                         qDebug() << msg;
                     #else
                         tError() << msg;
                     #endif
                 #endif
            }
            break;
    }
}

void TupProjectCommand::layerCommand()
{
    TupLayerResponse *res = static_cast<TupLayerResponse *>(response);

    switch (res->getAction()) {
            case TupProjectRequest::Add:
            {
                 executor->createLayer(res);
            }
            break;
            case TupProjectRequest::AddLipSync:
            {
                 executor->addLipSync(res);
            }
            break;
            case TupProjectRequest::UpdateLipSync:
            {
                 executor->updateLipSync(res);
            }
            break;
            case TupProjectRequest::Remove:
            {
                 executor->removeLayer(res);
            }
            break;
            case TupProjectRequest::RemoveLipSync:
            {
                 executor->removeLipSync(res);
            }
            break;
            case TupProjectRequest::Move:
            {
                 executor->moveLayer(res);
            }
            break;
            case TupProjectRequest::Lock:
            {
                 executor->lockLayer(res);
            }
            break;
            case TupProjectRequest::Rename:
            {
                 executor->renameLayer(res);
            }
            break;
            case TupProjectRequest::Select:
            {
                 executor->selectLayer(res);
            }
            break;
            case TupProjectRequest::View:
            {
                 executor->setLayerVisibility(res);
            }
            break;
            default: 
            {
                 #ifdef TUP_DEBUG
                     QString msg = "TupProjectCommand::layerCommand() - Error: Unknown project response";
                     #ifdef Q_OS_WIN
                         qDebug() << msg;
                     #else
                         tError() << msg;
                     #endif
                 #endif
            }
            break;
    }
}

void TupProjectCommand::sceneCommand()
{
    TupSceneResponse *res = static_cast<TupSceneResponse *>(response);

    switch (res->getAction()) {
	    // SQA: Check if this case is valid 
            case TupProjectRequest::GetInfo:
            {
                 executor->getScenes(res);
            }
	    break;
            case TupProjectRequest::Add:
            {
                 executor->createScene(res);
            }
            break;
            case TupProjectRequest::Remove:
            {
                 executor->removeScene(res);
            }
            break;
            case TupProjectRequest::Reset:
            {
                 executor->resetScene(res);
            }
            break;
            case TupProjectRequest::Move:
            {
                 executor->moveScene(res);
            }
            break;
            case TupProjectRequest::Lock:
            {
                 executor->lockScene(res);
            }
            break;
            case TupProjectRequest::Rename:
            {
                 executor->renameScene(res);
            }
            break;
            case TupProjectRequest::Select:
            {
                 executor->selectScene(res);
            }
            break;
            case TupProjectRequest::View:
            {
                 executor->setSceneVisibility(res);
            }
            break;
            case TupProjectRequest::BgColor:
            {
                 executor->setBgColor(res);
            }
            break;

            default: 
            {
                 #ifdef TUP_DEBUG
                     QString msg = "TupProjectCommand::sceneCommand() - Error: Unknown project response";
                     #ifdef Q_OS_WIN
                         qDebug() << msg;
                     #else
                         tError() << msg;
                     #endif
                 #endif
            }
            break;
    }
}

void TupProjectCommand::itemCommand()
{
    /*
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProjectCommand::itemCommand()]";
        #else
            T_FUNCINFO;
        #endif
    #endif
    */

    TupItemResponse *res = static_cast<TupItemResponse *>(response);

    switch (res->getAction()) {
            case TupProjectRequest::Add:
            {
                 executor->createItem(res);
            }
            break;
            case TupProjectRequest::Remove:
            {
                 executor->removeItem(res);
            }
            break;
            case TupProjectRequest::Move:
            {
                 executor->moveItem(res);
            }
            break;
            case TupProjectRequest::Lock:
            {
            }
            break;
            case TupProjectRequest::Rename:
            {
            }
            break;
            case TupProjectRequest::Convert:
            {
                 executor->convertItem(res);
            }
            break;
            case TupProjectRequest::EditNodes:
            {
                 executor->setPathItem(res);
            }
            break;
            case TupProjectRequest::Pen:
            {
                 executor->setPen(res);
            }
            break;
            case TupProjectRequest::Brush:
            {
                 executor->setBrush(res);
            }
            break;
            /*
            case TupProjectRequest::Select:
            {
            }
            break;
            case TupProjectRequest::View:
            {
            }
            break;
            */
            case TupProjectRequest::Transform:
            {
                 executor->transformItem(res);
            }
            break;
            case TupProjectRequest::Group:
            {
                 executor->groupItems(res);
            }
            break;
            case TupProjectRequest::Ungroup:
            {
                 executor->ungroupItems(res);
            }
            break;
            case TupProjectRequest::SetTween:
            {
                 executor->setTween(res);
            }
            break;
            case TupProjectRequest::UpdateTweenPath:
            {
                 executor->updateTweenPath(res);
            }
            break;
            default: 
            {
                 #ifdef TUP_DEBUG
                     QString msg = "TupProjectCommand::itemCommand() - Error: Unknown project response";
                     #ifdef Q_OS_WIN
                         qDebug() << msg;
                     #else
                         tError() << msg;
                     #endif
                 #endif
            }
            break;
    }
}

void TupProjectCommand::libraryCommand()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProjectCommand::libraryCommand()]";
        #else
            T_FUNCINFO;
        #endif
    #endif
    
    TupLibraryResponse *res = static_cast<TupLibraryResponse *>(response);

    switch (res->getAction()) {
            case TupProjectRequest::Add:
            {
                 executor->createSymbol(res);
            }
            break;

            case TupProjectRequest::Remove:
            {
                 executor->removeSymbol(res);
            }
            break;

            case TupProjectRequest::InsertSymbolIntoFrame:
            {
                 executor->insertSymbolIntoFrame(res);
            }
            break;

            case TupProjectRequest::RemoveSymbolFromFrame:
            {
                 executor->removeSymbolFromFrame(res);
            }
            break;

            default:
            {
                 #ifdef TUP_DEBUG
                     QString msg = "TupProjectCommand::libraryCommand() - Error: Unknown project response";
                     #ifdef Q_OS_WIN
                         qDebug() << msg;
                     #else
                         tError() << msg;
                     #endif
                 #endif
            }
            break;
    }
}

void TupProjectCommand::paintAreaCommand()
{
    // SQA: FIX ME in tupprojectcommand.cpp
	
    /*
     if (redo)
         executor->reemitEvent(response);
     else
         executor->reemitEvent(response);
    */
}
