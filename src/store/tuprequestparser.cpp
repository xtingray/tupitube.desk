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

#include "tuprequestparser.h"
#include "tuplibraryobject.h"

TupRequestParser::TupRequestParser() : TupXmlParserBase()
{
    response = nullptr;
}

TupRequestParser::~TupRequestParser()
{
}

void TupRequestParser::initialize()
{
    response = nullptr;
}

bool TupRequestParser::startTag(const QString& qname, const QXmlAttributes& atts)
{
    if (qname == "project_request") {
        sign = atts.value("sign");
    } else if (qname == "item") {
               static_cast<TupItemResponse *>(response)->setItemIndex(atts.value("index").toInt());
    } else if (qname == "objectType") {
               static_cast<TupItemResponse *>(response)->setItemType(TupLibraryObject::Type(atts.value("id").toInt()));
    } else if (qname == "position") {
               static_cast<TupItemResponse *>(response)->setPosX(atts.value("x").toDouble());
               static_cast<TupItemResponse *>(response)->setPosY(atts.value("y").toDouble());
    } else if (qname == "spaceMode") {
               static_cast<TupItemResponse *>(response)->setSpaceMode(TupProject::Mode(atts.value("current").toInt()));
    } else if (qname == "frame") {
               static_cast<TupFrameResponse *>(response)->setFrameIndex(atts.value("index").toInt());
    } else if (qname == "data") {
               setReadText(true);
    } else if (qname == "layer") {
               static_cast<TupLayerResponse *>(response)->setLayerIndex(atts.value("index").toInt());
    } else if (qname == "scene") {
               static_cast<TupSceneResponse *>(response)->setSceneIndex(atts.value("index").toInt());
    } else if (qname == "symbol") {
               static_cast<TupLibraryResponse*>(response)->setSymbolType(TupLibraryObject::Type(atts.value("type").toInt()));
               static_cast<TupLibraryResponse*>(response)->setParent(atts.value("folder"));
               static_cast<TupLibraryResponse*>(response)->setSpaceMode(TupProject::Mode(atts.value("spaceMode").toInt()));
    } else if (qname == "action") {
               response = TupProjectResponseFactory::create(atts.value("part").toInt(), atts.value("id").toInt());
               response->setArg(atts.value("arg"));
    }

    return true;
}

bool TupRequestParser::endTag(const QString& qname)
{
    Q_UNUSED(qname);
    return true;
}

void TupRequestParser::text(const QString &ch)
{
    if (currentTag() == "data")
        response->setData(QByteArray::fromBase64(QByteArray(ch.toLocal8Bit())));
}

TupProjectResponse *TupRequestParser::getResponse() const
{
    return response;
}

QString TupRequestParser::getSign() const
{
    return sign;
}
