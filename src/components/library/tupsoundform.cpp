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

void TupSoundForm::setSoundParams(SoundResource params, QStringList scenesList)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundForm::setSoundParams()]";
    #endif

    soundParams = params;

    QList<SoundScene> scenes = params.scenes;
    for (int i=0; i<scenes.size(); i++)
        framesMaxList << 0;

    loadScenesCombo(scenesList);
}

void TupSoundForm::updateFrameLimits(int sceneIndex, int maxFrame)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundForm::updateFrameLimits()] - sceneIndex ->" << sceneIndex;
        qDebug() << "[TupSoundForm::updateFrameLimits()] - maxFrame ->" << maxFrame;
    #endif

    // setFramesLimit(currentSceneIndex, framesCount);
}

void TupSoundForm::setFramesLimit(int sceneIndex, int framesCount)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundForm::setFramesList()] - sceneIndex ->" << sceneIndex;
        qDebug() << "[TupSoundForm::setFramesList()] - framesCount ->" << framesCount;
    #endif

    /*
    if (project->sceneAt(sceneIndex)) {
        int framesMax = project->sceneAt(sceneIndex)->framesCount();
        qDebug() << "sceneIndex->" << sceneIndex;
        qDebug() << "framesMax ->" << framesMax;
        framesBox->setRange(1, framesMax);
        framesBox->setMaximum(framesMax);
    }
    */
}

void TupSoundForm::loadScenesCombo(QStringList scenes)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundForm::loadScenesCombo()]";
    #endif

    scenesCombo->blockSignals(true);

    scenesCombo->clear();
    scenesCombo->addItems(scenes);

    scenesCombo->addItem(bgTrackLabel);

    qDebug() << "soundParams.isBackgroundTrack ->" << soundParams.isBackgroundTrack;

    if (soundParams.isBackgroundTrack) {
        scenesCombo->setCurrentText(bgTrackLabel);
        if (framesListWidget->isVisible()) {
            framesListWidget->setVisible(false);
            buttonBar->setVisible(false);
        }
    } else {
        setFramesLimit(0, 1);

        if (!framesListWidget->isVisible()) {
            framesListWidget->setVisible(true);
            buttonBar->setVisible(true);
        }
    }

    scenesCombo->blockSignals(false);
}

void TupSoundForm::updateFramesList(int sceneIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundForm::updateFramesList()] - sceneIndex ->" << sceneIndex;
    #endif

    if (sceneIndex != currentSceneIndex) {
        currentSceneIndex = sceneIndex;

        QString backgroundTrack = scenesCombo->currentText();
        if (backgroundTrack.compare(bgTrackLabel) == 0) { // background track
            soundParams.isBackgroundTrack = true;
            framesListWidget->setVisible(false);
            buttonBar->setVisible(false);
        } else { // Adding frames for the scene
            if (soundParams.isBackgroundTrack)
                soundParams.isBackgroundTrack = false;

            framesListWidget->clear();
            if (!framesListWidget->isVisible()) {
                qDebug() << "*** Enabling frames list and button bar!";
                framesListWidget->setVisible(true);
                buttonBar->setVisible(true);
            }

            // setFramesLimit(sceneIndex, 1);

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
                    break;
                }
            }
        }
        emit soundResourceModified(soundParams);
    }
}
