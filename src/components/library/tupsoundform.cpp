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
#include "tupscene.h"

#include <QBoxLayout>

TupSoundForm::TupSoundForm(QWidget *parent) : QWidget(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundForm()]";
    #endif

    currentSceneIndex = 0;
    bgTrackLabel = tr("All Scenes (Background Track)");

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

    buttonBar = new QWidget();

    framesBox = new QSpinBox();
    framesBox->setSingleStep(1);
    framesBox->setValue(1);
    framesBox->setToolTip(tr("Add Frame"));

    addFrameButton = new TImageButton(QPixmap(THEME_DIR + "icons/plus_sign.png"), 22, this);
    addFrameButton->setToolTip(tr("Add Frame"));
    removeFrameButton = new TImageButton(QPixmap(THEME_DIR + "icons/minus_sign.png"), 22, this);
    removeFrameButton->setToolTip(tr("Remove Frame"));

    QHBoxLayout *framesControlLayout = new QHBoxLayout(buttonBar);
    framesControlLayout->addStretch();
    framesControlLayout->addWidget(framesBox);
    framesControlLayout->addWidget(addFrameButton);
    framesControlLayout->addWidget(new TSeparator(Qt::Vertical));
    framesControlLayout->addWidget(removeFrameButton);
    framesControlLayout->addStretch();
    framesControlLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *blockLayout = new QVBoxLayout(this);
    blockLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    blockLayout->addWidget(playAtLabel);
    blockLayout->addLayout(scenesLayout);
    blockLayout->addLayout(framesLayout);
    blockLayout->addWidget(buttonBar);
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

void TupSoundForm::setSoundParams(SoundResource params, QStringList scenesList, QList<int> frameLimits)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundForm::setSoundParams()]";
    #endif

    soundParams = params;

    QList<SoundScene> scenes = params.scenes;
    for (int i=0; i<scenes.size(); i++)
        framesMaxList << frameLimits.at(i);

    loadScenesCombo(scenesList, true);
}

void TupSoundForm::updateFrameLimit(int sceneIndex, int maxFrame)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundForm::updateFrameLimit()] - sceneIndex ->" << sceneIndex;
        qDebug() << "[TupSoundForm::updateFrameLimit()] - maxFrame ->" << maxFrame;
    #endif

    framesMaxList.replace(sceneIndex, maxFrame);
    if (currentSceneIndex == sceneIndex)
        setFramesLimit(sceneIndex, maxFrame);
}

void TupSoundForm::setFramesLimit(int sceneIndex, int maxFrame)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundForm::setFramesList()] - sceneIndex ->" << sceneIndex;
        qDebug() << "[TupSoundForm::setFramesList()] - maxFrame ->" << maxFrame;
    #endif

    framesBox->setRange(1, maxFrame);
    framesBox->setMaximum(maxFrame);
}

void TupSoundForm::loadScenesCombo(QStringList scenes, bool firstTime)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundForm::loadScenesCombo()] - scenes ->" << scenes;
    #endif

    scenesCombo->blockSignals(true);

    scenesCombo->clear();
    scenesCombo->addItems(scenes);
    scenesCombo->addItem(bgTrackLabel);

    if (soundParams.isBackgroundTrack) {
        scenesCombo->setCurrentText(bgTrackLabel);
        if (framesListWidget->isVisible()) {
            framesListWidget->setVisible(false);
            buttonBar->setVisible(false);
        }
    } else {
        if (!framesListWidget->isVisible()) {
            framesListWidget->setVisible(true);
            buttonBar->setVisible(true);
        }

        /*
        if (!firstTime)
            updateFramesList(currentSceneIndex);
        else
            // load frames list for first time here
        */
    }

    scenesCombo->blockSignals(false);
}

void TupSoundForm::updateFramesList(int sceneIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundForm::updateFramesList()] - sceneIndex ->" << sceneIndex;
        qDebug() << "[TupSoundForm::updateFramesList()] - currentSceneIndex ->" << currentSceneIndex;
    #endif

    if (sceneIndex != currentSceneIndex) {
        currentSceneIndex = sceneIndex;

        QString backgroundTrack = scenesCombo->currentText();
        if (backgroundTrack.compare(bgTrackLabel) == 0) { // background track
            qDebug() << "[TupSoundForm::updateFramesList()] - FLAG -1";
            soundParams.isBackgroundTrack = true;
            framesListWidget->setVisible(false);
            buttonBar->setVisible(false);
        } else { // Adding frames for the scene
            qDebug() << "[TupSoundForm::updateFramesList()] - FLAG 0";

            if (soundParams.isBackgroundTrack)
                soundParams.isBackgroundTrack = false;

            framesListWidget->clear();
            if (!framesListWidget->isVisible()) {
                framesListWidget->setVisible(true);
                buttonBar->setVisible(true);
            }

            setFramesLimit(sceneIndex, framesMaxList.at(sceneIndex));

            qDebug() << "[TupSoundForm::updateFramesList()] - FLAG 1";

            QList<SoundScene> scenes = soundParams.scenes;
            int scenesTotal = scenes.size();
            qDebug() << "[TupSoundForm::updateFramesList()] - scenes total ->" << scenesTotal;
            for(int i=0; i<scenesTotal; i++) {
                SoundScene scene = scenes.at(i);
                if (scene.sceneIndex == sceneIndex) {
                    QList<int> frames = scene.frames;
                    qDebug() << "[TupSoundForm::updateFramesList()] - frames ->" << frames;
                    int framesTotal = frames.size();
                    for(int j=0; j<framesTotal; j++) {
                        QString frameIndex = QString::number(frames.at(j));
                        framesListWidget->addItem(frameIndex);
                    }
                    break;
                }
            }
        }
        emit soundResourceModified(soundParams);
    }
}
