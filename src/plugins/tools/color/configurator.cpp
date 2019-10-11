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
    framesCount = 1;
    currentFrame = 0;

    toolMode = TupToolPlugin::View;
    state = Configurator::Manager;

    layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    QLabel *toolTitle = new QLabel;
    toolTitle->setAlignment(Qt::AlignHCenter);
    QPixmap pic(THEME_DIR + "icons/coloring_tween.png");
    toolTitle->setPixmap(pic.scaledToWidth(20, Qt::SmoothTransformation));
    toolTitle->setToolTip(tr("Coloring Tween Properties"));
    layout->addWidget(toolTitle);
    layout->addWidget(new TSeparator(Qt::Horizontal));

    settingsLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    settingsLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    settingsLayout->setMargin(0);
    settingsLayout->setSpacing(0);

    setTweenManagerPanel();
    setButtonsPanel();
    setPropertiesPanel();

    layout->addLayout(settingsLayout);
    layout->addStretch(2);
}

Configurator::~Configurator()
{
    delete layout;
    delete settingsLayout;
    delete settingsPanel;
    delete tweenManager;
    delete controlPanel;
    delete currentTween;
}

void Configurator::loadTweenList(QList<QString> tweenList)
{
    tweenManager->loadTweenList(tweenList);
    if (tweenList.count() > 0)
        activeButtonsPanel(true);
}

void Configurator::setPropertiesPanel()
{
    settingsPanel = new PenSettings(this);

    connect(settingsPanel, SIGNAL(startingPointChanged(int)), this, SIGNAL(startingPointChanged(int)));
    connect(settingsPanel, SIGNAL(clickedSelect()), this, SIGNAL(clickedSelect()));
    connect(settingsPanel, SIGNAL(clickedDefineProperties()), this, SIGNAL(clickedDefineProperties()));
    connect(settingsPanel, SIGNAL(clickedApplyTween()), this, SLOT(applyItem()));
    connect(settingsPanel, SIGNAL(clickedResetTween()), this, SLOT(closeTweenProperties()));

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

void Configurator::setCurrentTween(TupItemTweener *lCurrentTween)
{
    currentTween = lCurrentTween;
}

void Configurator::setTweenManagerPanel()
{
    tweenManager = new TweenManager(this);
    connect(tweenManager, SIGNAL(addNewTween(const QString &)), this, SLOT(addTween(const QString &)));
    connect(tweenManager, SIGNAL(editCurrentTween(const QString &)), this, SLOT(editTween()));
    connect(tweenManager, SIGNAL(removeCurrentTween(const QString &)), this, SLOT(removeTween(const QString &)));
    connect(tweenManager, SIGNAL(getTweenData(const QString &)), this, SLOT(updateTweenData(const QString &)));

    settingsLayout->addWidget(tweenManager);
    state = Configurator::Manager;
}

void Configurator::activeTweenManagerPanel(bool enable)
{
    if (enable)
        tweenManager->show();
    else
        tweenManager->hide();

    if (tweenManager->listSize() > 0)
        activeButtonsPanel(enable);
}

void Configurator::setButtonsPanel()
{
    controlPanel = new ButtonsPanel(this);
    connect(controlPanel, SIGNAL(clickedEditTween()), this, SLOT(editTween()));
    connect(controlPanel, SIGNAL(clickedRemoveTween()), this, SLOT(removeTween()));

    settingsLayout->addWidget(controlPanel);

    activeButtonsPanel(false);
}

void Configurator::activeButtonsPanel(bool enable)
{
    if (enable)
        controlPanel->show();
    else
        controlPanel->hide();
}

void Configurator::initStartCombo(int lFramesCount, int lCurrentFrame)
{
    framesCount = lFramesCount;
    currentFrame = lCurrentFrame;
    settingsPanel->initStartCombo(lFramesCount, lCurrentFrame);
}

void Configurator::setStartFrame(int currentIndex)
{
    currentFrame = currentIndex;
    settingsPanel->setStartFrame(currentIndex);
}

int Configurator::startFrame()
{
    return settingsPanel->startFrame();
}

int Configurator::startComboSize()
{
    return settingsPanel->startComboSize();
}

QString Configurator::tweenToXml(int currentScene, int currentLayer, int currentFrame)
{
    return settingsPanel->tweenToXml(currentScene, currentLayer, currentFrame);
}

int Configurator::totalSteps()
{
    return settingsPanel->totalSteps();
}

void Configurator::activateMode(TupToolPlugin::EditMode mode)
{
    settingsPanel->activateMode(mode);
}

void Configurator::addTween(const QString &name)
{
    activeTweenManagerPanel(false);

    toolMode = TupToolPlugin::Add;
    state = Configurator::Properties;

    settingsPanel->setParameters(name, framesCount, currentFrame);
    activePropertiesPanel(true);

    emit setMode(toolMode);
}

void Configurator::editTween()
{
    toolMode = TupToolPlugin::Edit;
    emit setMode(toolMode);

    activeTweenManagerPanel(false);

    // mode = TupToolPlugin::Edit;
    state = Configurator::Properties;

    settingsPanel->notifySelection(true);
    settingsPanel->setParameters(currentTween);
    activePropertiesPanel(true);

    // emit setMode(mode);
}

void Configurator::removeTween()
{
    QString name = tweenManager->currentTweenName();
    tweenManager->removeItemFromList();

    removeTween(name);
}

void Configurator::removeTween(const QString &name)
{
    if (tweenManager->listSize() == 0)
        activeButtonsPanel(false);

    emit clickedRemoveTween(name);
}

QString Configurator::currentTweenName() const
{
    QString oldName = tweenManager->currentTweenName();
    QString newName = settingsPanel->currentTweenName();

    if (oldName.compare(newName) != 0)
        tweenManager->updateTweenName(newName);

    return newName;
}

void Configurator::notifySelection(bool flag)
{
    settingsPanel->notifySelection(flag);
}

void Configurator::setInitialColor(QColor color)
{
    settingsPanel->setInitialColor(color);
}

void Configurator::closeTweenProperties()
{
    if (toolMode == TupToolPlugin::Add)
        tweenManager->removeItemFromList();

    emit clickedResetInterface();

    closeSettingsPanel();
}

void Configurator::closeSettingsPanel()
{
    if (state == Configurator::Properties) {
        activeTweenManagerPanel(true);
        activePropertiesPanel(false);
        toolMode = TupToolPlugin::View;
        state = Configurator::Manager;
    }
}

TupToolPlugin::Mode Configurator::mode()
{
    return toolMode;
}

void Configurator::applyItem()
{
     toolMode = TupToolPlugin::Edit;
     emit clickedApplyTween();
}

void Configurator::resetUI()
{
    tweenManager->resetUI();
    closeSettingsPanel();
    settingsPanel->notifySelection(false);
}

void Configurator::updateTweenData(const QString &name)
{
    emit getTweenData(name);
}

