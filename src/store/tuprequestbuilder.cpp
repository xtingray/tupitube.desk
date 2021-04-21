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

#include "tuprequestbuilder.h"
#include "tupprojectrequest.h"
#include "tupprojectresponse.h"

TupRequestBuilder::TupRequestBuilder()
{
}

TupRequestBuilder::~TupRequestBuilder()
{
}

TupProjectRequest TupRequestBuilder::createItemRequest(int sceneIndex, int layerIndex, int frameIndex, int itemIndex, QPointF point, TupProject::Mode spaceMode, 
                                                     TupLibraryObject::ObjectType type, int actionId, const QVariant &arg, const QByteArray &data)
{
    QDomDocument doc;

    QDomElement root = doc.createElement("project_request");

    QDomElement scene = doc.createElement("scene");
    scene.setAttribute("index", sceneIndex);

    QDomElement layer = doc.createElement("layer");
    layer.setAttribute("index", layerIndex);

    QDomElement frame = doc.createElement("frame");
    frame.setAttribute("index", frameIndex);

    QDomElement item = doc.createElement("item");
    item.setAttribute("index", itemIndex);

    QDomElement objectType = doc.createElement("objectType");
    objectType.setAttribute("id", type);

    QDomElement position = doc.createElement("position");
    double px = point.x(); 
    double py = point.y();
    position.setAttribute("x", QString::number(px));
    position.setAttribute("y", QString::number(py));

    QDomElement space = doc.createElement("spaceMode");
    space.setAttribute("current", spaceMode);

    QDomElement action = doc.createElement("action");
    action.setAttribute("id", actionId);
    action.setAttribute("arg", arg.toString());
    action.setAttribute("part", TupProjectRequest::Item);

    TupRequestBuilder::appendData(doc, action, data);
    root.appendChild(action);
    item.appendChild(objectType);
    item.appendChild(position);
    item.appendChild(space);
    frame.appendChild(item);
    layer.appendChild(frame);
    scene.appendChild(layer);
    root.appendChild(scene);

    doc.appendChild(root);
    TupProjectRequest request(doc.toString(0));

    return request;
}

TupProjectRequest TupRequestBuilder::createFrameRequest(int sceneIndex, int layerIndex, int frameIndex, int actionId, const QVariant &arg, const QByteArray &data)
{
    QDomDocument doc;

    QDomElement root = doc.createElement("project_request");

    QDomElement scene = doc.createElement("scene");
    scene.setAttribute("index", sceneIndex);

    QDomElement layer = doc.createElement("layer");
    layer.setAttribute("index", layerIndex);

    QDomElement frame = doc.createElement("frame");
    frame.setAttribute("index", frameIndex);

    QDomElement action = doc.createElement("action");
    action.setAttribute("id", actionId);
    action.setAttribute("arg", arg.toString());
    action.setAttribute("part", TupProjectRequest::Frame);

    TupRequestBuilder::appendData(doc, action, data);

    root.appendChild(action);
    layer.appendChild(frame);
    scene.appendChild(layer);
    root.appendChild(scene);
    doc.appendChild(root);

    return TupProjectRequest(doc.toString(0));
}


TupProjectRequest TupRequestBuilder::createLayerRequest(int sceneIndex, int layerIndex, int actionId, const QVariant &arg, const QByteArray &data)
{
    QDomDocument doc;

    QDomElement root = doc.createElement("project_request");

    QDomElement scene = doc.createElement("scene");
    scene.setAttribute("index", sceneIndex);

    QDomElement layer = doc.createElement("layer");
    layer.setAttribute("index", layerIndex);

    QDomElement action = doc.createElement("action");
    action.setAttribute("id", actionId);
    action.setAttribute("arg", arg.toString());
    action.setAttribute("part", TupProjectRequest::Layer);

    TupRequestBuilder::appendData(doc, action, data);

    root.appendChild(action);
    scene.appendChild(layer);
    root.appendChild(scene);

    doc.appendChild(root);

    return TupProjectRequest(doc.toString(0));
}

TupProjectRequest TupRequestBuilder::createSceneRequest(int sceneIndex, int actionId, const QVariant &arg, const QByteArray &data)
{
    QDomDocument doc;
    QDomElement root = doc.createElement("project_request");

    QDomElement scene = doc.createElement("scene");
    scene.setAttribute("index", sceneIndex);

    QDomElement action = doc.createElement("action");
    action.setAttribute("id", actionId);
    action.setAttribute("arg", arg.toString());
    action.setAttribute("part", TupProjectRequest::Scene);

    TupRequestBuilder::appendData(doc, action, data);

    root.appendChild(action);
    root.appendChild(scene);

    doc.appendChild(root);

    return TupProjectRequest(doc.toString(0));
}

