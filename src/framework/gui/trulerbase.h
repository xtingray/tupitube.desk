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

#ifndef RULERBASE_H
#define RULERBASE_H

#include "tglobal.h"

#include <QMouseEvent>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QMenu>
#include <QPolygonF>
#include <QFrame>
#include <QPainter>

#define UNITCOUNT 5

class TRulerBase;

class T_GUI_EXPORT TRulerBase : public QFrame
{
    Q_OBJECT
    
    public:
        TRulerBase(Qt::Orientation direction=Qt::Horizontal, QWidget *parent = nullptr);
        virtual ~TRulerBase();
        
        // const double unitGetRatioFromIndex(const int index);
        // const double pts2mm(double pts);
        // const double mm2pts(double mm);

        virtual void drawScale(QPainter *painter);
        Qt::Orientation orientation();
        
        int separation() const;
        double scaleFactor() const;
        QPointF zero() const;
        
        void translateArrow(double dx, double dy);
        
    public slots:

        void setZeroAt(const QPointF & pos);
        void scale(double factor);
        
    private:
        enum { ChangeScaleToFive, ChangeScaleToTen };

        bool drawPointer;
        int position;
        int ruleSeparation;
        int ruleWidth;
        int ruleHeight;

        Qt::Orientation ruleDirection;
        QPointF ruleZero;
        QPolygonF pArrow;
        QMenu *menu;
        double sFactor;

signals:
        void displayMenu(TRulerBase *, QPoint pos);
        
    protected:
        virtual void paintEvent(QPaintEvent *e);
        virtual void resizeEvent(QResizeEvent *);
        virtual void mouseMoveEvent(QMouseEvent *e);
        virtual void mousePressEvent(QMouseEvent *e);
        virtual QSize sizeHint() const;
        
    public slots:
        virtual void movePointers(const QPointF &pos) = 0;
        void setSeparation(int sep);
        void setDrawPointer(bool yes = true);
        void slide(int value);
        
        virtual void showMenu(TRulerBase *, QPoint pos);
        
    private slots:
        void changeScaleTo5pts();
        void changeScaleTo10pts();
};

#endif
