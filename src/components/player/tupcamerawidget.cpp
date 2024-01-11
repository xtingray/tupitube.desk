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

#include "tupcamerawidget.h"
#include "tconfig.h"
#include "tupinfodialog.h"
#include "tupprojectresponse.h"
#include "tuprequestbuilder.h"

#include <QMainWindow>
#include <QApplication>

TupCameraWidget::TupCameraWidget(TupProject *work, QWidget *parent) : QFrame(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraWidget()]";
    #endif

    setObjectName("TupCameraWidget_");
    screen = QGuiApplication::screens().at(0);

    TCONFIG->beginGroup("Theme");
    uiTheme = TCONFIG->value("UITheme", DARK_THEME).toInt();

    playMode = OneScene;
    currentSceneIndex = 0;
    QSize projectSize = work->getDimension();
    double factor = static_cast<double>(projectSize.width()) / static_cast<double>(projectSize.height());
    int percent = 40;
    int height = screen->geometry().height();

    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraWidget()] - Screen height ->" << height;
        qDebug() << "[TupCameraWidget()] - Project height ->" << projectSize.height();
        qDebug() << "[TupCameraWidget()] - Factor ->" << factor;
    #endif

    if (height <= 800) {
        percent = 30;
    } else if (height >= 1080) {
        if (factor < 1.5)
            percent = 40;
        else
            percent = 55;
    }

    int desktopWidth = (percent * screen->geometry().width()) / 100;
    int desktopHeight = (percent * height) / 100;
    screenDimension = QSize(desktopWidth, desktopHeight);

    project = work;

    layout = new QBoxLayout(QBoxLayout::TopToBottom, this);

    addVideoHeader();
    addTimerPanel();
    layout->addSpacing(10);

    addAnimationDisplay();

    layout->addSpacing(10);
    addPlayerButtonsBar();
    addStatusPanel();
}

TupCameraWidget::~TupCameraWidget()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupCameraWidget()]";
    #endif

    if (cameraBar) {
        cameraBar = nullptr;
        delete cameraBar;
    }

    if (progressBar) {
       progressBar = nullptr;
       delete progressBar;
    }

    if (cameraStatus) {
        cameraStatus = nullptr;
        delete cameraStatus;
    }

    if (previewScreen) {
        previewScreen->clearPhotograms();
        previewScreen = nullptr;
        delete previewScreen;
    }
}

void TupCameraWidget::addVideoHeader()
{
    QHBoxLayout *labelLayout = new QHBoxLayout;
    labelLayout->setAlignment(Qt::AlignCenter);
    labelLayout->setSpacing(0);

    QLabel *icon = new QLabel();
    icon->setPixmap(QPixmap(ICONS_DIR + "player.png"));
    QLabel *title = new QLabel(tr("Scene Preview"));
    QFont font = this->font();
    font.setBold(true);
    title->setFont(font);

    titleWidget = new QWidget();
    QHBoxLayout *titleLayout = new QHBoxLayout(titleWidget);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setAlignment(Qt::AlignCenter);
    titleLayout->addWidget(icon);
    titleLayout->addWidget(title);

    setProgressBar();

    QString name = project->getName();
    if (name.length() > 25)
        name = name.left(25) + "...";
    projectLabel = new QLabel(name + " ");
    projectLabel->setFont(font);

    setDimensionLabel(project->getDimension());

    QWidget *scaleWidget = new QWidget();
    QHBoxLayout *scaleLayout = new QHBoxLayout(scaleWidget);
    scaleLayout->setContentsMargins(0, 0, 0, 0);
    scaleLayout->setAlignment(Qt::AlignCenter);
    scaleLayout->addWidget(scaleLabel);

    QPushButton *editButton = new QPushButton();
    editButton->setIcon(QIcon(ICONS_DIR + "edit_sign.png"));
    editButton->setFocusPolicy(Qt::NoFocus);
    editButton->setToolTip(tr("Edit Project Information"));
    connect(editButton, SIGNAL(pressed()), this, SLOT(infoDialog()));

    labelLayout->addWidget(projectLabel);
    labelLayout->addSpacing(5);
    labelLayout->addWidget(editButton);
    labelLayout->addSpacing(20);
    labelLayout->addWidget(scaleWidget);

    layout->addWidget(titleWidget, 0, Qt::AlignCenter);
    layout->addWidget(progressBar, 0, Qt::AlignCenter);
    layout->addLayout(labelLayout, Qt::AlignCenter);
}

