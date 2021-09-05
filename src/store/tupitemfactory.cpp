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

#include "tupitemfactory.h"
#include "tupsvg2qt.h"
#include "tuppathitem.h"
#include "tuppixmapitem.h"
#include "tuptextitem.h"
#include "tupbuttonitem.h"
#include "tuprectitem.h"
#include "tupellipseitem.h"
#include "tuplineitem.h"
#include "tupgraphiclibraryitem.h"
#include "tuplibrary.h"
#include "tupgraphicalgorithm.h"
#include "tupserializer.h"

TupItemFactory::TupItemFactory() : QObject()
{
    library = nullptr;
}

TupItemFactory::~TupItemFactory()
{
}

void TupItemFactory::setLibrary(const TupLibrary *assets)
{
    library = assets;
}

/*
void TupItemFactory::text(const QString &input)
{
    handler.text(input);
}
*/

QGraphicsItem *TupItemFactory::create(const QString &xml)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupItemFactory::create()] - xml:";
        qDebug() << xml;
    #endif

    TupFactoryHandler handler(xml, library);
    return handler.getItem();
}

QString TupItemFactory::itemID(const QString &xml)
{
    QDomDocument document;

    if (!document.setContent(xml))
        return "item";

    QDomElement root = document.documentElement();
    QString id = root.attribute("id");

    if (id.length() > 0)
        return id;

    return "item";
}

TupFactoryHandler::Type TupItemFactory::getType()
{
    return handler.getType();
}
