/***************************************************************************
 *   Project TUPITUBE DESK                                                 *
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

#include "tcollapsiblewidget.h"

// Collapsible Widget

TCollapsibleWidget::TCollapsibleWidget(QWidget *parent): QWidget(parent)
{    
    innerWidget = nullptr;
    mainLayout = new QGridLayout(this);
    mainLayout->setMargin(0);
    status = false;

    setExpanded(false);
    setEnabled(false);

}

TCollapsibleWidget::~TCollapsibleWidget()
{
}

QWidget *TCollapsibleWidget::getWidget() const
{
    return innerWidget;
}

void TCollapsibleWidget::setWidget(QWidget *widget)
{
    if (!widget)
        return;
    
    QGroupBox *container = new QGroupBox(this);
    widget->setParent(container);
    
    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    innerWidget = widget;
    
    containerLayout->addWidget(widget);

    mainLayout->addWidget(container, 1, 1);
    // mainLayout->setRowStretch(2, 1);
    
    setEnabled(true);
    setExpanded(isExpanded());
}

void TCollapsibleWidget::setExpanded(bool expanded)
{
    if (innerWidget) {
        setUpdatesEnabled(false);

        status = expanded;
        innerWidget->parentWidget()->setVisible(expanded);
        innerWidget->setVisible(expanded);
        
        setUpdatesEnabled(true);
    }
}

bool TCollapsibleWidget::isExpanded() const
{
    return status;
}
