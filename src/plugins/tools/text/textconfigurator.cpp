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

#include "textconfigurator.h"
#include "tapplicationproperties.h"
#include "tseparator.h"
#include "tupsvg2qt.h"

#include <QBoxLayout>
#include <QPushButton>

TextConfigurator::TextConfigurator(QWidget *parent) : QWidget(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextConfigurator()]";
    #endif

    mode = Add;
    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);

    QLabel *pencilTitle = new QLabel;
    pencilTitle->setAlignment(Qt::AlignHCenter);
    QPixmap pencilPic(THEME_DIR + "icons/text.png");
    pencilTitle->setPixmap(pencilPic.scaledToWidth(16, Qt::SmoothTransformation));
    pencilTitle->setToolTip(tr("Text Properties"));

    mainLayout->addWidget(pencilTitle);
    mainLayout->addWidget(new TSeparator(Qt::Horizontal));

    fontChooser = new TFontChooser;
    mainLayout->addWidget(fontChooser);

    QLabel *textLabel = new QLabel(tr("Text"));
    mainLayout->addWidget(textLabel);
    textBox = new QTextEdit(this);
    textBox->setAlignment(Qt::AlignLeft);
    mainLayout->addWidget(textBox);

    controlsWidget = createTransformationTools();
    controlsWidget->setVisible(false);

    mainLayout->addWidget(controlsWidget);
    mainLayout->addWidget(new TSeparator(Qt::Horizontal));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    addButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/plus_sign.png")), "");
    addButton->setToolTip(tr("Add Text"));
    addButton->setMaximumWidth(50);
    connect(addButton, SIGNAL(clicked()), this, SLOT(callAction()));

    resetButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/reset.png")), "");
    resetButton->setToolTip(tr("Reset Transformations"));
    resetButton->setMaximumWidth(50);
    connect(resetButton, SIGNAL(clicked()), this, SIGNAL(resetActionCalled()));

    clearButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/new.png")), "");
    clearButton->setToolTip(tr("Clear Text"));
    clearButton->setMaximumWidth(50);
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clearText()));

    buttonLayout->addStretch(1);
    buttonLayout->addWidget(new QWidget);
    buttonLayout->addWidget(addButton, Qt::AlignHCenter);
    buttonLayout->addWidget(resetButton, Qt::AlignHCenter);
    buttonLayout->addWidget(clearButton, Qt::AlignHCenter);
    buttonLayout->addWidget(new QWidget);
    buttonLayout->addStretch(1);

    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch(1);

    connect(fontChooser, SIGNAL(fontChanged()), this, SLOT(changeFont()));
    connect(fontChooser, SIGNAL(alignmentUpdated(Qt::Alignment)), this, SLOT(updateTextAlignment(Qt::Alignment)));
    changeFont();
}

TextConfigurator::~TextConfigurator()
{
}

