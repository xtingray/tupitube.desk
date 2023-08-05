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

#include "papagayosettings.h"
#include "timagebutton.h"
#include "tseparator.h"
#include "tupsvg2qt.h"
#include "tresponsiveui.h"

PapagayoSettings::PapagayoSettings(QWidget *parent) : QWidget(parent)
{
    layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    setInnerForm();
}

PapagayoSettings::~PapagayoSettings()
{
}

void PapagayoSettings::setInnerForm()
{
    innerPanel = new QWidget;

    QBoxLayout *innerLayout = new QBoxLayout(QBoxLayout::TopToBottom, innerPanel);
    innerLayout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    QLabel *nameLabel = new QLabel(tr("Editing") + ": ");
    lipSyncName = new QLabel;

    QHBoxLayout *nameLayout = new QHBoxLayout;
    nameLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    nameLayout->setMargin(0);
    nameLayout->setSpacing(0);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(lipSyncName);

    QHBoxLayout *fpsLayout = new QHBoxLayout;
    fpsLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    fpsLayout->setMargin(0);
    fpsLayout->setSpacing(0);

    QLabel *startingLabel = new QLabel(tr("Starting at frame") + ": ");
    startingLabel->setAlignment(Qt::AlignVCenter);

    comboInit = new QSpinBox();
    comboInit->setEnabled(false);
    comboInit->setMinimum(1);
    comboInit->setMaximum(999);
    connect(comboInit, SIGNAL(valueChanged(int)), this, SLOT(updateInitFrame(int)));
 
    endingLabel = new QLabel;
    endingLabel->setAlignment(Qt::AlignVCenter);

    QHBoxLayout *startLayout = new QHBoxLayout;
    startLayout->setAlignment(Qt::AlignHCenter);
    startLayout->setMargin(0);
    startLayout->setSpacing(0);
    startLayout->addWidget(startingLabel);
    startLayout->addWidget(comboInit);

    QHBoxLayout *endLayout = new QHBoxLayout;
    endLayout->setAlignment(Qt::AlignHCenter);
    endLayout->setMargin(0);
    endLayout->setSpacing(0);
    endLayout->addWidget(endingLabel);

    totalLabel = new QLabel;
    totalLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    QHBoxLayout *totalLayout = new QHBoxLayout;
    totalLayout->setAlignment(Qt::AlignHCenter);
    totalLayout->setMargin(0);
    totalLayout->setSpacing(0);
    totalLayout->addWidget(totalLabel);

    phonemeLabel = new QLabel;
    phonemeLabel->setAlignment(Qt::AlignHCenter);

    // Position section

    QLabel *mouthPosLabel = new QLabel("<b>" + tr("Mouth Position") + "</b>");
    mouthPosLabel->setAlignment(Qt::AlignHCenter);

    QLabel *xLabel = new QLabel(tr("X") + ": ");
    xLabel->setMaximumWidth(20);

    xPosField = new QSpinBox;
    xPosField->setMinimum(-5000);
    xPosField->setMaximum(5000);
    connect(xPosField, SIGNAL(valueChanged(int)), this, SIGNAL(xPosChanged(int)));

    QLabel *yLabel = new QLabel(tr("Y") + ": ");
    yLabel->setMaximumWidth(20);

    yPosField = new QSpinBox;
    yPosField->setMinimum(-5000);
    yPosField->setMaximum(5000);
    connect(yPosField, SIGNAL(valueChanged(int)), this, SIGNAL(yPosChanged(int)));

    QBoxLayout *xLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    xLayout->setMargin(0);
    xLayout->setSpacing(0);
    xLayout->addStretch();
    xLayout->addWidget(xLabel);
    xLayout->addWidget(xPosField);
    xLayout->addStretch();

    QBoxLayout *yLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    yLayout->setMargin(0);
    yLayout->setSpacing(0);
    yLayout->addStretch();
    yLayout->addWidget(yLabel);
    yLayout->addWidget(yPosField);
    yLayout->addStretch();

    // Rotation section

    QLabel *rotationLabel = new QLabel("<b>" + tr("Mouth Rotation") + "</b>");
    rotationLabel->setAlignment(Qt::AlignHCenter);

    QLabel *angleLabel = new QLabel(tr("Angle") + ": ");

    angleField = new QSpinBox;
    angleField->setMinimum(0);
    angleField->setMaximum(360);
    connect(angleField, SIGNAL(valueChanged(int)), this, SIGNAL(rotationChanged(int)));

    QBoxLayout *angleLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    angleLayout->setMargin(0);
    angleLayout->setSpacing(0);
    angleLayout->addStretch();
    angleLayout->addWidget(angleLabel);
    angleLayout->addWidget(angleField);
    angleLayout->addStretch();

    // Scale section

    QLabel *scaleLabel = new QLabel("<b>" + tr("Mouth Scale") + "</b>");
    scaleLabel->setAlignment(Qt::AlignHCenter);

    QBoxLayout *scaleLayout = new QBoxLayout(QBoxLayout::TopToBottom);

    QLabel *factorXLabel = new QLabel(tr("X") + ": ");
    factorXField = new QDoubleSpinBox;
    factorXField->setDecimals(2);
    factorXField->setMinimum(0.01);
    factorXField->setMaximum(10);
    factorXField->setSingleStep(0.01);
    connect(factorXField, SIGNAL(valueChanged(double)), this, SLOT(notifyXScale(double)));

    QBoxLayout *factorXLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    factorXLayout->setMargin(0);
    factorXLayout->setSpacing(0);
    factorXLayout->addStretch();
    factorXLayout->addWidget(factorXLabel);
    factorXLayout->addWidget(factorXField);
    factorXLayout->addStretch();

    scaleLayout->addLayout(factorXLayout);

    QLabel *factorYLabel = new QLabel(tr("Y") + ": ");
    factorYField = new QDoubleSpinBox;
    factorYField->setDecimals(2);
    factorYField->setMinimum(0.01);
    factorYField->setMaximum(10);
    factorYField->setSingleStep(0.01);
    connect(factorYField, SIGNAL(valueChanged(double)), this, SLOT(notifyYScale(double)));

    QBoxLayout *factorYLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    factorYLayout->setMargin(0);
    factorYLayout->setSpacing(0);
    factorYLayout->addStretch();
    factorYLayout->addWidget(factorYLabel);
    factorYLayout->addWidget(factorYField);
    factorYLayout->addStretch();

    scaleLayout->addLayout(factorYLayout);

    propCheck = new QCheckBox(tr("Proportion"), this);
    connect(propCheck, SIGNAL(stateChanged(int)), this, SLOT(enableProportion(int)));
    scaleLayout->addWidget(propCheck);
    scaleLayout->setAlignment(propCheck, Qt::AlignHCenter);

    forwardCheck = new QCheckBox(tr("Apply Forward"), this);
    connect(forwardCheck, SIGNAL(stateChanged(int)), this, SIGNAL(forwardActivated(int)));
    scaleLayout->addWidget(forwardCheck);
    scaleLayout->setAlignment(forwardCheck, Qt::AlignHCenter);

    // Bottom section    
    int iconSize = TResponsiveUI::fitRightPanelIconSize();

    TImageButton *resetButton = new TImageButton(QPixmap(kAppProp->themeDir() + "icons/reset.png"), iconSize);
    resetButton->setToolTip(tr("Reset Mouth"));
    resetButton->setMaximumWidth(50);
    connect(resetButton, SIGNAL(clicked()), this, SIGNAL(objectHasBeenReset()));

    TImageButton *closeButton = new TImageButton(QPixmap(kAppProp->themeDir() + "icons/close_properties.png"), iconSize);
    closeButton->setToolTip(tr("Close properties"));
    connect(closeButton, SIGNAL(clicked()), this, SIGNAL(closeLipSyncProperties()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    buttonsLayout->setSpacing(20);
    buttonsLayout->addWidget(resetButton);
    buttonsLayout->addWidget(closeButton);

    // Header block

    innerLayout->addLayout(nameLayout);
    innerLayout->addLayout(fpsLayout);
    innerLayout->addLayout(startLayout);
    innerLayout->addLayout(endLayout);
    innerLayout->addLayout(totalLayout);
    innerLayout->addWidget(new TSeparator());
    innerLayout->addWidget(phonemeLabel);

    // Position block

    innerLayout->addWidget(mouthPosLabel);
    innerLayout->addLayout(xLayout);
    innerLayout->addLayout(yLayout);

    // Rotation block

    innerLayout->addWidget(rotationLabel);
    innerLayout->addLayout(angleLayout);

    // Scale block

    innerLayout->addWidget(scaleLabel);
    innerLayout->addLayout(scaleLayout);

    innerLayout->addSpacing(5);
    innerLayout->addLayout(buttonsLayout);
    innerLayout->addSpacing(5);

    innerLayout->addWidget(new TSeparator());
    layout->addWidget(innerPanel);
}

// Editing lip-sync record

void PapagayoSettings::openLipSyncProperties(TupLipSync *lipsync)
{
    name = lipsync->getLipSyncName();
    initFrame = lipsync->getInitFrame();
    framesCount = lipsync->getFramesTotal();

    lipSyncName->setText("<b>" + name + "</b>");

    comboInit->setEnabled(true);
    comboInit->setValue(initFrame + 1);

    int endIndex = initFrame + framesCount;
    endingLabel->setText(tr("Ending at frame") + ": <b>" + QString::number(endIndex) + "</b>");
    totalLabel->setText(tr("Frames Total") + ": <b>" + QString::number(framesCount) + "</b>");

    if (forwardCheck->isChecked())
        forwardCheck->setChecked(false);
}

void PapagayoSettings::updateInitFrame(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoSettings::updateInitFrame()] - index -> " << index;
    #endif

    int frame = index - 1;

    if (frame != initFrame) {
        initFrame = frame;
        emit initFrameHasChanged(frame);
    }
}

void PapagayoSettings::updateInterfaceRecords()
{
    int endIndex = initFrame + framesCount;
    endingLabel->setText(tr("Ending at frame") + ": <b>" + QString::number(endIndex) + "</b>");
}

void PapagayoSettings::setTransformations(const QDomElement &dom)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoSettings::setTransformations()]";
    #endif

    QPointF pos;
    TupSvg2Qt::parsePointF(dom.attribute("pos"), pos);
    TupTransformation::Parameters transStructure;
    transStructure.pos = pos;
    transStructure.rotationAngle = dom.attribute("rotation").toInt();
    transStructure.scaleFactor.setX(dom.attribute("scale_x").toDouble());
    transStructure.scaleFactor.setY(dom.attribute("scale_y").toDouble());

    setTransformations(transStructure);
}

