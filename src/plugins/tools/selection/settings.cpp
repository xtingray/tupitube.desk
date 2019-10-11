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
#include "tdebug.h"
#include "tapplicationproperties.h"
#include "timagebutton.h"
#include "tseparator.h"

PenSettings::PenSettings(QWidget *parent) : QWidget(parent)
{
    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);

    QLabel *toolTitle = new QLabel;
    toolTitle->setAlignment(Qt::AlignHCenter);
    QPixmap pic(THEME_DIR + "icons/selection.png");
    toolTitle->setPixmap(pic.scaledToWidth(16, Qt::SmoothTransformation));
    toolTitle->setToolTip(tr("Selection Properties"));
    mainLayout->addWidget(toolTitle);
    mainLayout->addWidget(new TSeparator(Qt::Horizontal));

    formPanel = new QWidget;
    QBoxLayout *formLayout = new QBoxLayout(QBoxLayout::TopToBottom,formPanel);

    QLabel *alignLabel = new QLabel(tr("Alignment"));
    alignLabel->setAlignment(Qt::AlignHCenter);
    formLayout->addWidget(alignLabel);

    QBoxLayout *alignLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    alignLayout->setMargin(0);
    alignLayout->setSpacing(0);

    TImageButton *hAlignButton = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/h_center.png"), 22);
    hAlignButton->setToolTip(tr("Horizontal Center"));
    TImageButton *vAlignButton = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/v_center.png"), 22);
    vAlignButton->setToolTip(tr("Vertical Center"));
    TImageButton *aAlignButton = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/a_center.png"), 22);
    aAlignButton->setToolTip(tr("Absolute Center"));
    connect(hAlignButton, SIGNAL(clicked()), this, SLOT(alignObjectHorizontally()));
    connect(vAlignButton, SIGNAL(clicked()), this, SLOT(alignObjectVertically()));
    connect(aAlignButton, SIGNAL(clicked()), this, SLOT(alignObjectAbsolutely()));

    alignLayout->addWidget(hAlignButton);
    alignLayout->addWidget(vAlignButton);
    alignLayout->addWidget(aAlignButton);

    formLayout->addLayout(alignLayout);
    formLayout->addWidget(new TSeparator(Qt::Horizontal));

    QLabel *flips = new QLabel(tr("Flips"));
    flips->setAlignment(Qt::AlignHCenter);
    formLayout->addWidget(flips);

    QBoxLayout *buttonsLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    buttonsLayout->setMargin(0);
    buttonsLayout->setSpacing(0);

    TImageButton *horizontalFlip = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/horizontal_flip.png"), 22);
    horizontalFlip->setToolTip(tr("Horizontal Flip"));
    TImageButton *verticalFlip = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/vertical_flip.png"), 22);
    verticalFlip->setToolTip(tr("Vertical Flip"));
    TImageButton *crossedFlip = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/crossed_flip.png"), 22);
    crossedFlip->setToolTip(tr("Crossed Flip"));
    connect(horizontalFlip, SIGNAL(clicked()), this, SLOT(hFlip()));
    connect(verticalFlip, SIGNAL(clicked()), this, SLOT(vFlip()));
    connect(crossedFlip, SIGNAL(clicked()), this, SLOT(cFlip()));

    buttonsLayout->addWidget(horizontalFlip);
    buttonsLayout->addWidget(verticalFlip);
    buttonsLayout->addWidget(crossedFlip);

    formLayout->addLayout(buttonsLayout);
    formLayout->addWidget(new TSeparator(Qt::Horizontal));

    QLabel *order = new QLabel(tr("Order"));
    order->setAlignment(Qt::AlignHCenter);
    formLayout->addWidget(order);

    QBoxLayout *orderButtonsLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    orderButtonsLayout->setMargin(0);
    orderButtonsLayout->setSpacing(0);

    TImageButton *toBack = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/to_back.png"), 22);
    toBack->setToolTip(tr("Send object to back"));

    TImageButton *toBackOneLevel = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/to_back_one.png"), 22);
    toBackOneLevel->setToolTip(tr("Send object to back one level"));

    TImageButton *toFront = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/to_front.png"), 22);
    toFront->setToolTip(tr("Send object to front"));

    TImageButton *toFrontOneLevel = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/to_front_one.png"), 22);
    toFrontOneLevel->setToolTip(tr("Send object to front one level"));

    connect(toBack, SIGNAL(clicked()), this, SLOT(sendToBack()));
    connect(toBackOneLevel, SIGNAL(clicked()), this, SLOT(sendToBackOneLevel()));
    connect(toFront, SIGNAL(clicked()), this, SLOT(sendToFront()));
    connect(toFrontOneLevel, SIGNAL(clicked()), this, SLOT(sendToFrontOneLevel()));

    orderButtonsLayout->addWidget(toBack);
    orderButtonsLayout->addWidget(toBackOneLevel);
    orderButtonsLayout->addWidget(toFront);
    orderButtonsLayout->addWidget(toFrontOneLevel);

    formLayout->addLayout(orderButtonsLayout);
    formLayout->addWidget(new TSeparator(Qt::Horizontal));

    // Group/Ungroup actions

    QLabel *groupLayer = new QLabel(tr("Group"));
    groupLayer->setAlignment(Qt::AlignHCenter);
    formLayout->addWidget(groupLayer);

    QBoxLayout *groupButtonsLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    groupButtonsLayout->setMargin(0);
    groupButtonsLayout->setSpacing(0);

    TImageButton *groupButton = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/group.png"), 22);
    groupButton->setToolTip(tr("Group Objects"));

    TImageButton *ungroupButton = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/ungroup.png"), 22);
    ungroupButton->setToolTip(tr("Ungroup Objects"));

    connect(groupButton, SIGNAL(clicked()), this, SLOT(groupItems()));
    connect(ungroupButton, SIGNAL(clicked()), this, SLOT(ungroupItems()));

    groupButtonsLayout->addWidget(groupButton);
    groupButtonsLayout->addWidget(ungroupButton);

    formLayout->addLayout(groupButtonsLayout);
    formLayout->addWidget(new TSeparator(Qt::Horizontal));

    // Transformation Panels

    QLabel *position = new QLabel(tr("Position"));
    position->setAlignment(Qt::AlignHCenter);
    formLayout->addWidget(position);

    QLabel *xLabel = new QLabel(tr("X") + ": ");
    xLabel->setMaximumWidth(20);

    xPosField = new QSpinBox;
    xPosField->setMinimum(-5000);
    xPosField->setMaximum(5000);
    connect(xPosField, SIGNAL(valueChanged(int)), this, SLOT(notifyXMovement(int)));

    QLabel *yLabel = new QLabel(tr("Y") + ": ");
    yLabel->setMaximumWidth(20);

    yPosField = new QSpinBox;
    yPosField->setMinimum(-5000);
    yPosField->setMaximum(5000);
    connect(yPosField, SIGNAL(valueChanged(int)), this, SLOT(notifyYMovement(int)));

    QBoxLayout *xLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    xLayout->setMargin(0);
    xLayout->setSpacing(0);
    xLayout->addWidget(xLabel);
    xLayout->addWidget(xPosField);

    formLayout->addLayout(xLayout);

    QBoxLayout *yLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    yLayout->setMargin(0);
    yLayout->setSpacing(0);
    yLayout->addWidget(yLabel);
    yLayout->addWidget(yPosField);

    formLayout->addLayout(yLayout);
    formLayout->addWidget(new TSeparator(Qt::Horizontal));

    QLabel *rotation = new QLabel(tr("Rotation"));
    rotation->setAlignment(Qt::AlignHCenter);
    formLayout->addWidget(rotation);

    QLabel *angleLabel = new QLabel(tr("Angle") + ": ");

    angleField = new QSpinBox;
    angleField->setMinimum(0);
    angleField->setMaximum(360);
    connect(angleField, SIGNAL(valueChanged(int)), this, SLOT(notifyRotation(int)));

    QBoxLayout *angleLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    angleLayout->setMargin(0);
    angleLayout->setSpacing(0);
    angleLayout->addWidget(angleLabel);
    angleLayout->addWidget(angleField);

    formLayout->addLayout(angleLayout);
    formLayout->addWidget(new TSeparator(Qt::Horizontal));

    QLabel *scale = new QLabel(tr("Scale"));
    scale->setAlignment(Qt::AlignHCenter);
    formLayout->addWidget(scale);

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
    factorXLayout->addWidget(factorXLabel);
    factorXLayout->addWidget(factorXField);

    formLayout->addLayout(factorXLayout);

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
    factorYLayout->addWidget(factorYLabel);
    factorYLayout->addWidget(factorYField);

    formLayout->addLayout(factorYLayout);

    propCheck = new QCheckBox(tr("Proportion"), this);
    //propCheck->setChecked(true);
    connect(propCheck, SIGNAL(stateChanged(int)), this, SLOT(enableProportion(int)));
    formLayout->addWidget(propCheck);
    formLayout->setAlignment(propCheck, Qt::AlignHCenter);

    formLayout->addWidget(propCheck);
    formLayout->setAlignment(propCheck, Qt::AlignHCenter);

    formLayout->addWidget(formPanel);
    formPanel->setVisible(false);

    mainLayout->addWidget(formPanel);

    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
    tips = new QPushButton(tr("Show Tips"));
    tips->setToolTip(tr("A little help for the Selection tool"));
    layout->addWidget(tips);
    connect(tips, SIGNAL(clicked()), this, SLOT(openTipPanel()));

    mainLayout->addLayout(layout);

    help = new QWidget(this);
    help->hide();
    QBoxLayout *helpLayout = new QBoxLayout(QBoxLayout::TopToBottom,help);

    int h = height();
    textArea = new QTextEdit;

    // SQA: Check this code with several screen resolutions. It must looks good with everyone! 
    textArea->setHtml("<p><b>" + tr("Double Click on any node or Shortcut Alt + R") + ":</b> " + tr("Rotation mode") + "</p>");
    textArea->append("<p><b>" + tr("Arrows") + ":</b> " +  tr("Movement on selection") + "</p>");
    textArea->append("<p><b>" + tr("Shift + Arrows") + ":</b> " +  tr("Slow movement on selection") + "</p>");
    textArea->append("<p><b>" + tr("Ctrl + Arrows") + ":</b> " +  tr("Fast movement on selection") + "</p>");
    textArea->append("<p><b>" + tr("Ctrl + Left Mouse Button") + ":</b> " +  tr("Proportional scaling on selection") + "</p>");

    help->setFixedHeight(h);
    helpLayout->addWidget(textArea);

    mainLayout->addWidget(help);
    mainLayout->addStretch(2);
    isVisible = false;
}

