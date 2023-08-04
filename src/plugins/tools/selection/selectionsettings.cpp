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

#include "selectionsettings.h"
#include "tconfig.h"
#include "tdebug.h"
#include "tapplicationproperties.h"
#include "tlabel.h"
#include "timagebutton.h"
#include "tseparator.h"
#include "talgorithm.h"

#include <QButtonGroup>

SelectionSettings::SelectionSettings(QWidget *parent) : QWidget(parent)
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    screenW = dimension.first;
    screenH = dimension.second;

    iconSize = PANEL_ICON_SIZE;
    int toolIconSize = PLUGIN_ICON_SIZE;
    // Big resolutions
    if (screenW > HD_WIDTH) {
        iconSize = (screenW*2)/100;
        float value = (screenW*0.8)/100;
        toolIconSize = value;
    }

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);

    QPixmap pic(THEME_DIR + "icons/selection.png");
    QLabel *toolTitle = new QLabel;
    toolTitle->setAlignment(Qt::AlignHCenter);
    toolTitle->setPixmap(pic.scaledToWidth(toolIconSize, Qt::SmoothTransformation));
    toolTitle->setToolTip(tr("Selection Properties"));
    mainLayout->addWidget(toolTitle);
    mainLayout->addWidget(new TSeparator(Qt::Horizontal));

    formPanel = new QWidget;

#if defined(Q_OS_MAC)
    if (screenH >= 900)
        setLargetInterface();
    else
        setCompactInterface();
#else
    if (screenH >= 1080)
        setLargetInterface();
    else
        setCompactInterface();
#endif

    mainLayout->addWidget(formPanel);

    tips = new QPushButton(tr("Show Tips"));
    tips->setToolTip(tr("A little help for the Selection tool"));
    if (screenH < 1080) {
        QFont font = this->font();
        font.setPointSize(8);
        tips->setFont(font);
    }

    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
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
    mainLayout->addStretch();
    isVisible = false;
}

SelectionSettings::~SelectionSettings()
{
}

void SelectionSettings::setLargetInterface()
{
    QBoxLayout *formLayout = new QBoxLayout(QBoxLayout::TopToBottom, formPanel);

    QLabel *alignLabel = new QLabel("<b>" + tr("Alignment") + "</b>");
    alignLabel->setAlignment(Qt::AlignHCenter);
    formLayout->addWidget(alignLabel);

    formLayout->addLayout(setAlignBlock());
    formLayout->addWidget(new TSeparator(Qt::Horizontal));

    QLabel *flips = new QLabel("<b>" + tr("Flips") + "</b>");
    flips->setAlignment(Qt::AlignHCenter);
    formLayout->addWidget(flips);

    formLayout->addLayout(setFlipsBlock());
    formLayout->addWidget(new TSeparator(Qt::Horizontal));

    QLabel *order = new QLabel("<b>" + tr("Order") + "</b>");
    order->setAlignment(Qt::AlignHCenter);
    formLayout->addWidget(order);

    formLayout->addLayout(setOrderBlock());
    formLayout->addWidget(new TSeparator(Qt::Horizontal));

    // Group/Ungroup actions

    QLabel *groupLayer = new QLabel("<b>" + tr("Group") + "</b>");
    groupLayer->setAlignment(Qt::AlignHCenter);
    formLayout->addWidget(groupLayer);

    formLayout->addLayout(setGroupBlock());
    formLayout->addWidget(new TSeparator(Qt::Horizontal));

    // Transformation Panels

    QLabel *position = new QLabel("<b>" + tr("Position") + "</b>");
    position->setAlignment(Qt::AlignHCenter);
    formLayout->addWidget(position);

    formLayout->addLayout(setPosBlock());
    // formLayout->addWidget(new TSeparator(Qt::Horizontal));

    QLabel *rotation = new QLabel("<b>" + tr("Rotation") + "</b>");
    rotation->setAlignment(Qt::AlignHCenter);
    formLayout->addWidget(rotation);

    formLayout->addLayout(setRotateBlock());
    // formLayout->addWidget(new TSeparator(Qt::Horizontal));

    QLabel *scale = new QLabel("<b>" + tr("Scale") + "</b>");
    scale->setAlignment(Qt::AlignHCenter);
    formLayout->addWidget(scale);

    formLayout->addLayout(setScaleBlock());
    formLayout->addWidget(new TSeparator(Qt::Horizontal));

    formLayout->addLayout(setPasteBlock());
    formPanel->setVisible(false);
}

