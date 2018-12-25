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

#include "tupcamerastatus.h"
#include "tupscene.h"
#include "tupexportwidget.h"

#include <QHBoxLayout>

TupCameraStatus::TupCameraStatus(bool isNetworked, QWidget *parent) : QFrame(parent)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupCameraStatus()]";
        #else
           TINIT;
        #endif
    #endif

    framesTotal = 1;
    mute = false;

    setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    QBoxLayout *sceneInfoLayout = new QBoxLayout(QBoxLayout::LeftToRight, parent);

    QLabel *sceneNameText = new QLabel("<B>" + tr("Scene") + ":</B> ");
    scenesCombo = new QComboBox();
    connect(scenesCombo, SIGNAL(activated(int)), this, SIGNAL(sceneIndexChanged(int)));

    sceneInfoLayout->addWidget(sceneNameText, 1);
    sceneInfoLayout->addWidget(scenesCombo, 1);
    sceneInfoLayout->addSpacing(15);

    QLabel *label = new QLabel("<B>" + tr("Frames Total") + ":</B> ");
    framesCount = new QLabel;

    sceneInfoLayout->addWidget(label, 1);
    sceneInfoLayout->addWidget(framesCount, 1);

    sceneInfoLayout->addSpacing(15);

    QLabel *fpsText = new QLabel("<B>" + tr("FPS") + ":</B> ");

    fpsBox = new QSpinBox();
    fpsBox->setMinimum(1);
    fpsBox->setValue(24);
    connect(fpsBox, SIGNAL(valueChanged(int)), this, SIGNAL(fpsChanged(int)));

    sceneInfoLayout->addWidget(fpsText, 1);
    sceneInfoLayout->addWidget(fpsBox, 1);

    sceneInfoLayout->addSpacing(15);

    loopBox = new QCheckBox();
    loopBox->setToolTip(tr("Loop"));
    loopBox->setIcon(QPixmap(THEME_DIR + "icons/loop.png"));
    loopBox->setFocusPolicy(Qt::NoFocus);
    loopBox->setShortcut(QKeySequence(tr("Ctrl+L")));
    connect(loopBox, SIGNAL(clicked()), this, SIGNAL(loopChanged()));

    TCONFIG->beginGroup("AnimationParameters");
    loop = TCONFIG->value("Loop").toBool();
    if (loop)
        loopBox->setChecked(true);

    sceneInfoLayout->addWidget(loopBox, 1);

    sceneInfoLayout->addSpacing(15);

    soundButton = new TImageButton(QPixmap(THEME_DIR + "icons/speaker.png"), 22, this, true);
    soundButton->setShortcut(QKeySequence(tr("M")));
    soundButton->setToolTip(tr("Mute"));
    connect(soundButton, SIGNAL(clicked()), this, SLOT(muteAction()));
    sceneInfoLayout->addWidget(soundButton, 1);

    sceneInfoLayout->addSpacing(15);

    exportButton = new QPushButton(tr("Export"));
    exportButton->setIcon(QIcon(THEME_DIR + "icons/export_button.png"));
    exportButton->setFocusPolicy(Qt::NoFocus);
    exportButton->setToolTip(tr("Export Project as Video File"));
    connect(exportButton, SIGNAL(pressed()), this, SIGNAL(exportChanged()));
    sceneInfoLayout->addWidget(exportButton, 1);

    if (isNetworked) {
        sceneInfoLayout->addSpacing(5);
        QPushButton *postButton = new QPushButton(tr("Post"));
        postButton->setIcon(QIcon(THEME_DIR + "icons/import_project.png"));
        postButton->setFocusPolicy(Qt::NoFocus);
        connect(postButton, SIGNAL(pressed()), this, SIGNAL(postChanged()));
        sceneInfoLayout->addWidget(postButton, 1);
    }

    setLayout(sceneInfoLayout);
}

TupCameraStatus::~TupCameraStatus()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[~TupCameraStatus()]";
        #else
            TEND;
        #endif
    #endif
}

void TupCameraStatus::setFPS(int frames)
{
    /*
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen::setFPS()]";
        #else
            T_FUNCINFO;
        #endif
    #endif
    */

    if (frames > 0 && frames < 101)
        fpsBox->setValue(frames);
    else
        fpsBox->setValue(24);
}

int TupCameraStatus::getFPS()
{
    return fpsBox->value();
}

void TupCameraStatus::setCurrentScene(int index)
{
    if (scenesCombo->currentIndex() != index)
        scenesCombo->setCurrentIndex(index);
}

void TupCameraStatus::setScenes(TupProject *project)
{
    if (scenesCombo->count())
        scenesCombo->clear();

    QStringList scenes;
    int scenesCount = project->scenes().size();
    for (int i = 0; i < scenesCount; i++) {
         TupScene *scene = project->scenes().at(i);
         if (scene)
             scenes << scene->sceneName();
    }
    scenes.sort(Qt::CaseSensitive);
    scenesCombo->addItems(scenes);
}

void TupCameraStatus::setFramesTotal(const QString &frames)
{
    framesCount->setText(frames);
    framesTotal = frames.toInt();
}

bool TupCameraStatus::isLooping()
{
    loop = loopBox->isChecked();
    TCONFIG->beginGroup("AnimationParameters");
    TCONFIG->setValue("Loop", loop);

    return loop;
}

void TupCameraStatus::muteAction()
{
    QString img("icons/mute.png");

    if (mute) {
        mute = false;
        img = "icons/speaker.png";
        soundButton->setToolTip(tr("Mute"));
    } else {
        mute = true;
        soundButton->setToolTip(tr("Unmute"));
    }

    soundButton->setImage(QPixmap(THEME_DIR + img));

    emit muteEnabled(mute);
}

void TupCameraStatus::enableExportButton(bool flag)
{
    exportButton->setVisible(flag);
}
