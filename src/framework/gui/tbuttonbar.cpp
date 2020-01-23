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

#include "tbuttonbar.h"

TButtonBar::TButtonBar(Qt::ToolBarArea area, QWidget *parent) : QToolBar(parent), m_shouldBeVisible(true)
{
    setMovable(false);
    setIconSize(QSize(16, 16));
    m_buttons.setExclusive(true);

    switch (area) {
        case Qt::LeftToolBarArea:
             {
                setWindowTitle(tr("Left button bar"));
             }
             break;
        case Qt::RightToolBarArea:
             {
                setWindowTitle(tr("Right button bar"));
             }
             break;
        case Qt::BottomToolBarArea:
             {
                setWindowTitle(tr("Bottom button bar"));
             }
             break;
        case Qt::TopToolBarArea:
             {
                setWindowTitle(tr("Top button bar"));
             }
             break;
        default: break;
    }

    setObjectName("TButtonBar-"+windowTitle());

    /*
    m_separator = addAction("");
    m_separator->setEnabled(false); // Separator
    m_separator->setVisible(false);
    */

    connect(&m_hider, SIGNAL(timeout()), this, SLOT(hide()));
    connect(&m_buttons, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(closeOtherPanels(QAbstractButton *)));
}

TButtonBar::~TButtonBar()
{
}

void TButtonBar::addButton(TViewButton *viewButton)
{
    /*
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TButtonBar::addButton()]";
        #else
            T_FUNCINFO;
        #endif
    #endif
    */

    QAction *action = addWidget(viewButton);
    m_buttons.addButton(viewButton);

    // if (viewButton->toolView()->isVisible())
    //     viewButton->toggleView();
    //     closeOtherPanels(viewButton);

    m_actionForWidget[viewButton] = action;
    action->setVisible(true);

    addSeparator();

    if (!isVisible()) 
        show();
}

void TButtonBar::removeButton(TViewButton *viewButton)
{
    /*
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TButtonBar::removeButton()]";
        #else
            T_FUNCINFO;
        #endif
    #endif
    */

    if (!m_buttons.buttons().contains(viewButton)) 
        return;

    m_buttons.removeButton(viewButton);
    removeAction(m_actionForWidget[viewButton]);
    viewButton->setParent(nullptr);

    if (isEmpty()) 
        hide();
}

bool TButtonBar::isEmpty() const
{
    return m_buttons.buttons().isEmpty(); 
}

bool TButtonBar::shouldBeVisible() const
{
    return m_shouldBeVisible;
}

void TButtonBar::disable(TViewButton *view)
{
    QAction *action = m_actionForWidget[view];

    if (action)
        action->setVisible(false);
}

void TButtonBar::enable(TViewButton *view)
{
    QAction *action = m_actionForWidget[view];

    if (action)
        action->setVisible(true);
}

void TButtonBar::closeOtherPanels(QAbstractButton *source)
{
    #ifdef TUP_DEBUG
        qInfo() << "[TButtonBar::closeOtherPanels()]";
    #endif

    /*
    if (!m_buttons.exclusive()) {
        static_cast<TViewButton *>(source)->toggleView();
        return;
    }
    */

    // setUpdatesEnabled(false);

    foreach (QAbstractButton *item, m_buttons.buttons()) {
        TViewButton *button = static_cast<TViewButton *>(item);
        if (source != button) {
            if (button->toolView()->isVisible()) {
                button->blockSignals(true);
                button->toggleView();
                button->blockSignals(false);
                break;
            }
        }
    }

    static_cast<TViewButton *>(source)->toggleView();

    // setUpdatesEnabled(true);
}

/*
void TButtonBar::showSeparator(bool event)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TButtonBar::showSeparator()]";
        #else
            T_FUNCINFO << event;
        #endif
    #endif

    m_separator->setVisible(event);
}
*/

int TButtonBar::count() const
{
    return m_buttons.buttons().count();
}
