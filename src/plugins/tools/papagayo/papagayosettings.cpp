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

    QLabel *mouthPosLabel = new QLabel(tr("Current Mouth Position") + ": ");
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

    TImageButton *remove = new TImageButton(QPixmap(kAppProp->themeDir() + "icons/close_properties.png"), 22);
    remove->setToolTip(tr("Close properties"));
    connect(remove, SIGNAL(clicked()), this, SIGNAL(closeLipSyncProperties()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    buttonsLayout->setMargin(0);
    buttonsLayout->setSpacing(10);
    buttonsLayout->addWidget(remove);

    innerLayout->addLayout(nameLayout);
    innerLayout->addLayout(fpsLayout);
    innerLayout->addLayout(startLayout);
    innerLayout->addLayout(endLayout);
    innerLayout->addLayout(totalLayout);
    innerLayout->addWidget(phonemeLabel);
    innerLayout->addWidget(mouthPosLabel);
    innerLayout->addLayout(xLayout);
    innerLayout->addLayout(yLayout);

    innerLayout->addSpacing(10);
    innerLayout->addLayout(buttonsLayout);
    innerLayout->addSpacing(5);

    layout->addWidget(innerPanel);
}

// Editing new LipSync 

void PapagayoSettings::openLipSyncProperties(TupLipSync *lipsync)
{
    name = lipsync->getLipSyncName();
    initFrame = lipsync->getInitFrame();
    framesCount = lipsync->getFramesCount();

    lipSyncName->setText("<b>" + name + "</b>");

    comboInit->setEnabled(true);
    comboInit->setValue(initFrame + 1);

    int endIndex = initFrame + framesCount;
    endingLabel->setText(tr("Ending at frame") + ": <b>" + QString::number(endIndex) + "</b>");
    totalLabel->setText(tr("Frames Total") + ": <b>" + QString::number(framesCount) + "</b>");

    voices = lipsync->getVoices();
}

void PapagayoSettings::updateInitFrame(int index)
{
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

void PapagayoSettings::setPos(const QPointF &point) 
{
    qreal x = point.x();
    qreal y = point.y();

    xPosField->blockSignals(true);
    yPosField->blockSignals(true);

    xPosField->setValue(x);
    yPosField->setValue(y);

    xPosField->blockSignals(false);
    yPosField->blockSignals(false);
}

void PapagayoSettings::setPhoneme(const QString &phoneme)
{
    phonemeLabel->setText(tr("Current Phoneme") + ": " + "<b>" + phoneme + "</b>");
}