void TupCameraWidget::setProgressBar()
{
    progressBar = new QProgressBar(this);
    QString style1 = "QProgressBar { background-color: #DDDDDD; text-align: center; "
                     "color: #FFFFFF; border-radius: 2px; } ";
    QString color = "#666666";
    if (uiTheme == DARK_THEME)
        color = "#444444";
    QString style2 = "QProgressBar::chunk { background-color: " + color + "; border-radius: 2px; }";

    progressBar->setStyleSheet(style1 + style2);
    progressBar->setMaximumHeight(5);
    progressBar->setTextVisible(false);
    progressBar->setVisible(false);
}

void TupCameraWidget::addTimerPanel()
{
    QFont font = this->font();
    font.setBold(true);

    QLabel *timerFramesLabel = new QLabel(tr("Current Frame: "));
    timerFramesLabel->setFont(font);
    currentFrameBox = new QLabel("1");
    currentFrameBox->setAlignment(Qt::AlignCenter);
    currentFrameBox->setMinimumWidth(40);

    QString labelColor = "#ffffff";
    if (uiTheme == DARK_THEME)
        labelColor = "#c8c8c8";
    QString style = "QLabel { background-color: " + labelColor
                    + "; border: 1px solid #777777; border-radius: 2px; }";
    currentFrameBox->setStyleSheet(style);

    framesCount = new QLabel;

    QLabel *stopwatchLabel = new QLabel(tr("Timer: "));
    stopwatchLabel->setFont(font);
    timerSecsLabel = new QLabel("00.00");
    timerSecsLabel->setAlignment(Qt::AlignCenter);
    timerSecsLabel->setMinimumWidth(50);
    timerSecsLabel->setStyleSheet(style);

    QLabel *durationLabel = new QLabel(tr("Duration: "));
    durationLabel->setFont(font);
    duration = new QLabel("");

    QFrame *timerWidget = new QFrame(this);
    timerWidget->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    QHBoxLayout *timerLayout = new QHBoxLayout(timerWidget);
    timerLayout->setContentsMargins(10, 5, 10, 5);
    timerLayout->setAlignment(Qt::AlignCenter);
    timerLayout->addWidget(timerFramesLabel);
    timerLayout->addWidget(currentFrameBox);
    timerLayout->addWidget(framesCount);
    timerLayout->addSpacing(10);

    timerLayout->addWidget(stopwatchLabel);
    timerLayout->addWidget(timerSecsLabel);
    timerLayout->addWidget(new QLabel(tr("secs")));
    timerLayout->addSpacing(10);

    timerLayout->addWidget(durationLabel);
    timerLayout->addWidget(duration);

    layout->addWidget(timerWidget, 0, Qt::AlignCenter|Qt::AlignTop);
}

void TupCameraWidget::addAnimationDisplay()
{
    previewScreen = new TupScreen(project, playerDimension, isScaled);
    previewScreen->setFixedSize(playerDimension);
    connect(previewScreen, SIGNAL(isRendering(int)), this, SLOT(updateProgressBar(int)));
    connect(previewScreen, SIGNAL(frameChanged(int)), this, SLOT(updateTimerPanel(int)));
    connect(previewScreen, SIGNAL(activePause()), this, SLOT(doPause()));

    connect(previewScreen, SIGNAL(sceneResponseActivated(int,TupProjectRequestArgument,int)),
            this, SLOT(sceneResponse(int,TupProjectRequestArgument,int)));

    layout->addWidget(previewScreen, 0, Qt::AlignCenter);
}

void TupCameraWidget::addPlayerButtonsBar()
{
    cameraBar = new TupCameraBar;

    connect(cameraBar, SIGNAL(play()), this, SLOT(doPlay()));
    connect(cameraBar, SIGNAL(playBack()), this, SLOT(doPlayBack()));
    connect(cameraBar, SIGNAL(pause()), this, SLOT(doPause()));
    connect(cameraBar, SIGNAL(stop()), this, SLOT(doStop()));
    connect(cameraBar, SIGNAL(ff()), previewScreen, SLOT(nextFrame()));
    connect(cameraBar, SIGNAL(rew()), previewScreen, SLOT(previousFrame()));
    connect(previewScreen, SIGNAL(playerStopped()), this, SLOT(disablePlayButtons()));

    layout->addWidget(cameraBar, 0, Qt::AlignCenter);
}