QWidget * TextConfigurator::createTransformationTools()
{
    QWidget *widget = new QWidget;

    // Position section

    QLabel *textPosLabel = new QLabel;
    textPosLabel->setPixmap(QPixmap(kAppProp->themeDir() + "/icons/position.png"));
    textPosLabel->setToolTip(tr("Text Position"));
    textPosLabel->setAlignment(Qt::AlignVCenter);

    QVBoxLayout *textLayout = new QVBoxLayout;
    textLayout->addWidget(textPosLabel, Qt::AlignVCenter);

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

    QVBoxLayout *posLayout = new QVBoxLayout;
    posLayout->addLayout(xLayout);
    posLayout->addLayout(yLayout);

    QHBoxLayout *posBlockLayout = new QHBoxLayout;
    posBlockLayout->addStretch();
    posBlockLayout->addLayout(textLayout);
    posBlockLayout->addLayout(posLayout);
    posBlockLayout->addStretch();

    // Rotation section

    QLabel *rotationLabel = new QLabel;
    rotationLabel->setPixmap(QPixmap(kAppProp->themeDir() + "icons/rotation.png"));
    rotationLabel->setToolTip(tr("Text Rotation"));
    rotationLabel->setAlignment(Qt::AlignVCenter);

    angleField = new QSpinBox;
    angleField->setMinimum(0);
    angleField->setMaximum(360);
    connect(angleField, SIGNAL(valueChanged(int)), this, SIGNAL(rotationChanged(int)));

    QBoxLayout *rotationBlockLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    rotationBlockLayout->setMargin(0);
    rotationBlockLayout->setSpacing(10);
    rotationBlockLayout->addStretch();
    rotationBlockLayout->addWidget(rotationLabel);
    rotationBlockLayout->addWidget(angleField);
    rotationBlockLayout->addStretch();

    // Scale section

    QLabel *scaleLabel = new QLabel("<b>" + tr("Text Scale") + "</b>");
    scaleLabel->setAlignment(Qt::AlignHCenter);

    QBoxLayout *scaleLayout = new QBoxLayout(QBoxLayout::TopToBottom);

    QLabel *factorXLabel = new QLabel(tr("X") + ": ");
    factorXField = new QDoubleSpinBox;
    factorXField->setDecimals(2);
    factorXField->setMinimum(0.01);
    factorXField->setMaximum(10);
    factorXField->setSingleStep(0.01);
    factorXField->setValue(1);
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
    factorYField->setValue(1);
    connect(factorYField, SIGNAL(valueChanged(double)), this, SLOT(notifyYScale(double)));

    QBoxLayout *factorYLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    factorYLayout->setMargin(0);
    factorYLayout->setSpacing(0);
    factorYLayout->addStretch();
    factorYLayout->addWidget(factorYLabel);
    factorYLayout->addWidget(factorYField);
    factorYLayout->addStretch();

    scaleLayout->addLayout(factorYLayout);

    QLabel *scaleTextLabel = new QLabel;
    scaleTextLabel->setPixmap(QPixmap(kAppProp->themeDir() + "/icons/scale.png"));
    scaleTextLabel->setToolTip(tr("Text Scale"));
    scaleTextLabel->setAlignment(Qt::AlignVCenter);

    QVBoxLayout *scaleLabelLayout = new QVBoxLayout;
    scaleLabelLayout->addWidget(scaleTextLabel, Qt::AlignVCenter);

    propCheck = new QCheckBox(tr("Proportion"), this);
    connect(propCheck, SIGNAL(stateChanged(int)), this, SLOT(enableProportion(int)));

    scaleLayout->addWidget(propCheck);
    scaleLayout->setAlignment(propCheck, Qt::AlignHCenter);

    QHBoxLayout *scaleBlockLayout = new QHBoxLayout;
    scaleBlockLayout->addStretch();
    scaleBlockLayout->addLayout(scaleLabelLayout);
    scaleBlockLayout->addLayout(scaleLayout);

    scaleBlockLayout->addStretch();

    QHBoxLayout *transBlockLayout = new QHBoxLayout;
    transBlockLayout->addStretch();
    transBlockLayout->addLayout(posBlockLayout);
    transBlockLayout->addWidget(new TSeparator(Qt::Vertical));
    transBlockLayout->addLayout(rotationBlockLayout);
    transBlockLayout->addWidget(new TSeparator(Qt::Vertical));
    transBlockLayout->addLayout(scaleBlockLayout);
    transBlockLayout->addStretch();

    widget->setLayout(transBlockLayout);

    return widget;
}

QString TextConfigurator::text() const
{
    return textBox->toPlainText();
}

QFont TextConfigurator::textFont() const
{
    return font;
}

void TextConfigurator::changeFont()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextConfigurator::changeFont()]";
    #endif

    font = fontChooser->currentFont();
    textBox->setFont(font);
    adjustSize();
}

void TextConfigurator::loadTextSettings(const QFont &itemFont, const QString &text, const QColor &color)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextConfigurator::loadTextSettings()] - text -> " << text;
        qDebug() << "[TextConfigurator::loadTextSettings()] - color -> " << color.name(QColor::HexArgb);
    #endif

    font = itemFont;
    textBox->setFont(itemFont);
    textBox->setTextColor(color);
    textBox->setPlainText(text);

    fontChooser->updateFontSettings(itemFont);
    updateMode(Update);
}

void TextConfigurator::callAction()
{
    if (mode == Add)
        emit textAdded();
    else
        emit textUpdated();
}

