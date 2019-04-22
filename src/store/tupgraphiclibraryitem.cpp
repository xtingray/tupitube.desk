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

#include "tupgraphiclibraryitem.h"
#include "tupserializer.h"
#include "tupitemgroup.h"

TupGraphicLibraryItem::TupGraphicLibraryItem() : TupProxyItem()
{
}

TupGraphicLibraryItem::TupGraphicLibraryItem(TupLibraryObject *object) : TupProxyItem()
{
    setObject(object);
    itemType = object->getType();
}

TupGraphicLibraryItem::~TupGraphicLibraryItem()
{
}

TupLibraryObject::Type TupGraphicLibraryItem::getItemType()
{
    return itemType;
}

QDomElement TupGraphicLibraryItem::toXml(QDomDocument &doc) const
{
    QDomElement library = doc.createElement("symbol");
    library.setAttribute("id", symbolName);
    library.appendChild(TupSerializer::properties(this, doc));
    
    return library;
}

void TupGraphicLibraryItem::fromXml(const QString &xml)
{
    Q_UNUSED(xml);
}

void TupGraphicLibraryItem::setObject(TupLibraryObject *object)
{
    if (!object) {
        #ifdef TUP_DEBUG
            QString msg = "TupGraphicLibraryItem::setObject() - Setting null library object";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif

        return;
    }
    
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qWarning() << "TupGraphicLibraryItem::setObject() - object->symbolName(): " << object->getSymbolName();
        #else
            T_FUNCINFOX("library") << object->getSymbolName();
        #endif
    #endif

    symbolName = object->getSymbolName();
    symbolPath = object->getDataPath();
    switch(object->getType()) {
        case TupLibraryObject::Item:
        case TupLibraryObject::Text:
        case TupLibraryObject::Image:
        {
             setItem(qvariant_cast<QGraphicsItem *>(object->getData()));
        }
        break;
        case TupLibraryObject::Svg:
        {
             setSvgContent(object->getDataPath());
        }
        break;
        default: 
        break;
    }
}

void TupGraphicLibraryItem::setSymbolName(const QString &name)
{
    symbolName = name;
}

QString TupGraphicLibraryItem::getSymbolName() const
{
    return symbolName;
}

void TupGraphicLibraryItem::setSvgContent(const QString &path)
{
    svgContent = path;
}

QString TupGraphicLibraryItem::getSvgContent()
{
   return svgContent;
}

QString TupGraphicLibraryItem::getSymbolPath() const
{
    return symbolPath;
}