void SelectionSettings::setCompactInterface()
{
    buttonLabels << tr("Alignment") << tr("Flips") << tr("Order") << tr("Group");
    buttonLabels << tr("Position") << tr("Rotation") << tr("Scale");

    QFont font = this->font();
    font.setPointSize(8);

    QBoxLayout *formLayout = new QBoxLayout(QBoxLayout::TopToBottom, formPanel);
    formPanel->setVisible(false);

    actionLayout[0] = setAlignBlock();
    actionLayout[1] = setFlipsBlock();
    actionLayout[2] = setOrderBlock();
    actionLayout[3] = setGroupBlock();
    actionLayout[4] = setPosBlock();
    actionLayout[5] = setRotateBlock();
    actionLayout[6] = setScaleBlock();

    QButtonGroup *actionsGroup = new QButtonGroup(this);

    int i = 0;
    foreach (QString label, buttonLabels) {
        actionButton[i] = new QPushButton(label);
        actionButton[i]->setFont(font);
        actionButton[i]->setCheckable(true);
        actionsGroup->addButton(actionButton[i]);
        actionsGroup->setId(actionButton[i], i);
        formLayout->addWidget(actionButton[i]);

        actionWidget[i] = new QWidget;
        actionWidget[i]->setLayout(actionLayout[i]);
        actionWidget[i]->setVisible(false);
        formLayout->addWidget(actionWidget[i]);

        formLayout->addWidget(new TSeparator(Qt::Horizontal));
        i++;
    }

    connect(actionsGroup, SIGNAL(buttonClicked(int)), this, SLOT(showActionPanel(int)));
    formLayout->addLayout(setPasteBlock());
}

void SelectionSettings::showActionPanel(int index)
{
    bool checked = !actionWidget[index]->isVisible();
    actionWidget[index]->setVisible(checked);
    updatePanel(index);
}

void SelectionSettings::updatePanel(int index)
{
    for (int i = 0; i < buttonLabels.size(); i++) {
        if (i != index) {
            actionButton[i]->setChecked(false);
            actionWidget[i]->setVisible(false);
        }
    }
}

QBoxLayout * SelectionSettings::setAlignBlock()
{
    QBoxLayout *alignLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    alignLayout->setMargin(0);
    alignLayout->setSpacing(0);

    TImageButton *hAlignButton = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/h_center.png"), iconSize);
    hAlignButton->setToolTip(tr("Horizontal Center"));
    TImageButton *vAlignButton = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/v_center.png"), iconSize);
    vAlignButton->setToolTip(tr("Vertical Center"));
    TImageButton *aAlignButton = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/a_center.png"), iconSize);
    aAlignButton->setToolTip(tr("Absolute Center"));
    connect(hAlignButton, SIGNAL(clicked()), this, SLOT(alignObjectHorizontally()));
    connect(vAlignButton, SIGNAL(clicked()), this, SLOT(alignObjectVertically()));
    connect(aAlignButton, SIGNAL(clicked()), this, SLOT(alignObjectAbsolutely()));

    alignLayout->addWidget(hAlignButton);
    alignLayout->addWidget(vAlignButton);
    alignLayout->addWidget(aAlignButton);

    return alignLayout;
}

QBoxLayout * SelectionSettings::setFlipsBlock()
{
    QBoxLayout *flipLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    flipLayout->setMargin(0);
    flipLayout->setSpacing(0);

    TImageButton *horizontalFlip = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/horizontal_flip.png"), iconSize);
    horizontalFlip->setToolTip(tr("Horizontal Flip"));
    TImageButton *verticalFlip = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/vertical_flip.png"), iconSize);
    verticalFlip->setToolTip(tr("Vertical Flip"));
    TImageButton *crossedFlip = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/crossed_flip.png"), iconSize);
    crossedFlip->setToolTip(tr("Crossed Flip"));
    connect(horizontalFlip, SIGNAL(clicked()), this, SLOT(hFlip()));
    connect(verticalFlip, SIGNAL(clicked()), this, SLOT(vFlip()));
    connect(crossedFlip, SIGNAL(clicked()), this, SLOT(cFlip()));

    flipLayout->addWidget(horizontalFlip);
    flipLayout->addWidget(verticalFlip);
    flipLayout->addWidget(crossedFlip);

    return flipLayout;
}

