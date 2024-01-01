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
#include "tuprequestbuilder.h"
#include "tupprojectrequest.h"
#include "tupprojectresponse.h"

TupCommandExecutor::TupCommandExecutor(TupProject *animation) : QObject(animation), project(animation)
{
}

TupCommandExecutor::~TupCommandExecutor()
{
}

void TupCommandExecutor::getScenes(TupSceneResponse *response)
{
    response->setScenes(project->getScenes());
    emit responsed(response);
}

bool TupCommandExecutor::createScene(TupSceneResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupCommandExecutor::createScene()]";
    #endif

    int pos = response->getSceneIndex();
    QString name = response->getArg().toString();
    if (pos < 0)
        return false;

    if (response->getMode() == TupProjectResponse::Do) {
        TupScene *scene = project->createScene(name, pos);
        if (!scene) 
            return false;
    }

    if (response->getMode() == TupProjectResponse::Redo || response->getMode() == TupProjectResponse::Undo) { 
        bool success = project->restoreScene(pos);
        if (!success)
            return false;
    }

    emit responsed(response);
    return true;
}

bool TupCommandExecutor::removeScene(TupSceneResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCommandExecutor::removeScene()]";
    #endif

    int pos = response->getSceneIndex();
    TupScene *scene = project->sceneAt(pos);
    if (scene) {
        QDomDocument document;
        document.appendChild(scene->toXml(document));
        response->setState(document.toString());
        response->setArg(scene->getSceneName());

        if (project->removeScene(pos)) {
            emit responsed(response);
            return true;
        } 
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "TupCommandExecutor::removeScene() - Fatal Error: No scene at index -> " + QString::number(pos);
        #endif
    }

    return false;
}

bool TupCommandExecutor::moveScene(TupSceneResponse *response)
{
    int oldPos = response->getSceneIndex();
    int newPos = response->getArg().toInt();

    #ifdef TUP_DEBUG
        qDebug() << "[TupCommandExecutor::moveScene()] - oldPos ->" << oldPos;
        qDebug() << "[TupCommandExecutor::moveScene()] - newPos ->" << newPos;
    #endif

    if (project->moveScene(oldPos, newPos)) {
        emit responsed(response);

        return true;
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupCommandExecutor::moveScene()] - Fatal Error: Can't move scene at index ->" << oldPos;
        #endif
    }

    return false;
}

bool TupCommandExecutor::lockScene(TupSceneResponse *response)
{
    int pos = response->getSceneIndex();
    bool lock = response->getArg().toBool();

    #ifdef TUP_DEBUG
        qWarning() << "TupCommandExecutor::lockScene() - Scene is locked: " + QString::number(lock);
    #endif  

    TupScene *scene = project->sceneAt(pos);
    if (scene) {
        scene->setSceneLocked(lock);
        emit responsed(response);
        return true;
    }

    return false;
}

bool TupCommandExecutor::renameScene(TupSceneResponse *response)
{
    int pos = response->getSceneIndex();
    QString newName = response->getArg().toString();

    TupScene *scene = project->sceneAt(pos);
    if (scene) {
        scene->setSceneName(newName);
        emit responsed(response);
        return true;
    }

    return false;
}

void TupCommandExecutor::selectScene(TupSceneResponse *response)
{
    emit responsed(response);
}

bool TupCommandExecutor::setSceneVisibility(TupSceneResponse *response)
{
    int pos = response->getSceneIndex();
    bool view = response->getArg().toBool();
    
    TupScene *scene = project->sceneAt(pos);
    if (scene) {
        scene->setVisibility(view);
        emit responsed(response);
        return true;
    }

    return false;
}

bool TupCommandExecutor::resetScene(TupSceneResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCommandExecutor::resetScene()]";
    #endif

    int index = response->getSceneIndex();
    QString newName = response->getArg().toString();

    TupScene *scene = project->sceneAt(index);
    if (scene) {
        if (response->getMode() == TupProjectResponse::Do || response->getMode() == TupProjectResponse::Redo) {
            if (project->resetScene(index, newName)) {
                emit responsed(response);
                return true;
            }
        }

        if (response->getMode() == TupProjectResponse::Undo) {
            QString oldName = project->recoverScene(index);
            response->setArg(oldName);
            emit responsed(response);
            return true;
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "TupCommandExecutor::resetScene() - Fatal Error: No scene at index -> " + QString::number(index);
        #endif
    }

    return false;
}

void TupCommandExecutor::setBgColor(TupSceneResponse *response)
{
    int index = response->getSceneIndex();
    QString colorName = response->getArg().toString();
    project->setSceneBgColor(index, QColor(colorName));

    emit responsed(response);
}
