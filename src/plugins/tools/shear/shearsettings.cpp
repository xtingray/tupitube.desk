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

#include "shearsettings.h"
#include "tseparator.h"
#include "tosd.h"

#include <QDir>

ShearSettings::ShearSettings(QWidget *parent) : QWidget(parent)
{
    shearAxes = TupItemTweener::XY;
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

    apply = new TImageButton(QPixmap(kAppProp->themeDir() + "icons/apply.png"), 22);
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

ShearSettings::~ShearSettings()
{
}

void ShearSettings::setInnerForm()
{
    innerPanel = new QWidget;

    QBoxLayout *innerLayout = new QBoxLayout(QBoxLayout::TopToBottom, innerPanel);
    innerLayout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    QLabel *startingLabel = new QLabel(tr("Starting at frame") + ": ");
    startingLabel->setAlignment(Qt::AlignVCenter);

    initFrameSpin = new QSpinBox;
    initFrameSpin->setEnabled(false);
    initFrameSpin->setMaximum(999);
    connect(initFrameSpin, SIGNAL(valueChanged(int)), this, SLOT(updateRangeFromInit(int)));

    QLabel *endingLabel = new QLabel(tr("Ending at frame") + ": ");
    endingLabel->setAlignment(Qt::AlignVCenter);

    endFrameSpin = new QSpinBox;
    endFrameSpin->setEnabled(true);
    endFrameSpin->setValue(1);
    endFrameSpin->setMaximum(999);
    connect(endFrameSpin, SIGNAL(valueChanged(int)), this, SLOT(updateRangeFromEnd(int)));

    QHBoxLayout *startLayout = new QHBoxLayout;
    startLayout->setAlignment(Qt::AlignHCenter);
    startLayout->setMargin(0);
    startLayout->setSpacing(0);
    startLayout->addWidget(startingLabel);
    startLayout->addWidget(initFrameSpin);

    QHBoxLayout *endLayout = new QHBoxLayout;
    endLayout->setAlignment(Qt::AlignHCenter);
    endLayout->setMargin(0);
    endLayout->setSpacing(0);
    endLayout->addWidget(endingLabel);
    endLayout->addWidget(endFrameSpin);

    totalLabel = new QLabel(tr("Frames Total") + ": 1");
    totalLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    QHBoxLayout *totalLayout = new QHBoxLayout;
    totalLayout->setAlignment(Qt::AlignHCenter);
    totalLayout->setMargin(0);
    totalLayout->setSpacing(0);
    totalLayout->addWidget(totalLabel);

    comboAxes = new QComboBox();
    comboAxes->addItem(tr("Width & Height"));
    comboAxes->addItem(tr("Only Width"));
    comboAxes->addItem(tr("Only Height"));
    QLabel *axesLabel = new QLabel(tr("Shear in") + ": ");
    axesLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QHBoxLayout *axesLayout = new QHBoxLayout;
    axesLayout->setAlignment(Qt::AlignHCenter);
    axesLayout->setMargin(0);
    axesLayout->setSpacing(0);
    axesLayout->addWidget(axesLabel);
    axesLayout->addWidget(comboAxes);

    comboFactor = new QDoubleSpinBox;
    comboFactor->setMinimum(-9.0);
    comboFactor->setMaximum(9.0);
    comboFactor->setDecimals(2);
    comboFactor->setSingleStep(0.05);
    comboFactor->setValue(0.10);

    QLabel *speedLabel = new QLabel(tr("Scaling Factor") + ": ");
    speedLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QHBoxLayout *speedLayout = new QHBoxLayout;
    speedLayout->setAlignment(Qt::AlignHCenter);
    speedLayout->setMargin(0);
    speedLayout->setSpacing(0);
    speedLayout->addWidget(speedLabel);
    speedLayout->addWidget(comboFactor);

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

    innerLayout->addSpacing(15);
    innerLayout->addWidget(new TSeparator(Qt::Horizontal));

    innerLayout->addLayout(axesLayout);
    innerLayout->addLayout(speedLayout);
    innerLayout->addLayout(iterationsLayout);
    innerLayout->addLayout(loopLayout);
    innerLayout->addLayout(reverseLayout);

    innerLayout->addWidget(new TSeparator(Qt::Horizontal));

    layout->addWidget(innerPanel);

    activeInnerForm(false);
}

void ShearSettings::activeInnerForm(bool enable)
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

void ShearSettings::setParameters(const QString &name, int framesCount, int initFrame)
{
    mode = TupToolPlugin::Add;
    input->setText(name);

    initFrameSpin->setEnabled(false);
    activateMode(TupToolPlugin::Selection);
    apply->setToolTip(tr("Save Tween"));
    remove->setIcon(QPixmap(kAppProp->themeDir() + "icons/close.png"));
    remove->setToolTip(tr("Cancel Tween"));

    initStartCombo(framesCount, initFrame);
}

// Editing new Tween

void ShearSettings::setParameters(TupItemTweener *currentTween)
{
    setEditMode();
    activateMode(TupToolPlugin::Properties);

    input->setText(currentTween->getTweenName());

    initFrameSpin->setEnabled(true);
    initFrameSpin->setValue(currentTween->getInitFrame() + 1);
    endFrameSpin->setValue(currentTween->getInitFrame() + currentTween->getFrames());

    int end = endFrameSpin->value();
    updateRangeFromEnd(end);

    comboAxes->setCurrentIndex(currentTween->tweenShearAxes());
    comboFactor->setValue(currentTween->tweenShearFactor());
    iterationsCombo->setValue(currentTween->tweenShearIterations());
    loopBox->setChecked(currentTween->tweenShearLoop());
    reverseLoopBox->setChecked(currentTween->tweenShearReverseLoop());
}

void ShearSettings::initStartCombo(int framesCount, int currentIndex)
{
    initFrameSpin->clear();
    endFrameSpin->clear();

    initFrameSpin->setMinimum(1);
    initFrameSpin->setMaximum(framesCount);
    initFrameSpin->setValue(currentIndex + 1);

    endFrameSpin->setMinimum(1);
    endFrameSpin->setValue(framesCount);
}

void ShearSettings::setStartFrame(int currentIndex)
{
    initFrameSpin->setValue(currentIndex + 1);
    int end = endFrameSpin->value();
    if (end < currentIndex+1)
        endFrameSpin->setValue(currentIndex + 1);
}

int ShearSettings::startFrame()
{
    return initFrameSpin->value() - 1;
}

int ShearSettings::startComboSize()
{
    return initFrameSpin->maximum();
}

int ShearSettings::totalSteps()
{
    return endFrameSpin->value() - (initFrameSpin->value() - 1);
}

void ShearSettings::setEditMode()
{
    mode = TupToolPlugin::Edit;
    apply->setToolTip(tr("Update Tween"));
    remove->setIcon(QPixmap(kAppProp->themeDir() + "icons/close_properties.png"));
    remove->setToolTip(tr("Close Tween Properties"));
}

void ShearSettings::applyTween()
{
    if (!selectionDone) {
        TOsd::self()->display(tr("Info"), tr("You must select at least one object!"), TOsd::Info);
        return;
    }

    if (!propertiesDone) {
        TOsd::self()->display(tr("Info"), tr("You must set Tween properties first!"), TOsd::Info);
        return;
    }

    // SQA: Verify whether tween is really well applied before call setEditMode!
    setEditMode();

    if (!initFrameSpin->isEnabled())
        initFrameSpin->setEnabled(true);

    checkFramesRange();

    emit clickedApplyTween();
}

void ShearSettings::notifySelection(bool flag)
{
    selectionDone = flag;
}

QString ShearSettings::currentTweenName() const
{
    QString tweenName = input->text();
    if (tweenName.length() > 0)
        input->setFocus();

    return tweenName;
}

void ShearSettings::emitOptionChanged(int option)
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

QString ShearSettings::tweenToXml(int currentScene, int currentLayer, int currentFrame, QPointF point)
{
    QDomDocument doc;

    QDomElement root = doc.createElement("tweening");
    root.setAttribute("name", currentTweenName());
    root.setAttribute("type", TupItemTweener::Shear);
    root.setAttribute("initFrame", currentFrame);
    root.setAttribute("initLayer", currentLayer);
    root.setAttribute("initScene", currentScene);
   
    root.setAttribute("frames", stepsCounter);
    root.setAttribute("origin", QString::number(point.x()) + "," + QString::number(point.y()));
    shearAxes = TupItemTweener::TransformAxes(comboAxes->currentIndex());
    root.setAttribute("shearAxes", shearAxes);

    double factor = comboFactor->value();
    root.setAttribute("shearFactor", QString::number(factor));

    int iterations = iterationsCombo->value();
    if (iterations == 0) {
        iterations = 1;
        iterationsCombo->setValue(1);
    }
    root.setAttribute("shearIterations", iterations);

    bool loop = loopBox->isChecked();
    if (loop)
        root.setAttribute("shearLoop", "1");
    else
        root.setAttribute("shearLoop", "0");

    bool reverse = reverseLoopBox->isChecked();
    if (reverse)
        root.setAttribute("shearReverseLoop", "1");
    else
        root.setAttribute("shearReverseLoop", "0");

    double factorX = 1.0;
    double factorY = 1.0;
    double shearX = 1.0;
    double shearY = 1.0;
    double lastShearX = 1.0;
    double lastShearY = 1.0;

    if (shearAxes == TupItemTweener::XY) {
        factorX = factor;
        factorY = factor;
    } else if (shearAxes == TupItemTweener::X) {
        factorX = factor;
    } else {
        factorY = factor;
    }

    int cycle = 1;
    int reverseTop = (iterations*2)-2;

    for (int i=0; i < stepsCounter; i++) {
         if (cycle <= iterations) {
             if (cycle == 1) {
                 shearX = 0;
                 shearY = 0;
                 lastShearX = 0;
                 lastShearY = 0;
             } else {
                 shearX += factorX;
                 shearY += factorY;
                 lastShearX = shearX;
                 lastShearY = shearY;
             }
             cycle++;
         } else {
             // if repeat option is enabled
             if (loop) {
                 cycle = 2;
                 shearX = 0;
                 shearY = 0;
             } else if (reverse) { // if reverse option is enabled
                 shearX -= factorX;
                 shearY -= factorY;

                 if (cycle < reverseTop)
                     cycle++;
                 else
                     cycle = 1;
             } else { // If cycle is done and no loop and no reverse
                 shearX = lastShearX;
                 shearY = lastShearY;
             }
         }

         TupTweenerStep *step = new TupTweenerStep(i);
         step->setShear(shearX, shearY);
         root.appendChild(step->toXml(doc));
    }

    doc.appendChild(root);

    return doc.toString();
}

void ShearSettings::activateMode(TupToolPlugin::EditMode mode)
{
    options->setCurrentIndex(mode);
}

void ShearSettings::checkFramesRange()
{
    int begin = initFrameSpin->value();
    int end = endFrameSpin->value();

    if (begin > end) {
        initFrameSpin->blockSignals(true);
        endFrameSpin->blockSignals(true);
        int tmp = end;
        end = begin;
        begin = tmp;
        initFrameSpin->setValue(begin);
        endFrameSpin->setValue(end);
        initFrameSpin->blockSignals(false);
        endFrameSpin->blockSignals(false);
    }

    stepsCounter = end - begin + 1;
    totalLabel->setText(tr("Frames Total") + ": " + QString::number(stepsCounter));

    int iterations = iterationsCombo->value();
    if (iterations > stepsCounter)
        iterationsCombo->setValue(stepsCounter);
}

void ShearSettings::updateLoopCheckbox(int state)
{
    Q_UNUSED(state);

    if (reverseLoopBox->isChecked() && loopBox->isChecked())
        loopBox->setChecked(false);
}

void ShearSettings::updateReverseCheckbox(int state)
{
    Q_UNUSED(state);

    if (reverseLoopBox->isChecked() && loopBox->isChecked())
        reverseLoopBox->setChecked(false);
}

void ShearSettings::updateRangeFromInit(int begin)
{
    int end = endFrameSpin->value();
    stepsCounter = end - begin + 1;
    totalLabel->setText(tr("Frames Total") + ": " + QString::number(stepsCounter));
}

void ShearSettings::updateRangeFromEnd(int end)
{
    int begin = initFrameSpin->value();
    stepsCounter = end - begin + 1;
    totalLabel->setText(tr("Frames Total") + ": " + QString::number(stepsCounter));
}
