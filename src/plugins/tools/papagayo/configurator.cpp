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

#include "configurator.h"
#include "tapplicationproperties.h"
#include "tseparator.h"

Configurator::Configurator(QWidget *parent) : QFrame(parent)
{
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    QLabel *toolTitle = new QLabel;
    toolTitle->setAlignment(Qt::AlignHCenter);
    QPixmap pic(THEME_DIR + "icons/papagayo.png");
    toolTitle->setPixmap(pic.scaledToWidth(20, Qt::SmoothTransformation));
    toolTitle->setToolTip(tr("Papagayo LipSync Files"));
    layout->addWidget(toolTitle);
    layout->addWidget(new TSeparator(Qt::Horizontal));

    settingsLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    settingsLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    settingsLayout->setMargin(0);
    settingsLayout->setSpacing(0);

    setLipSyncManagerPanel();
    setPropertiesPanel();

    layout->addLayout(settingsLayout);
    layout->addStretch(2);
}

Configurator::~Configurator()
{
}

void Configurator::loadLipSyncList(QList<QString> list)
{
    manager->loadLipSyncList(list);
}

void Configurator::setPropertiesPanel()
{
    settingsPanel = new Settings(this);
    connect(settingsPanel, SIGNAL(selectMouth(const QString &, int)), this, SIGNAL(selectMouth(const QString &, int)));
    connect(settingsPanel, SIGNAL(closeLipSyncProperties()), this, SLOT(closeSettingsPanel()));
    connect(settingsPanel, SIGNAL(initFrameHasChanged(int)), this, SIGNAL(initFrameHasChanged(int)));
    connect(settingsPanel, SIGNAL(xPosChanged(int)), this, SIGNAL(xPosChanged(int)));
    connect(settingsPanel, SIGNAL(yPosChanged(int)), this, SIGNAL(yPosChanged(int)));

    settingsLayout->addWidget(settingsPanel);

    activePropertiesPanel(false);
}

void Configurator::activePropertiesPanel(bool enable)
{
    if (enable)
        settingsPanel->show();
    else
        settingsPanel->hide();
}

void Configurator::setLipSyncManagerPanel()
{
    manager = new LipSyncManager(this);
    connect(manager, SIGNAL(importLipSync()), this, SIGNAL(importLipSync()));
    connect(manager, SIGNAL(editCurrentLipSync(const QString &)), this, SLOT(editCurrentLipSync(const QString &)));
    connect(manager, SIGNAL(removeCurrentLipSync(const QString &)), this, SIGNAL(removeCurrentLipSync(const QString &)));

    settingsLayout->addWidget(manager);
}

void Configurator::activeLipSyncManagerPanel(bool enable)
{
    if (enable)
        manager->show();
    else
        manager->hide();
}

void Configurator::addLipSyncRecord(const QString &name)
{
    manager->addNewRecord(name);
}

void Configurator::editCurrentLipSync(const QString &name)
{
    emit editLipSyncSelection(name);

    activeLipSyncManagerPanel(false);
    activePropertiesPanel(true);
}

void Configurator::openLipSyncProperties(TupLipSync *lipsync)
{
    settingsPanel->openLipSyncProperties(lipsync);
}

void Configurator::resetUI()
{
    manager->resetUI();
    closeSettingsPanel();
}

void Configurator::closeSettingsPanel()
{
    emit closeLipSyncProperties();
    closePanels();
}

void Configurator::closePanels()
{
    activeLipSyncManagerPanel(true);
    activePropertiesPanel(false);
}

void Configurator::updateInterfaceRecords()
{
    settingsPanel->updateInterfaceRecords();
}

void Configurator::setPos(const QPointF &point)
{
    settingsPanel->setPos(point);
}

void Configurator::setPhoneme(const QString &phoneme)
{
    settingsPanel->setPhoneme(phoneme);
}
