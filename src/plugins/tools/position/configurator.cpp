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
#include "stepsviewer.h"
#include "tuptweenerstep.h"
#include "tosd.h"
#include "tradiobuttongroup.h"

Configurator::Configurator(QWidget *parent) : QFrame(parent)
{
    framesCount = 1;
    currentFrame = 0;

    currentMode = TupToolPlugin::View;
    selectionDone = false;
    state = Manager;

    layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    QLabel *toolTitle = new QLabel;
    toolTitle->setAlignment(Qt::AlignHCenter);
    QPixmap pic(THEME_DIR + "icons/position_tween.png");
    toolTitle->setPixmap(pic.scaledToWidth(20, Qt::SmoothTransformation));
    toolTitle->setToolTip(tr("Position Tween Properties"));
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
}

void Configurator::loadTweenList(QList<QString> tweenList)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[Configurator::loadTweenList()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    tweenManager->loadTweenList(tweenList);
    if (tweenList.count() > 0)
        activeButtonsPanel(true); 
}

void Configurator::setPropertiesPanel()
{
    settingsPanel = new PosSettings(this);

    connect(settingsPanel, SIGNAL(startingFrameChanged(int)), this, SIGNAL(startingFrameChanged(int)));

    connect(settingsPanel, SIGNAL(clickedSelect()), this, SIGNAL(clickedSelect()));
    connect(settingsPanel, SIGNAL(clickedCreatePath()), this, SIGNAL(clickedCreatePath()));

    connect(settingsPanel, SIGNAL(clickedApplyTween()), this, SLOT(applyItem()));
    connect(settingsPanel, SIGNAL(clickedResetTween()), this, SLOT(closeTweenProperties()));

    connect(settingsPanel, SIGNAL(framesTotalChanged()), this, SIGNAL(framesTotalChanged()));


    settingsLayout->addWidget(settingsPanel);

    activePropertiesPanel(false);
}

void Configurator::activePropertiesPanel(bool enable)
{
    if (enable) {
        settingsPanel->show();
    } else {
        settingsPanel->clearData();
        settingsPanel->hide();
    }
}

void Configurator::setTweenManagerPanel()
{
    tweenManager = new TweenManager(this);

    connect(tweenManager, SIGNAL(addNewTween(const QString &)), this, SLOT(addTween(const QString &)));
    connect(tweenManager, SIGNAL(editCurrentTween(const QString &)), this, SLOT(editTween()));
    connect(tweenManager, SIGNAL(removeCurrentTween(const QString &)), this, SLOT(removeTween(const QString &)));
    connect(tweenManager, SIGNAL(getTweenData(const QString &)), this, SLOT(updateTweenData(const QString &)));

    settingsLayout->addWidget(tweenManager);

    state = Manager;
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

void Configurator::initStartCombo(int frames, int frameIndex)
{
    framesCount = frames;
    currentFrame = frameIndex;
    settingsPanel->initStartCombo(framesCount, currentFrame);
}

void Configurator::setStartFrame(int currentIndex)
{
    currentFrame = currentIndex;
    settingsPanel->setStartFrame(currentFrame);
}

int Configurator::startFrame()
{
    return settingsPanel->startFrame();
}

int Configurator::startComboSize()
{
    return settingsPanel->startComboSize();
}

void Configurator::updateSteps(const QGraphicsPathItem *path)
{
    settingsPanel->updateSteps(path);
}

QString Configurator::tweenToXml(int currentScene, int currentLayer, int currentFrame, QPointF point, QString &path)
{
    return settingsPanel->tweenToXml(currentScene, currentLayer, currentFrame, point, path);
}

int Configurator::totalSteps()
{
    return settingsPanel->totalSteps();
}

QList<QPointF> Configurator::tweenPoints()
{
    return settingsPanel->tweenPoints();
}

void Configurator::activateMode(TupToolPlugin::EditMode mode)
{
    settingsPanel->activateMode(mode);
}

void Configurator::clearData()
{
    settingsPanel->clearData();
}

void Configurator::addTween(const QString &name)
{
    currentMode = TupToolPlugin::Add;

    settingsPanel->setParameters(name, framesCount, currentFrame);

    activeTweenManagerPanel(false);
    activePropertiesPanel(true);

    state = Properties;

    emit setMode(currentMode);
}

void Configurator::editTween()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[Configurator::editTween()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    activeTweenManagerPanel(false);

    currentMode = TupToolPlugin::Edit;
    state = Properties;

    settingsPanel->notifySelection(true);
    settingsPanel->setParameters(currentTween);
    activePropertiesPanel(true);

    emit setMode(currentMode);
}

void Configurator::closeTweenProperties()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[Configurator::closeTweenProperties()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (currentMode == TupToolPlugin::Add)
        tweenManager->removeItemFromList();

    emit clickedResetInterface();

    closeSettingsPanel();
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
    // SQA: if name has been changed... change the item at TweenManager!
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

void Configurator::closeSettingsPanel()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[Configurator::closeSettingsPanel()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (state == Properties) {
        activeTweenManagerPanel(true);
        activePropertiesPanel(false);
        currentMode = TupToolPlugin::View;
        state = Manager;
    }
}

TupToolPlugin::Mode Configurator::mode()
{
    return currentMode;
}

void Configurator::applyItem()
{
     currentMode = TupToolPlugin::Edit;
     emit clickedApplyTween();
}

void Configurator::resetUI()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[Configurator::resetUI()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    tweenManager->resetUI();
    closeSettingsPanel();
    settingsPanel->notifySelection(false);
}

void Configurator::updateTweenData(const QString &name)
{
    emit getTweenData(name);
}

void Configurator::setCurrentTween(TupItemTweener *tween)
{
    currentTween = tween;
}

void Configurator::undoSegment(const QPainterPath path)
{
    settingsPanel->undoSegment(path);
}

void Configurator::redoSegment(const QPainterPath path)
{
    settingsPanel->redoSegment(path);
}

void Configurator::enableSaveOption(bool flag)
{
    settingsPanel->enableSaveOption(flag);
}

int Configurator::stepsTotal()
{
    return settingsPanel->stepsTotal();
}

void Configurator::updateSegments(const QPainterPath path)
{
    settingsPanel->updateSegments(path);
}
