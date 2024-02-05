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
    bgTrackEnabled = false;

    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenHeight = dimension.second;

    playAtLabel = new QLabel(tr("Play audio at:"));
    playAtLabel->setAlignment(Qt::AlignHCenter);

    scenesCombo = new QComboBox();
    connect(scenesCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateFramesList(int)));

    QLabel *framesLabel = new QLabel(tr("At Frames:"));
    framesLabel->setAlignment(Qt::AlignHCenter);

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
    connect(addFrameButton, SIGNAL(clicked()), this, SLOT(addFrame()));

    removeFrameButton = new TImageButton(QPixmap(THEME_DIR + "icons/minus_sign.png"), 22, this);
    removeFrameButton->setToolTip(tr("Remove Frame"));
    connect(removeFrameButton, SIGNAL(clicked()), this, SLOT(removeFrame()));

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
    blockLayout->addWidget(framesLabel);
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
        qDebug() << "[TupSoundForm::setSoundParams()] - scenesList ->" << scenesList;
        qDebug() << "[TupSoundForm::setSoundParams()] - frameLimits ->" << frameLimits;
    #endif

    if (!params.scenes.isEmpty()) {
        SoundScene scene = params.scenes.first();
        currentSceneIndex = scene.sceneIndex;
    }

    soundParams = params;
    if (!frameLimits.isEmpty())
        framesMaxList = QList<int>(frameLimits);

    if (!scenesList.isEmpty())
        loadScenesCombo(scenesList);
}

void TupSoundForm::loadScenesCombo(QStringList scenes)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundForm::loadScenesCombo()] - scenes ->" << scenes;
        qDebug() << "[TupSoundForm::loadScenesCombo()] - scenes.count() ->" << scenes.count();
    #endif

    scenesCombo->blockSignals(true);

    scenesCombo->clear();
    scenesCombo->addItems(scenes);
    scenesCombo->addItem(bgTrackLabel);

    if (soundParams.isBackgroundTrack) {
        scenesCombo->setCurrentText(bgTrackLabel);
        bgTrackEnabled = true;
        if (framesListWidget->isVisible()) {
            framesListWidget->setVisible(false);
            buttonBar->setVisible(false);
        }
    } else { // Scene Sound
        if (!framesListWidget->isVisible()) {
            framesListWidget->setVisible(true);
            buttonBar->setVisible(true);
        }

        scenesCombo->setCurrentIndex(currentSceneIndex);
        populateFramesList(currentSceneIndex);
    }

    scenesCombo->blockSignals(false);
}

void TupSoundForm::updateFrameLimit(int sceneIndex, int maxFrame)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundForm::updateFrameLimit()] - sceneIndex ->" << sceneIndex;
        qDebug() << "[TupSoundForm::updateFrameLimit()] - maxFrame ->" << maxFrame;
    #else
        Q_UNUSED(sceneIndex)
    #endif

    if (sceneIndex < framesMaxList.size()) {
        framesMaxList.replace(sceneIndex, maxFrame);
        if (currentSceneIndex == sceneIndex)
            setFramesLimit(sceneIndex, maxFrame);
    } else {
        if (sceneIndex == framesMaxList.size())
            framesMaxList << maxFrame;
    }
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

void TupSoundForm::updateFramesList(int sceneIndex)
{
    populateFramesList(sceneIndex);

    QString comboLabel = scenesCombo->currentText();
    SoundScene scene;
    QList<int> frames;
    frames << 1;

    if (comboLabel.compare(bgTrackLabel) == 0) {
        bgTrackEnabled = true;
        soundParams.isBackgroundTrack = true;
        soundParams.scenes.clear();

        scene.sceneIndex = 0;
        scene.frames = frames;
        soundParams.scenes << scene;

        emit soundResourceModified(soundParams);
    } else if (bgTrackEnabled) {
        bgTrackEnabled = false;
        soundParams.isBackgroundTrack = false;

        scene.sceneIndex = scenesCombo->currentIndex();
        scene.frames = frames;
        soundParams.scenes << scene;

        emit soundResourceModified(soundParams);
    }
}

