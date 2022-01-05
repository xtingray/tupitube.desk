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

#include "eyedroppersettings.h"
#include "tconfig.h"
#include "tseparator.h"

#include <QBoxLayout>
#include <QPushButton>

EyeDropperSettings::EyeDropperSettings(QWidget *parent): QWidget(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[EyeDropperSettings()]";
    #endif

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
    layout->setAlignment(Qt::AlignHCenter);

    QLabel *eyeDropperLabel = new QLabel;
    eyeDropperLabel->setAlignment(Qt::AlignHCenter);
    QPixmap pencilPic(THEME_DIR + "icons/eyedropper.png");
    eyeDropperLabel->setPixmap(pencilPic.scaledToWidth(16, Qt::SmoothTransformation));
    eyeDropperLabel->setToolTip(tr("Eye Dropper Properties"));

    layout->addWidget(eyeDropperLabel);
    layout->addWidget(new TSeparator(Qt::Horizontal));
    layout->addSpacing(10);

    colorCell = new TColorCell(TColorCell::None, Qt::white, QSize(50, 50));
    layout->addWidget(colorCell);

    QHBoxLayout *colorLayout = new QHBoxLayout;
    colorLabel = new QLabel("");
    colorLabel->setAlignment(Qt::AlignCenter);
    colorLayout->addWidget(colorLabel);

    mainLayout->addLayout(layout);
    mainLayout->addLayout(colorLayout);
    mainLayout->addStretch(2);
}

EyeDropperSettings::~EyeDropperSettings()
{
}

void EyeDropperSettings::updateColor(const QColor &color)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[EyeDropperSettings::updateColor()] - color -> " << color;
    #endif
    */

    colorCell->setBrush(QBrush(color));
    colorLabel->setText(color.name());
}
