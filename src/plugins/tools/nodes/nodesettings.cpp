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

#include "nodesettings.h"
#include "tconfig.h"
#include "tseparator.h"
#include "talgorithm.h"
#include "tresponsiveui.h"

#include <QBoxLayout>
#include <QLabel>

NodeSettings::NodeSettings(QWidget *parent) : QWidget(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeSettings()]";
    #endif

    QPair<int, int> dimension = TAlgorithm::screenDimension();
    screenHeight = dimension.second;
    int helpHeight = 0;
    if (screenHeight >= HD_HEIGHT)
        helpHeight = (screenHeight * 32)/100;
    else
        helpHeight = (screenHeight * 53)/100;

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    clearWidget = new QWidget;
    QBoxLayout *clearLayout = new QBoxLayout(QBoxLayout::TopToBottom, clearWidget);
    clearLayout->setAlignment(Qt::AlignHCenter);

    QLabel *nodesTitle = new QLabel;
    nodesTitle->setAlignment(Qt::AlignHCenter);
    QPixmap nodesPic(THEME_DIR + "icons/nodes.png");
    nodesTitle->setPixmap(nodesPic.scaledToWidth(TResponsiveUI::fitTitleIconSize(), Qt::SmoothTransformation));
    nodesTitle->setToolTip(tr("Nodes Properties"));

    QLabel *clearLabel = new QLabel("<b>" + tr("Nodes Editor") + "</b>");
    clearLabel->setAlignment(Qt::AlignHCenter);

    QHBoxLayout *controlLayout = new QHBoxLayout;

    clearSpinBox = new QSpinBox(this);
    clearSpinBox->setMinimum(2);
    clearSpinBox->setMaximum(100);
    clearSpinBox->setSingleStep(1);
    clearSpinBox->setValue(2);
    connect(clearSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateNodesFromBox(int)));
    connect(clearSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(nodesChanged(int)));
    controlLayout->addWidget(clearSpinBox);

    clearSlider = new QSlider(Qt::Horizontal);
    clearSlider->setMinimum(2);
    clearSlider->setMaximum(100);
    clearSlider->setValue(2);
    connect(clearSlider, SIGNAL(valueChanged(int)), this, SLOT(updateNodesFromSlider(int)));
    connect(clearSlider, SIGNAL(valueChanged(int)), this, SIGNAL(nodesChanged(int)));
    controlLayout->addWidget(clearSlider);

    QLabel *policyLabel = new QLabel("Remove:");
    policyLabel->setAlignment(Qt::AlignHCenter);
    policyCombo = new QComboBox(this);
    policyCombo->addItem(tr("First Node"));
    policyCombo->addItem(tr("Middle Node"));
    policyCombo->addItem(tr("Random Node"));
    policyCombo->addItem(tr("Last Node"));
    policyCombo->setCurrentIndex(1);
    policy = MiddleNode;
    connect(policyCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePolicyParam(int)));

    clearLayout->addWidget(clearLabel);
    clearLayout->addLayout(controlLayout);
    clearLayout->addWidget(policyLabel);
    clearLayout->addWidget(policyCombo);
    clearWidget->setVisible(false);

    QFont font = this->font();
    font.setPointSize(8);

    tips = new QPushButton(tr("Hide Tips"));
    if (screenHeight < HD_HEIGHT)
        tips->setFont(font);
    tips->setToolTip(tr("A little help for the Nodes tool"));

    QBoxLayout *smallLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    smallLayout->addWidget(tips);
    connect(tips, SIGNAL(clicked()), this, SLOT(openTipPanel()));

    int minWidth = TResponsiveUI::fitRightPanelWidth();
    helpComponent = new QTextEdit;
    if (screenHeight < HD_HEIGHT)
        helpComponent->setFont(font);
    helpComponent->setMinimumWidth(minWidth);
    helpComponent->setMaximumWidth(minWidth*2);
    helpComponent->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    helpComponent->append("<p><b>" + tr("Ctrl Key + Left Mouse Button") + ":</b> "
                     + tr("Append a line segment to the last node of the path or add a new line node between two nodes") + "</p>");
    helpComponent->append("<p><b>" + tr("Shift Key + Left Mouse Button") + ":</b> "
                     + tr("Append a curve to the last node of the path or add a new curve between two nodes") + "</p>");
    helpComponent->append("<p><b>" + tr("X Key") + ":</b> " +  tr("Remove selected node") + "</p>");
    helpComponent->append("<p><b>" + tr("M Key") + ":</b> " +  tr("Switch selected node to line/curve") + "</p>");

    helpComponent->setFixedHeight(helpHeight);

    mainLayout->addWidget(nodesTitle);
    mainLayout->addWidget(new TSeparator(Qt::Horizontal));
    mainLayout->addWidget(clearWidget);
    mainLayout->addLayout(smallLayout);
    mainLayout->addWidget(helpComponent);
    mainLayout->addStretch(2);
}

