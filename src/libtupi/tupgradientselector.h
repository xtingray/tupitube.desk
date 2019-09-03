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

#ifndef TUPGRADIENTSELECTOR_H
#define TUPGRADIENTSELECTOR_H

#include "tglobal.h"

#include <QAbstractSlider>
#include <QColor>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QPoint>
#include <QLinearGradient>
#include <QPainterPath>
#include <qdrawutil.h>
#include <QPolygon>
#include <QList>

class TUPITUBE_EXPORT TupGradientSelector : public QAbstractSlider
{
   Q_OBJECT
   
   private:

      class TUPITUBE_EXPORT TupGradientArrow : public QObject
      {
         public:

            TupGradientArrow(QPoint pos, const QColor& color, QObject *parent = 0) : QObject(parent), m_color(color)
            {
               QPolygon array(6);
               array.setPoint(0, pos.x() + 0, pos.y() + 0);
               array.setPoint(1, pos.x() + 5, pos.y() + 5);
               array.setPoint(2, pos.x() + 5, pos.y() + 9);
               array.setPoint(3, pos.x() - 5, pos.y() + 9);
               array.setPoint(4, pos.x() - 5, pos.y() + 5);
               array.setPoint(5, pos.x() + 0, pos.y() + 0);

               m_form.addPolygon(array);
            }

            ~TupGradientArrow(){}

            double position() 
            {
               return m_form.currentPosition().x();
            }

            bool contains(const QPoint &pt)
            {
               return m_form.contains(pt);
            }

            void moveArrow(const QPoint &pos)
            {   
               QTransform t;
               t.translate(pos.x() - m_form.currentPosition().x(), 0);
               m_form = t.map(m_form); 
            }

            QPainterPath form()
            {
               return m_form;
            }

            QColor color() const
            {
               return m_color;
            }

            void setColor(const QColor &color)
            {
               m_color = color;
            }

            void moveVertical(const QPoint &pos)
            {
               QTransform t;
               t.translate(0, pos.y() - m_form.currentPosition().y());
               m_form = t.map(m_form);
            }

            QPainterPath m_form;
            QColor m_color;
      };
      
   public:

      TupGradientSelector(QWidget *parent=0);
      TupGradientSelector(Qt::Orientation o, QWidget *parent = 0);
      ~TupGradientSelector();

      void setStops(const QGradientStops &);
      
      Qt::Orientation orientation() const
      {   
         return _orientation; 
      }

      QRect contentsRect() const;

      void setValue(int value)
      {
         QAbstractSlider::setValue(value);
      }

      int value() const
      {
         return QAbstractSlider
            ::value();
      }

      void setMaxArrows(int value);

      QGradientStops  gradientStops() const
      {
         return m_gradient.stops();
      }

      void createGradient();
      
      void setCurrentColor(const QColor& color);

   signals:
      void newValue(int value);
      void gradientChanged(const QGradientStops&);
      void arrowAdded();
      
   public slots:
      void addArrow(QPoint position, QColor color);
      virtual void valueChange(int value);

   protected:
      virtual void drawContents(QPainter *);
      
      virtual void paintEvent(QPaintEvent *);
      virtual void mousePressEvent(QMouseEvent *event);
      virtual void mouseMoveEvent(QMouseEvent *event);
      virtual void wheelEvent(QWheelEvent *event);

      virtual QSize minimumSize() const
      { 
         return sizeHint(); 
      }

      virtual void resizeEvent(QResizeEvent * event);

      virtual QSize sizeHint() const
      {
         return QSize(width(), 35);
      }
      
   private:
      QPoint calcArrowPos(int val);
      void moveArrow(const QPoint &pos);
      double valueToGradient(int _value) const;
      void init();

   private:
      Qt::Orientation _orientation;
      int m_currentArrowIndex;
      QLinearGradient m_gradient;
      QList<TupGradientArrow*> m_arrows;
      bool m_update;
      int m_maxArrows;
      QColor m_currentColor;
};

#endif
