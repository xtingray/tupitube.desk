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
#include "tradiobuttongroup.h"
#include "tuptweenerstep.h"
#include "timagebutton.h"
#include "tseparator.h"
#include "tosd.h"

#include <QLabel>
#include <QLineEdit>
#include <QBoxLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QDir>

struct Settings::Private
{
    QWidget *innerPanel;
    QBoxLayout *layout;
    TupToolPlugin::Mode mode;

    QLineEdit *input;

    QSpinBox *initFrame;
    QSpinBox *endFrame;

    TRadioButtonGroup *options;

    QDoubleSpinBox *comboInitFactor;
    QDoubleSpinBox *comboEndFactor;

    QSpinBox *iterationsCombo;

    QCheckBox *loopBox;
    QCheckBox *reverseLoopBox;

    QLabel *totalLabel;
    int totalSteps;

    bool selectionDone;
    bool propertiesDone;

    TImageButton *apply;
    TImageButton *remove;
};

Settings::Settings(QWidget *parent) : QWidget(parent), k(new Private)
{
    k->selectionDone = false;
    k->totalSteps = 0;

    k->layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    k->layout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    QLabel *nameLabel = new QLabel(tr("Name") + ": ");
    k->input = new QLineEdit;

    QHBoxLayout *nameLayout = new QHBoxLayout;
    nameLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    nameLayout->setMargin(0);
    nameLayout->setSpacing(0);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(k->input);

    k->options = new TRadioButtonGroup(tr("Options"), Qt::Vertical);
    k->options->addItem(tr("Select object"), 0);
    k->options->addItem(tr("Set Properties"), 1);
    connect(k->options, SIGNAL(clicked(int)), this, SLOT(emitOptionChanged(int)));

    k->apply = new TImageButton(QPixmap(kAppProp->themeDir() + "icons/save.png"), 22);
    connect(k->apply, SIGNAL(clicked()), this, SLOT(applyTween()));

    k->remove = new TImageButton(QPixmap(kAppProp->themeDir() + "icons/close.png"), 22);
    connect(k->remove, SIGNAL(clicked()), this, SIGNAL(clickedResetTween()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    buttonsLayout->setMargin(0);
    buttonsLayout->setSpacing(10);
    buttonsLayout->addWidget(k->apply);
    buttonsLayout->addWidget(k->remove);

    k->layout->addLayout(nameLayout);
    k->layout->addWidget(k->options);

    setInnerForm();

    k->layout->addSpacing(10);
    k->layout->addLayout(buttonsLayout);
    k->layout->setSpacing(5);

    activateMode(TupToolPlugin::Selection);
}

Settings::~Settings()
{
    delete k;
}

void Settings::setInnerForm()
{
    k->innerPanel = new QWidget;

    QBoxLayout *innerLayout = new QBoxLayout(QBoxLayout::TopToBottom, k->innerPanel);
    innerLayout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    QLabel *startingLabel = new QLabel(tr("Starting at frame") + ": ");
    startingLabel->setAlignment(Qt::AlignVCenter);

    k->initFrame = new QSpinBox;
    k->initFrame->setEnabled(false);
    k->initFrame->setMaximum(999);
    connect(k->initFrame, SIGNAL(valueChanged(int)), this, SLOT(updateRangeFromInit(int)));

    QLabel *endingLabel = new QLabel(tr("Ending at frame") + ": ");
    endingLabel->setAlignment(Qt::AlignVCenter);

    k->endFrame = new QSpinBox;
    k->endFrame->setEnabled(true);
    k->endFrame->setMaximum(999);
    k->endFrame->setValue(1);
    connect(k->endFrame, SIGNAL(valueChanged(int)), this, SLOT(updateRangeFromEnd(int)));

    QHBoxLayout *startLayout = new QHBoxLayout;
    startLayout->setAlignment(Qt::AlignHCenter);
    startLayout->setMargin(0);
    startLayout->setSpacing(0);
    startLayout->addWidget(startingLabel);
    startLayout->addWidget(k->initFrame);

    QHBoxLayout *endLayout = new QHBoxLayout;
    endLayout->setAlignment(Qt::AlignHCenter);
    endLayout->setMargin(0);
    endLayout->setSpacing(0);
    endLayout->addWidget(endingLabel);
    endLayout->addWidget(k->endFrame);

    k->totalLabel = new QLabel(tr("Frames Total") + ": 1");
    k->totalLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    QHBoxLayout *totalLayout = new QHBoxLayout;
    totalLayout->setAlignment(Qt::AlignHCenter);
    totalLayout->setMargin(0);
    totalLayout->setSpacing(0);
    totalLayout->addWidget(k->totalLabel);

    k->comboInitFactor = new QDoubleSpinBox;
    k->comboInitFactor->setMinimum(0.00);
    k->comboInitFactor->setMaximum(1.00);
    k->comboInitFactor->setDecimals(2);
    k->comboInitFactor->setSingleStep(0.05);
    k->comboInitFactor->setValue(1.00);

    QLabel *opacityInitLabel = new QLabel(tr("Initial Opacity") + ": ");
    opacityInitLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QHBoxLayout *opacityInitLayout = new QHBoxLayout;
    opacityInitLayout->setAlignment(Qt::AlignHCenter);
    opacityInitLayout->setMargin(0);
    opacityInitLayout->setSpacing(0);
    opacityInitLayout->addWidget(opacityInitLabel);
    opacityInitLayout->addWidget(k->comboInitFactor);

    k->comboEndFactor = new QDoubleSpinBox;
    k->comboEndFactor->setMinimum(0.00);
    k->comboEndFactor->setMaximum(1.00);
    k->comboEndFactor->setDecimals(2);
    k->comboEndFactor->setSingleStep(0.05);
    k->comboEndFactor->setValue(0.00);

    QLabel *opacityEndLabel = new QLabel(tr("Ending Opacity") + ": ");
    opacityEndLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QHBoxLayout *opacityEndLayout = new QHBoxLayout;
    opacityEndLayout->setAlignment(Qt::AlignHCenter);
    opacityEndLayout->setMargin(0);
    opacityEndLayout->setSpacing(0);
    opacityEndLayout->addWidget(opacityEndLabel);
    opacityEndLayout->addWidget(k->comboEndFactor);

    k->iterationsCombo = new QSpinBox;
    k->iterationsCombo->setEnabled(true);
    k->iterationsCombo->setMinimum(1);
    k->iterationsCombo->setMaximum(999);

    QLabel *iterationsLabel = new QLabel(tr("Iterations") + ": ");
    iterationsLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QHBoxLayout *iterationsLayout = new QHBoxLayout;
    iterationsLayout->setAlignment(Qt::AlignHCenter);
    iterationsLayout->setMargin(0);
    iterationsLayout->setSpacing(0);
    iterationsLayout->addWidget(iterationsLabel);
    iterationsLayout->addWidget(k->iterationsCombo);

    k->loopBox = new QCheckBox(tr("Loop"), k->innerPanel);
    connect(k->loopBox, SIGNAL(stateChanged(int)), this, SLOT(updateReverseCheckbox(int)));

    QVBoxLayout *loopLayout = new QVBoxLayout;
    loopLayout->setAlignment(Qt::AlignHCenter);
    loopLayout->setMargin(0);
    loopLayout->setSpacing(0);
    loopLayout->addWidget(k->loopBox);

    k->reverseLoopBox = new QCheckBox(tr("Loop with Reverse"), k->innerPanel);
    connect(k->reverseLoopBox, SIGNAL(stateChanged(int)), this, SLOT(updateLoopCheckbox(int)));

    QVBoxLayout *reverseLayout = new QVBoxLayout;
    reverseLayout->setAlignment(Qt::AlignHCenter);
    reverseLayout->setMargin(0);
    reverseLayout->setSpacing(0);
    reverseLayout->addWidget(k->reverseLoopBox);

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

    k->layout->addWidget(k->innerPanel);

    activeInnerForm(false);
}

void Settings::activeInnerForm(bool enable)
{
    if (enable && !k->innerPanel->isVisible()) {
        k->propertiesDone = true;
        k->innerPanel->show();
    } else {
        k->propertiesDone = false;
        k->innerPanel->hide();
    }
}

// Adding new Tween

void Settings::setParameters(const QString &name, int framesCount, int initFrame)
{
    k->mode = TupToolPlugin::Add;
    k->input->setText(name);

    activateMode(TupToolPlugin::Selection);
    k->apply->setToolTip(tr("Save Tween"));
    k->remove->setIcon(QPixmap(kAppProp->themeDir() + "icons/close.png"));
    k->remove->setToolTip(tr("Cancel Tween"));

    initStartCombo(framesCount, initFrame);
}

// Editing current Tween

void Settings::setParameters(TupItemTweener *currentTween)
{
    setEditMode();
    activateMode(TupToolPlugin::Properties);

    k->input->setText(currentTween->getTweenName());

    k->initFrame->setEnabled(true);
    k->initFrame->setValue(currentTween->getInitFrame() + 1);
    k->endFrame->setValue(currentTween->getInitFrame() + currentTween->getFrames());

    int end = k->endFrame->value();
    updateRangeFromEnd(end);

    k->comboInitFactor->setValue(currentTween->tweenOpacityInitialFactor());
    k->comboEndFactor->setValue(currentTween->tweenOpacityEndingFactor());
    k->iterationsCombo->setValue(currentTween->tweenOpacityIterations());
    k->loopBox->setChecked(currentTween->tweenOpacityLoop());
    k->reverseLoopBox->setChecked(currentTween->tweenOpacityReverseLoop());
}

void Settings::initStartCombo(int framesCount, int currentIndex)
{
    k->initFrame->clear();
    k->endFrame->clear();

    k->initFrame->setMinimum(1);
    k->initFrame->setMaximum(framesCount);
    k->initFrame->setValue(currentIndex + 1);

    k->endFrame->setMinimum(1);
    k->endFrame->setValue(framesCount);
}

void Settings::setStartFrame(int currentIndex)
{
    k->initFrame->setValue(currentIndex + 1);
    int end = k->endFrame->value();
    if (end < currentIndex+1)
        k->endFrame->setValue(currentIndex + 1);
}

int Settings::startFrame()
{
    return k->initFrame->value() - 1;
}

int Settings::startComboSize()
{
    return k->initFrame->maximum();
}

int Settings::totalSteps()
{
    return k->endFrame->value() - (k->initFrame->value() - 1);
}

void Settings::setEditMode()
{
    k->mode = TupToolPlugin::Edit;
    k->apply->setToolTip(tr("Update Tween"));
    k->remove->setIcon(QPixmap(kAppProp->themeDir() + "icons/close_properties.png"));
    k->remove->setToolTip(tr("Close Tween Properties"));
}

void Settings::applyTween()
{
    if (!k->selectionDone) {
        TOsd::self()->display(tr("Info"), tr("You must select at least one object!"), TOsd::Info);
        return;
    }

    if (!k->propertiesDone) {
        TOsd::self()->display(tr("Info"), tr("You must set Tween properties first!"), TOsd::Info);
        return;
    }

    // SQA: Verify Tween is really well applied before call setEditMode!
    setEditMode();

    if (!k->initFrame->isEnabled())
        k->initFrame->setEnabled(true);

    checkFramesRange();
    emit clickedApplyTween();
}

void Settings::notifySelection(bool flag)
{
    k->selectionDone = flag;
}

QString Settings::currentTweenName() const
{
    QString tweenName = k->input->text();
    if (tweenName.length() > 0)
        k->input->setFocus();

    return tweenName;
}

void Settings::emitOptionChanged(int option)
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
            if (k->selectionDone) {
                activeInnerForm(true);
                emit clickedDefineProperties();
            } else {
                k->options->setCurrentIndex(0);
                TOsd::self()->display(tr("Info"), tr("Select objects for Tweening first!"), TOsd::Info);
            }
        }
    }
}

