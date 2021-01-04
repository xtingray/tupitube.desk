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

#ifndef TWIDGETLISTVIEW_H
#define TWIDGETLISTVIEW_H

#include "tglobal.h"

#include <QTableWidget>
#include <QMap>
#include <QHeaderView>
#include <QResizeEvent>

class T_GUI_EXPORT TWidgetListView : public QTableWidget
{
    Q_OBJECT

    public:
          TWidgetListView(QWidget * parent = nullptr);
          ~TWidgetListView();

          QTableWidgetItem *addWidget(QWidget *widget);
          QTableWidgetItem *insertWidget(int pos, QWidget *widget);

          QWidget *widget(QTableWidgetItem *treeItem);

          QTableWidgetItem *item(QWidget *widget);

          void moveItemUp(int index);
          void moveItemDown(int index);
          int currentVisualRow() const;

    protected:
          void resizeEvent(QResizeEvent *e);

    signals:
          void itemSelected(int index);

    private:
        QMap<QWidget *, QTableWidgetItem *> m_items;
};

#endif
