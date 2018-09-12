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

ToolView::ToolView(const QString &title, const QIcon &icon, const QString &code, QWidget *parent) : 
                   QDockWidget(title, parent), m_perspective(0)
{
    // setFeatures(AllDockWidgetFeatures);
    setWindowIcon(icon);
    setup(title);
    setObjectName("ToolView-" + code);

    name = title;
    expanded = false;
}

ToolView::~ToolView()
{
}

void ToolView::setup(const QString &label)
{
    m_button = new TViewButton(this);
    m_button->setToolTip(label);
}

TViewButton *ToolView::button() const
{
    return m_button;
}

void ToolView::expandDock(bool flag)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[ToolView::expandDock()]";
        #else
            T_FUNCINFO << flag;
        #endif
    #endif

    expanded = flag;
    if (flag)
        show();
    else 
        close();

    m_button->setActivated(flag);
}

bool ToolView::isExpanded()
{
    return expanded;
}

void ToolView::setExpandingFlag() 
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[ToolView::setExpandingFlag()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (expanded)
        expanded = false;
    else 
        expanded = true;
}

void ToolView::setDescription(const QString &description)
{
    m_button->setStatusTip(description);
}

void ToolView::setShortcut(QKeySequence shortcut)
{
    m_button->setShortcut(shortcut);
}

void ToolView::setPerspective(int wsp)
{
    m_perspective = wsp;
}

int ToolView::perspective() const
{
    return m_perspective;
}

void ToolView::enableButton(bool flag)
{
    m_button->setEnabled(flag);
}

QString ToolView::getObjectID() 
{
    return objectName();
}

bool ToolView::isChecked()
{
    return m_button->isChecked();
}