QString Settings::tweenToXml(int currentScene, int currentLayer, int currentFrame)
{
    QDomDocument doc;

    QDomElement root = doc.createElement("tweening");
    root.setAttribute("name", currentTweenName());
    root.setAttribute("type", TupItemTweener::Opacity);
    root.setAttribute("initFrame", currentFrame);
    root.setAttribute("initLayer", currentLayer);
    root.setAttribute("initScene", currentScene);
  
    root.setAttribute("frames", k->totalSteps);
    root.setAttribute("origin", "0,0");

    double initFactor = k->comboInitFactor->value();
    root.setAttribute("initOpacityFactor", QString::number(initFactor));

    double endFactor = k->comboEndFactor->value();
    root.setAttribute("endOpacityFactor", QString::number(endFactor));

    int iterations = k->iterationsCombo->value();
    if (iterations == 0) {
        iterations = 1;
        k->iterationsCombo->setValue(1);
    }
    root.setAttribute("opacityIterations", iterations);

    bool loop = k->loopBox->isChecked();
    if (loop)
        root.setAttribute("opacityLoop", "1");
    else
        root.setAttribute("opacityLoop", "0");

    bool reverse = k->reverseLoopBox->isChecked();
    if (reverse)
        root.setAttribute("opacityReverseLoop", "1");
    else
        root.setAttribute("opacityReverseLoop", "0");

    double delta = static_cast<double>(initFactor - endFactor) / static_cast<double>(iterations - 1);
    double reference = 0;

    int cycle = 1;
    int reverseTop = (iterations*2)-2;

    for (int i=0; i < k->totalSteps; i++) {
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

void Settings::activateMode(TupToolPlugin::EditMode mode)
{
    k->options->setCurrentIndex(mode);
}

void Settings::checkFramesRange()
{
    int begin = k->initFrame->value();
    int end = k->endFrame->value();

    if (begin > end) {
        k->initFrame->blockSignals(true);
        k->endFrame->blockSignals(true);
        int tmp = end;
        end = begin;
        begin = tmp;
        k->initFrame->setValue(begin);
        k->endFrame->setValue(end);
        k->initFrame->blockSignals(false);
        k->endFrame->blockSignals(false);
    }

    k->totalSteps = end - begin + 1;
    k->totalLabel->setText(tr("Frames Total") + ": " + QString::number(k->totalSteps));

    int iterations = k->iterationsCombo->value();
    if (iterations > k->totalSteps)
        k->iterationsCombo->setValue(k->totalSteps);
}

void Settings::updateLoopCheckbox(int state)
{
    Q_UNUSED(state);

    if (k->reverseLoopBox->isChecked() && k->loopBox->isChecked())
        k->loopBox->setChecked(false);
}

void Settings::updateReverseCheckbox(int state)
{
    Q_UNUSED(state);

    if (k->reverseLoopBox->isChecked() && k->loopBox->isChecked())
        k->reverseLoopBox->setChecked(false);
}

void Settings::updateRangeFromInit(int begin)
{
    int end = k->endFrame->value();
    k->totalSteps = end - begin + 1;
    k->totalLabel->setText(tr("Frames Total") + ": " + QString::number(k->totalSteps));
}

void Settings::updateRangeFromEnd(int end)
{
    int begin = k->initFrame->value();
    k->totalSteps = end - begin + 1;
    k->totalLabel->setText(tr("Frames Total") + ": " + QString::number(k->totalSteps));
}
