/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez                                                     *
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

#ifndef TUPMODESITEM_H
#define TUPMODESITEM_H

#include "tglobal.h"
#include "tupbackground.h"

#include <QWidget>
#include <QPushButton>

class TUPITUBE_EXPORT TupModesItem: public QWidget
{
    Q_OBJECT

    public:
        TupModesItem(TupBackground::BgType id, const QString &title, bool isVisible,
                     QWidget *parent = nullptr);
        ~TupModesItem();

        TupBackground::BgType bgType();
        QString & itemLabel();
        bool visibility();
        QPair<TupBackground::BgType, bool> getValues();

    private slots:
        void updateVisibility(bool clicked);

    private:
        TupBackground::BgType itemId;
        QString label;
        QPixmap viewIconOn;
        QPixmap viewIconOff;
        QPushButton *viewButton;
        bool isVisible;
};

#endif // TUPMODESITEM_H
