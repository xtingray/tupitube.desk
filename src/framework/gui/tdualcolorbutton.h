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

#ifndef TDUALCOLORBUTTON_H
#define TDUALCOLORBUTTON_H

#include "tglobal.h"

class T_GUI_EXPORT TDualColorButton : public QWidget
{
    Q_OBJECT

    Q_ENUMS(ColorSpace)
    Q_PROPERTY(QBrush foreground READ foreground WRITE setForeground)
    Q_PROPERTY(QBrush background READ background WRITE setBackground)
    Q_PROPERTY(QBrush currentColor READ currentColor WRITE setCurrentColor STORED false DESIGNABLE false)
    Q_PROPERTY(ColorSpace current READ current WRITE setCurrent)

    public:
        enum ColorSpace {Foreground, Background};
        TDualColorButton(QWidget *parent=0);

        TDualColorButton(const QBrush &fgColor, const QBrush &bgColor, QWidget *parent=0);
        ~TDualColorButton();

        QBrush foreground() const;
        QBrush background() const;

        ColorSpace current() const;
        QBrush currentColor() const;

        virtual QSize sizeHint() const;

    public slots:
        void setForeground(const QBrush &c);
        void setBackground(const QBrush &c);
        void setCurrent(ColorSpace s);
        void setCurrentColor(const QBrush &c);

    signals:
        void selectionChanged(TDualColorButton::ColorSpace space);
        void switchColors();
        void resetColors();

    protected:
        virtual void metrics(QRect &fgRect, QRect &bgRect);
        virtual void paintEvent(QPaintEvent *ev);
        virtual void mousePressEvent(QMouseEvent *ev);

    private:
        struct Private;
        Private *const k;
};

#endif
