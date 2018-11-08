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

#include <QCheckBox>

struct TupGeneralPreferences::Private
{
    QStringList startup;
    QStringList confirmation;
    QStringList player;

    QList<QCheckBox *> startupList;
    QList<QCheckBox *> confirmList;
    QList<QCheckBox *> playerList;
};

TupGeneralPreferences::TupGeneralPreferences() : k(new Private)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    k->startup << "OpenLastProject" << "ShowTipOfDay";

    QStringList labels;
    labels << tr("Always open last project") << tr("Show tip of the day");

    QGridLayout *startupForm = createForm("General", Startup, k->startup, labels);

    k->confirmation << "ConfirmRemoveFrame" << "ConfirmRemoveLayer"
                    << "ConfirmRemoveScene" << "ConfirmRemoveObject";

    labels.clear();
    labels << tr("Confirm \"Remove frame\" action") << tr("Confirm \"Remove layer\" action")
           << tr("Confirm \"Remove scene\" action") << tr("Confirm \"Remove object\" action from library");

    QGridLayout *confirmForm = createForm("General", Confirm, k->confirmation, labels);

    k->player << "AutoPlay";

    labels.clear();
    labels << tr("Render and play project automatically");

    QGridLayout *playerForm = createForm("AnimationParameters", Player, k->player, labels);

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
        k->startupList = list;
    else if (group == Confirm)
        k->confirmList = list;
    else if (group == Player)
        k->playerList = list;

    return form;
}

void TupGeneralPreferences::saveValues()
{
    TCONFIG->beginGroup("General");

    int total = k->startup.count();
    for (int i=0; i<total; i++)
         TCONFIG->setValue(k->startup.at(i), k->startupList.at(i)->isChecked());

    total = k->confirmation.count();
    for (int i=0; i<total; i++)
         TCONFIG->setValue(k->confirmation.at(i), k->confirmList.at(i)->isChecked());

    TCONFIG->beginGroup("AnimationParameters");
    total = k->player.count();
    for (int i=0; i<total; i++)
         TCONFIG->setValue(k->player.at(i), k->playerList.at(i)->isChecked());

    TCONFIG->sync();
}
