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

#include "motionsettings.h"
#include "tupitemtweener.h"
#include "tuptweenerstep.h"
#include "tosd.h"

MotionSettings::MotionSettings(QWidget *parent) : QWidget(parent)
{
    selectionDone = false;

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
    options->addItem(tr("Select Objects"), 0);
    options->addItem(tr("Set Path Properties"), 1);
    connect(options, SIGNAL(clicked(int)), this, SLOT(emitOptionChanged(int)));

    applyButton = new TImageButton(QPixmap(kAppProp->themeDir() + "icons/apply.png"), 22);
    connect(applyButton, SIGNAL(clicked()), this, SLOT(applyTween()));

    remove = new TImageButton(QPixmap(kAppProp->themeDir() + "icons/close.png"), 22);
    connect(remove, SIGNAL(clicked()), this, SIGNAL(clickedResetTween()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    buttonsLayout->setMargin(0);
    buttonsLayout->setSpacing(10);
    buttonsLayout->addWidget(applyButton);
    buttonsLayout->addWidget(remove);

    layout->addLayout(nameLayout);
    layout->addWidget(options);

    setInnerForm();

    layout->addSpacing(10);
    layout->addLayout(buttonsLayout);
    layout->setSpacing(5);

    activateMode(TupToolPlugin::Selection);
}

MotionSettings::~MotionSettings()
{
}

void MotionSettings::setInnerForm()
{
    innerPanel = new QWidget;

    QBoxLayout *innerLayout = new QBoxLayout(QBoxLayout::TopToBottom, innerPanel);
    innerLayout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    QLabel *startingLabel = new QLabel(tr("Starting at frame") + ": ");
    endingLabel = new QLabel(tr("Ending at frame") + ": 0");
    endingLabel->setAlignment(Qt::AlignHCenter);
    comboInit = new QSpinBox();

    connect(comboInit, SIGNAL(valueChanged(int)), this, SIGNAL(startingFrameChanged(int)));

    QHBoxLayout *startLayout = new QHBoxLayout;
    startLayout->setAlignment(Qt::AlignHCenter);
    startLayout->setMargin(0);
    startLayout->setSpacing(0);
    startLayout->addWidget(startingLabel);
    startLayout->addWidget(comboInit);

    stepViewer = new StepsViewer;
    connect(stepViewer, SIGNAL(totalHasChanged(int)), this, SLOT(updateTotalLabel(int)));

    totalLabel = new QLabel(tr("Frames Total") + ": 0");
    totalLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    QHBoxLayout *totalLayout = new QHBoxLayout;
    totalLayout->setAlignment(Qt::AlignHCenter);
    totalLayout->setMargin(0);
    totalLayout->setSpacing(0);
    totalLayout->addWidget(totalLabel);

    innerLayout->addLayout(startLayout);
    innerLayout->addWidget(endingLabel);
    innerLayout->addWidget(stepViewer);

    innerLayout->addLayout(totalLayout);

    layout->addWidget(innerPanel);

    activeInnerForm(false);
}

void MotionSettings::activeInnerForm(bool enable)
{
    if (enable && !innerPanel->isVisible())
        innerPanel->show();
    else
        innerPanel->hide();
}

// Adding new Tween

void MotionSettings::setParameters(const QString &name, int framesCount, int startFrame)
{
    #ifdef TUP_DEBUG
        qDebug() << "MotionSettings::setParameters() - Adding Tween context!";
    #endif

    mode = TupToolPlugin::Add;
    input->setText(name);

    activateMode(TupToolPlugin::Selection);
    stepViewer->clearInterface();
    totalLabel->setText(tr("Frames Total") + ": 0");

    comboInit->setEnabled(false);
    applyButton->setToolTip(tr("Save Tween"));
    applyButton->setEnabled(false);

    remove->setIcon(QPixmap(kAppProp->themeDir() + "icons/close.png"));
    remove->setToolTip(tr("Cancel Tween"));

    initStartCombo(framesCount, startFrame);
}

// Load properties of currentTween 

void MotionSettings::setParameters(TupItemTweener *currentTween)
{
    #ifdef TUP_DEBUG
        qDebug() << "Settings::setParameters() - Loading Tween context!";
    #endif

    setEditMode();

    notifySelection(true);
    activateMode(TupToolPlugin::Properties);

    input->setText(currentTween->getTweenName());
    comboInit->setEnabled(true);

    initStartCombo(currentTween->getFrames(), currentTween->getInitFrame());

    stepViewer->loadPath(currentTween->graphicsPath(), currentTween->getIntervals());
    totalLabel->setText(tr("Frames Total") + ": " + QString::number(stepViewer->totalSteps()));
}

void MotionSettings::initStartCombo(int framesCount, int currentIndex)
{
    comboInit->clear();
    comboInit->setMinimum(1);
    comboInit->setMaximum(framesCount);
    comboInit->setValue(currentIndex + 1);
}

void MotionSettings::setStartFrame(int currentIndex)
{
    comboInit->setValue(currentIndex + 1);
}

int MotionSettings::startFrame()
{
    return comboInit->value() - 1;
}

int MotionSettings::startComboSize()
{
    return comboInit->maximum();
}

void MotionSettings::updateSteps(const QGraphicsPathItem *path)
{
    if (path) {
        stepViewer->setPath(path);
        totalLabel->setText(tr("Frames Total") + ": " + QString::number(stepViewer->totalSteps()));
        endingLabel->setText(tr("Ending at frame") + ": " + QString::number(startFrame() + stepViewer->totalSteps()));
    }
}

void MotionSettings::emitOptionChanged(int option)
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
                emit clickedCreatePath();
            } else {
                options->setCurrentIndex(0);
                TOsd::self()->display(TOsd::Info, tr("Select objects for Tweening first!"));
            }
        }
    }
}