QBoxLayout * SelectionSettings::setOrderBlock()
{
    QBoxLayout *orderLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    orderLayout->setMargin(0);
    orderLayout->setSpacing(0);

    TImageButton *toBack = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/to_back.png"), iconSize);
    toBack->setToolTip(tr("Send object to back"));

    TImageButton *toBackOneLevel = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/to_back_one.png"), iconSize);
    toBackOneLevel->setToolTip(tr("Send object to back one level"));

    TImageButton *toFront = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/to_front.png"), iconSize);
    toFront->setToolTip(tr("Send object to front"));

    TImageButton *toFrontOneLevel = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/to_front_one.png"), iconSize);
    toFrontOneLevel->setToolTip(tr("Send object to front one level"));

    connect(toBack, SIGNAL(clicked()), this, SLOT(sendToBack()));
    connect(toBackOneLevel, SIGNAL(clicked()), this, SLOT(sendToBackOneLevel()));
    connect(toFront, SIGNAL(clicked()), this, SLOT(sendToFront()));
    connect(toFrontOneLevel, SIGNAL(clicked()), this, SLOT(sendToFrontOneLevel()));

    orderLayout->addWidget(toBack);
    orderLayout->addWidget(toBackOneLevel);
    orderLayout->addWidget(toFront);
    orderLayout->addWidget(toFrontOneLevel);

    return orderLayout;
}

QBoxLayout * SelectionSettings::setGroupBlock()
{
    QBoxLayout *groupLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    groupLayout->setMargin(0);
    groupLayout->setSpacing(0);

    TImageButton *groupButton = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/group.png"), iconSize);
    groupButton->setToolTip(tr("Group Objects"));

    TImageButton *ungroupButton = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/ungroup.png"), iconSize);
    ungroupButton->setToolTip(tr("Ungroup Objects"));

    connect(groupButton, SIGNAL(clicked()), this, SLOT(groupItems()));
    connect(ungroupButton, SIGNAL(clicked()), this, SLOT(ungroupItems()));

    groupLayout->addWidget(groupButton);
    groupLayout->addWidget(ungroupButton);

    return groupLayout;
}

QBoxLayout * SelectionSettings::setPosBlock()
{
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);

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

    layout->addLayout(xLayout);

    QBoxLayout *yLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    yLayout->setMargin(0);
    yLayout->setSpacing(0);
    yLayout->addWidget(yLabel);
    yLayout->addWidget(yPosField);

    layout->addLayout(yLayout);
    return layout;
}

QBoxLayout * SelectionSettings::setRotateBlock()
{
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

    return angleLayout;
}

QBoxLayout * SelectionSettings::setScaleBlock()
{
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);

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

    layout->addLayout(factorXLayout);

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

    layout->addLayout(factorYLayout);

    propCheck = new QCheckBox(tr("Proportion"), this);
    connect(propCheck, SIGNAL(stateChanged(int)), this, SLOT(enableProportion(int)));
    layout->addWidget(propCheck);
    layout->setAlignment(propCheck, Qt::AlignHCenter);

    return layout;
}

QBoxLayout * SelectionSettings::setPasteBlock()
{
    TCONFIG->beginGroup("PaintArea");
    bool onMouse = TCONFIG->value("PasteOnMousePos", false).toBool();
    pasteCheck = new QCheckBox;
    pasteCheck->setChecked(onMouse);
    connect(pasteCheck, SIGNAL(stateChanged(int)), this, SLOT(enablePasteOnMouse(int)));

    int pasteSize = 15;
    int resetSize = 18;
    if (screenW > HD_WIDTH) {
        float value = (screenW*0.8)/100;
        pasteSize = (int) value;
        value = (screenW*0.95)/100;
        resetSize = (int) value;
    }

    TLabel *pasteLabel = new TLabel;
    pasteLabel->setPixmap(QPixmap(kAppProp->themeDir() + "/icons/paste.png").scaledToWidth(pasteSize));
    pasteLabel->setToolTip(tr("Paste objects over mouse position"));
    connect(pasteLabel, SIGNAL(clicked()), this, SLOT(enablePasteOnMouse()));

    TImageButton *resetButton = new TImageButton(QPixmap(kAppProp->themeDir() + "icons/reset.png"), resetSize);
    resetButton->setToolTip(tr("Reset Item"));
    connect(resetButton, SIGNAL(clicked()), this, SIGNAL(objectHasBeenReset()));

    QBoxLayout *pasteLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    pasteLayout->setMargin(0);
    pasteLayout->addWidget(pasteCheck, Qt::AlignHCenter);
    pasteLayout->addWidget(pasteLabel, Qt::AlignHCenter);
    pasteLayout->addWidget(new TSeparator(Qt::Vertical));
    pasteLayout->addWidget(resetButton);

    return pasteLayout;
}

