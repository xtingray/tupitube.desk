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
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundPlayer()]";
    #endif

    setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    soundDuration = "";
    playing = false;
    loop = false;

    mainLabel = new QLabel();
    mainLabel->setAlignment(Qt::AlignHCenter);

    soundForm = new TupSoundForm();
    connect(soundForm, SIGNAL(soundResourceModified(SoundResource)),
            this, SIGNAL(soundResourceModified(SoundResource)));
    soundForm->setVisible(false);

    timer = new QLabel(tr("Duration:") + " " + soundDuration);
    QBoxLayout *timerLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    timerLayout->addStretch();
    timerLayout->addWidget(timer);
    timerLayout->addStretch();
    timerLayout->setContentsMargins(0, 0, 0, 0);

    playButton = new TImageButton(QPixmap(THEME_DIR + "icons/play_small.png"), 33, this);
    playButton->setToolTip(tr("Play"));
    connect(playButton, SIGNAL(clicked()), this, SLOT(playFile()));

    muteButton = new TImageButton(QPixmap(THEME_DIR + "icons/speaker.png"), 22, this);
    muteButton->setShortcut(QKeySequence(tr("M")));
    muteButton->setToolTip(tr("Mute"));
    connect(muteButton, SIGNAL(clicked()), this, SLOT(muteAction()));

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
    buttonLayout->addWidget(muteButton);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(loopBox);
    buttonLayout->addStretch();
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    layout->addWidget(mainLabel);
    layout->addWidget(soundForm);
    layout->addWidget(new TSeparator(Qt::Horizontal));
    layout->addSpacing(5);
    layout->addLayout(timerLayout);
    layout->addLayout(buttonLayout);
    layout->addStretch();
    layout->setContentsMargins(5, 5, 5, 5);    
}

TupSoundPlayer::~TupSoundPlayer()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupSoundPlayer()]";
    #endif

    reset();
}

QSize TupSoundPlayer::sizeHint() const
{
    return QWidget::sizeHint().expandedTo(QSize(100, 100));
}

void TupSoundPlayer::setSoundParams(SoundResource params, QStringList scenesList, QList<int> frameLimits)
{
    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupSoundPlayer::setSoundParams()] - params.type ->" << params.type;
        qDebug() << "[TupSoundPlayer::setSoundParams()] - params.muted ->" << params.muted;
        qDebug() << "[TupSoundPlayer::setSoundParams()] - params.path ->" << params.path;
        qDebug() << "[TupSoundPlayer::setSoundParams()] - params.duration ->" << params.duration;
        qDebug() << "[TupSoundPlayer::setSoundParams()] - params.scenes.size() ->" << params.scenes.size();
        for(int i=0; i<params.scenes.size(); i++) {
            SoundScene scene = params.scenes.at(i);
            qDebug() << "[TupSoundPlayer::setSoundParams()] -    sceneIndex ->" << scene.sceneIndex;
            qDebug() << "[TupSoundPlayer::setSoundParams()] -    frames ->" << scene.frames;
        }
        qDebug() << "[TupSoundPlayer::setSoundParams()] - scenes list ->" << scenesList;
        qDebug() << "---";
    #endif

    hasDuration = false;
    soundID = params.key;
    if (!params.duration.isEmpty()) {
        soundDuration = params.duration;
        hasDuration = true;
    } else {
        soundDuration = "00:00";
    }
    timer->setText(tr("Duration:") + " " + soundDuration);

    url = params.path;
    updateCurrentSoundPath(url);

    enableLipSyncInterface(params.type);

    mute = params.muted;
    if (mute) {
        muteButton->setToolTip(tr("Unmute"));
        if (playButton->isEnabled())
            playButton->setEnabled(false);
        muteButton->setImage(QPixmap(THEME_DIR + QString("icons/mute.png")));
    } else {
        muteButton->setToolTip(tr("Mute"));
        if (!playButton->isEnabled())
            playButton->setEnabled(true);
        muteButton->setImage(QPixmap(THEME_DIR + QString("icons/speaker.png")));
    }

    soundForm->setSoundParams(params, scenesList, frameLimits);
}

