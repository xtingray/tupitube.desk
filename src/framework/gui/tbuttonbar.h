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

#ifndef TBUTTONBAR_H
#define TBUTTONBAR_H

#include "tglobal.h"
#include "tviewbutton.h"
#include "toolview.h"

#include <QToolBar>
#include <QButtonGroup>
#include <QMap>
#include <QTimer>
#include <QToolButton>
#include <QBoxLayout>
#include <QAction>
#include <QCheckBox>
#include <QMenu>
#include <QMouseEvent>

class TViewButton;
class ToolView;
class QAction;
class QMenu;

class T_GUI_EXPORT TButtonBar : public QToolBar
{
    Q_OBJECT

    public:
        TButtonBar(Qt::ToolBarArea area, QWidget *parent = nullptr);
        ~TButtonBar();

        void addButton(TViewButton *viewButton);
        void removeButton(TViewButton *viewButton);
        bool isEmpty() const;
        void disable(TViewButton *viewButton);
        void enable(TViewButton *viewButton);

        bool shouldBeVisible() const;
        // void showSeparator(bool flag);

        int count() const;

    private slots:
        void closeOtherPanels(QAbstractButton *source);

    private:
        QButtonGroup m_buttons;
        QMap<QWidget *, QAction *> m_actionForWidget;
        // QAction *m_separator;
        QTimer m_hider;
        bool m_blockHider;
        bool m_shouldBeVisible;
};

#endif