void PapagayoSettings::setTransformations(const TupTransformation::Parameters params)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoSettings::setTransformations()]";
    #endif

     updatePositionCoords(params.pos.x(), params.pos.y());
     updateRotationAngle(params.rotationAngle);
     updateScaleFactor(params.scaleFactor.x(), params.scaleFactor.y());
}

void PapagayoSettings::setPhoneme(const TupPhoneme *phoneme)
{
    this->phoneme = phoneme;
    phonemeLabel->setText(tr("Current Phoneme") + ": <b>" + phoneme->value() + "</b>");
    setTransformations(phoneme->getTransformationParams());
}

void PapagayoSettings::updatePositionCoords(int x, int y)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoSettings::updatePositionCoords()] - x -> " << x;
        qDebug() << "[PapagayoSettings::updatePositionCoords()] - y -> " << y;
    #endif

   xPosField->blockSignals(true);
   yPosField->blockSignals(true);

   currentX = x;
   xPosField->setValue(x);

   currentY = y;
   yPosField->setValue(y);

   xPosField->blockSignals(false);
   yPosField->blockSignals(false);
}

void PapagayoSettings::updateRotationAngle(int angle)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoSettings::updateRotationAngle()] - angle -> " << angle;
    #endif

    angleField->blockSignals(true);

    if (angle > 359)
        angle = 0;
    angleField->setValue(angle);

    angleField->blockSignals(false);
}