TupProjectRequest TupRequestBuilder::createLibraryRequest(int actionId, const QVariant &arg, TupLibraryObject::ObjectType type, TupProject::Mode spaceMode,
                                                        const QByteArray &data, const QString &folder, int sceneIndex, int layerIndex, int frameIndex)
{
    QDomDocument doc;
    QDomElement root = doc.createElement("project_request");

    QDomElement scene = doc.createElement("scene");
    scene.setAttribute("index", sceneIndex);

    QDomElement layer = doc.createElement("layer");
    layer.setAttribute("index", layerIndex);

    QDomElement frame = doc.createElement("frame");
    frame.setAttribute("index", frameIndex);

    QDomElement library = doc.createElement("library");

    QDomElement symbol = doc.createElement("symbol");
    symbol.setAttribute("folder", folder);
    symbol.setAttribute("type", type);
    symbol.setAttribute("spaceMode", spaceMode);

    QDomElement action = doc.createElement("action");
    action.setAttribute("id", actionId);
    action.setAttribute("arg", arg.toString());
    action.setAttribute("part", TupProjectRequest::Library);

    TupRequestBuilder::appendData(doc, action, data);

    root.appendChild(action);

    library.appendChild(symbol);

    root.appendChild(library);

    root.appendChild(scene);
    scene.appendChild(layer);
    layer.appendChild(frame);

    doc.appendChild(root);

    return TupProjectRequest(doc.toString(0));
}

void TupRequestBuilder::appendData(QDomDocument &doc, QDomElement &element, const QByteArray &data)
{
    if (!data.isNull() && !data.isEmpty()) {
        QDomElement edata = doc.createElement("data");

        QDomCDATASection cdata = doc.createCDATASection(QString(data.toBase64()));

        edata.appendChild(cdata);
        element.appendChild(edata);
    }
}

TupProjectRequest TupRequestBuilder::fromResponse(TupProjectResponse *response)
{
    TupProjectRequest request;

    switch (response->getPart()) {
            case TupProjectRequest::Item:
                 {
                    request = TupRequestBuilder::createItemRequest(static_cast<TupItemResponse*> (response)->getSceneIndex(), static_cast<TupItemResponse*> (response)->getLayerIndex(), 
                                                                  static_cast<TupItemResponse*> (response)->getFrameIndex(), static_cast<TupItemResponse*> (response)->getItemIndex(), 
                                                                  static_cast<TupItemResponse*> (response)->position(), TupProject::Mode(static_cast<TupItemResponse*> (response)->spaceMode()), 
                                                                  TupLibraryObject::ObjectType(static_cast<TupItemResponse*> (response)->getItemType()), response->getAction(), response->getArg().toString(),
                                                                  response->getData());
                 }
            break;
            case TupProjectRequest::Frame:
                 {
                    request = TupRequestBuilder::createFrameRequest(static_cast<TupFrameResponse*> (response)->getSceneIndex(), static_cast<TupFrameResponse*> (response)->getLayerIndex(), static_cast<TupFrameResponse*> (response)->getFrameIndex(), response->getAction(), response->getArg().toString(), response->getData());
                 }
            break;
            case TupProjectRequest::Layer:
                 {
                    request = TupRequestBuilder::createLayerRequest(static_cast<TupLayerResponse*> (response)->getSceneIndex(), static_cast<TupLayerResponse*> (response)->getLayerIndex(), response->getAction(), response->getArg().toString(), response->getData());
                 }
            break;
            case TupProjectRequest::Scene:
                 {
                    request = TupRequestBuilder::createSceneRequest(static_cast<TupSceneResponse*> (response)->getSceneIndex(), response->getAction(), response->getArg().toString(), response->getData());
                 }
            break;
            case TupProjectRequest::Library:
                 {
                    request = TupRequestBuilder::createLibraryRequest(response->getAction(), response->getArg().toString(), TupLibraryObject::ObjectType(static_cast<TupLibraryResponse*>(response)->symbolType()),
                                                                     TupProject::Mode(static_cast<TupLibraryResponse*>(response)->getSpaceMode()), response->getData(), static_cast<TupLibraryResponse*>(response)->getParent(), 
                                                                     static_cast<TupLibraryResponse*>(response)->getSceneIndex(), static_cast<TupLibraryResponse*>(response)->getLayerIndex(),  
                                                                     static_cast<TupLibraryResponse*>(response)->getFrameIndex());
                 }
            break;
            default:
                 {
                    #ifdef TUP_DEBUG
                        qDebug() << "TupRequestBuilder::fromResponse() - Error: wOw! Unknown response! O_o";
                    #endif
                 }
    }

    return request;
}