PenSettings::~PenSettings()
{
}

void PenSettings::hFlip()
{
    emit callFlip(PenSettings::Horizontal);
}

void PenSettings::vFlip()
{
    emit callFlip(PenSettings::Vertical);
}

void PenSettings::cFlip()
{
    emit callFlip(PenSettings::Crossed);
}

void PenSettings::sendToBack()
{
    emit callOrderAction(PenSettings::ToBack);
}

void PenSettings::sendToBackOneLevel()
{
    emit callOrderAction(PenSettings::ToBackOneLevel);
}

void PenSettings::sendToFront()
{
    emit callOrderAction(PenSettings::ToFront);
}

void PenSettings::sendToFrontOneLevel()
{
    emit callOrderAction(PenSettings::ToFrontOneLevel);
}

void PenSettings::groupItems()
{
    emit callGroupAction(PenSettings::GroupItems);
}

void PenSettings::ungroupItems()
{
    emit callGroupAction(PenSettings::UngroupItems);
}

void PenSettings::openTipPanel()
{
    if (help->isVisible()) {
       help->hide();
       if (isVisible) {
           if (!formPanel->isVisible())
               formPanel->show();
       }
    } else {
       if (formPanel->isVisible())
           formPanel->hide();
       help->show();
    }
}

void PenSettings::enableFormControls(bool flag)
{
    if (flag) {
        if (help->isVisible())
           help->hide();
    }
    isVisible = flag;
    formPanel->setVisible(flag);
}