void TupCameraWidget::addStatusPanel()
{
    cameraStatus = new TupCameraStatus(project->getSceneNames().size());
    cameraStatus->setScenes(project->getSceneNames());
    connect(cameraStatus, SIGNAL(playModeChanged(PlayMode,int)), this, SLOT(updatePlayMode(PlayMode,int)));
    connect(cameraStatus, SIGNAL(sceneIndexChanged(int)), this, SLOT(selectScene(int)));
    connect(cameraStatus, SIGNAL(muteEnabled(bool)), previewScreen, SLOT(enableMute(bool)));
    connect(cameraStatus, SIGNAL(fpsChanged(int)), this, SLOT(updateFPS(int)));
    connect(cameraStatus, SIGNAL(loopChanged()), this, SLOT(setLoop()));
    connect(cameraStatus, SIGNAL(exportClicked()), this, SLOT(exportDialog()));
    connect(cameraStatus, SIGNAL(postClicked()), this, SLOT(postDialog()));

    updateFramesTotal(0);

    int fps = project->getFPS(currentSceneIndex);
    fpsDelta = 1.0 / fps;
    cameraStatus->setFPS(fps);

    setLoop();
    layout->addWidget(cameraStatus, 0, Qt::AlignCenter|Qt::AlignTop);
}

void TupCameraWidget::setDimensionLabel(const QSize dimension)
{
    QFont font = this->font();
    font.setBold(false);
    scaleLabel = new QLabel;
    scaleLabel->setFont(font);

    int screenWidth = screenDimension.width();
    int screenHeight = screenDimension.height();

    int projectWidth = dimension.width();
    int projectHeight = dimension.height();

    QString scale = "<b>[</b> " + tr("Scale") + " ";
    isScaled = false;

    // The project dimension fits within the interface
    if (projectWidth <= screenWidth && projectHeight <= screenHeight) {
        playerDimension = project->getDimension();
        scale += "1:1";
    } else { // Project dimension is too huge. Player display must be scaled
        double proportion = 1;

        // If the project is wider than higher
        if (projectWidth > projectHeight) {
            int newH = (projectHeight * screenWidth) / projectWidth;
            playerDimension = QSize(screenWidth, newH);
            proportion = static_cast<double>(projectWidth) / static_cast<double>(screenWidth);
        } else { // Project is higher than wider
            int newW = (projectWidth * screenHeight) / projectHeight;
            playerDimension = QSize(newW, screenHeight);
            proportion = static_cast<double>(projectHeight) / static_cast<double>(screenHeight);
        }

        scale += "1:" + QString::number(proportion, 'g', 2);
        isScaled = true;
    }

    scale += " | " + tr("Size") + ": ";
    scale += QString::number(projectWidth) + "x" + QString::number(projectHeight);
    scale += " px <b>]</b>";

    scaleLabel->setText(scale);
}

void TupCameraWidget::setLoop()
{
    previewScreen->setLoop(cameraStatus->isLooping());
}

QSize TupCameraWidget::sizeHint() const
{
    QSize size = QWidget::sizeHint();
    return size.expandedTo(QApplication::globalStrut());
}

void TupCameraWidget::doPlay()
{
    int frames = previewScreen->sceneFramesTotal();

    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraWidget::doPlay()] - frames -> " << frames;
        qDebug() << "[TupCameraWidget::doPlay()] - Starting player...";
    #endif

    previewScreen->play();

    bool flag = false;
    if (frames > 1)
        flag = true;

    cameraStatus->enableButtons(flag);
    cameraBar->updatePlaybackButton(false);
    cameraBar->updatePlayButton(flag);
}

void TupCameraWidget::doPlayBack()
{
    int frames = previewScreen->sceneFramesTotal();

    bool flag = false;
    if (frames > 1)
        flag = true;

    previewScreen->playBack();
    cameraBar->updatePlayButton(false);
    cameraBar->updatePlaybackButton(flag);
}

void TupCameraWidget::doPause()
{
    int frames = previewScreen->sceneFramesTotal();

    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraWidget::doPause()] - frames -> " << frames;
    #endif

    if (frames > 1) {
        bool playOn = previewScreen->isPlaying();
        if (previewScreen->getPlayDirection() == Forward)
            cameraBar->updatePlayButton(!playOn);
        else
            cameraBar->updatePlaybackButton(!playOn);

        previewScreen->pause();
    } else {
        if (previewScreen->getPlayDirection() == Forward)
            cameraBar->updatePlayButton(false);
        else
            cameraBar->updatePlaybackButton(false);
    }
}

void TupCameraWidget::doStop()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraWidget::doStop()]";
    #endif

    if (previewScreen->getPlayDirection() == Forward)
        cameraBar->updatePlayButton(false);
    else
        cameraBar->updatePlaybackButton(false);

    previewScreen->stop();
}

