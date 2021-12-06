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
#include "tupscene.h"
#include "tuplayer.h"
#include "tupprojectrequest.h"
#include "tuprequestbuilder.h"
#include "tupprojectresponse.h"

bool TupCommandExecutor::createLayer(TupLayerResponse *response)
{	
    #ifdef TUP_DEBUG
        qDebug() << "[TupCommandExecutor::createLayer()]";
    #endif

    int scenePosition = response->getSceneIndex();
    int position = response->getLayerIndex();
    QString name = response->getArg().toString();
    // QString state = response->getState();

    TupScene *scene = project->sceneAt(scenePosition);
    if (scene) {
        if (response->getMode() == TupProjectResponse::Do) {
            TupLayer *layer = scene->createLayer(name, position);
            if (!layer) 
                return false;
        }

        if (response->getMode() == TupProjectResponse::Redo || response->getMode() == TupProjectResponse::Undo) {
            bool success = scene->restoreLayer(position);
            if (!success)
                return false;
        }

        emit responsed(response);
        return true;
    }

    return false;
}

bool TupCommandExecutor::removeLayer(TupLayerResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCommandExecutor::removeLayer()]";
    #endif

    int scenePos = response->getSceneIndex();
    int position = response->getLayerIndex();

    TupScene *scene = project->sceneAt(scenePos);
    if (scene) {
        TupLayer *layer = scene->layerAt(position);
        if (layer) {
            QDomDocument document;
            document.appendChild(layer->toXml(document));
            response->setState(document.toString());
            response->setArg(layer->getLayerName());

            if (scene->removeLayer(position)) {
                emit responsed(response);
                return true;
            } 
        } 
    }

    return false;
}

bool TupCommandExecutor::moveLayer(TupLayerResponse *response)
{
    int scenePos = response->getSceneIndex();
    int position = response->getLayerIndex();
    int newPosition = response->getArg().toInt();

    #ifdef TUP_DEBUG
        qDebug() << "[TupCommandExecutor::moveLayer()] - oldPosition -> " << position;
        qDebug() << "[TupCommandExecutor::moveLayer()] - newPosition -> " << newPosition;
    #endif

    TupScene *scene = project->sceneAt(scenePos);

    if (!scene)
        return false;

    if (! scene->moveLayer(position, newPosition)) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupCommandExecutor::moveLayer()] - Error while moving layer!";
        #endif	
        return false;
    } else {
        emit responsed(response);
        return true;
    }

    return false;
}

bool TupCommandExecutor::lockLayer(TupLayerResponse *response)
{
    int scenePos = response->getSceneIndex();
    int position = response->getLayerIndex();
    bool lock = response->getArg().toBool();

    TupScene *scene = project->sceneAt(scenePos);

    if (!scene)
        return false;

    TupLayer *layer = scene->layerAt(position);

    if (layer) {
        layer->setLocked(lock);
        emit responsed(response);
        return true;
    }

    return false;
}

bool TupCommandExecutor::renameLayer(TupLayerResponse *response)
{
    int scenePos = response->getSceneIndex();
    int position = response->getLayerIndex();
    QString newName = response->getArg().toString();
	
    #ifdef TUP_DEBUG
        qWarning() << "[TupCommandExecutor::renameLayer()] - Renaming layer to: " << newName;
    #endif	

    // QString oldName;

    TupScene *scene = project->sceneAt(scenePos);

    if (!scene)
        return false;

    TupLayer *layer = scene->layerAt(position);

    if (layer) {
        QString current = layer->getLayerName();
        layer->setLayerName(newName);

        emit responsed(response);
        response->setArg(current);

        return true;
    }

    return false;
}

bool TupCommandExecutor::selectLayer(TupLayerResponse *response)
{
    emit responsed(response);

    return true;
}

bool TupCommandExecutor::setLayerVisibility(TupLayerResponse *response)
{
    int scenePos = response->getSceneIndex();
    int position = response->getLayerIndex();
    bool view = response->getArg().toBool();

    TupScene *scene = project->sceneAt(scenePos);

    if (!scene)
        return false;

    TupLayer *layer = scene->layerAt(position);

    if (layer) {
        layer->setLayerVisibility(view);
        emit responsed(response);

        return true;
    }

    return false;
}

bool TupCommandExecutor::addLipSync(TupLayerResponse *response)
{
    #ifdef TUP_DEBUG
        qWarning() << "[TupCommandExecutor::addLipSync()] - Adding lipsync...";
    #endif

    int scenePos = response->getSceneIndex();
    int position = response->getLayerIndex();
    QString xml = response->getArg().toString();

    qDebug() << "";
    qDebug() << "4 LIPSYNC";
    qDebug() << xml;

    TupScene *scene = project->sceneAt(scenePos);

    if (!scene)
        return false;

    TupLayer *layer = scene->layerAt(position);
    if (layer) {
        TupLipSync *lipsync = new TupLipSync();
        lipsync->fromXml(xml);

        qDebug() << "";
        qDebug() << "4A LIPSYNC";
        qDebug() << lipsync->toString();

        layer->addLipSync(lipsync);

        emit responsed(response);
        return true;
    }

    return false;
}

bool TupCommandExecutor::updateLipSync(TupLayerResponse *response)
{
    #ifdef TUP_DEBUG
        qWarning() << "[TupCommandExecutor::updateLipSync()] - Updating lipsync...";
    #endif

    int scenePos = response->getSceneIndex();
    QString xml = response->getArg().toString();

    TupScene *scene = project->sceneAt(scenePos);

    if (!scene)
        return false;

    TupLipSync *lipsync = new TupLipSync();
    lipsync->fromXml(xml);
    if (scene->updateLipSync(lipsync)) {
        emit responsed(response);
        return true;
    }

    return false;
}

bool TupCommandExecutor::removeLipSync(TupLayerResponse *response)
{
    #ifdef TUP_DEBUG
        qWarning() << "[TupCommandExecutor::removeLipSync()] - Adding lipsync...";
    #endif

    int scenePos = response->getSceneIndex();
    QString name = response->getArg().toString();

    TupScene *scene = project->sceneAt(scenePos);
    if (scene) {
        project->releaseLipSyncVoices(scenePos, name);
        if (scene->removeLipSync(name)) {
            emit responsed(response);
            return true;
        }
    }

    return false;
}

bool TupCommandExecutor::setLayerOpacity(TupLayerResponse *response)
{
    int scenePos = response->getSceneIndex();
    int position = response->getLayerIndex();
    qreal opacity = response->getArg().toReal();

    #ifdef TUP_DEBUG
        qWarning() << "[TupCommandExecutor::setLayerOpacity()] - Updating layer opacity -> " << opacity;
    #endif

    TupScene *scene = project->sceneAt(scenePos);
    if (!scene)
        return false;

    TupLayer *layer = scene->layerAt(position);
    if (layer) {
        if (response->getMode() == TupProjectResponse::Do)
            layer->setOpacity(opacity);

        if (response->getMode() == TupProjectResponse::Undo)
            layer->undoOpacity();

        if (response->getMode() == TupProjectResponse::Redo)
            layer->redoOpacity();

        responsed(response);

        return true;
    }

    return false;
}