void PapagayoSettings::updateScaleFactor(double x, double y)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoSettings::updateScaleFactor()] - x -> " << x;
        qDebug() << "[PapagayoSettings::updateScaleFactor()] - y -> " << y;
    #endif

   factorXField->blockSignals(true);
   factorYField->blockSignals(true);

   currentXFactor = x;
   factorXField->setValue(x);

   currentYFactor = y;
   factorYField->setValue(y);

   factorXField->blockSignals(false);
   factorYField->blockSignals(false);
}

void PapagayoSettings::notifyRotation(int angle)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoSettings::notifyRotation()] - angle -> " << angle;
    #endif

    if (angle == 360) {
        angle = 0;
        angleField->setValue(0);
    }
    emit rotationChanged(angle);
}

void PapagayoSettings::notifyXScale(double factor)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoSettings::notifyXScale()] - factor -> " << factor;
    #endif

    if (propCheck->isChecked()) {
        currentYFactor = factor;
        factorYField->setValue(factor);
    }

    emit scaleChanged(factor, currentYFactor);
    currentXFactor = factor;
}

void PapagayoSettings::notifyYScale(double factor)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoSettings::notifyYScale()] - factor -> " << factor;
    #endif

    if (propCheck->isChecked()) {
       currentXFactor = factor;
       factorXField->setValue(factor);
    }

    emit scaleChanged(currentXFactor, factor);
    currentYFactor = factor;
}

void PapagayoSettings::enableProportion(int flag)
{
    #ifdef TUP_DEBUG
        qDebug() << "[PapagayoSettings::enableProportion()] - flag -> " << flag;
    #endif

    bool enable = false;
    if (flag == Qt::Checked) {
        double factor =factorXField->value();
        factorYField->setValue(factor);
        emit scaleChanged(factor, factor);
        enable = true;
    }
    emit proportionActivated(enable);
}

void PapagayoSettings::setProportionState(int flag)
{
    propCheck->blockSignals(true);
    propCheck->setChecked(flag);
    propCheck->blockSignals(false);
}
