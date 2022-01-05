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

#include "tuprequestparserhandler.h"

TupRequestParserHandler::TupRequestParserHandler() : QXmlStreamReader()
{
}

TupRequestParserHandler::TupRequestParserHandler(const QString &xml) : QXmlStreamReader(xml)
{
}

TupRequestParserHandler::~TupRequestParserHandler()
{
}

bool TupRequestParserHandler::parse()
{
    while (!atEnd()) {
        readNext();
        QString tag = name().toString();
        if (isStartElement()) {
            if (tag == "project_request") {
                sign = attributes().value("sign").toString();
            } else if (tag == "item") {
                       static_cast<TupItemResponse *>(response)->setItemIndex(attributes().value("index").toInt());
            } else if (tag == "objectType") {
                       static_cast<TupItemResponse *>(response)->setItemType(TupLibraryObject::ObjectType(attributes().value("id").toInt()));
            } else if (tag == "position") {
                       static_cast<TupItemResponse *>(response)->setPosX(attributes().value("x").toDouble());
                       static_cast<TupItemResponse *>(response)->setPosY(attributes().value("y").toDouble());
            } else if (tag == "spaceMode") {
                       static_cast<TupItemResponse *>(response)->setSpaceMode(TupProject::Mode(attributes().value("current").toInt()));
            } else if (tag == "frame") {
                       static_cast<TupFrameResponse *>(response)->setFrameIndex(attributes().value("index").toInt());
            } else if (tag == "data") {
                       static_cast<TupLibraryResponse*>(response)->setData(QByteArray::fromBase64(QByteArray(readElementText().toLocal8Bit())));
            } else if (tag == "layer") {
                       static_cast<TupLayerResponse *>(response)->setLayerIndex(attributes().value("index").toInt());
            } else if (tag == "scene") {
                       static_cast<TupSceneResponse *>(response)->setSceneIndex(attributes().value("index").toInt());
            } else if (tag == "symbol") {
                       static_cast<TupLibraryResponse*>(response)->setSymbolType(TupLibraryObject::ObjectType(attributes().value("type").toInt()));
                       static_cast<TupLibraryResponse*>(response)->setParent(attributes().value("folder").toString());
                       static_cast<TupLibraryResponse*>(response)->setSpaceMode(TupProject::Mode(attributes().value("spaceMode").toInt()));
            } else if (tag == "action") {
                       response = TupProjectResponseFactory::create(attributes().value("part").toInt(), attributes().value("id").toInt());
                       response->setArg(attributes().value("arg").toString());
            }
        } /* else if (isEndElement()) { // Ending Tag

        } */
    }

    if (hasError()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupRequestParserHandler::parse()] - Fatal Error: Can't process xml!";
        #endif
        return false;
    }

    return true;
}

TupProjectResponse* TupRequestParserHandler::getResponse() const
{
    return response;
}

QString TupRequestParserHandler::getSign() const
{
    return sign;
}
