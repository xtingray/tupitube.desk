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
        qDebug() << "[TupProjectManager()]";
    #endif

    modified = false;
    handler = nullptr;

    project = new TupProject(this);
    undoStack = new QUndoStack(this);
    commandExecutor = new TupCommandExecutor(project);

    connect(commandExecutor, SIGNAL(responsed(TupProjectResponse*)), this, SLOT(emitResponse(TupProjectResponse *)));
    connect(project, SIGNAL(responsed(TupProjectResponse*)), this, SIGNAL(responsed(TupProjectResponse *)));
}

TupProjectManager::~TupProjectManager()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupProjectManager()]";
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
        disconnect(handler, SIGNAL(projectPathChanged()), this, SIGNAL(projectPathChanged()));

        delete handler;
        handler = nullptr;
    }

    handler = pHandler;
    handler->setParent(this);
    handler->setProject(project);

    connect(handler, SIGNAL(sendCommand(const TupProjectRequest *, bool)),
            this, SLOT(createCommand(const TupProjectRequest *, bool)));
    connect(handler, SIGNAL(sendLocalCommand(const TupProjectRequest *)),
            this, SLOT(handleLocalRequest(const TupProjectRequest *)));
    connect(handler, SIGNAL(projectPathChanged()), this, SIGNAL(projectPathChanged()));

    isNetworked = networked;
}

TupAbstractProjectHandler *TupProjectManager::getHandler() const
{
    return handler;
}

void TupProjectManager::setupNewProject()
{	
    #ifdef TUP_DEBUG
        qDebug() << "[TupProjectManager::setupNewProject()]";
    #endif

    if (!handler || !params) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupProjectManager::setupNewProject()] - Error: No handler available or no params!";
        #endif
        return;
    }

    closeProject();

    project->setProjectName(params->getProjectManager());
    project->setAuthor(params->getAuthor());
    // project->setTags(params->getTags());
    project->setDescription(params->getDescription());
    project->setBgColor(params->getBgColor());
    project->setDimension(params->getDimension());
    project->setFPS(params->getFPS());

    if (!handler->setupNewProject(params)) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupProjectManager::setupNewProject()] - Error: Project params misconfiguration";
        #endif
        return;
    }

    if (!isNetworked) {
        QString projectPath = CACHE_DIR + params->getProjectManager();
        QDir projectDir(projectPath); 
        if (projectDir.exists()) {
            if (!projectDir.removeRecursively()) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupProjectManager::setupNewProject()] - Error: Can't remove CACHE path -> " << projectPath;
                #endif
            }
        }
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
        qDebug() << "[TupProjectManager::closeProject()]";
    #endif

    if (!handler)
        return;

    if (project->isProjectOpen()) {
        if (!handler->closeProject())
            return;
        project->clear();
    }

    project->setOpen(false);
    modified = false;
    undoStack->clear();
}

bool TupProjectManager::saveProject(const QString &fileName)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProjectManager::saveProject()]";
    #endif

    bool result = handler->saveProject(fileName, project);
    modified = !result;

    return result;
}

bool TupProjectManager::loadProject(const QString &fileName)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProjectManager::loadProject()] - fileName -> " << fileName;
    #endif

    if (!handler) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupProjectManager::loadProject()] - Fatal Error: No project handler available!";
        #endif
        return false;
    }

    bool ok = handler->loadProject(fileName, project);

    if (ok) {
        project->setOpen(true);
        modified = false;
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupProjectManager::loadProject()] - Fatal Error: Can't load project -> " << fileName;
        #endif
    }

    return ok;
}

// Returns true if project is open

bool TupProjectManager::isOpen() const
{
    return project->isProjectOpen();
}

bool TupProjectManager::projectWasModified() const
{
    return modified;
}

void TupProjectManager::setModificationStatus(bool changed)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProjectManager::setModificationStatus()] - changed -> " << changed;
    #endif

    modified = changed;
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
        qWarning() << "[TupProjectManager::handleProjectRequest()] - Package:";
        // qWarning() << request->getXml();
    #endif

    // SQA: the handler must advise when to build the command
    
    if (handler) {
        handler->handleProjectRequest(request);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupProjectManager::handleProjectRequest()] - Error: No handler available";
        #endif
    }
}

