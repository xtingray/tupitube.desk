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

#include "tabbedmainwindow.h"

// TabbedMainWindow

TabbedMainWindow::TabbedMainWindow(const QString &winKey, QWidget *parent): TMainWindow(winKey, parent)
{
    currentTab = new QTabWidget;
    connect(currentTab, SIGNAL(currentChanged(int)), this, SLOT(emitWidgetChanged(int)));
    setCentralWidget(currentTab);
}

TabbedMainWindow::~TabbedMainWindow()
{
}

void TabbedMainWindow::addWidget(QWidget *widget, bool persistant, int perspective)
{
    if (perspective & currentPerspective())
        currentTab->addTab(widget, widget->windowIcon(), widget->windowTitle());

    if (persistant)
        persistentWidgets << widget;

    pages << widget;
    tabs[widget] = perspective;
}

void TabbedMainWindow::removeWidget(QWidget *widget, bool force)
{
    if (force) 
        persistentWidgets.removeAll(widget);

    if (persistentWidgets.contains(widget))
        return;

    int index = currentTab->indexOf(widget);
    if (index >= 0)
        currentTab->removeTab(index);

    tabs.remove(widget);
    pages.removeAll(widget);
}

void TabbedMainWindow::removeAllWidgets()
{
    persistentWidgets.clear();
    currentTab->clear();
    tabs.clear();
    pages.clear();
}

int TabbedMainWindow::tabCount()
{
    return pages.count();
}

// Close the current tab.
void TabbedMainWindow::closeCurrentTab()
{
    int index = currentTab->currentIndex();
    if (index >= 0)
        removeWidget(currentTab->widget(index));
}

// Return the current tab widget.
QTabWidget *TabbedMainWindow::tabWidget() const
{
    return currentTab;
}

void TabbedMainWindow::emitWidgetChanged(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TabbedMainWindow::emitWidgetChanged()]";
    #endif

    if (index != -1) {
        switch (index) {
           case 0:
                setCurrentPerspective(Animation);
           break;
           case 1:
                setCurrentPerspective(Player);
           break;
           case 2:
                setCurrentPerspective(Help);
           break;
           case 3:
                setCurrentPerspective(News);
           break;
        }
        emit tabHasChanged(index);
    }
}

void TabbedMainWindow::setCurrentTab(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TabbedMainWindow::setCurrentTab()] - index: " << index;
    #endif

    if (index != -1)
        currentTab->setCurrentIndex(index);
}
