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

#include "toolview.h"

ToolView::ToolView(const QString &title, const QIcon &icon, const QString &code, QWidget *parent):
                   QDockWidget(title, parent), currentPerspective(0)
{
    setFeatures(QDockWidget::NoDockWidgetFeatures);
    setWindowIcon(icon);

    currentButton = new TViewButton(this);
    currentButton->setToolTip(title);

    setObjectName("ToolView-" + code);
    expanded = false;
}

ToolView::~ToolView()
{
}

TViewButton *ToolView::button() const
{
    return currentButton;
}

void ToolView::expandDock(bool state)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[ToolView::expandDock()] - state -> " << state;
    #endif
    */

    expanded = state;
    if (state)
        show();
    else 
        close();
}

bool ToolView::isExpanded()
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[ToolView::isExpanded()] - expanded: " << expanded;
    #endif
    */

    return expanded;
}

void ToolView::setExpandingFlag() 
{
    #ifdef TUP_DEBUG
        qDebug() << "[ToolView::setExpandingFlag()]";
    #endif

    if (expanded)
        expanded = false;
    else 
        expanded = true;
}

void ToolView::setShortcut(QKeySequence shortcut)
{
    currentButton->setShortcut(shortcut);
}

void ToolView::setPerspective(int wSpace)
{
    currentPerspective = wSpace;
}

int ToolView::perspective() const
{
    return currentPerspective;
}

void ToolView::enableButton(bool flag)
{
    currentButton->setEnabled(flag);
}
