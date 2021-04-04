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

#include <QFont>
#include <QFocusEvent>
#include <QTimer>
#include <QTextCursor>

TupTextItem::TupTextItem(QGraphicsItem *parent) : QGraphicsTextItem(parent), textFlags(flags()), isEditable(false)
{
    setOpenExternalLinks(true);
    setEditable(false);
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
    QDomText text = doc.createTextNode(toHtml());
    root.appendChild(text);
    
    root.appendChild(TupSerializer::properties(this, doc));
    QFont font = this->font();
    
    root.appendChild(TupSerializer::font(&font, doc));
    
    return root;
}

void TupTextItem::setEditable(bool editable)
{
    isEditable = editable;
    
    if (editable) {
        textFlags = flags(); // save flags
        setTextInteractionFlags(Qt::TextEditorInteraction);
        setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
        setFocus(Qt::MouseFocusReason);

        /*
        setTextInteractionFlags(Qt::TextEditorInteraction);
        setFocus(Qt::MouseFocusReason);
        setSelected(true); // this ensures that itemChange() gets called when we click out of the item
        QTextCursor c = textCursor();
        c.select(QTextCursor::Document);
        setTextCursor(c);
        */
    } else {
        setTextInteractionFlags(Qt::TextBrowserInteraction);
        setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable); // restore flags
    }

    update();
}

void TupTextItem::toggleEditable()
{
    setEditable(!isEditable);
}

void TupTextItem::focusOutEvent(QFocusEvent * event)
{
    QGraphicsTextItem::focusOutEvent(event);

    if (textInteractionFlags() & Qt::TextEditorInteraction && isEditable) {
        QTimer::singleShot(0, this, SLOT(toggleEditable()));
        emit edited();
    }
}

void TupTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    setEditable(true);
    QGraphicsTextItem::mouseDoubleClickEvent(event);
}
