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

#include "tupsoundform.h"
#include "tseparator.h"
#include "talgorithm.h"

TupSoundForm::TupSoundForm(QWidget *parent) : QWidget(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundForm()]";
    #endif

    currentSceneIndex = 0;

    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenHeight = dimension.second;

    playAtLabel = new QLabel(tr("Play audio at:"));
    playAtLabel->setAlignment(Qt::AlignHCenter);

    scenesCombo = new QComboBox();
    connect(scenesCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateFramesList(int)));

    QHBoxLayout *scenesLayout = new QHBoxLayout;
    scenesLayout->addWidget(scenesCombo, Qt::AlignHCenter);

    framesListWidget = new QListWidget;
    framesListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    framesListWidget->setViewMode(QListView::ListMode);
    framesListWidget->setFlow(QListView::TopToBottom);
    framesListWidget->setMovement(QListView::Static);
    framesListWidget->setFixedHeight((screenHeight * 6)/100);

    QHBoxLayout *framesLayout = new QHBoxLayout;
    framesLayout->addWidget(framesListWidget, Qt::AlignHCenter);

    framesBox = new QSpinBox();
    framesBox->setRange(1, 100);
    framesBox->setSingleStep(1);
    framesBox->setValue(1);
    framesBox->setToolTip(tr("Add Frame"));

    addFrameButton = new TImageButton(QPixmap(THEME_DIR + "icons/plus_sign.png"), 22, this);
    addFrameButton->setToolTip(tr("Add Frame"));
    // QLabel *frameLabel = new QLabel(tr("Add Frame"));
    removeFrameButton = new TImageButton(QPixmap(THEME_DIR + "icons/minus_sign.png"), 22, this);
    removeFrameButton->setToolTip(tr("Remove Frame"));

    QHBoxLayout *framesControlLayout = new QHBoxLayout;
    framesControlLayout->addStretch();
    framesControlLayout->addWidget(framesBox);
    framesControlLayout->addWidget(addFrameButton);
    // framesControlLayout->addWidget(frameLabel);
    framesControlLayout->addWidget(new TSeparator(Qt::Vertical));
    framesControlLayout->addWidget(removeFrameButton);
    framesControlLayout->addStretch();
    framesControlLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *blockLayout = new QVBoxLayout(this);
    blockLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    blockLayout->addWidget(playAtLabel);
    blockLayout->addLayout(scenesLayout);
    blockLayout->addLayout(framesLayout);
    blockLayout->addLayout(framesControlLayout);
    // blockLayout->addStretch();

    /*
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->addLayout(blockLayout);
    */
}

TupSoundForm::~TupSoundForm()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupSoundForm()]";
    #endif
}

QSize TupSoundForm::sizeHint() const
{
    return QWidget::sizeHint().expandedTo(QSize(100, 100));
}

void TupSoundForm::setSoundParams(QStringList scenes, SoundResource params)
{
    soundParams = params;
    loadScenesCombo(scenes);
    // updateFramesList(int sceneInde);
}

void TupSoundForm::loadScenesCombo(QStringList scenes)
{
    foreach(QString scene, scenes)
        scenesCombo->addItem(scene);

    scenesCombo->addItem(tr("All Scenes (Background Track)"));
}

void TupSoundForm::updateFramesList(int sceneIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundForm::updateFramesList()] - sceneIndex ->" << sceneIndex;
    #endif

    if (sceneIndex != currentSceneIndex) {
        currentSceneIndex = sceneIndex;
        framesListWidget->clear();
        QList<SoundScene> scenes = soundParams.scenes;
        int scenesTotal = scenes.size();
        for(int i=0; i<scenesTotal; i++) {
            SoundScene scene = scenes.at(i);
            int soundSceneIndex = scene.sceneIndex;
            if (soundSceneIndex == sceneIndex) {
                QList<int> frames = scene.frames;
                int framesTotal = frames.size();
                for(int j=0; j<framesTotal; j++) {
                    QString frameIndex = QString::number(frames.at(j));
                    framesListWidget->addItem(frameIndex);
                }
                return;
            }
        }
    }
}