void TupCameraWidget::nextFrame()
{
    previewScreen->nextFrame();
}

void TupCameraWidget::previousFrame()
{
    previewScreen->previousFrame();
}

void TupCameraWidget::sceneResponse(int action, TupProjectRequestArgument arg, int sceneIndex)
{
    Q_UNUSED(arg)

    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraWidget::sceneResponse()] - action ->" << action;
        qDebug() << "[TupCameraWidget::sceneResponse()] - sceneIndex ->" << sceneIndex;
    #endif

    switch (action) {
        case TupProjectRequest::Add:
        {
             cameraStatus->setScenes(project->getSceneNames());
             cameraStatus->setCurrentScene(sceneIndex);
             updateFramesTotal(sceneIndex);
        }
        break;
        case TupProjectRequest::Remove:
        {
             if (sceneIndex == project->scenesCount())
                 sceneIndex--;

             cameraStatus->setScenes(project->getSceneNames());
             cameraStatus->setCurrentScene(sceneIndex);
             updateFramesTotal(sceneIndex);
        }
        break;
        case TupProjectRequest::Reset:
        {
             cameraStatus->setScenes(project->getSceneNames());
        }
        break;
        case TupProjectRequest::Select:
        {
             currentSceneIndex = sceneIndex;

             int fps = project->getFPS(currentSceneIndex);
             fpsDelta = 1.0 / fps;
             cameraStatus->setFPS(fps);

             updateFramesTotal(sceneIndex);
             cameraStatus->setCurrentScene(sceneIndex);
        }
        break;
        case TupProjectRequest::Rename:
        {
             cameraStatus->setScenes(project->getSceneNames());
             cameraStatus->setCurrentScene(sceneIndex);
        }
        break;
        case TupProjectRequest::Move:
        {
             cameraStatus->setScenes(project->getSceneNames());
             cameraStatus->setCurrentScene(sceneIndex);
        }
        break;
        default:
        {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupCameraWidget::sceneResponse()] - "
                             "Unknown/Unhandled project action: "
                             << action;
             #endif
        }
        break;
    }
}

bool TupCameraWidget::handleProjectResponse(TupProjectResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraWidget::handleProjectResponse()]";
        response->toString();
    #endif

    if (previewScreen)
        return previewScreen->handleResponse(response);

    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraWidget::handleProjectResponse()] - "
                    "Fatal Error: previewScreen is NULL!";
    #endif

    return false;
}

void TupCameraWidget::setFPS(int fps)
{
    project->setFPS(fps, currentSceneIndex);
    previewScreen->setFPS(fps);
    fpsDelta = 1.0/fps;
}

void TupCameraWidget::setDuration(int fps)
{
    double time = static_cast<double> (framesTotal) / static_cast<double> (fps);
    duration->setText(QString::number(time, 'f', 2) + QString(" " + tr("secs")));
}

void TupCameraWidget::updateFPS(int fps)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraWidget::updateFPS()] - fps -> " << fps;
    #endif

    emit projectHasChanged(true);
    emit fpsUpdated(fps);

    setFPS(fps);
    setDuration(fps);
}

void TupCameraWidget::setFpsStatus(int fps)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraWidget::setStatusFPS()] - fps ->" << fps;
    #endif

    cameraStatus->blockSignals(true);
    cameraStatus->setFPS(fps);
    cameraStatus->blockSignals(false);

    project->setFPS(fps);
    previewScreen->setFPS(fps);
    setDuration(fps);
}

void TupCameraWidget::updateFramesTotal(int sceneIndex)
{
    if (playMode == OneScene) {
        TupScene *scene = project->sceneAt(sceneIndex);
        if (scene) {
            framesTotal = scene->framesCount();
            framesCount->setText("/ " + QString::number(framesTotal));
            progressBar->setRange(0, framesTotal);
            setDuration(project->getFPS());
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupCameraWidget::updateFramesTotal()] - "
                            "Fatal Error: Scene index is invalid ->" << sceneIndex;
            #endif
        }
    } else { // Play All mode
        framesTotal = 0;
        int scenesTotal = project->scenesCount();
        for(int i=0; i<scenesTotal; i++)
            framesTotal += project->sceneAt(i)->framesCount();

        framesCount->setText("/ " + QString::number(framesTotal));
        progressBar->setRange(0, framesTotal);
        setDuration(project->getFPS());
    }

    progressBar->setRange(1, framesTotal);
}

void TupCameraWidget::exportDialog()
{
    if (previewScreen->isPlaying())
        previewScreen->pause();

    emit exportRequested();
}

