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

#include "tuptextitem.h"
#include "tupserializer.h"

#include <QTextDocument>

TupTextItem::TupTextItem(QGraphicsItem *parent) : QGraphicsTextItem(parent), textFlags(flags())
{
    setAcceptDrops(true);
}

TupTextItem::~TupTextItem()
{
}

void TupTextItem::fromXml(const QString &xml)
{
    Q_UNUSED(xml)
}

QDomElement TupTextItem::toXml(QDomDocument &doc) const
{
    QDomElement root = doc.createElement("text");
    QDomText text = doc.createTextNode(toPlainText());
    root.appendChild(text);

    QTextOption option = this->document()->defaultTextOption();
    root.appendChild(TupSerializer::properties(this, doc, this->data(0).toString(), textWidth(), option.alignment()));
    QFont font = this->font();    
    root.appendChild(TupSerializer::font(&font, doc));
    QBrush brush(this->defaultTextColor());
    root.appendChild(TupSerializer::brush(&brush, doc));

    /*
    qDebug() << "";
    qDebug() << "XML:";
    qDebug() << "" << doc.toString();
    */

    return root;
}

bool TupTextItem::contains(const QPointF &point) const
{
    return QGraphicsTextItem::contains(point);
}
