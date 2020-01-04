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

#include "tupthemepreferences.h"
#include "tosd.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

TupThemePreferences::TupThemePreferences(QWidget *parent) : QWidget(parent)
{
    setupPage();
}

TupThemePreferences::~TupThemePreferences()
{
}

void TupThemePreferences::setupPage()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QWidget *widget = new QWidget;
    QVBoxLayout *pageLayout = new QVBoxLayout;

    QLabel *generalLabel = new QLabel(tr("Theme Preferences"));
    QFont labelFont = font();
    labelFont.setBold(true);
    labelFont.setPointSize(labelFont.pointSize() + 3);
    generalLabel->setFont(labelFont);
    pageLayout->addWidget(generalLabel);

    pageLayout->addSpacing(15);

    lightTheme = new QRadioButton(tr("Light Theme"), this);
    pageLayout->addWidget(lightTheme);

    darkTheme = new QRadioButton(tr("Dark Theme"), this);
    pageLayout->addWidget(darkTheme);

    TCONFIG->beginGroup("General");
    QString themeName = TCONFIG->value("Theme", "Light").toString();
    if (themeName.compare("Light") == 0)
        lightTheme->setChecked(true);
    else
        darkTheme->setChecked(true);

    connect(lightTheme, SIGNAL(clicked(bool)), this, SLOT(showRestartMsg(bool)));
    connect(darkTheme, SIGNAL(clicked(bool)), this, SLOT(showRestartMsg(bool)));

    pageLayout->addSpacing(15);

    /*
    QLabel *noteLabel = new QLabel(tr("Note: You must restart TupiTube to apply theme changes"));
    pageLayout->addWidget(noteLabel);
    */

    widget->setLayout(pageLayout);
    layout->addWidget(widget);
    layout->setAlignment(widget, Qt::AlignLeft);
    layout->addStretch(3);
}

void TupThemePreferences::saveValues()
{
    TCONFIG->beginGroup("General");
    if (lightTheme->isChecked())
        TCONFIG->setValue("Theme", "Light");
    else
        TCONFIG->setValue("Theme", "Dark");

    TCONFIG->sync();
}

void TupThemePreferences::showRestartMsg(bool enabled)
{
    if (enabled)
        TOsd::self()->display(tr("Warning"), tr("Please restart TupiTube"), TOsd::Warning);
}
