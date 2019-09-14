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

#include "tviewbutton.h"
#include "toolview.h"

TViewButton::TViewButton(ToolView *toolView, QWidget *parent): QToolButton(parent), m_area(Qt::LeftToolBarArea), m_toolView(toolView)
{
    setText(m_toolView->windowTitle());
    setIcon(m_toolView->windowIcon());
}

TViewButton::~TViewButton()
{
}

void TViewButton::setArea(Qt::ToolBarArea area)
{
    m_area = area;
    update();
}

Qt::ToolBarArea TViewButton::area() const
{
    return m_area;
}

void TViewButton::mousePressEvent(QMouseEvent *event)
{
    m_toolView->setExpandingFlag();
    QToolButton::mousePressEvent(event);
}

void TViewButton::toggleView()
{
    #ifdef TUP_DEBUG
       qDebug() << "[ToolView::toggleView()]";
    #endif

    m_toolView->setUpdatesEnabled(false);
    m_toolView->toggleViewAction()->trigger();
    m_toolView->setUpdatesEnabled(true);
}

ToolView *TViewButton::toolView() const
{
    return m_toolView;
}
