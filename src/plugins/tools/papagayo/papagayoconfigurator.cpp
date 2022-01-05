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

#include "papagayoconfigurator.h"
#include "tapplicationproperties.h"
#include "tseparator.h"
#include "mouthsdialog.h"

PapagayoConfigurator::PapagayoConfigurator(QWidget *parent) : QFrame(parent)
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

    QHBoxLayout *mouthsLayout = new QHBoxLayout;
    QPushButton *mouthsButton = new QPushButton(" " + tr("Mouth Samples"));
    mouthsButton->setStyleSheet("QPushButton { padding: 5px; }");
    mouthsButton->setIcon(QIcon(THEME_DIR + "icons/mouth_samples.png"));
    connect(mouthsButton, SIGNAL(clicked()), this, SLOT(openMouthsDialog()));
    mouthsLayout->addWidget(new QWidget);
    mouthsLayout->addWidget(mouthsButton);
    mouthsLayout->addWidget(new QWidget);
    layout->addLayout(mouthsLayout);

    layout->addStretch(2);
}

PapagayoConfigurator::~PapagayoConfigurator()
{
}

void PapagayoConfigurator::loadLipSyncList(QList<QString> list)
{
    manager->loadLipSyncList(list);
}

void PapagayoConfigurator::setPropertiesPanel()
{
    settingsPanel = new PapagayoSettings(this);
    connect(settingsPanel, &PapagayoSettings::selectMouth, this, &PapagayoConfigurator::selectMouth);
    // connect(settingsPanel, &PapagayoSettings::saveMouthTransRequested, this, &PapagayoConfigurator::saveMouthTransRequested);
    connect(settingsPanel, &PapagayoSettings::closeLipSyncProperties, this, &PapagayoConfigurator::closeSettingsPanel);
    connect(settingsPanel, &PapagayoSettings::initFrameHasChanged, this, &PapagayoConfigurator::initFrameHasChanged);
    connect(settingsPanel, &PapagayoSettings::xPosChanged, this, &PapagayoConfigurator::xPosChanged);
    connect(settingsPanel, &PapagayoSettings::yPosChanged, this, &PapagayoConfigurator::yPosChanged);
    connect(settingsPanel, &PapagayoSettings::rotationChanged, this, &PapagayoConfigurator::rotationChanged);
    connect(settingsPanel, &PapagayoSettings::scaleChanged, this, &PapagayoConfigurator::scaleChanged);

    connect(settingsPanel, &PapagayoSettings::objectHasBeenReset, this, &PapagayoConfigurator::objectHasBeenReset);
    connect(settingsPanel, &PapagayoSettings::proportionActivated, this, &PapagayoConfigurator::proportionActivated);

    settingsLayout->addWidget(settingsPanel);

    activePropertiesPanel(false);
}

void PapagayoConfigurator::activePropertiesPanel(bool enable)
{
    if (enable)
        settingsPanel->show();
    else
        settingsPanel->hide();
}

void PapagayoConfigurator::setLipSyncManagerPanel()
{
    manager = new LipSyncManager(this);
    connect(manager, &LipSyncManager::lipsyncCreatorRequested, this, &PapagayoConfigurator::lipsyncCreatorRequested);
    connect(manager, &LipSyncManager::lipsyncEditionRequested, this, &PapagayoConfigurator::lipsyncEditionRequested);
    connect(manager, &LipSyncManager::mouthEditionRequested, this, &PapagayoConfigurator::editCurrentLipSync);
    connect(manager, &LipSyncManager::currentLipSyncRemoved, this, &PapagayoConfigurator::currentLipsyncRemoved);

    settingsLayout->addWidget(manager);
}

void PapagayoConfigurator::activeLipSyncManagerPanel(bool enable)
{
    if (enable)
        manager->show();
    else
        manager->hide();
}

void PapagayoConfigurator::addLipSyncRecord(const QString &name)
{
    manager->addNewRecord(name);
}

void PapagayoConfigurator::removeLipSyncRecord(const QString &name)
{
    manager->removeRecordFromList(name);
}

void PapagayoConfigurator::editCurrentLipSync(const QString &name)
{
    emit mouthEditionRequested(name);

    activeLipSyncManagerPanel(false);
    activePropertiesPanel(true);
}

void PapagayoConfigurator::openLipSyncProperties(TupLipSync *lipsync)
{
    settingsPanel->openLipSyncProperties(lipsync);
}

void PapagayoConfigurator::resetUI()
{
    manager->resetUI();
    closeSettingsPanel();
}

void PapagayoConfigurator::closeSettingsPanel()
{
    emit closeLipSyncProperties();
    closePanels();
}

void PapagayoConfigurator::closePanels()
{
    activeLipSyncManagerPanel(true);
    activePropertiesPanel(false);
}

void PapagayoConfigurator::updateInterfaceRecords()
{
    settingsPanel->updateInterfaceRecords();
}

void PapagayoConfigurator::setTransformations(const QDomElement &dom)
{
    settingsPanel->setTransformations(dom);
}

void PapagayoConfigurator::setTransformations(const TupTransformation::Parameters parameters)
{
    settingsPanel->setTransformations(parameters);
}

void PapagayoConfigurator::updatePositionCoords(int x, int y)
{
    settingsPanel->updatePositionCoords(x, y);
}

void PapagayoConfigurator::updateRotationAngle(int angle)
{
    settingsPanel->updateRotationAngle(angle);
}

void PapagayoConfigurator::updateScaleFactor(double x, double y)
{
    settingsPanel->updateScaleFactor(x, y);
}

void PapagayoConfigurator::setPhoneme(const TupPhoneme *phoneme)
{
    settingsPanel->setPhoneme(phoneme);
}

void PapagayoConfigurator::openMouthsDialog()
{
    MouthsDialog *dialog = new MouthsDialog();
    dialog->show();
}

void PapagayoConfigurator::setProportionState(bool flag)
{
    settingsPanel->setProportionState(flag);
}