void TupSoundPlayer::updateCurrentSoundPath(const QString &soundPath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundPlayer::updateCurrentSoundPath()] - soundPath ->" << soundPath;
    #endif

    if (!soundPlayer.isEmpty()) {
        while(!soundPlayer.isEmpty()) {
            if (!hasDuration)
                disconnect(soundPlayer.at(0), SIGNAL(durationChanged(qint64)), this, SLOT(durationChanged(qint64)));
            disconnect(soundPlayer.at(0), SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(stateChanged(QMediaPlayer::State)));

            QMediaPlayer *player = soundPlayer.takeFirst();
            player->stop();
            player->setMedia(QMediaContent());
            delete player;
            player = nullptr;
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupSoundPlayer::updateCurrentSoundPath()] - Warning: Media player list is empty!";
        #endif
    }

    url = soundPath;
    soundPlayer << new QMediaPlayer();
    soundPlayer.at(0)->setMedia(QUrl::fromLocalFile(url));

    if (!hasDuration)
        connect(soundPlayer.at(0), SIGNAL(durationChanged(qint64)), this, SLOT(durationChanged(qint64)));
    connect(soundPlayer.at(0), SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(stateChanged(QMediaPlayer::State)));
}

void TupSoundPlayer::updateFrameLimit(int sceneIndex, int maxFrames)
{
    soundForm->updateFrameLimit(sceneIndex, maxFrames);
}

void TupSoundPlayer::enableLipSyncInterface(SoundType type)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundPlayer::enableLipSyncInterface()] - type ->" << type;
    #endif

    QString title = "<b>" + tr("Audio Properties") + "</b>";

    if (type == Lipsync) {
        title = "<b>" + tr("Lipsync Audio") + "</b>";
        if (soundForm->isVisible())
            soundForm->setVisible(false);
    } else {
        if (!soundForm->isVisible())
            soundForm->setVisible(true);
    }

    mainLabel->setText(title);
}

void TupSoundPlayer::playFile()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundPlayer::playFile()]";
    #endif

    if (!playing)
        startPlayer();
    else
        stopFile();
}

void TupSoundPlayer::startPlayer()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundPlayer::startPlayer()] - Playing audio ->" << url;
    #endif

    playButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/pause.png")));
    playing = true;

    timer->setText(tr("Duration:") + " " + soundDuration);

    soundPlayer.at(0)->setVolume(60);
    soundPlayer.at(0)->play();
}

void TupSoundPlayer::stopFile()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundPlayer::stopFile()]";
    #endif

    playButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/play_small.png")));
    playing = false;

    soundPlayer.at(0)->pause();
}

void TupSoundPlayer::updateLoopState()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundPlayer::updateLoopState()]";
    #endif

    if (loopBox->isChecked())
        loop = true;
    else
        loop = false;
}

void TupSoundPlayer::durationChanged(qint64 value)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundPlayer::durationChanged()]";
    #endif

    if (soundDuration.compare("00:00") == 0) {
        qint64 duration = value/1000;
        QTime soundTotalTime = QTime((duration/3600)%60, (duration/60)%60, duration%60, (duration*1000)%1000);
        QString format = "mm:ss";
        if (duration > 3600)
            format = "hh:mm:ss";

        soundDuration = soundTotalTime.toString(format);
        hasDuration = true;
        timer->setText(tr("Duration:") + " " + soundDuration);
    }
}

void TupSoundPlayer::stateChanged(QMediaPlayer::State state)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundPlayer::stateChanged()] - state -> " << state;
    #endif

    if (state == QMediaPlayer::StoppedState) {
        playButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/play_small.png")));
        playing = false;

        if (loop)
            QTimer::singleShot(200, this, SLOT(startPlayer()));
    }
}

void TupSoundPlayer::updateSoundPos(int pos)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundPlayer::updateSoundPos()] - pos -> " << pos;
    #endif

    soundPlayer.at(0)->setPosition(pos*1000);
}

bool TupSoundPlayer::isPlaying()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundPlayer::isPlaying()]";
    #endif

    return playing;
}

void TupSoundPlayer::reset()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundPlayer::reset()]";
    #endif

    hide();

    loop = false;
    loopBox->setChecked(false);
    resetMediaPlayer();
}

void TupSoundPlayer::resetMediaPlayer()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundPlayer::resetMediaPlayer()]";
    #endif

    if (!soundPlayer.isEmpty()) {
        while(!soundPlayer.isEmpty()) {
            QMediaPlayer *player = soundPlayer.takeFirst();
            player->stop();
            player->setMedia(QMediaContent());
            delete player;
            player = nullptr;
        }
    }
}

void TupSoundPlayer::muteAction()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundPlayer::muteAction()]";
    #endif

    QString img("icons/mute.png");
    if (mute) {
        mute = false;
        img = "icons/speaker.png";
        muteButton->setToolTip(tr("Mute"));
        playButton->setEnabled(true);
    } else {
        mute = true;
        muteButton->setToolTip(tr("Unmute"));
        playButton->setEnabled(false);
        if (playing)
            stopFile();
    }
    muteButton->setImage(QPixmap(THEME_DIR + img));

    emit muteEnabled(mute);
}

QString TupSoundPlayer::getSoundID() const
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundPlayer::getSoundID()]";
    #endif

    return soundID;
}
