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

#include "polylinesettings.h"
#include "tapplicationproperties.h"
#include "tseparator.h"
#include "tresponsiveui.h"

PolylineSettings::PolylineSettings(QWidget *parent) : QWidget(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "PolylineSettings()";
    #endif

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);

    QLabel *toolTitle = new QLabel;
    toolTitle->setAlignment(Qt::AlignHCenter);
    QPixmap pic(THEME_DIR + "icons/polyline.png");
    toolTitle->setPixmap(pic.scaledToWidth(TResponsiveUI::fitTitleIconSize(), Qt::SmoothTransformation));
    toolTitle->setToolTip(tr("PolyLine Properties"));
    layout->addWidget(toolTitle);
    layout->addWidget(new TSeparator(Qt::Horizontal));

    QLabel *label = new QLabel(tr("Tips"));
    label->setAlignment(Qt::AlignHCenter); 
    layout->addWidget(label);

    mainLayout->addLayout(layout);

    int minWidth = TResponsiveUI::fitRightPanelWidth();
    QTextEdit *textArea = new QTextEdit;
    textArea->setMinimumWidth(minWidth);
    textArea->setMaximumWidth(minWidth*2);
    textArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    textArea->setFixedHeight(250);
    textArea->setHtml("<p><b>" + tr("X Key or Right Mouse Button") + ":</b> " + tr("Close line") + "</p>"); 
    mainLayout->addWidget(textArea);
   
    mainLayout->addStretch(2);
}

PolylineSettings::~PolylineSettings()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~Settings()]";
    #endif
}

