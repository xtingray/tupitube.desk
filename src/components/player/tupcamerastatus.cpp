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

#include <QLabel>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QStatusBar>
#include <QApplication>
#include <QSpinBox>

struct TupCameraStatus::Private
{
    QSpinBox *fpsBox;
    QComboBox *scenes;
    QLabel *framesCount;
    QCheckBox *loopBox;
    QPushButton *exportButton;

    bool loop;
    int framesTotal;
    bool mute;
    TImageButton *soundButton;
};

TupCameraStatus::TupCameraStatus(TupCameraWidget *camera, bool isNetworked, QWidget *parent) : QFrame(parent), k(new Private)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupCameraStatus()]";
        #else
           TINIT;
        #endif
    #endif

    k->framesTotal = 1;
    k->mute = false;

    setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    QBoxLayout *sceneInfoLayout = new QBoxLayout(QBoxLayout::LeftToRight, parent);

    QLabel *sceneNameText = new QLabel("<B>" + tr("Scene") + ":</B> ");
    k->scenes = new QComboBox();
    connect(k->scenes, SIGNAL(activated(int)), this, SIGNAL(sceneIndexChanged(int)));

    sceneInfoLayout->addWidget(sceneNameText, 1);
    sceneInfoLayout->addWidget(k->scenes, 1);
    sceneInfoLayout->addSpacing(15);

    QLabel *label = new QLabel("<B>" + tr("Frames Total") + ":</B> ");
    k->framesCount = new QLabel;

    sceneInfoLayout->addWidget(label, 1);
    sceneInfoLayout->addWidget(k->framesCount, 1);

    sceneInfoLayout->addSpacing(15);

    QLabel *fpsText = new QLabel("<B>" + tr("FPS") + ":</B> ");

    k->fpsBox = new QSpinBox();
    k->fpsBox->setMinimum(1);
    k->fpsBox->setValue(24);

    connect(k->fpsBox, SIGNAL(valueChanged(int)), camera, SLOT(setFPS(int)));
    connect(k->fpsBox, SIGNAL(valueChanged(int)), this, SIGNAL(fpsChanged(int)));

    sceneInfoLayout->addWidget(fpsText, 1);
    sceneInfoLayout->addWidget(k->fpsBox, 1);

    sceneInfoLayout->addSpacing(15);

    k->loopBox = new QCheckBox();
    k->loopBox->setToolTip(tr("Loop"));
    k->loopBox->setIcon(QPixmap(THEME_DIR + "icons/loop.png")); 
    k->loopBox->setFocusPolicy(Qt::NoFocus);

    k->loopBox->setShortcut(QKeySequence(tr("Ctrl+L")));

    connect(k->loopBox, SIGNAL(clicked()), camera, SLOT(setLoop()));

    TCONFIG->beginGroup("AnimationParameters");
    k->loop = TCONFIG->value("Loop").toBool();
    if (k->loop)
        k->loopBox->setChecked(true);

    sceneInfoLayout->addWidget(k->loopBox, 1);

    sceneInfoLayout->addSpacing(15);

    k->soundButton = new TImageButton(QPixmap(THEME_DIR + "icons/speaker.png"), 22, this, true);
    k->soundButton->setShortcut(QKeySequence(tr("M")));
    k->soundButton->setToolTip(tr("Mute"));
    connect(k->soundButton, SIGNAL(clicked()), this, SLOT(mute()));
    sceneInfoLayout->addWidget(k->soundButton, 1);

    sceneInfoLayout->addSpacing(15);

    k->exportButton = new QPushButton(tr("Export"));
    k->exportButton->setIcon(QIcon(THEME_DIR + "icons/export_button.png"));
    k->exportButton->setFocusPolicy(Qt::NoFocus);
    k->exportButton->setToolTip(tr("Export Project as Video File"));

    connect(k->exportButton, SIGNAL(pressed()), camera, SLOT(exportDialog()));
    sceneInfoLayout->addWidget(k->exportButton, 1);

    if (isNetworked) {
        sceneInfoLayout->addSpacing(5);
        QPushButton *postButton = new QPushButton(tr("Post"));
        postButton->setIcon(QIcon(THEME_DIR + "icons/import_project.png"));
        postButton->setFocusPolicy(Qt::NoFocus);

        connect(postButton, SIGNAL(pressed()), camera, SLOT(postDialog()));
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
        k->fpsBox->setValue(frames);
    else
        k->fpsBox->setValue(24);
}

int TupCameraStatus::getFPS()
{
    return k->fpsBox->value();
}

void TupCameraStatus::setCurrentScene(int index)
{
    if (k->scenes->currentIndex() != index)
        k->scenes->setCurrentIndex(index);
}

void TupCameraStatus::setScenes(TupProject *project)
{
    if (k->scenes->count())
        k->scenes->clear(); 

    int scenesCount = project->scenes().size();
    for (int i = 0; i < scenesCount; i++) {
         TupScene *scene = project->scenes().at(i);
         if (scene)
             k->scenes->addItem(scene->sceneName());
    }
}

void TupCameraStatus::setFramesTotal(const QString &frames)
{
    k->framesCount->setText(frames);
    k->framesTotal = frames.toInt();
}

bool TupCameraStatus::isLooping()
{
    k->loop = k->loopBox->isChecked();
    TCONFIG->beginGroup("AnimationParameters");
    TCONFIG->setValue("Loop", k->loop);

    return k->loop;
}

void TupCameraStatus::mute()
{
    QString img("icons/mute.png");

    if (k->mute) {
        k->mute = false;
        img = "icons/speaker.png";
        k->soundButton->setToolTip(tr("Mute"));
    } else {
        k->mute = true;
        k->soundButton->setToolTip(tr("Unmute"));
    }

    k->soundButton->setImage(QPixmap(THEME_DIR + img));

    emit muteEnabled(k->mute);
}

void TupCameraStatus::enableExportButton(bool flag)
{
    k->exportButton->setVisible(flag);
}
