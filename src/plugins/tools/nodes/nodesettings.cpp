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

#include <QBoxLayout>
// #include <QPushButton>

NodeSettings::NodeSettings(QWidget *parent) : QWidget(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeSettings()]";
    #endif

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
    layout->setAlignment(Qt::AlignHCenter);

    QLabel *nodesTitle = new QLabel;
    nodesTitle->setAlignment(Qt::AlignHCenter);
    QPixmap nodesPic(THEME_DIR + "icons/nodes.png");
    nodesTitle->setPixmap(nodesPic.scaledToWidth(16, Qt::SmoothTransformation));
    nodesTitle->setToolTip(tr("Nodes Properties"));

    layout->addWidget(nodesTitle);
    layout->addWidget(new TSeparator(Qt::Horizontal));

    QLabel *clearLabel = new QLabel(tr("Nodes Cleaner"));
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
    policyCombo->addItem(tr("Last Node"));
    policyCombo->setCurrentIndex(1);
    policy = MiddleNode;
    connect(policyCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePolicyParam(int)));

    layout->addWidget(clearLabel);
    layout->addLayout(controlLayout);
    layout->addWidget(policyLabel);
    layout->addWidget(policyCombo);

    mainLayout->addLayout(layout);
    mainLayout->addStretch(2);
}

NodeSettings::~NodeSettings()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~NodeSettings()]";
    #endif
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

    clearSlider->setMaximum(value);
    updateNodesFromBox(value);
    updateNodesFromSlider(value);
}

void NodeSettings::undo()
{
    if (!undoValues.isEmpty()) {
        int value = undoValues.takeLast();
        redoValues << clearSpinBox->value();

        clearSpinBox->blockSignals(true);
        clearSpinBox->setValue(value);
        clearSpinBox->blockSignals(false);

        clearSlider->blockSignals(true);
        clearSlider->setValue(value);
        clearSlider->blockSignals(false);
    }
}

void NodeSettings::redo()
{
    if (!redoValues.isEmpty()) {
        int value = redoValues.takeLast();
        undoValues << clearSpinBox->value();

        clearSpinBox->blockSignals(true);
        clearSpinBox->setValue(value);
        clearSpinBox->blockSignals(false);

        clearSlider->blockSignals(true);
        clearSlider->setValue(value);
        clearSlider->blockSignals(false);
    }
}

void NodeSettings::updatePolicyParam(int index)
{
    policy = NodePosition(index);
}

NodePosition NodeSettings::policyParam()
{
    return policy;
}