void TextConfigurator::clearText()
{
    textBox->clear();
    displayControls(false);
}

void TextConfigurator::updateMode(Mode action)
{
    if (action != mode) {
        mode = action;

        QString tip = tr("Update Text");
        QString icon = "edit_sign.png";
        if (mode == Add) {
            tip = tr("Add Text");
            icon = "plus_sign.png";
        }

        addButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/" + icon)));
        addButton->setToolTip(tip);
    }
}

void TextConfigurator::updateTextAlignment(Qt::Alignment value)
{
    textBox->selectAll();
    textBox->setAlignment(value);
    textAlignmentValue = value;

    QTextCursor cursor = textBox->textCursor();
    cursor.movePosition(QTextCursor::End);
    textBox->setTextCursor(cursor);
}

Qt::Alignment TextConfigurator::textAlignment()
{
    return textAlignmentValue;
}

void TextConfigurator::resetText()
{
    textBox->clear();
    updateMode(Add);
}

void TextConfigurator::setTextColor(const QColor &color)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextConfigurator::setTextColor()] - color -> " << color.name(QColor::HexArgb);
    #endif

    QString text = textBox->toPlainText();
    textBox->clear();
    textBox->setTextColor(color);
    textBox->setText(text);
}

QColor TextConfigurator::getTextColor() const
{
    return textBox->textColor();
}

void TextConfigurator::displayControls(bool flag, const QPointF &pos, const QDomElement &e)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextConfigurator::displayControls()] - flag -> " << flag;
    #endif

    controlsWidget->setVisible(flag);
    resetButton->setVisible(flag);
    if (flag) {
        updatePositionCoords(pos.x(), pos.y());
        updateRotationAngle(e.attribute("rotation").toInt());
        updateScaleFactor(e.attribute("scale_x").toDouble(), e.attribute("scale_y").toDouble());
    } else {
        emit textObjectReleased();
    }
}

void TextConfigurator::updatePositionCoords(int x, int y)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextConfigurator::updatePositionCoords()] - x -> " << x;
        qDebug() << "[TextConfigurator::updatePositionCoords()] - y -> " << y;
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

void TextConfigurator::updateRotationAngle(int angle)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextConfigurator::updateRotationAngle()] - angle -> " << angle;
    #endif

    angleField->blockSignals(true);

    if (angle > 359)
        angle = 0;
    angleField->setValue(angle);

    angleField->blockSignals(false);
}

void TextConfigurator::updateScaleFactor(double x, double y)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextConfigurator::updateScaleFactor()] - x -> " << x;
        qDebug() << "[TextConfigurator::updateScaleFactor()] - y -> " << y;
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

void TextConfigurator::notifyRotation(int angle)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextConfigurator::notifyRotation()] - angle -> " << angle;
    #endif

    if (angle == 360) {
        angle = 0;
        angleField->setValue(0);
    }
    emit rotationChanged(angle);
}

void TextConfigurator::notifyXScale(double factor)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextConfigurator::notifyXScale()] - factor -> " << factor;
    #endif

    if (propCheck->isChecked()) {
        currentYFactor = factor;
        factorYField->setValue(factor);
    }

    emit scaleChanged(factor, currentYFactor);
    currentXFactor = factor;
}

void TextConfigurator::notifyYScale(double factor)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextConfigurator::notifyYScale()] - factor -> " << factor;
    #endif

    if (propCheck->isChecked()) {
       currentXFactor = factor;
       factorXField->setValue(factor);
    }

    emit scaleChanged(currentXFactor, factor);
    currentYFactor = factor;
}

void TextConfigurator::enableProportion(int flag)
{
    bool enable = false;
    if (flag == Qt::Checked) {
        double factor = factorXField->value();
        factorYField->setValue(factor);
        emit scaleUpdated(factor, factor);
        enable = true;
    }

    emit activateProportion(enable);
}

void TextConfigurator::setProportionState(int flag)
{
    propCheck->blockSignals(true);
    propCheck->setChecked(flag);
    if (flag)
        factorYField->setValue(factorXField->value());
    propCheck->blockSignals(false);
}
