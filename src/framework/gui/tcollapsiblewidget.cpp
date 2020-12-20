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
    mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    stack = new QStackedWidget;
    separator = new TSeparator;
    mainLayout->addWidget(separator);
    mainLayout->addWidget(stack, Qt::AlignCenter);

    status = false;
    setExpanded(false);
    setEnabled(false);
}

TCollapsibleWidget::~TCollapsibleWidget()
{
}

void TCollapsibleWidget::addWidget(QWidget *widget)
{
    if (!widget)
        return;
    stack->addWidget(widget);
}

void TCollapsibleWidget::setCurrentIndex(int index)
{
    stack->setCurrentIndex(index);
}

void TCollapsibleWidget::setExpanded(bool expanded)
{
    if (stack) {
        setUpdatesEnabled(false);

        status = expanded;
        separator->setVisible(expanded);
        stack->setVisible(expanded);
        setEnabled(expanded);

        setUpdatesEnabled(true);
    }
}

bool TCollapsibleWidget::isExpanded() const
{
    return status;
}