QString MotionSettings::tweenToXml(int currentScene, int currentLayer, int currentFrame, QPointF point, QString &path)
{
    QDomDocument doc;

    QDomElement root = doc.createElement("tweening");
    root.setAttribute("name", currentTweenName());
    root.setAttribute("type", TupItemTweener::Motion);
    root.setAttribute("initFrame", currentFrame);
    root.setAttribute("initLayer", currentLayer);
    root.setAttribute("initScene", currentScene);
    root.setAttribute("frames", stepViewer->totalSteps());
    root.setAttribute("origin", QString::number(point.x()) + "," + QString::number(point.y()));
    root.setAttribute("coords", path);
    root.setAttribute("intervals", stepViewer->intervals());

    foreach (TupTweenerStep *step, stepViewer->steps())
        root.appendChild(step->toXml(doc));

    doc.appendChild(root);

    return doc.toString();
}

int MotionSettings::totalSteps()
{
    return stepViewer->totalSteps();
}

QList<QPointF> MotionSettings::tweenPoints()
{
    return stepViewer->tweenPoints();
}

void MotionSettings::activateMode(TupToolPlugin::EditMode mode)
{
    options->setCurrentIndex(mode);
}

void MotionSettings::clearData()
{
    stepViewer->clearInterface();
}

void MotionSettings::notifySelection(bool flag)
{
    #ifdef TUP_DEBUG
        qDebug() << "Settings::notifySelection() - selection is done? -> " << flag;
    #endif

    selectionDone = flag;
}

void MotionSettings::applyTween()
{
    if (!selectionDone) {
        options->setCurrentIndex(0);
        TOsd::self()->display(TOsd::Info, tr("You must select at least one object!"));
        return;
    }

    /*
    if (totalSteps() <= 2) {
        TOsd::self()->display(tr("Information"), tr("You must define a path for this Tween!"), TOsd::Info);
        return;
    }
    */

    // SQA: Verify if Tween is already saved before calling setEditMode!
    setEditMode();

    if (!comboInit->isEnabled())
        comboInit->setEnabled(true);

    emit clickedApplyTween();
}

void MotionSettings::setEditMode()
{
    mode = TupToolPlugin::Edit;
    applyButton->setToolTip(tr("Update Tween"));
    applyButton->setEnabled(true);
    remove->setIcon(QPixmap(kAppProp->themeDir() + "icons/close_properties.png"));
    remove->setToolTip(tr("Close Tween Properties"));
}

QString MotionSettings::currentTweenName() const
{
    QString tweenName = input->text();
    if (tweenName.length() > 0)
        input->setFocus();

    return tweenName;
}

void MotionSettings::updateTotalLabel(int total)
{
    endingLabel->setText(tr("Ending at frame") + ": " + QString::number(startFrame() + stepViewer->totalSteps()));
    totalLabel->setText(tr("Frames Total") + ": " + QString::number(total));
    emit framesTotalChanged(); 
}

void MotionSettings::undoSegment(const QPainterPath path)
{
    stepViewer->undoSegment(path);
}

void MotionSettings::redoSegment(const QPainterPath path)
{
    stepViewer->redoSegment(path);
}

void MotionSettings::enableSaveOption(bool flag)
{
    #ifdef TUP_DEBUG
        qDebug() << "MotionSettings::enableSaveOption() - flag -> " << flag;
    #endif

    applyButton->setEnabled(flag);
}

int MotionSettings::stepsTotal() 
{
    return stepViewer->rowCount();
}

void MotionSettings::updateSegments(const QPainterPath path)
{
    stepViewer->updateSegments(path);
}
