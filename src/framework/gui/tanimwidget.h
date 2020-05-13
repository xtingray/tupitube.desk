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

#ifndef TANIMWIDGET_H
#define TANIMWIDGET_H

// #include "tglobal.h"
#include "tapplication.h"

#include <QPixmap>
#include <QHideEvent>
#include <QShowEvent>
#include <QPainter>
#include <QFontMetricsF>
#include <QList>
#include <QWidget>

typedef QList<QPixmap> ListOfPixmaps;

class T_GUI_EXPORT TAnimWidget : public QWidget
{
    public:
        enum Type { AnimText = 0, AnimPixmap };

        TAnimWidget(const QPixmap &px, const QString &text, QWidget *parent = nullptr);
        TAnimWidget(ListOfPixmaps lop, QWidget *parent = nullptr);
        ~TAnimWidget();

        void setBackgroundPixmap(const QPixmap &px);
        void activateAnimation();

    protected:
        void showEvent(QShowEvent *);
        void hideEvent(QHideEvent *);

    protected:
        void timerEvent(QTimerEvent *);
        void paintEvent(QPaintEvent *);

    private:
        Type m_type;
        class Controller;
        Controller *m_controller;
        QPixmap m_background;
        QString m_text;
        QRectF m_textRect;

        ListOfPixmaps m_pixmaps;
        int m_pixmapIndex;
        int m_counter;
        int m_lines;
        int fontSize;
        int m_end;
};

#endif
