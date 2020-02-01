/***************************************************************************
 *   Project TUPITUBE DESK                                               *
 *   Project Contact: info@maefloresta.com                                *
 *   Project Website: http://www.maefloresta.com                          *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>               *
 *                                                                        *
 *   Developers:                                                          *
 *                                                                        *
 *   2019:                                                                *
 *    Alejandro Carrasco                                                  *
 *   2010:                                                                *
 *    Gustavo Gonzalez / xtingray                                         *
 *                                                                        *
 *   KTooN's versions:                                                    * 
 *                                                                        *
 *   2006:                                                                *
 *    David Cuadrado                                                      *
 *    Jorge Cuadrado                                                      *
 *   2003:                                                                *
 *    Fernado Roldan                                                      *
 *    Simena Dinas                                                        *
 *                                                                        *
 *   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com      *
 *   License:                                                             *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or    *
 *   (at your option) any later version.                                  *
 *                                                                        *
 *   This program is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *   GNU General Public License for more details.                         *
 *                                                                        *
 *   You should have received a copy of the GNU General Public License    *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "colorsettings.h"
#include "tradiobuttongroup.h"
#include "tuptweenerstep.h"
#include "timagebutton.h"
#include "tseparator.h"
#include "tosd.h"

#include <QBoxLayout>
#include <QColorDialog>
#include <QDir>

ColorSettings::ColorSettings(QWidget *parent) : QWidget(parent)
{
    selectionDone = false;
    totalStepsCount = 0;

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

    activatePropertiesMode(TupToolPlugin::Selection);
}

ColorSettings::~ColorSettings()
{
    delete innerPanel;
    delete layout;
    delete input;
    delete initFrame;
    delete endFrame;
    delete options;
    delete fillTypeCombo;
    delete initColorButton;
    delete endColorButton;
    delete iterationsCombo;
    delete loopBox;
    delete reverseLoopBox;
    delete totalLabel;
    delete apply;
    delete remove;
}

void ColorSettings::setInnerForm()
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

    initialColor = QColor("#fff");
    initColorButton = new QPushButton();
    initColorButton->setText(tr("White"));
    initColorButton->setPalette(QPalette(initialColor));
    initColorButton->setAutoFillBackground(true);
    connect(initColorButton, SIGNAL(clicked()), this, SLOT(setInitialColor()));

    QLabel *typeLabel = new QLabel(tr("Fill Type") + ": ");
    typeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    fillTypeCombo = new QComboBox();
    fillTypeCombo->addItem(tr("Internal Fill"));
    fillTypeCombo->addItem(tr("Line Fill"));
    fillTypeCombo->addItem(tr("Line & Internal Fill"));
    QHBoxLayout *fillLayout = new QHBoxLayout;
    fillLayout->setAlignment(Qt::AlignHCenter);
    fillLayout->setMargin(0);
    fillLayout->setSpacing(0);
    fillLayout->addWidget(typeLabel);
    fillLayout->addWidget(fillTypeCombo);

    QLabel *coloringInitLabel = new QLabel(tr("Initial Color") + ": ");
    coloringInitLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QHBoxLayout *coloringInitLayout = new QHBoxLayout;
    coloringInitLayout->setAlignment(Qt::AlignHCenter);
    coloringInitLayout->setMargin(0);
    coloringInitLayout->setSpacing(0);
    coloringInitLayout->addWidget(coloringInitLabel);
    coloringInitLayout->addWidget(initColorButton);

    endingColor = QColor("#fff");
    endColorButton = new QPushButton();
    endColorButton->setText(tr("White"));
    endColorButton->setPalette(QPalette(endingColor));
    endColorButton->setAutoFillBackground(true);
    connect(endColorButton, SIGNAL(clicked()), this, SLOT(setEndingColor()));

    QLabel *coloringEndLabel = new QLabel(tr("Ending Color") + ": ");
    coloringEndLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QHBoxLayout *coloringEndLayout = new QHBoxLayout;
    coloringEndLayout->setAlignment(Qt::AlignHCenter);
    coloringEndLayout->setMargin(0);
    coloringEndLayout->setSpacing(0);
    coloringEndLayout->addWidget(coloringEndLabel);
    coloringEndLayout->addWidget(endColorButton);

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
    innerLayout->addLayout(fillLayout);

    innerLayout->addLayout(coloringInitLayout);
    innerLayout->addLayout(coloringEndLayout);

    innerLayout->addLayout(iterationsLayout);
    innerLayout->addLayout(loopLayout);
    innerLayout->addLayout(reverseLayout);

    innerLayout->addWidget(new TSeparator(Qt::Horizontal));

    layout->addWidget(innerPanel);

    activeInnerForm(false);
}

void ColorSettings::activeInnerForm(bool enable)
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

void ColorSettings::setParameters(const QString &name, int framesCount, int initFrame)
{
    mode = TupToolPlugin::Add;
    input->setText(name);

    activatePropertiesMode(TupToolPlugin::Selection);
    apply->setToolTip(tr("Save Tween"));
    remove->setIcon(QPixmap(kAppProp->themeDir() + "icons/close.png"));
    remove->setToolTip(tr("Cancel Tween"));

    initStartCombo(framesCount, initFrame);
}

// Editing current Tween

void ColorSettings::setParameters(TupItemTweener *currentTween)
{
    setEditMode();
    activatePropertiesMode(TupToolPlugin::Properties);

    input->setText(currentTween->getTweenName());

    initFrame->setEnabled(true);
    initFrame->setValue(currentTween->getInitFrame() + 1);

    int lastFrame = currentTween->getInitFrame() + currentTween->getFrames();
    endFrame->setValue(lastFrame);

    int end = endFrame->value();
    updateRangeFromEnd(end);

    updateColor(currentTween->tweenInitialColor(), initColorButton);
    updateColor(currentTween->tweenEndingColor(), endColorButton);

    int iterations = currentTween->tweenColorIterations();

    iterationsCombo->setValue(iterations);

    loopBox->setChecked(currentTween->tweenColorLoop());
    reverseLoopBox->setChecked(currentTween->tweenColorReverseLoop());
}

void ColorSettings::initStartCombo(int framesCount, int currentIndex)
{
    initFrame->clear();
    endFrame->clear();

    initFrame->setMinimum(1);
    initFrame->setMaximum(framesCount);
    initFrame->setValue(currentIndex + 1);

    endFrame->setMinimum(1);
    endFrame->setValue(framesCount);
}

void ColorSettings::setStartFrame(int currentIndex)
{
    initFrame->setValue(currentIndex + 1);
    int end = endFrame->value();
    if (end < currentIndex+1)
       endFrame->setValue(currentIndex + 1);
}

int ColorSettings::startFrame()
{
    return initFrame->value() - 1;
}

int ColorSettings::startComboSize()
{
    return initFrame->maximum();
}

int ColorSettings::totalSteps()
{
    return endFrame->value() - (initFrame->value() - 1);
}

void ColorSettings::setEditMode()
{
    mode = TupToolPlugin::Edit;
    apply->setToolTip(tr("Update Tween"));
    remove->setIcon(QPixmap(kAppProp->themeDir() + "icons/close_properties.png"));
    remove->setToolTip(tr("Close Tween Properties"));
}

void ColorSettings::applyTween()
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

void ColorSettings::notifySelection(bool flag)
{
    selectionDone = flag;
}

void ColorSettings::setInitialColor()
{
    initialColor = QColorDialog::getColor(initialColor, this);
    updateColor(initialColor, initColorButton);
}

void ColorSettings::setInitialColor(QColor color) {
    initialColor = color;
    endingColor = QColor("#fff");
    updateColor(initialColor, initColorButton);
    updateColor(endingColor, endColorButton);
}

QString ColorSettings::currentTweenName() const
{
    QString tweenName = input->text();
    if (tweenName.length() > 0)
       input->setFocus();

    return tweenName;
}

void ColorSettings::emitOptionChanged(int option)
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

QString ColorSettings::tweenToXml(int currentScene, int currentLayer, int currentFrame)
{
    QDomDocument doc;

    QDomElement root = doc.createElement("tweening");
    root.setAttribute("name", currentTweenName());
    root.setAttribute("type", TupItemTweener::Coloring);
    root.setAttribute("initFrame", currentFrame);
    root.setAttribute("initLayer", currentLayer);
    root.setAttribute("initScene", currentScene);

    root.setAttribute("fillType", fillTypeCombo->currentIndex());
    root.setAttribute("frames", totalStepsCount);
    root.setAttribute("origin", "0,0");

    int initialRed = initialColor.red();
    int initialGreen = initialColor.green();
    int initialBlue = initialColor.blue();

    QString colorText = QString::number(initialRed) + "," + QString::number(initialGreen)
                       + "," + QString::number(initialBlue);
    root.setAttribute("initialColor", colorText);

    int endingRed = endingColor.red();
    int endingGreen = endingColor.green();
    int endingBlue = endingColor.blue();

    colorText = QString::number(endingRed) + "," + QString::number(endingGreen)
                       + "," + QString::number(endingBlue);
    root.setAttribute("endingColor", colorText);

    int iterations = iterationsCombo->value();
    if (iterations == 0) {
       iterations = 1;
       iterationsCombo->setValue(1);
    }
    root.setAttribute("colorIterations", iterations);

    bool loop = loopBox->isChecked();
    if (loop)
       root.setAttribute("colorLoop", "1");
    else
       root.setAttribute("colorLoop", "0");

    bool reverse = reverseLoopBox->isChecked();
    if (reverse)
       root.setAttribute("colorReverseLoop", "1");
    else
       root.setAttribute("colorReverseLoop", "0");

    double redDelta = static_cast<double>(initialRed - endingRed) / static_cast<double>(iterations - 1);
    double greenDelta = static_cast<double>(initialGreen - endingGreen) / static_cast<double>(iterations - 1);
    double blueDelta = static_cast<double>(initialBlue - endingBlue)/ static_cast<double>(iterations - 1);

    double redReference = 0;
    double greenReference = 0;
    double blueReference = 0;

    int cycle = 1;
    int reverseTop = (iterations*2)-2;

    for (int i=0; i < totalStepsCount; i++) {
        if (cycle <= iterations) {
            if (cycle == 1) {
                redReference = initialRed;
                greenReference = initialGreen;
                blueReference = initialBlue;
            } else if (cycle == iterations) {
                redReference = endingRed;
                greenReference = endingGreen;
                blueReference = endingBlue;
            } else {
                redReference -= redDelta;
                greenReference -= greenDelta;
                blueReference -= blueDelta;
            }
            cycle++;
        } else {
            // if repeat option is enabled
            if (loop) { 
                cycle = 2;
                redReference = initialRed;
                greenReference = initialGreen;
                blueReference = initialBlue;
            } else if (reverse) { // if reverse option is enabled
                redReference += redDelta;
                greenReference += greenDelta;
                blueReference += blueDelta;

                if (cycle < reverseTop)
                    cycle++;
                else
                    cycle = 1;
            } else { // If cycle is done and no loop and no reverse 
                redReference = endingRed;
                greenReference = endingGreen;
                blueReference = endingBlue;
            }
        }

        TupTweenerStep *step = new TupTweenerStep(i);
        QColor color = QColor(static_cast<int> (redReference), static_cast<int> (greenReference),
                              static_cast<int> (blueReference));
        step->setColor(color);
        root.appendChild(step->toXml(doc));
    }

    doc.appendChild(root);

    return doc.toString();
}

void ColorSettings::activateMode(TupToolPlugin::EditMode mode)
{
    options->setCurrentIndex(mode);
}

void ColorSettings::activatePropertiesMode(TupToolPlugin::EditMode mode)
{
    options->setCurrentIndex(mode);
}

void ColorSettings::checkFramesRange()
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

    totalStepsCount = end - begin + 1;
    totalLabel->setText(tr("Frames Total") + ": " + QString::number(totalStepsCount));
}

void ColorSettings::updateLoopCheckbox(int state)
{
    Q_UNUSED(state)
    if (reverseLoopBox->isChecked() && loopBox->isChecked())
       loopBox->setChecked(false);
}

void ColorSettings::updateReverseCheckbox(int state)
{
    Q_UNUSED(state)
    if (reverseLoopBox->isChecked() && loopBox->isChecked())
       reverseLoopBox->setChecked(false);
}

void ColorSettings::setEndingColor()
{
    endingColor = QColorDialog::getColor(endingColor, this);
    updateColor(endingColor, endColorButton);
}

void ColorSettings::updateColor(QColor color, QPushButton *colorButton)
{
    if (color.isValid()) {
        colorButton->setText(color.name());
        colorButton->setStyleSheet("QPushButton { background-color: " + color.name()
                                    + "; color: " + labelColor(color) + "; }");
    }
}

QString ColorSettings::labelColor(QColor color) const
{
    QString text = "white";
    if (color.red() > 50 && color.green() > 50 && color.blue() > 50)
        text = "black";
    return text;
}

void ColorSettings::updateRangeFromInit(int begin)
{
    int end = endFrame->value();
    totalStepsCount = end - begin + 1;
    totalLabel->setText(tr("Frames Total") + ": " + QString::number(totalStepsCount));
}

void ColorSettings::updateRangeFromEnd(int end)
{
    int begin = initFrame->value();
    totalStepsCount = end - begin + 1;
    totalLabel->setText(tr("Frames Total") + ": " + QString::number(totalStepsCount));
}
