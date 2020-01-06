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

#include "tupbackgrounditem.h"
#include "tconfig.h"
#include "tapplicationproperties.h"
#include "tseparator.h"

#include <QBoxLayout>
#include <QLabel>

TupBackgroundItem::TupBackgroundItem(TupBackground::BgType id, const QString &title, bool viewFlag,
                                     QWidget *parent) : QWidget(parent)
{
    itemId = id;
    isVisible = viewFlag;

    QBoxLayout *layout = new QHBoxLayout(this);
    viewIconOn = QPixmap(THEME_DIR + "icons/show_layer.png");
    viewIconOff = QPixmap(THEME_DIR + "icons/hide_layer.png");

    viewButton = new QPushButton;
    viewButton->setToolTip(tr("Background Layer Visibility"));
    if (isVisible)
        viewButton->setIcon(QIcon(viewIconOn));
    else
        viewButton->setIcon(QIcon(viewIconOff));

    viewButton->setCheckable(true);
    viewButton->setChecked(isVisible);
    viewButton->setFixedWidth(30);

    connect(viewButton, SIGNAL(clicked(bool)), this, SLOT(updateVisibility(bool)));

    TSeparator *separator = new TSeparator(Qt::Vertical);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setAttribute(Qt::WA_TranslucentBackground);

    layout->addWidget(viewButton);
    layout->addWidget(separator);
    layout->addWidget(titleLabel);
}

TupBackgroundItem::~TupBackgroundItem()
{
}

void TupBackgroundItem::updateVisibility(bool clicked)
{
    isVisible = clicked;

    if (clicked)
        viewButton->setIcon(QIcon(viewIconOn));
    else
        viewButton->setIcon(QIcon(viewIconOff));
}

QPair<TupBackground::BgType, bool> TupBackgroundItem::getValues()
{
    QPair<TupBackground::BgType, bool> response;
    response.first = itemId;
    response.second = isVisible;

    return response;
}
