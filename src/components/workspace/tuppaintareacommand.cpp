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

#include "tuppaintareacommand.h"
#include "tupbrushmanager.h"

TupPaintAreaCommand::TupPaintAreaCommand(TupPaintArea *area, const TupPaintAreaEvent *paintEvent) : QUndoCommand()
{
    paintArea = area;
    event = paintEvent->clone();
}

TupPaintAreaCommand::~TupPaintAreaCommand()
{
}

void TupPaintAreaCommand::undo()
{
    switch(event->action()) {
           case TupPaintAreaEvent::ChangePen:
                {
                  paintArea->brushManager()->setPen(qvariant_cast<QPen>(oldData));
                }
                break;
           case TupPaintAreaEvent::ChangePenColor:
                {
                  paintArea->brushManager()->setPenColor(qvariant_cast<QColor>(oldData));
                }
                break;
           case TupPaintAreaEvent::ChangePenThickness:
                {
                  paintArea->brushManager()->setPenWidth(qvariant_cast<int>(oldData));
                }
                break;
           case TupPaintAreaEvent::ChangeBrush:
                {
                  paintArea->brushManager()->setBrush(qvariant_cast<QBrush>(oldData));
                }
                break;
           case TupPaintAreaEvent::ChangeBgColor:
                {
                  paintArea->brushManager()->setBgColor(qvariant_cast<QColor>(oldData));
                }
                break;
           default: 
                break;
    }
}

void TupPaintAreaCommand::redo()
{
    switch (event->action()) {
            case TupPaintAreaEvent::ChangePen:
                 {
                   oldData = paintArea->brushManager()->pen();
                   QPen pen = qvariant_cast<QPen>(event->data());
                   if (!pen.color().isValid()) {
                       QPen old = paintArea->brushManager()->pen();
                       pen.setColor(old.color());
                       pen.setBrush(old.brush());
                   } 
                   paintArea->brushManager()->setPen(pen);
                 }
                 break;
            case TupPaintAreaEvent::ChangePenColor:
                 {
                   oldData = paintArea->brushManager()->pen().color();
                   paintArea->brushManager()->setPenColor(qvariant_cast<QColor>(event->data()));
                 }
                 break;
            case TupPaintAreaEvent::ChangePenThickness:
                 {
                   oldData = paintArea->brushManager()->pen().width();
                   paintArea->brushManager()->setPenWidth(qvariant_cast<int>(event->data()));
                 }
                 break;
            case TupPaintAreaEvent::ChangeBrush:
                 {
                   oldData = paintArea->brushManager()->brush();
                   paintArea->brushManager()->setBrush(qvariant_cast<QBrush>(event->data()));
                 }
                 break;
            case TupPaintAreaEvent::ChangeBgColor:
                 {
                   oldData = paintArea->brushManager()->bgColor();
                   paintArea->brushManager()->setBgColor(qvariant_cast<QColor>(event->data()));
                 }
                 break;
            default: 
                 break;
    }
}