void TupProjectManager::handleLocalRequest(const TupProjectRequest *request)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProjectManager::handleLocalRequest()]";
    #endif	

    TupRequestParser parser;

    if (parser.parse(request->getXml())) {
        if (TupFrameResponse *response = static_cast<TupFrameResponse *>(parser.getResponse())) {
            sceneIndex = response->getSceneIndex();
            layerIndex = response->getLayerIndex();
            frameIndex = response->getFrameIndex();

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

            /*
            if (response->getAction() == TupProjectRequest::UpdateOpacity) {
                double opacity = response->getArg().toReal();
                TupScene *scene = project->sceneAt(sceneIndex);
                if (scene) {
                    TupLayer *layer = scene->layerAt(layerIndex);
                    if (layer) {
                        layer->setOpacity(opacity);
                    } else {
                        #ifdef TUP_DEBUG
                            QString msg = "[TupProjectManager::handleLocalRequest()] - Fatal Error: "
                                          " Layer pointer is NULL [index = " +  QString::number(layerIndex) + "]";
                            qDebug() << msg;
                        #endif
                    }
                }
            }
            */
        }

        parser.getResponse()->setExternal(request->isRequestExternal());
        emit responsed(parser.getResponse());
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
        qDebug() << "[TupProjectManager::createCommand()]";
        // qDebug() << request->getXml();
    #endif

    if (request->isValid()) {
        TupProjectCommand *command = new TupProjectCommand(commandExecutor, request);
        if (command) {
            if (addToStack) {
                undoStack->push(command);
                /*
                #ifdef TUP_DEBUG
                    QString msg = "TupProjectManager::createCommand() * command counter: " + QString::number(undoStack->count());
                    qWarning() << msg;
                #endif
                */
            } else {
                command->redo();
            }
        } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupProjectManager::createCommand()] - Invalid command";
        #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupProjectManager::createCommand()] - Invalid request";
        #endif
    }
}

void TupProjectManager::createCommand(TupProjectCommand *command)
{
    undoStack->push(command);
    /*
    #ifdef TUP_DEBUG
        qWarning() << "TupProjectManager::createCommand() - command counter: " + QString::number(undoStack->count());
    #endif
    */
}

TupProject *TupProjectManager::getProject()
{
    return project;
}

void TupProjectManager::undo()
{
    if (undoStack->count() > 0) {
        if (undoStack->canUndo()) {
            /*
            qDebug() << "";
            qDebug() << "[TupProjectManager::undo()] - count: " << undoStack->count();
            qDebug() << "[TupProjectManager::undo()] - undo text: " << undoStack->undoText();
            qDebug() << "";
            */
            undoStack->undo();
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupProjectManager::undo()] - No undo actions available!";
            #endif
        }
    }
}

void TupProjectManager::redo()
{
    if (undoStack->count() > 0) {
       if (undoStack->canRedo()) {
           /*
           qDebug() << "";
           qDebug() << "[TupProjectManager::redo()] - count: " << undoStack->count();
           qDebug() << "[TupProjectManager::redo()] - redo text: " << undoStack->redoText();
           qDebug() << "";
           */
           undoStack->redo();
       } else {
           #ifdef TUP_DEBUG
               qWarning() << "[TupProjectManager::redo()] - No redo actions available!";
           #endif
       }
   }
}

void TupProjectManager::clearUndoStack()
{
    undoStack->clear();
}

void TupProjectManager::emitResponse(TupProjectResponse *response)
{	
    #ifdef TUP_DEBUG
        qDebug() << "[TupProjectManager::emitResponse()] - response->action() -> " << response->getAction();
    #endif

    if (response->getAction() != TupProjectRequest::Select)
        modified = true;

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

void TupProjectManager::updateProjectDimension(const QSize size)
{
    project->setDimension(size);
}

int TupProjectManager::framesCount(int sceneIndex)
{
    int total = 0;
    TupScene *scene = project->sceneAt(sceneIndex);
    if (scene)
        total = scene->framesCount();

    return total;
}

/*
bool TupProjectManager::removeProjectPath(const QString &projectPath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProjectManager::removeProjectPath()] - Removing project path: " << projectPath;
        qDebug() << "*** CACHE_DIR: " << CACHE_DIR;
    #endif

    bool result = true;
    QDir dir(projectPath);

    if (dir.exists(projectPath)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden
                                                    | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                QString path = info.absoluteFilePath();
                if (CACHE_DIR.compare(path) != 0) {
                    result = removeProjectPath(path);
                } else {
                    #ifdef TUP_DEBUG
                       qWarning() << "[TupProjectManager::removeProjectPath()] - Cache Path reached! -> " << path;
                    #endif
                    return true;
                }
            } else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result)
                return result;
        }
        result = dir.rmdir(projectPath);
    }

    #ifdef TUP_DEBUG
        qWarning() << "TupProjectManager::removeProjectPath() - Result -> " + QString::number(result);
    #endif

    return result;
}
*/
