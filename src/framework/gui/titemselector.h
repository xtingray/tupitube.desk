/***************************************************************************
 *   Project TUPITUBE DESK                                                 *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2019:                                                                 *
 *    Alejandro Carrasco Rodríguez                                         *
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

#ifndef TITEMSELECTOR_H
#define TITEMSELECTOR_H

#include "tglobal.h"
#include "tapplicationproperties.h"

#include <QWidget>
#include <QListWidget>
#include <QLabel>

class T_GUI_EXPORT TItemSelector : public QWidget
{
    Q_OBJECT

    public:
        TItemSelector(const QString &topLabel, const QString &bottomLabel,
                      QWidget *parent = nullptr);
        ~TItemSelector();
        
        void setItems(const QStringList &items);
        int addItem(const QString &item);
        void addSelectedItem(const QString &itemLabel);
        void addItems(const QStringList &items);
        
        QStringList selectedItems() const;
        QList<int> selectedIndexes() const;
        
        void clear();
        void reset();
        void selectFirstItem();

        void setDurationLabelVisible(bool visible);
        void updateDurationLabel(const QString &duration);

    private slots:
        void addCurrent();
        void removeCurrent();
        void upCurrent();
        void downCurrent();
        
    signals:
        void changed();
        
    private:
        QLabel *durationLabel;
        QListWidget *available;
        QListWidget *selected;
};

#endif
