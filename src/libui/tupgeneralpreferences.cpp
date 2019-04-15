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

#include "tupgeneralpreferences.h"
#include "tconfig.h"
#include "tformfactory.h"

TupGeneralPreferences::TupGeneralPreferences()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    startup << "OpenLastProject" << "ShowTipOfDay";

    QStringList labels;
    labels << tr("Always open last project") << tr("Show tip of the day");

    QGridLayout *startupForm = createForm("General", Startup, startup, labels);

    confirmation << "ConfirmRemoveFrame" << "ConfirmRemoveLayer"
                    << "ConfirmRemoveScene" << "ConfirmRemoveObject";

    labels.clear();
    labels << tr("Confirm \"Remove frame\" action") << tr("Confirm \"Remove layer\" action")
           << tr("Confirm \"Remove scene\" action") << tr("Confirm \"Remove object\" action from library");

    QGridLayout *confirmForm = createForm("General", Confirm, confirmation, labels);

    player << "AutoPlay";

    labels.clear();
    labels << tr("Render and play project automatically");

    QGridLayout *playerForm = createForm("AnimationParameters", Player, player, labels);

    QWidget *widget = new QWidget;
    QVBoxLayout *widgetLayout = new QVBoxLayout; 

    QLabel *generalLabel = new QLabel(tr("General Preferences"));
    QFont labelFont = font();
    labelFont.setBold(true);
    labelFont.setPointSize(labelFont.pointSize() + 3);
    generalLabel->setFont(labelFont);
    widgetLayout->addWidget(generalLabel);
    widgetLayout->addSpacing(15);

    QLabel *startupLabel = new QLabel(tr("On Startup"));
    labelFont = font();
    labelFont.setBold(true);
    startupLabel->setFont(labelFont);
    widgetLayout->addWidget(startupLabel);
    widgetLayout->addLayout(startupForm);

    widgetLayout->addSpacing(15);

    QLabel *confirmLabel = new QLabel(tr("Confirmation Dialogs"));
    confirmLabel->setFont(labelFont);
    widgetLayout->addWidget(confirmLabel);
    widgetLayout->addLayout(confirmForm);

    widgetLayout->addSpacing(15);

    QLabel *playerLabel = new QLabel(tr("On Player"));
    playerLabel->setFont(labelFont);
    widgetLayout->addWidget(playerLabel);
    widgetLayout->addLayout(playerForm);

    widget->setLayout(widgetLayout);
    layout->addWidget(widget);
    layout->setAlignment(widget, Qt::AlignLeft);
    layout->addStretch(3);
}

TupGeneralPreferences::~TupGeneralPreferences()
{
}

QGridLayout * TupGeneralPreferences::createForm(const QString &groupName, Group group,
                                                QStringList keys, QStringList labels)
{
    QGridLayout *form = new QGridLayout;
    int total = labels.count();

    TCONFIG->beginGroup(groupName);
    QList<QCheckBox *> list;
    for (int i=0; i<total; i++) {
         bool flag = TCONFIG->value(keys.at(i)).toBool();
         QCheckBox *check = new QCheckBox(labels.at(i));
         check->setChecked(flag);
         list << check;
         form->addWidget(check, i, 0, Qt::AlignLeft);
    }

    if (group == Startup)
        startupList = list;
    else if (group == Confirm)
        confirmList = list;
    else if (group == Player)
        playerList = list;

    return form;
}

void TupGeneralPreferences::saveValues()
{
    TCONFIG->beginGroup("General");

    int total = startup.count();
    for (int i=0; i<total; i++)
         TCONFIG->setValue(startup.at(i), startupList.at(i)->isChecked());

    total = confirmation.count();
    for (int i=0; i<total; i++)
         TCONFIG->setValue(confirmation.at(i), confirmList.at(i)->isChecked());

    TCONFIG->beginGroup("AnimationParameters");
    total = player.count();
    for (int i=0; i<total; i++)
         TCONFIG->setValue(player.at(i), playerList.at(i)->isChecked());

    TCONFIG->sync();
}
