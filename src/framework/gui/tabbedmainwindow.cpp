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

/**
 * Construct a tabbed main window.
 * @param parent 
 * @return 
 */
TabbedMainWindow::TabbedMainWindow(QWidget *parent) : TMainWindow(parent)
{
    currentTab = new QTabWidget;
    setupTabWidget(currentTab);
    setCentralWidget(currentTab);
}

/**
 * Destructor
 * @return 
 */
TabbedMainWindow::~TabbedMainWindow()
{
}

/**
 * Setup the tab widget.
 * @param w 
 */
void TabbedMainWindow::setupTabWidget(QTabWidget *widget)
{
#ifdef TUP_DEBUG
    #ifdef Q_OS_WIN
        qDebug() << "[TabbedMainWindow::setupTabWidget()]";
    #else
        T_FUNCINFO;
    #endif
#endif

    connect(widget, SIGNAL(currentChanged(int)), this, SLOT(emitWidgetChanged(int)));
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

/**
 * Remove a widget from the window.
 * @param widget 
 */
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

int TabbedMainWindow::tabCount()
{
    return pages.count();
}

/**
 * Close the current tab.
 */
void TabbedMainWindow::closeCurrentTab()
{
    int index = currentTab->currentIndex();
    if (index >= 0)
        removeWidget(currentTab->widget(index));
}

/**
 * Sets other tab widget.
 * @param w 
 */
void TabbedMainWindow::setTabWidget(QTabWidget *widget)
{
#ifdef TUP_DEBUG
    #ifdef Q_OS_WIN
        qDebug() << "[TabbedMainWindow::setTabWidget()]";
    #else
        T_FUNCINFO;
    #endif
#endif

    currentTab->close();
    setupTabWidget(widget);

    delete currentTab;
    currentTab = 0;

    setCentralWidget(widget);
    currentTab = widget;
}

/**
 * Return the current tab widget.
 * @return 
 */
QTabWidget *TabbedMainWindow::tabWidget() const
{
    return currentTab;
}

void TabbedMainWindow::emitWidgetChanged(int index)
{
#ifdef TUP_DEBUG
    #ifdef Q_OS_WIN
        qDebug() << "[TabbedMainWindow::emitWidgetChanged()]";
    #else
        T_FUNCINFO << index;
    #endif
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
    #ifdef Q_OS_WIN
        qDebug() << "[TabbedMainWindow::setCurrentTab()]";
    #else
        T_FUNCINFO << index;
    #endif
#endif

    if (index != -1)
        currentTab->setCurrentIndex(index);
}