void PenSettings::setPos(int x, int y)
{
   xPosField->blockSignals(true);
   yPosField->blockSignals(true);

   xPosField->setValue(x);
   yPosField->setValue(y);
   currentX = x;
   currentY = y;

   xPosField->blockSignals(false);
   yPosField->blockSignals(false);
}

void PenSettings::updateRotationAngle(int angle)
{
#ifdef TUP_DEBUG
    #ifdef Q_OS_WIN
        qDebug() << "[Settings::updateRotationAngle()]";
    #else
        T_FUNCINFO << angle;
    #endif
#endif

    angleField->blockSignals(true);

    if (angle > 359)
        angle = 0;
    angleField->setValue(angle);
    currentAngle = angle;

    angleField->blockSignals(false);
}

void PenSettings::updateScaleFactor(double x, double y)
{
#ifdef TUP_DEBUG
    QString msg1 = "Settings::updateScaleFactor() - x: " + QString::number(x);
    QString msg2 = "Settings::updateScaleFactor() - y: " + QString::number(y);
    #ifdef Q_OS_WIN
        qDebug() << msg1;
        qDebug() << msg2;
    #else
        T_FUNCINFO << msg1;
        T_FUNCINFO << msg2;
    #endif
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

void PenSettings::notifyXMovement(int x)
{
    emit positionUpdated(x -currentX, 0);
    currentX =xPosField->value();
}

void PenSettings::notifyYMovement(int y)
{
    emit positionUpdated(0, y -currentY);
    currentY =yPosField->value();
}

void PenSettings::notifyRotation(int angle)
{
    if (angle == 360) {
        angle = 0;
        angleField->setValue(0);
    }
    emit rotationUpdated(angle);
    currentAngle =angleField->value();
}

void PenSettings::notifyXScale(double factor)
{
    if (propCheck->isChecked()) {
        currentYFactor = factor;
        factorYField->setValue(factor);
    }

    emit scaleUpdated(factor,currentYFactor);
    currentXFactor = factor;
}

void PenSettings::notifyYScale(double factor)
{
    if (propCheck->isChecked()) {
       currentXFactor = factor;
       factorXField->setValue(factor);
    }

    emit scaleUpdated(currentXFactor, factor);
    currentYFactor = factor;
}

void PenSettings::enableProportion(int flag)
{
    bool enable = false;
    if (flag == Qt::Checked) {
        double factor =factorXField->value();
        factorYField->setValue(factor);
        emit scaleUpdated(factor, factor);
        enable = true;
    }
    emit activateProportion(enable);
}

void PenSettings::setProportionState(int flag)
{
    propCheck->blockSignals(true);
    propCheck->setChecked(flag);
    propCheck->blockSignals(false);
}

bool PenSettings::formIsVisible()
{
    return isVisible;
}

void PenSettings::alignObjectHorizontally()
{
    emit callAlignAction(PenSettings::hAlign);
}

void PenSettings::alignObjectVertically()
{
    emit callAlignAction(PenSettings::vAlign);
}

void PenSettings::alignObjectAbsolutely()
{
    emit callAlignAction(PenSettings::totalAlign);
}