void SelectionSettings::hFlip()
{
    emit callFlip(SelectionSettings::Horizontal);
}

void SelectionSettings::vFlip()
{
    emit callFlip(SelectionSettings::Vertical);
}

void SelectionSettings::cFlip()
{
    emit callFlip(SelectionSettings::Crossed);
}

void SelectionSettings::sendToBack()
{
    emit callOrderAction(SelectionSettings::ToBack);
}

void SelectionSettings::sendToBackOneLevel()
{
    emit callOrderAction(SelectionSettings::ToBackOneLevel);
}

void SelectionSettings::sendToFront()
{
    emit callOrderAction(SelectionSettings::ToFront);
}

void SelectionSettings::sendToFrontOneLevel()
{
    emit callOrderAction(SelectionSettings::ToFrontOneLevel);
}

void SelectionSettings::groupItems()
{
    emit callGroupAction(SelectionSettings::GroupItems);
}

void SelectionSettings::ungroupItems()
{
    emit callGroupAction(SelectionSettings::UngroupItems);
}

void SelectionSettings::openTipPanel()
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

void SelectionSettings::enableFormControls(bool flag)
{
    if (flag) {
        if (help->isVisible())
           help->hide();
    }
    isVisible = flag;
    formPanel->setVisible(flag);
}

void SelectionSettings::setPos(int x, int y)
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

void SelectionSettings::updateRotationAngle(int angle)
{
    #ifdef TUP_DEBUG
        qDebug() << "[SelectionSettings::updateRotationAngle()] - angle -> " << angle;
    #endif

    angleField->blockSignals(true);

    if (angle > 359)
        angle = 0;
    angleField->setValue(angle);
    currentAngle = angle;

    angleField->blockSignals(false);
}

void SelectionSettings::updateScaleFactor(double x, double y)
{
    #ifdef TUP_DEBUG
        qDebug() << "[SelectionSettings::updateScaleFactor()] - x -> " << x;
        qDebug() << "[SelectionSettings::updateScaleFactor()] - y -> " << y;
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

void SelectionSettings::notifyXMovement(int x)
{
    emit positionUpdated(x -currentX, 0);
    currentX =xPosField->value();
}

void SelectionSettings::notifyYMovement(int y)
{
    emit positionUpdated(0, y -currentY);
    currentY =yPosField->value();
}

void SelectionSettings::notifyRotation(int angle)
{
    if (angle == 360) {
        angle = 0;
        angleField->setValue(0);
    }
    emit rotationUpdated(angle);
    currentAngle =angleField->value();
}

void SelectionSettings::notifyXScale(double factor)
{
    if (propCheck->isChecked()) {
        currentYFactor = factor;
        factorYField->setValue(factor);
    }

    emit scaleUpdated(factor,currentYFactor);
    currentXFactor = factor;
}

void SelectionSettings::notifyYScale(double factor)
{
    if (propCheck->isChecked()) {
       currentXFactor = factor;
       factorXField->setValue(factor);
    }

    emit scaleUpdated(currentXFactor, factor);
    currentYFactor = factor;
}

void SelectionSettings::enableProportion(int flag)
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

void SelectionSettings::enablePasteOnMouse(int)
{
    TCONFIG->beginGroup("PaintArea");
    TCONFIG->setValue("PasteOnMousePos", pasteCheck->isChecked());
    TCONFIG->sync();
}

void SelectionSettings::enablePasteOnMouse()
{
    if (pasteCheck->isChecked())
        pasteCheck->setChecked(false);
    else
        pasteCheck->setChecked(true);
}

void SelectionSettings::setProportionState(int flag)
{
    propCheck->blockSignals(true);
    propCheck->setChecked(flag);
    propCheck->blockSignals(false);
}

bool SelectionSettings::formIsVisible()
{
    return isVisible;
}

void SelectionSettings::alignObjectHorizontally()
{
    emit callAlignAction(SelectionSettings::hAlign);
}

void SelectionSettings::alignObjectVertically()
{
    emit callAlignAction(SelectionSettings::vAlign);
}

void SelectionSettings::alignObjectAbsolutely()
{
    emit callAlignAction(SelectionSettings::totalAlign);
}
