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
#include "timagebutton.h"

PenSettings::PenSettings(QWidget *parent) : QWidget(parent)
{
    layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    setInnerForm();
}

PenSettings::~PenSettings()
{
}

void PenSettings::setInnerForm()
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

    fpsLabel = new QLabel;

    QHBoxLayout *fpsLayout = new QHBoxLayout;
    fpsLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    fpsLayout->setMargin(0);
    fpsLayout->setSpacing(0);
    fpsLayout->addWidget(fpsLabel);

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

    QBoxLayout *listLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    listLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    QLabel *mouthsLabel = new QLabel(tr("Mouths") + ": ");
    mouthsLabel->setAlignment(Qt::AlignHCenter);

    mouthsList = new QListWidget;
    mouthsList->setContextMenuPolicy(Qt::CustomContextMenu);
    mouthsList->setViewMode(QListView::ListMode);
    mouthsList->setFlow(QListView::TopToBottom);
    mouthsList->setMovement(QListView::Static);
    mouthsList->setFixedHeight(68);

    listLayout->addWidget(mouthsLabel);
    listLayout->addWidget(mouthsList);

    QLabel *textLabel = new QLabel(tr("Text") + ": ");
    textLabel->setAlignment(Qt::AlignHCenter);

    textArea = new QTextEdit;
    textArea->setReadOnly(true);

    // phonemeLabel = new QLabel(tr("Current Phoneme") + ": " + phoneme);
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
    xLayout->addWidget(xLabel);
    xLayout->addWidget(xPosField);

    QBoxLayout *yLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    yLayout->setMargin(0);
    yLayout->setSpacing(0);
    yLayout->addWidget(yLabel);
    yLayout->addWidget(yPosField);

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
    innerLayout->addLayout(listLayout);
    innerLayout->addWidget(textLabel);
    innerLayout->addWidget(textArea);
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

void PenSettings::openLipSyncProperties(TupLipSync *lipsync)
{
    name = lipsync->getLipSyncName();
    initFrame = lipsync->getInitFrame();
    framesCount = lipsync->getFramesCount();

    lipSyncName->setText(name);
    fpsLabel->setText(tr("Lip-Sync FPS") + ": " + QString::number(lipsync->getFPS()));

    comboInit->setEnabled(true);
    comboInit->setValue(initFrame + 1);

    int endIndex = initFrame + framesCount;
    endingLabel->setText(tr("Ending at frame") + ": " + QString::number(endIndex));
    totalLabel->setText(tr("Frames Total") + ": " + QString::number(framesCount));

    disconnect(mouthsList, SIGNAL(currentRowChanged(int)), this, SLOT(setCurrentMouth(int)));
    mouthsList->clear();

    voices = lipsync->getVoices();
    int total = voices.size();
    if (total > 0) {
        for (int i=0; i < total; i++) {
             QListWidgetItem *item = new QListWidgetItem(mouthsList);
             item->setText(tr("mouth") + "_" + QString::number(i));
             item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        }

        TupVoice *voice = voices.at(0);
        textArea->setText(voice->text());

        connect(mouthsList, SIGNAL(currentRowChanged(int)), this, SLOT(setCurrentMouth(int)));
        mouthsList->setCurrentRow(0);
    }
}

void PenSettings::setCurrentMouth(int index) 
{
    QString tail = ":" + QString::number(index);
    QString id = "lipsync:" + name + tail;

    TupVoice *voice = voices.at(index);
    textArea->setText(voice->text());

    emit selectMouth(id, index);
}

void PenSettings::updateInitFrame(int index)
{
    int frame = index - 1;

    if (frame != initFrame) {
        initFrame = frame;
        emit initFrameHasChanged(frame);
    }
}

void PenSettings::updateInterfaceRecords()
{
    int endIndex = initFrame + framesCount;
    endingLabel->setText(tr("Ending at frame") + ": " + QString::number(endIndex));
}

void PenSettings::setPos(const QPointF &point) 
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

void PenSettings::setPhoneme(const QString &phoneme)
{
    phonemeLabel->setText(tr("Current Phoneme") + ": " + "<b>" + phoneme + "</b>");
}