void TupSoundForm::populateFramesList(int sceneIndex)
{
    QString comboLabel = scenesCombo->currentText();
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundForm::populateFramesList()] - sceneIndex ->" << sceneIndex;
        qDebug() << "[TupSoundForm::populateFramesList()] - currentSceneIndex ->" << currentSceneIndex;
        qDebug() << "[TupSoundForm::populateFramesList()] - scene combo label ->" << comboLabel;
    #endif

    currentSceneIndex = sceneIndex;

    if (comboLabel.compare(bgTrackLabel) == 0) { // background track
        soundParams.isBackgroundTrack = true;
        soundParams.scenes.clear();
        SoundScene scene;
        scene.sceneIndex = 0;
        QList<int> frames;
        frames << 1;
        scene.frames = frames;
        soundParams.scenes << scene;

        framesListWidget->setVisible(false);
        buttonBar->setVisible(false);
    } else { // Adding frames for the scene
        if (soundParams.isBackgroundTrack)
            soundParams.isBackgroundTrack = false;

        framesListWidget->clear();
        if (!framesListWidget->isVisible()) {
            framesListWidget->setVisible(true);
            buttonBar->setVisible(true);
        }

        if ((sceneIndex >= 0) && (sceneIndex < framesMaxList.size()))
            setFramesLimit(sceneIndex, framesMaxList.at(sceneIndex));

        QList<SoundScene> scenes = soundParams.scenes;
        int scenesTotal = scenes.size();
        for(int i=0; i<scenesTotal; i++) {
            SoundScene scene = scenes.at(i);
            if (scene.sceneIndex == sceneIndex) {
                QList<int> frames = scene.frames;
                std::sort(frames.begin(), frames.end());
                int framesTotal = frames.size();
                for(int j=0; j<framesTotal; j++) {
                    QString frameIndex = QString::number(frames.at(j));
                    framesListWidget->addItem(frameIndex);
                }
                break;
            }
        }
    }
}

void TupSoundForm::addFrame()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundForm::addFrame()]";
    #endif

    int frameIndex = framesBox->value();
    QString frame = QString::number(frameIndex);
    QList<QListWidgetItem *> items = framesListWidget->findItems(frame, Qt::MatchExactly);
    if (items.isEmpty()) {
        int sceneIndex = scenesCombo->currentIndex();
        bool found = false;
        QList<int> framesList;
        foreach(SoundScene scene, soundParams.scenes) {
            if (scene.sceneIndex == sceneIndex) {
                scene.frames << frameIndex;
                std::sort(scene.frames.begin(), scene.frames.end());
                framesList = scene.frames;
                soundParams.scenes.replace(sceneIndex, scene);
                found = true;
                break;
            }
        }

        if (!found) {
            SoundScene scene;
            scene.sceneIndex = sceneIndex;
            QList<int> frames;
            frames << frameIndex;
            scene.frames = frames;
            framesList = scene.frames;
            soundParams.scenes.append(scene);
        }

        QStringList strFrames;
        foreach(int frame, framesList)
            strFrames << QString::number(frame);
        framesListWidget->clear();
        framesListWidget->addItems(strFrames);

        emit soundResourceModified(soundParams);
    }
}

void TupSoundForm::removeFrame()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundForm::removeFrame()]";
    #endif

    QListWidgetItem *item = framesListWidget->currentItem();
    if (item) {
        int sceneIndex = scenesCombo->currentIndex();
        int i = 0;
        foreach(SoundScene scene, soundParams.scenes) {
            if (scene.sceneIndex == sceneIndex) {
                QString frame = item->text();
                scene.frames.removeOne(frame.toInt());

                if (scene.frames.isEmpty())
                    soundParams.scenes.removeAt(i);
                else
                    soundParams.scenes.replace(i, scene);

                framesListWidget->takeItem(framesListWidget->row(item));
                break;
            }
            i++;
        }

        emit soundResourceModified(soundParams);
    }
}
