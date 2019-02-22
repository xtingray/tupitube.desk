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

#include "tupprojectmanager.h"
#include "tupproject.h"
#include "tupscene.h"
#include "tuplayer.h"
#include "tupframe.h"

#include "tupprojectrequest.h"
#include "tupprojectcommand.h"
#include "tupcommandexecutor.h"
#include "tupprojectmanagerparams.h"
#include "tupabstractprojectmanagerhandler.h"
#include "tupprojectresponse.h"
#include "tuprequestbuilder.h"
#include "tuprequestparser.h"
#include "talgorithm.h"

// This class handles the current animation project 

TupProjectManager::TupProjectManager(QObject *parent) : QObject(parent)
{	
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProjectManager()]";
        #else
            TINIT;
        #endif
    #endif
    
    isModified = false;
    handler = 0;

    project = new TupProject(this);
    undoStack = new QUndoStack(this);
    commandExecutor = new TupCommandExecutor(project);

    connect(commandExecutor, SIGNAL(responsed(TupProjectResponse*)), this, SLOT(emitResponse(TupProjectResponse *)));
    connect(project, SIGNAL(responsed(TupProjectResponse*)), this, SIGNAL(responsed(TupProjectResponse *)));
}

TupProjectManager::~TupProjectManager()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[~TupProjectManager()]";
        #else
            TEND;
        #endif
    #endif

    delete handler;
    delete undoStack;
    delete params;
    delete commandExecutor;
}

void TupProjectManager::setParams(TupProjectManagerParams *parameters)
{
    // if (params)
    //     delete params;

    params = parameters;
    handler->initialize(params);
}

TupProjectManagerParams *TupProjectManager::getParams() const
{
    return params;
}

void TupProjectManager::setHandler(TupAbstractProjectHandler *pHandler, bool networked)
{
    if (handler) {
        disconnect(handler, SIGNAL(sendCommand(const TupProjectRequest *, bool)),
                   this, SLOT(createCommand(const TupProjectRequest *, bool)));
        disconnect(handler, SIGNAL(sendLocalCommand(const TupProjectRequest *)),
                   this, SLOT(handleLocalRequest(const TupProjectRequest *)));
        delete handler;
        handler = 0;
    }

    handler = pHandler;
    handler->setParent(this);
    handler->setProject(project);

    connect(handler, SIGNAL(sendCommand(const TupProjectRequest *, bool)), this, SLOT(createCommand(const TupProjectRequest *, bool)));
    connect(handler, SIGNAL(sendLocalCommand(const TupProjectRequest *)), this, SLOT(handleLocalRequest(const TupProjectRequest *)));

    isNetworked = networked;
}

TupAbstractProjectHandler *TupProjectManager::getHandler() const
{
    return handler;
}

void TupProjectManager::setupNewProject()
{	
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProjectManager::setupNewProject()]";
        #else
            T_FUNCINFO;
        #endif
    #endif
	
    if (!handler || !params) {
        #ifdef TUP_DEBUG
            QString msg = "TupProjectManager::setupNewProject() - Error: No handler available or no params!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return;
    }

    closeProject();

    project->setProjectName(params->projectName());
    project->setAuthor(params->author());
    project->setTags(params->tags());
    project->setDescription(params->description());
    project->setBgColor(params->bgColor());
    project->setDimension(params->dimension());
    project->setFPS(params->fps());

    if (! handler->setupNewProject(params)) {
        #ifdef TUP_DEBUG
            QString msg = "TupProjectManager::setupNewProject() - Error: Project params misconfiguration";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif		
        return;
    }

    if (!isNetworked) {
        QString projectPath = CACHE_DIR + params->projectName();
        QDir projectDir(projectPath); 
        if (projectDir.exists())
            removeProjectPath(projectPath);

        project->setDataDir(projectPath);

        TupProjectRequest request = TupRequestBuilder::createSceneRequest(0, TupProjectRequest::Add, tr("Scene %1").arg(1));
        handleProjectRequest(&request);

        request = TupRequestBuilder::createLayerRequest(0, 0, TupProjectRequest::Add, tr("Layer %1").arg(1));
        handleProjectRequest(&request);

        request = TupRequestBuilder::createFrameRequest(0, 0, 0, TupProjectRequest::Add, tr("Frame"));
        handleProjectRequest(&request);
    }
}

void TupProjectManager::closeProject()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProjectManager::closeProject()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (!handler)
        return;

    if (project->isProjectOpen()) {
        if (!handler->closeProject())
            return;
        project->clear();
    }

    project->setOpen(false);
    isModified = false;
    undoStack->clear();
}

bool TupProjectManager::saveProject(const QString &fileName)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProjectManager::saveProject()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    bool result = handler->saveProject(fileName, project);
    isModified = !result;

    return result;
}

bool TupProjectManager::loadProject(const QString &fileName)
{
    if (! handler) {
        #ifdef TUP_DEBUG
            QString msg = "TupProjectManager::loadProject() - Fatal Error: No project handler available!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif		

        return false;
    }

    bool ok = handler->loadProject(fileName, project);

    if (ok) {
        project->setOpen(true);
        isModified = false;
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupProjectManager::loadProject() - Fatal Error: Can't load project -> " + fileName;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }

    return ok;
}

// Returns true if project is open

bool TupProjectManager::isOpen() const
{
    return project->isProjectOpen();
}

bool TupProjectManager::isProjectModified() const
{
    return isModified;
}

void TupProjectManager::undoModified()
{
    isModified = false;
}

bool TupProjectManager::isValid() const
{
    if (!handler)
        return false;

    return handler->isValid();
}

/**
 *  This function is called when some event is triggered by the project
 *  It must be re-implemented if you want to deal with the event in another way, i.ex: send it through the net.
 *  By default, it sends the event through the signal commandExecuted
 *  @param event 
 **/

void TupProjectManager::handleProjectRequest(const TupProjectRequest *request)
{	
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProjectManager::handleProjectRequest()]";
        #else
            T_FUNCINFO;
            // SQA: Enable these lines only for hard/tough debugging
            tWarning() << "Package: ";
            tWarning() << request->xml();			
        #endif
    #endif

    // SQA: the handler must advise when to build the command
    
    if (handler) {
        handler->handleProjectRequest(request);
    } else {
	#ifdef TUP_DEBUG
            QString msg = "TupProjectManager::handleProjectRequest() - Error: No handler available";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }
}

