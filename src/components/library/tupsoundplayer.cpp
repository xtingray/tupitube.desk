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

#include "tupsoundplayer.h"
#include "tseparator.h"

#include <QTimer>

TupSoundPlayer::TupSoundPlayer(QWidget *parent) : QFrame(parent)
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    setMidLineWidth(2);
    setLineWidth(1);

    playing = false;
    loop = false;
    player = new QMediaPlayer;
    connect(player, SIGNAL(positionChanged(qint64)), SLOT(positionChanged(qint64)));
    connect(player, SIGNAL(durationChanged(qint64)), SLOT(durationChanged(qint64)));
    connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), SLOT(stateChanged(QMediaPlayer::State)));

    frameWidget = new QWidget;

    QLabel *frameLabel = new QLabel(tr("Play at frame:") + " ");
    frameBox = new QSpinBox();
    frameBox->setMinimum(1);
    frameBox->setMaximum(999);
    frameBox->setValue(1);
    connect(frameBox, SIGNAL(valueChanged(int)), this, SIGNAL(frameUpdated(int)));

    QBoxLayout *effectLayout = new QBoxLayout(QBoxLayout::LeftToRight, frameWidget);
    effectLayout->addStretch();
    effectLayout->addWidget(frameLabel);
    effectLayout->addWidget(frameBox);
    effectLayout->addStretch();

    /*
    lipSyncWidget = new QWidget;
    QBoxLayout *lipLayout = new QBoxLayout(QBoxLayout::LeftToRight, lipSyncWidget);
    lipLayout->addStretch();
    lipLayout->addWidget(new QLabel(tr("Papagayo Lip-Sync Sound")));
    lipLayout->addStretch();
    */

    timer = new QLabel("");
    QBoxLayout *timerLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    timerLayout->addStretch();
    timerLayout->addWidget(timer);
    timerLayout->addStretch();
    timerLayout->setContentsMargins(0, 0, 0, 0);

    slider = new QSlider(Qt::Horizontal, this);
    connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(updateSoundPos(int)));

    QBoxLayout *sliderLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    sliderLayout->addWidget(slider);
    sliderLayout->setContentsMargins(0, 0, 0, 0);

    playButton = new TImageButton(QPixmap(THEME_DIR + "icons/play_small.png"), 33, this, true);
    playButton->setToolTip(tr("Play"));
    connect(playButton, SIGNAL(clicked()), this, SLOT(playFile()));

    loopBox = new QCheckBox();
    loopBox->setToolTip(tr("Loop"));
    loopBox->setIcon(QPixmap(THEME_DIR + "icons/loop.png"));
    loopBox->setFocusPolicy(Qt::NoFocus);
    connect(loopBox, SIGNAL(clicked()), this, SLOT(updateLoopState()));

    QBoxLayout *buttonLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    buttonLayout->addStretch();
    buttonLayout->addWidget(playButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(new TSeparator(Qt::Vertical));
    buttonLayout->addStretch();
    buttonLayout->addWidget(loopBox);
    buttonLayout->addStretch();
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->addSpacing(5);
    layout->addWidget(frameWidget);
    // layout->addWidget(lipSyncWidget);
    layout->addLayout(timerLayout);
    layout->addLayout(sliderLayout);
    layout->addLayout(buttonLayout);
    layout->addSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);
}

TupSoundPlayer::~TupSoundPlayer()
{
}

QSize TupSoundPlayer::sizeHint() const
{
    return QWidget::sizeHint().expandedTo(QSize(100, 100));
}

void TupSoundPlayer::setSoundParams(const QString &path, int frameIndex)
{
    player->setMedia(QUrl::fromLocalFile(path));
    frameBox->blockSignals(true);
    frameBox->setValue(frameIndex);
    frameBox->blockSignals(false);
}

void TupSoundPlayer::playFile()
{
    if (!playing)
        startPlayer();
    else
        stopFile();
}

void TupSoundPlayer::startPlayer()
{
    playButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/pause.png")));
    playing = true;
    player->setVolume(60);

    QString initTime = "00:00";
    if (duration > 3600)
        initTime = "00:00:00";
    initTime = initTime + " / " + totalTime;
    timer->setText(initTime);

    player->play();
}

void TupSoundPlayer::stopFile()
{
    playButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/play_small.png")));
    playing = false;
    player->pause();
}

void TupSoundPlayer::updateLoopState()
{
    if (loopBox->isChecked())
        loop = true;
    else
        loop = false;
}

void TupSoundPlayer::positionChanged(qint64 value)
{
    qint64 currentInfo = value / 1000;
    slider->setValue(static_cast<int>(currentInfo));
    QString time;

    if (currentInfo || duration) {
        QTime currentTime((currentInfo/3600)%60, (currentInfo/60)%60, currentInfo%60, (currentInfo*1000)%1000);
        QString format = "mm:ss";
        if (duration > 3600)
            format = "hh:mm:ss";
        time = currentTime.toString(format) + " / " + totalTime;
    }

    timer->setText(time);
}

void TupSoundPlayer::durationChanged(qint64 value)
{
    duration = value/1000;
    slider->setMinimum(0);
    slider->setMaximum(static_cast<int>(duration));

    soundTotalTime = QTime((duration/3600)%60, (duration/60)%60, duration%60, (duration*1000)%1000);
    QString format = "mm:ss";
    if (duration > 3600)
        format = "hh:mm:ss";
    totalTime = soundTotalTime.toString(format);
}

void TupSoundPlayer::stateChanged(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::StoppedState) {
        slider->setValue(0);
        playButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/play_small.png")));
        playing = false;
        QString init = "00:00";
        if (duration > 3600)
            init = "00:00:00";
        timer->setText(init + " / " + totalTime);

        if (loop)
            QTimer::singleShot(200, this, SLOT(startPlayer()));
    }
}

void TupSoundPlayer::updateSoundPos(int pos)
{
    player->setPosition(pos*1000);
}

bool TupSoundPlayer::isPlaying()
{
    return playing;
}

void TupSoundPlayer::reset()
{
    loop = false;
    loopBox->setChecked(false);
    hide();
}
