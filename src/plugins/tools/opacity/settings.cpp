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

#include "settings.h"
#include "tuptweenerstep.h"
#include "tseparator.h"
#include "tosd.h"

PenSettings::PenSettings(QWidget *parent) : QWidget(parent)
{
    selectionDone = false;
    stepsCounter = 0;

    layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    QLabel *nameLabel = new QLabel(tr("Name") + ": ");
    input = new QLineEdit;

    QHBoxLayout *nameLayout = new QHBoxLayout;
    nameLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    nameLayout->setMargin(0);
    nameLayout->setSpacing(0);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(input);

    options = new TRadioButtonGroup(tr("Options"), Qt::Vertical);
    options->addItem(tr("Select object"), 0);
    options->addItem(tr("Set Properties"), 1);
    connect(options, SIGNAL(clicked(int)), this, SLOT(emitOptionChanged(int)));

    apply = new TImageButton(QPixmap(kAppProp->themeDir() + "icons/save.png"), 22);
    connect(apply, SIGNAL(clicked()), this, SLOT(applyTween()));

    remove = new TImageButton(QPixmap(kAppProp->themeDir() + "icons/close.png"), 22);
    connect(remove, SIGNAL(clicked()), this, SIGNAL(clickedResetTween()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    buttonsLayout->setMargin(0);
    buttonsLayout->setSpacing(10);
    buttonsLayout->addWidget(apply);
    buttonsLayout->addWidget(remove);

    layout->addLayout(nameLayout);
    layout->addWidget(options);

    setInnerForm();

    layout->addSpacing(10);
    layout->addLayout(buttonsLayout);
    layout->setSpacing(5);

    activateMode(TupToolPlugin::Selection);
}

PenSettings::~PenSettings()
{
}

void PenSettings::setInnerForm()
{
    innerPanel = new QWidget;

    QBoxLayout *innerLayout = new QBoxLayout(QBoxLayout::TopToBottom, innerPanel);
    innerLayout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    QLabel *startingLabel = new QLabel(tr("Starting at frame") + ": ");
    startingLabel->setAlignment(Qt::AlignVCenter);

    initFrame = new QSpinBox;
    initFrame->setEnabled(false);
    initFrame->setMaximum(999);
    connect(initFrame, SIGNAL(valueChanged(int)), this, SLOT(updateRangeFromInit(int)));

    QLabel *endingLabel = new QLabel(tr("Ending at frame") + ": ");
    endingLabel->setAlignment(Qt::AlignVCenter);

    endFrame = new QSpinBox;
    endFrame->setEnabled(true);
    endFrame->setMaximum(999);
    endFrame->setValue(1);
    connect(endFrame, SIGNAL(valueChanged(int)), this, SLOT(updateRangeFromEnd(int)));

    QHBoxLayout *startLayout = new QHBoxLayout;
    startLayout->setAlignment(Qt::AlignHCenter);
    startLayout->setMargin(0);
    startLayout->setSpacing(0);
    startLayout->addWidget(startingLabel);
    startLayout->addWidget(initFrame);

    QHBoxLayout *endLayout = new QHBoxLayout;
    endLayout->setAlignment(Qt::AlignHCenter);
    endLayout->setMargin(0);
    endLayout->setSpacing(0);
    endLayout->addWidget(endingLabel);
    endLayout->addWidget(endFrame);

    totalLabel = new QLabel(tr("Frames Total") + ": 1");
    totalLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    QHBoxLayout *totalLayout = new QHBoxLayout;
    totalLayout->setAlignment(Qt::AlignHCenter);
    totalLayout->setMargin(0);
    totalLayout->setSpacing(0);
    totalLayout->addWidget(totalLabel);

    comboInitFactor = new QDoubleSpinBox;
    comboInitFactor->setMinimum(0.00);
    comboInitFactor->setMaximum(1.00);
    comboInitFactor->setDecimals(2);
    comboInitFactor->setSingleStep(0.05);
    comboInitFactor->setValue(1.00);

    QLabel *opacityInitLabel = new QLabel(tr("Initial Opacity") + ": ");
    opacityInitLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QHBoxLayout *opacityInitLayout = new QHBoxLayout;
    opacityInitLayout->setAlignment(Qt::AlignHCenter);
    opacityInitLayout->setMargin(0);
    opacityInitLayout->setSpacing(0);
    opacityInitLayout->addWidget(opacityInitLabel);
    opacityInitLayout->addWidget(comboInitFactor);

    comboEndFactor = new QDoubleSpinBox;
    comboEndFactor->setMinimum(0.00);
    comboEndFactor->setMaximum(1.00);
    comboEndFactor->setDecimals(2);
    comboEndFactor->setSingleStep(0.05);
    comboEndFactor->setValue(0.00);

    QLabel *opacityEndLabel = new QLabel(tr("Ending Opacity") + ": ");
    opacityEndLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QHBoxLayout *opacityEndLayout = new QHBoxLayout;
    opacityEndLayout->setAlignment(Qt::AlignHCenter);
    opacityEndLayout->setMargin(0);
    opacityEndLayout->setSpacing(0);
    opacityEndLayout->addWidget(opacityEndLabel);
    opacityEndLayout->addWidget(comboEndFactor);

    iterationsCombo = new QSpinBox;
    iterationsCombo->setEnabled(true);
    iterationsCombo->setMinimum(1);
    iterationsCombo->setMaximum(999);

    QLabel *iterationsLabel = new QLabel(tr("Iterations") + ": ");
    iterationsLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QHBoxLayout *iterationsLayout = new QHBoxLayout;
    iterationsLayout->setAlignment(Qt::AlignHCenter);
    iterationsLayout->setMargin(0);
    iterationsLayout->setSpacing(0);
    iterationsLayout->addWidget(iterationsLabel);
    iterationsLayout->addWidget(iterationsCombo);

    loopBox = new QCheckBox(tr("Loop"), innerPanel);
    connect(loopBox, SIGNAL(stateChanged(int)), this, SLOT(updateReverseCheckbox(int)));

    QVBoxLayout *loopLayout = new QVBoxLayout;
    loopLayout->setAlignment(Qt::AlignHCenter);
    loopLayout->setMargin(0);
    loopLayout->setSpacing(0);
    loopLayout->addWidget(loopBox);

    reverseLoopBox = new QCheckBox(tr("Loop with Reverse"), innerPanel);
    connect(reverseLoopBox, SIGNAL(stateChanged(int)), this, SLOT(updateLoopCheckbox(int)));

    QVBoxLayout *reverseLayout = new QVBoxLayout;
    reverseLayout->setAlignment(Qt::AlignHCenter);
    reverseLayout->setMargin(0);
    reverseLayout->setSpacing(0);
    reverseLayout->addWidget(reverseLoopBox);

    innerLayout->addLayout(startLayout);
    innerLayout->addLayout(endLayout);
    innerLayout->addLayout(totalLayout);

    innerLayout->addSpacing(10);
    innerLayout->addWidget(new TSeparator(Qt::Horizontal));

    innerLayout->addLayout(opacityInitLayout);
    innerLayout->addLayout(opacityEndLayout);

    innerLayout->addLayout(iterationsLayout);
    innerLayout->addLayout(loopLayout);
    innerLayout->addLayout(reverseLayout);

    innerLayout->addWidget(new TSeparator(Qt::Horizontal));

    layout->addWidget(innerPanel);

    activeInnerForm(false);
}

void PenSettings::activeInnerForm(bool enable)
{
    if (enable && !innerPanel->isVisible()) {
        propertiesDone = true;
        innerPanel->show();
    } else {
        propertiesDone = false;
        innerPanel->hide();
    }
}

// Adding new Tween

void PenSettings::setParameters(const QString &name, int framesCount, int initFrame)
{
    mode = TupToolPlugin::Add;
    input->setText(name);

    activateMode(TupToolPlugin::Selection);
    apply->setToolTip(tr("Save Tween"));
    remove->setIcon(QPixmap(kAppProp->themeDir() + "icons/close.png"));
    remove->setToolTip(tr("Cancel Tween"));

    initStartCombo(framesCount, initFrame);
}

// Editing current Tween

void PenSettings::setParameters(TupItemTweener *currentTween)
{
    setEditMode();
    activateMode(TupToolPlugin::Properties);

    input->setText(currentTween->getTweenName());

    initFrame->setEnabled(true);
    initFrame->setValue(currentTween->getInitFrame() + 1);
    endFrame->setValue(currentTween->getInitFrame() + currentTween->getFrames());

    int end = endFrame->value();
    updateRangeFromEnd(end);

    comboInitFactor->setValue(currentTween->tweenOpacityInitialFactor());
    comboEndFactor->setValue(currentTween->tweenOpacityEndingFactor());
    iterationsCombo->setValue(currentTween->tweenOpacityIterations());
    loopBox->setChecked(currentTween->tweenOpacityLoop());
    reverseLoopBox->setChecked(currentTween->tweenOpacityReverseLoop());
}

void PenSettings::initStartCombo(int framesCount, int currentIndex)
{
    initFrame->clear();
    endFrame->clear();

    initFrame->setMinimum(1);
    initFrame->setMaximum(framesCount);
    initFrame->setValue(currentIndex + 1);

    endFrame->setMinimum(1);
    endFrame->setValue(framesCount);
}

void PenSettings::setStartFrame(int currentIndex)
{
    initFrame->setValue(currentIndex + 1);
    int end = endFrame->value();
    if (end < currentIndex+1)
        endFrame->setValue(currentIndex + 1);
}

int PenSettings::startFrame()
{
    return initFrame->value() - 1;
}

int PenSettings::startComboSize()
{
    return initFrame->maximum();
}

int PenSettings::totalSteps()
{
    return endFrame->value() - (initFrame->value() - 1);
}

void PenSettings::setEditMode()
{
    mode = TupToolPlugin::Edit;
    apply->setToolTip(tr("Update Tween"));
    remove->setIcon(QPixmap(kAppProp->themeDir() + "icons/close_properties.png"));
    remove->setToolTip(tr("Close Tween Properties"));
}

void PenSettings::applyTween()
{
    if (!selectionDone) {
        TOsd::self()->display(tr("Info"), tr("You must select at least one object!"), TOsd::Info);
        return;
    }

    if (!propertiesDone) {
        TOsd::self()->display(tr("Info"), tr("You must set Tween properties first!"), TOsd::Info);
        return;
    }

    // SQA: Verify Tween is really well applied before call setEditMode!
    setEditMode();

    if (!initFrame->isEnabled())
        initFrame->setEnabled(true);

    checkFramesRange();
    emit clickedApplyTween();
}

void PenSettings::notifySelection(bool flag)
{
    selectionDone = flag;
}

QString PenSettings::currentTweenName() const
{
    QString tweenName = input->text();
    if (tweenName.length() > 0)
        input->setFocus();

    return tweenName;
}

void PenSettings::emitOptionChanged(int option)
{
    switch (option) {
        case 0:
        {
            activeInnerForm(false);
            emit clickedSelect();
        }
        break;
        case 1:
        {
            if (selectionDone) {
                activeInnerForm(true);
                emit clickedDefineProperties();
            } else {
                options->setCurrentIndex(0);
                TOsd::self()->display(tr("Info"), tr("Select objects for Tweening first!"), TOsd::Info);
            }
        }
    }
}

QString PenSettings::tweenToXml(int currentScene, int currentLayer, int currentFrame)
{
    QDomDocument doc;

    QDomElement root = doc.createElement("tweening");
    root.setAttribute("name", currentTweenName());
    root.setAttribute("type", TupItemTweener::Opacity);
    root.setAttribute("initFrame", currentFrame);
    root.setAttribute("initLayer", currentLayer);
    root.setAttribute("initScene", currentScene);
  
    root.setAttribute("frames", stepsCounter);
    root.setAttribute("origin", "0,0");

    double initFactor = comboInitFactor->value();
    root.setAttribute("initOpacityFactor", QString::number(initFactor));

    double endFactor = comboEndFactor->value();
    root.setAttribute("endOpacityFactor", QString::number(endFactor));

    int iterations = iterationsCombo->value();
    if (iterations == 0) {
        iterations = 1;
        iterationsCombo->setValue(1);
    }
    root.setAttribute("opacityIterations", iterations);

    bool loop = loopBox->isChecked();
    if (loop)
        root.setAttribute("opacityLoop", "1");
    else
        root.setAttribute("opacityLoop", "0");

    bool reverse = reverseLoopBox->isChecked();
    if (reverse)
        root.setAttribute("opacityReverseLoop", "1");
    else
        root.setAttribute("opacityReverseLoop", "0");

    double delta = static_cast<double>(initFactor - endFactor) / static_cast<double>(iterations - 1);
    double reference = 0;

    int cycle = 1;
    int reverseTop = (iterations*2)-2;

    for (int i=0; i < stepsCounter; i++) {
         if (cycle <= iterations) {
             if (cycle == 1) {
                 reference = initFactor;
             } else if (cycle == iterations) {
                 reference = endFactor;
             } else {
                 reference -= delta;
             }
             cycle++;
         } else {
             // if repeat option is enabled
             if (loop) {
                 cycle = 2;
                 reference = initFactor;
             } else if (reverse) { // if reverse option is enabled
                 reference += delta;
                 if (cycle < reverseTop)
                     cycle++;
                 else
                     cycle = 1;

             } else { // If cycle is done and no loop and no reverse
                 reference = initFactor;
             }
         }

         TupTweenerStep *step = new TupTweenerStep(i);
         step->setOpacity(reference);
         root.appendChild(step->toXml(doc));
    }

    doc.appendChild(root);

    return doc.toString();
}

void PenSettings::activateMode(TupToolPlugin::EditMode mode)
{
    options->setCurrentIndex(mode);
}

void PenSettings::checkFramesRange()
{
    int begin = initFrame->value();
    int end = endFrame->value();

    if (begin > end) {
        initFrame->blockSignals(true);
        endFrame->blockSignals(true);
        int tmp = end;
        end = begin;
        begin = tmp;
        initFrame->setValue(begin);
        endFrame->setValue(end);
        initFrame->blockSignals(false);
        endFrame->blockSignals(false);
    }

    stepsCounter = end - begin + 1;
    totalLabel->setText(tr("Frames Total") + ": " + QString::number(stepsCounter));

    int iterations = iterationsCombo->value();
    if (iterations > stepsCounter)
        iterationsCombo->setValue(stepsCounter);
}

void PenSettings::updateLoopCheckbox(int state)
{
    Q_UNUSED(state);

    if (reverseLoopBox->isChecked() && loopBox->isChecked())
        loopBox->setChecked(false);
}

void PenSettings::updateReverseCheckbox(int state)
{
    Q_UNUSED(state);

    if (reverseLoopBox->isChecked() && loopBox->isChecked())
        reverseLoopBox->setChecked(false);
}

void PenSettings::updateRangeFromInit(int begin)
{
    int end = endFrame->value();
    stepsCounter = end - begin + 1;
    totalLabel->setText(tr("Frames Total") + ": " + QString::number(stepsCounter));
}

void PenSettings::updateRangeFromEnd(int end)
{
    int begin = initFrame->value();
    stepsCounter = end - begin + 1;
    totalLabel->setText(tr("Frames Total") + ": " + QString::number(stepsCounter));
}