NodeSettings::~NodeSettings()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~NodeSettings()]";
    #endif
}

void NodeSettings::showClearPanel(bool show)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeSettings::showClearPanel()] - show ->" << show;
    #endif

    int percent;
    if (!show) {
        clearSlider->blockSignals(true);
        clearSpinBox->blockSignals(true);

        clearSlider->setMaximum(0);
        clearSlider->setValue(0);
        clearSpinBox->setValue(0);

        clearSlider->blockSignals(false);
        clearSpinBox->blockSignals(false);

        if (screenHeight < HD_HEIGHT)
            percent = 53;
    } else {
        if (screenHeight < HD_HEIGHT)
            percent = 38;
    }

    int helpHeight = (screenHeight * percent)/100;
    helpComponent->setFixedHeight(helpHeight);

    clearWidget->setVisible(show);
}

void NodeSettings::updateNodesFromBox(int value)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeSettings::updateNodesFromBox()] - value ->" << value;
    #endif

    clearSlider->blockSignals(true);
    undoValues << clearSlider->value();
    clearSlider->setValue(value);
    clearSlider->blockSignals(false);
}

void NodeSettings::updateNodesFromSlider(int value)
{
    #ifdef TUP_DEBUG
            qDebug() << "[NodeSettings::updateNodesFromSlider()] - value ->" << value;
    #endif

    clearSpinBox->blockSignals(true);
    undoValues << clearSpinBox->value();
    clearSpinBox->setValue(value);
    clearSpinBox->blockSignals(false);
}

void NodeSettings::setNodesTotal(int value)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeSettings::setNodesTotal()] - value ->" << value;
    #endif

    if (!clearWidget->isVisible())
        showClearPanel(true);

    clearSpinBox->blockSignals(true);
    clearSpinBox->setMinimum(2);
    clearSpinBox->setMaximum(value);
    clearSpinBox->setValue(value);
    clearSpinBox->blockSignals(false);

    clearSlider->blockSignals(true);
    clearSlider->setMinimum(2);
    clearSlider->setMaximum(value);
    clearSlider->setValue(value);
    clearSlider->blockSignals(false);

    undoValues << value;
}

void NodeSettings::undo()
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeSettings::undo()]";
    #endif

    if (!undoValues.isEmpty()) {
        int value = undoValues.takeLast();
        redoValues << clearSpinBox->value();

        clearSpinBox->blockSignals(true);
        clearSpinBox->setValue(value);
        clearSpinBox->blockSignals(false);

        clearSlider->blockSignals(true);
        clearSlider->setValue(value);
        clearSlider->blockSignals(false);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[NodeSettings::undo()] - Undo list is empty!";
        #endif
    }
}

void NodeSettings::redo()
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeSettings::redo()]";
    #endif

    if (!redoValues.isEmpty()) {
        int value = redoValues.takeLast();
        undoValues << clearSpinBox->value();

        clearSpinBox->blockSignals(true);
        clearSpinBox->setValue(value);
        clearSpinBox->blockSignals(false);

        clearSlider->blockSignals(true);
        clearSlider->setValue(value);
        clearSlider->blockSignals(false);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[NodeSettings::redo()] - Redo list is empty!";
        #endif
    }
}

void NodeSettings::updatePolicyParam(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeSettings::updatePolicyParam()] - index ->" << index;
    #endif

    policy = NodeLocation(index);
    int value = clearSpinBox->value();
    clearSpinBox->setMinimum(2);
    clearSpinBox->setMaximum(value);

    clearSlider->blockSignals(true);
    clearSlider->setMinimum(2);
    clearSlider->setMaximum(value);
    clearSlider->setValue(value);
    clearSlider->blockSignals(false);

    emit policyChanged();
}

NodeLocation NodeSettings::policyParam()
{
    return policy;
}

void NodeSettings::openTipPanel()
{
    if (helpComponent->isVisible()) {
        tips->setText(tr("Show Tips"));
        helpComponent->hide();
    } else {
        if (screenHeight < HD_HEIGHT) { 
            int percent = 53;
            if (clearWidget->isVisible())
                percent = 38;

            int helpHeight = (screenHeight * percent)/100;
            helpComponent->setFixedHeight(helpHeight);
        }

        tips->setText(tr("Hide Tips"));
        helpComponent->show();
    }
}

void NodeSettings::resetHistory()
{
    undoValues.clear();
    redoValues.clear();
}
