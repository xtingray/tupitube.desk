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

#include "tupgradientcreator.h"

TupGradientCreator::TupGradientCreator(QWidget *parent) : QFrame(parent)
{
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight);

    layout->setSpacing(2);
    // layout->setMargin(2);
    setLayout(layout);
    
    QBoxLayout *selectorAndViewer = new QBoxLayout(QBoxLayout::TopToBottom);
    
    selector = new TupGradientSelector(this);
    viewer = new TupGradientViewer(this);
    
    connect(viewer, SIGNAL(gradientChanged()), this, SLOT(emitGradientChanged()));
    layout->addLayout(selectorAndViewer);
    
    selectorAndViewer->addWidget(viewer);
    selectorAndViewer->addWidget(selector);
    selectorAndViewer->addStretch(2);
    
    connect(selector, SIGNAL(gradientChanged(QGradientStops)),this, SLOT(changeGradientStops(QGradientStops)));
    connect(selector, SIGNAL(arrowAdded()), this, SIGNAL(controlArrowAdded()));
    
    QBoxLayout *subLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    layout->addLayout(subLayout);
    
    type = new QComboBox(this);
    QStringList list;
    list << tr("Linear") << tr("Radial") << tr("Conical");
    type->addItems(list);
    connect(type, SIGNAL(activated(int)),this, SLOT(changeType(int)));
    subLayout->addWidget(type);
    
    spread = new QComboBox(this);
    list.clear();
    list << tr("Pad") << tr("Reflect") << tr("Repeat");
    spread->addItems(list);
    connect(spread, SIGNAL(activated(int)),this, SLOT(changeSpread(int)));
    subLayout->addWidget(spread);
    
    spinControl = new SpinControl(this);
    
    connect(spinControl, SIGNAL(angleChanged(int)), viewer, SLOT(changeAngle(int)));
    connect(spinControl, SIGNAL(radiusChanged(int)), viewer, SLOT(changeRadius(int)));
    subLayout->addWidget(spinControl);
    
    subLayout->setSpacing(2);
    // subLayout->setMargin(2);
    
    setFrameStyle(QFrame::StyledPanel);
    
    spinControl->setSpin(QGradient::Type(0));
    spinControl->setRadius(50);
    
    subLayout->addStretch(2);
}

TupGradientCreator::~TupGradientCreator()
{
}

void TupGradientCreator::setCurrentColor(const QColor &color)
{
    Q_UNUSED(color)

    // SQA: This code has been disabled temporarily 
    /*
    selector->setCurrentColor(color);
    viewer->createGradient();
    */

    // emit gradientChanged(QBrush(viewer->gradient()));
}

int TupGradientCreator::gradientType()
{
    return type->currentIndex();
}

void TupGradientCreator::changeType(int type)
{
    viewer->changeType(type);
    
    spinControl->setSpin( QGradient::Type(type));
    adjustSize();

    emitGradientChanged();
}

void TupGradientCreator::changeSpread(int spread)
{
    viewer->setSpread(spread);

    emitGradientChanged();
}

void TupGradientCreator::changeGradientStops(const QGradientStops& stops)
{
    viewer->changeGradientStops(stops);

    emit gradientChanged(viewer->gradient());
}

void TupGradientCreator::setGradient(const QBrush &brush)
{
    const QGradient *gradient = brush.gradient();

    if (gradient) {
        type->setCurrentIndex(gradient->type());
        spread->setCurrentIndex(gradient->spread());
        selector->setStops(gradient->stops());
        viewer->setGradient(gradient);
        spinControl->setSpin(gradient->type());

        if (gradient->type() == QGradient::RadialGradient) {
            spinControl->setRadius((int) static_cast<const QRadialGradient*>(gradient)->radius());
        } else if (gradient->type() == QGradient::ConicalGradient) {
            spinControl->setAngle((int) static_cast<const QConicalGradient*>(gradient)->angle());
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupGradientCreator::setGradient()] - Error: Brush has no gradient (null)";
        #endif
    }
}

void TupGradientCreator::emitGradientChanged()
{
    viewer->changeGradientStops(selector->gradientStops());

    emit gradientChanged(viewer->gradient());
}

QBrush TupGradientCreator::currentGradient()
{
    return QBrush(viewer->gradient());
}

QSize TupGradientCreator::sizeHint() const
{
    QSize size = QFrame::sizeHint();
    
    // return size.expandedTo(QApplication::globalStrut());
    return size;
}
