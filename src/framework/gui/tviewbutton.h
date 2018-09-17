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

#ifndef TVIEWBUTTON_H
#define TVIEWBUTTON_H

#include "tglobal.h"

#include <QToolButton>
#include <QStyleOptionToolButton>
#include <QToolBar>
#include <QStylePainter>
#include <QMenu>
#include <QIcon>
#include <QMouseEvent>
#include <QMainWindow>
#include <QTimer>

class ToolView;

class T_GUI_EXPORT TViewButton : public QToolButton
{
    Q_OBJECT

    public:
        TViewButton(Qt::ToolBarArea area, ToolView *toolView, QWidget * parent = 0);
        TViewButton(ToolView *toolView, QWidget *parent = 0);
        ~TViewButton();

        void setArea(Qt::ToolBarArea area);
        Qt::ToolBarArea area() const;
        void setActivated(bool flag);
        ToolView *toolView() const;
        void setup();
        void setFlag(bool value);

    protected:
        virtual void mousePressEvent(QMouseEvent *);
		
    public slots:
        void toggleView();

    private:
        Qt::ToolBarArea m_area;
        class Animator;
        QPalette m_palette;
        ToolView *m_toolView;
};

#endif
