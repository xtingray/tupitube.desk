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

#ifndef TUPGRADIENTMANAGER_H
#define TUPGRADIENTMANAGER_H

#include "tglobal.h"

#include "tapplication.h"
#include "tupgradientselector.h"
#include "tupgradientviewer.h"
#include "timagebutton.h"
#include "tcirclebutton.h"
#include "txyspinbox.h"

#include <QFrame>
#include <QComboBox>
#include <QBoxLayout>

class SpinControl;

class TUPITUBE_EXPORT TupGradientCreator : public QFrame
{
    Q_OBJECT

    public:
        TupGradientCreator(QWidget *parent = 0);
        ~TupGradientCreator();
        
        void setCurrentColor(const QColor &);
        int gradientType();
        QBrush currentGradient();
        virtual QSize sizeHint () const;

    private:        
        TupGradientSelector *selector;
        TupGradientViewer *viewer;
        QComboBox *type;
        QComboBox *spread;
        QSpinBox *radius;
        QSpinBox *angle;
        SpinControl *spinControl;

    private slots:
        void emitGradientChanged();
        
    signals:
        void gradientChanged(const QBrush &);
        void controlArrowAdded();

    public slots:
        void changeType(int type);
        void changeSpread(int spread);
        void changeGradientStops( const QGradientStops& );
        void setGradient(const QBrush & gradient);
};

class TUPITUBE_EXPORT SpinControl: public QGroupBox
{
    Q_OBJECT
    public:
        SpinControl(QWidget *parent = 0) : QGroupBox(parent)
        {
            QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
            layout->setSizeConstraint(QLayout::SetFixedSize);
            m_title = new QLabel(this);
            layout->addWidget(m_title);
            
            m_radius = new QSpinBox(this);
            connect(m_radius, SIGNAL(valueChanged(int)), this, SIGNAL(radiusChanged(int)));
            m_radius->setMaximum(100);
            layout->addWidget(m_radius);
            
            m_angle = new QSpinBox(this);
            layout->addWidget(m_angle);
            connect(m_angle, SIGNAL(valueChanged(int)), this, SIGNAL(angleChanged(int)));
            m_angle->setMaximum(360);
        };

        ~SpinControl(){};
        
        void setSpin(QGradient::Type type)
        {
            switch(type)
            {
                case QGradient::LinearGradient:
                {
                    setVisible(false);
                    m_angle->setVisible(false);
                    m_radius->setVisible(false);
                    m_title->setVisible(false);
                    break;
                }
                case  QGradient::RadialGradient:
                {
                    setVisible(true);
                    m_radius->show();
                    m_angle->hide();
                    m_title->show();
                    m_title->setText(tr("Radius"));
                    break;
                }
                case  QGradient::ConicalGradient:
                {
                    setVisible(true);
                    m_radius->hide();
                    m_angle->show();
                    m_title->show();
                    m_title->setText(tr("Angle"));
                    break;
                }
                default: break;
            }
        };
        
        int angle()
        {
            return m_angle->value();
        };

        int radius()
        {
            return m_radius->value();
        };
        
        void setAngle(int angle)
        {
            m_angle->setValue(angle);
        }
        
        void setRadius(int radius)
        {
            m_radius->setValue(radius);
        }
        
    private:
        QSpinBox *m_angle, *m_radius;
        QLabel *m_title;
        
    signals:
        void angleChanged(int angle);
        void radiusChanged(int radius);
};

#endif