void TupCameraWidget::postDialog()
{
    if (previewScreen->isPlaying())
        previewScreen->pause();

    emit postRequested();
}

void TupCameraWidget::selectScene(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraWidget::selectScene()] - index ->" << index;
    #endif

    if (index != previewScreen->getCurrentSceneIndex()) {
        TupProjectRequest event = TupRequestBuilder::createSceneRequest(index, TupProjectRequest::Select);
        emit requestTriggered(&event);

        doStop();
        previewScreen->updateSceneIndex(index);
        previewScreen->initPlayerScreen();
        doPlay();
    }
}

void TupCameraWidget::updateHeaderParameters(PlayMode mode, int sceneIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraWidget::updateHeaderParameters()] - mode ->" << mode;
        qDebug() << "[TupCameraWidget::updateHeaderParameters()] - sceneIndex ->" << sceneIndex;
    #endif

    if (mode == OneScene) {
        framesTotal = project->sceneAt(sceneIndex)->framesCount();
        framesCount->setText("/ " + QString::number(framesTotal));
        setDuration(project->getFPS());
    } else { // Play All mode
        framesTotal = 0;
        for (int i=0; i < project->scenesCount(); i++) {
            TupScene *scene = project->sceneAt(i);
            framesTotal += scene->framesCount();
        }
        framesCount->setText("/ " + QString::number(framesTotal));
        setDuration(project->getFPS());
    }
}

void TupCameraWidget::updatePlayMode(PlayMode mode, int sceneIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraWidget::updatePlayMode()] - mode ->" << mode;
        qDebug() << "[TupCameraWidget::updatePlayMode()] - sceneIndex ->" << sceneIndex;
    #endif

    updateHeaderParameters(mode, sceneIndex);

    playMode = mode;

    cameraBar->updatePlaybackButton(false);
    cameraBar->updatePlayButton(true);
    previewScreen->setPlayMode(mode, sceneIndex);
    cameraBar->setFocus();
}

void TupCameraWidget::updateScenes(int sceneIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraWidget::updateScenes()] - sceneIndex ->" << sceneIndex;
    #endif

    previewScreen->resetSceneFromList(sceneIndex);
}

void TupCameraWidget::updateFirstFrame()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraWidget::updateFirstFrame()]";
    #endif

    previewScreen->initPlayerScreen();
    currentFrameBox->setText("1");
}

void TupCameraWidget::updateProgressBar(int advance)
{
    if (advance == 0) {
        QApplication::restoreOverrideCursor();
        titleWidget->setVisible(true);
        progressBar->setVisible(false);
    }

    if (advance == 1) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        titleWidget->setVisible(false);
        progressBar->setVisible(true);
    }

    progressBar->setValue(advance);
}

void TupCameraWidget::updateTimerPanel(int currentFrame)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraWidget::updateTimerPanel()] - currentFrame ->" << currentFrame;
    #endif
    */

    currentFrameBox->setText(QString::number(currentFrame));
    double time = fpsDelta * currentFrame;
    timerSecsLabel->setText(QString::number(time, 'f', 2));
}

void TupCameraWidget::updateSoundItems()
{
    previewScreen->loadSoundRecords();
}

void TupCameraWidget::infoDialog()
{    
    TupInfoDialog *settings = new TupInfoDialog(project->getAuthor(), project->getDescription(), this);

    connect(settings, SIGNAL(dataSent(const QString &, const QString &)),
            this, SLOT(saveProjectInfo(const QString &, const QString &)));

    settings->show();
}

void TupCameraWidget::saveProjectInfo(const QString &author, const QString &description)
{
    project->setAuthor(author);
    emit projectAuthorUpdated(author);
    project->setDescription(description);
}

void TupCameraWidget::clearMemory()
{
    previewScreen->clearPhotograms();
}

void TupCameraWidget::resetPlayerInterface()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraWidget::resetPlayerInterface()]";
    #endif

    previewScreen->releaseAudioResources();
    previewScreen->clearAllScenesPhotograms();
    previewScreen = nullptr;
    delete previewScreen;
}

void TupCameraWidget::loadSoundRecords()
{
    previewScreen->loadSoundRecords();
}

bool TupCameraWidget::removeSoundTrack(const QString &soundKey)
{
    return previewScreen->removeSoundTrack(soundKey);
}

void TupCameraWidget::releaseAudioResources()
{
    previewScreen->releaseAudioResources();
}

void TupCameraWidget::disablePlayButtons()
{
    cameraBar->updatePlayButton(false);
    cameraBar->updatePlaybackButton(false);
}
