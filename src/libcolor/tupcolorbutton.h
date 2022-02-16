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

#ifndef TUPCOLORBUTTON_H
#define TUPCOLORBUTTON_H

#include "tglobal.h"
#include "tconfig.h"
#include "tapplicationproperties.h"

#include <QBrush>
#include <QSize>
#include <QPaintEvent>
#include <QWidget>

class TUPITUBE_EXPORT TupColorButton : public QWidget
{
    Q_OBJECT

    public:
        TupColorButton(int colorIndex, const QString &name, const QBrush &brush,
                       const QSize &size, const QString &buttonParams);
        ~TupColorButton();

        QSize sizeHint() const;
        QColor color();
        void setState(bool isSelected);
        bool isSelected();
        void setBrush(const QBrush &brush);
        void setEditable(bool flag);
        int getIndex();

    protected:
        void paintEvent(QPaintEvent *painter);
        void mousePressEvent(QMouseEvent *event);
        void mouseDoubleClickEvent(QMouseEvent *event);

    signals:
        void clicked(int index);
        void doubledClicked(int index);

    private:
        int border1;
        int border2;
        int border3;

        QBrush brush;
        int index;
        bool editable;
        bool selected;
        QSize size;
        int uiTheme;
};

#endif