void TupProjectManager::handleLocalRequest(const TupProjectRequest *request)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProjectManager::handleLocalRequest()]";
        #else
            T_FUNCINFO;
        #endif
    #endif	

    TupRequestParser parser;

    if (parser.parse(request->xml())) {
        if (TupFrameResponse *response = static_cast<TupFrameResponse *>(parser.response())) {
            sceneIndex = response->sceneIndex();
            layerIndex = response->layerIndex();
            frameIndex = response->frameIndex();

            /*
            if (response->action() == TupProjectRequest::Copy) {
                TupScene *scene = project->sceneAt(sceneIndex);
                if (scene) {
                    TupLayer *layer = scene->layerAt(layerIndex);
                    if (layer) {
                        TupFrame *frame = layer->frameAt(frameIndex);
                        if (frame) {
                            QDomDocument doc;
                            doc.appendChild(frame->toXml(doc));
                            k->frameSelection = doc.toString(0);
                            response->setArg(k->frameSelection);
                        }
                    }
                }
            if (response->action() == TupProjectRequest::CopySelection) {
                k->frameSelection = response->arg().toString();
            } else if (response->action() == TupProjectRequest::PasteSelection) {
                response->setArg(k->frameSelection);
                TupProjectRequest request = TupRequestBuilder::fromResponse(response);
                handleProjectRequest(&request);
                return;
            } else if (response->action() == TupProjectRequest::UpdateOpacity) {
            */

            if (response->action() == TupProjectRequest::UpdateOpacity) {
                double opacity = response->arg().toReal();
                TupScene *scene = project->sceneAt(sceneIndex);
                if (scene) {
                    TupLayer *layer = scene->layerAt(layerIndex);
                    if (layer) {
                        layer->setOpacity(opacity);
                    } else {
                        #ifdef TUP_DEBUG
                            QString msg = "TupProjectManager::handleLocalRequest() - Fatal Error: "
                                          " Layer pointer is NULL [index = " +  QString::number(layerIndex) + "]";
                            #ifdef Q_OS_WIN
                                qDebug() << msg;
                            #else
                                tError() << msg;
                            #endif
                        #endif
                    }
                }
            }
        }

        parser.response()->setExternal(request->isExternal());
        emit responsed(parser.response());
    }
}

/*
 *  This function creates a command to execute an action, i.e. add a frame. 
 *  The command has the information necessary to undo its effect.
 *  Usually this command must be added into the commands stack.
 *  The command created is not deleted by this class, this task depends on the user.
 *  @param event 
 *  @return 
 */

void TupProjectManager::createCommand(const TupProjectRequest *request, bool addToStack)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProjectManager::createCommand(()]";
        #else
            T_FUNCINFO;
            tWarning() << request->xml();
        #endif
    #endif		

    if (request->isValid()) {
        TupProjectCommand *command = new TupProjectCommand(commandExecutor, request);
        if (addToStack)
            undoStack->push(command);
        else  
            command->redo();
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupProjectManager::createCommand() - Invalid request";
            #ifdef Q_OS_WIN
                qWarning() << msg;
            #else
                tWarning() << msg;
            #endif
        #endif
    }
}

void TupProjectManager::createCommand(TupProjectCommand *command)
{
    undoStack->push(command);
}

TupProject *TupProjectManager::getProject()
{
    return project;
}

void TupProjectManager::undo()
{
    undoStack->undo();
}

void TupProjectManager::redo()
{
    undoStack->redo();
}

void TupProjectManager::clearUndoStack()
{
    undoStack->clear();
}

void TupProjectManager::emitResponse(TupProjectResponse *response)
{	
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProjectManager::emitResponse()] - response->action(): " << response->action();
        #else
            T_FUNCINFO << response->action();
        #endif
    #endif	

    if (response->action() != TupProjectRequest::Select)
        isModified = true;

    if (!handler) {
        // SQA: Check if this is the right way to handle this condition 
        emit responsed(response);
    } else if (isNetworked) {
               if (handler->commandExecuted(response))
                   emit responsed(response);
    } else { // Local request
        emit responsed(response);
    }
}

void TupProjectManager::setOpen(bool isOpen)
{
    project->setOpen(isOpen);
}

bool TupProjectManager::removeProjectPath(const QString &projectPath)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProjectManager::removeProjectPath()]";
        #else
            T_FUNCINFO;
            tWarning() << "Removing project path: " << projectPath;
        #endif
    #endif

    bool result = true;
    QDir dir(projectPath);

    if (dir.exists(projectPath)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden 
                                                    | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                QString path = info.absoluteFilePath();
                result = removeProjectPath(path);
            } else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result)
                return result;
        }
        result = dir.rmdir(projectPath);
    }
	
    #ifdef TUP_DEBUG
        QString msg = "[TupProjectManager::removeProjectPath()] - Result -> " + QString::number(result);
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning() << msg;
        #endif
    #endif

    return result;
}

void TupProjectManager::updateProjectDimension(const QSize size)
{
    project->setDimension(size);
}
