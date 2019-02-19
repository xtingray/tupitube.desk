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

#include "tupcamerabar.h"
#include "tapplicationproperties.h"
// #include <QPainter>

#include <QBoxLayout>

TupCameraBar::TupCameraBar(QWidget *parent) : QFrame(parent)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupCameraBar()]";
        #else
            TINIT;
        #endif
    #endif

    // playOn = false;
    setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    setMidLineWidth(2);
    setLineWidth(1);

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::LeftToRight, parent);
    mainLayout->addStretch(1);

    mainLayout->setSpacing(10);
    mainLayout->setMargin(3);

    rewindButton = new TImageButton(QPixmap(THEME_DIR + "icons/rw.png"), 33, this, true);
    rewindButton->setToolTip(tr("Rewind"));
    mainLayout->addWidget(rewindButton);
    connect(rewindButton, SIGNAL(clicked()), this, SIGNAL(rew()));

    playBackButton = new TImageButton(QPixmap(THEME_DIR + "icons/play_back.png"), 25, this, true);
    playBackButton->setToolTip(tr("Play in reverse"));
    mainLayout->addWidget(playBackButton);
    connect(playBackButton, SIGNAL(clicked()), this, SIGNAL(playBack()));

    playButton = new TImageButton(QPixmap(THEME_DIR + "icons/play.png"), 33, this, true);
    playButton->setToolTip(tr("Play"));
    mainLayout->addWidget(playButton);
    connect(playButton, SIGNAL(clicked()), this, SIGNAL(play()));

    pauseButton = new TImageButton(QPixmap(THEME_DIR + "icons/pause.png"), 33, this, true);
    pauseButton->setToolTip(tr("Pause"));
    mainLayout->addWidget(pauseButton);
    connect(pauseButton, SIGNAL(clicked()), this, SIGNAL(pause()));

    stopButton = new TImageButton(QPixmap(THEME_DIR + "icons/stop.png"), 25, this, true);
    stopButton->setToolTip(tr("Stop"));
    mainLayout->addWidget(stopButton);
    connect(stopButton, SIGNAL(clicked()), this, SIGNAL(stop()));

    ffButton = new TImageButton(QPixmap(THEME_DIR + "icons/ff.png"), 33, this, true);
    ffButton->setToolTip(tr("Forward"));
    mainLayout->addWidget(ffButton);
    connect(ffButton, SIGNAL(clicked()), this, SIGNAL(ff()));

    setLayout(mainLayout);
}

TupCameraBar::~TupCameraBar()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[~TupCameraBar()]";
        #else
           TEND;
        #endif
    #endif
}

/*
void TupCameraBar::setPalette(const QPalette &)
{
}
*/

void TupCameraBar::updatePlayButton(bool playOn)
{
    QString pic = "";
    if (playOn)
        pic = "_on";

    playButton->setIcon(QPixmap(THEME_DIR + "icons/play" + pic + ".png"));
}
