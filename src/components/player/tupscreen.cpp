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

#include "tupscreen.h"
#include "tuplibrary.h"
#include "tupsoundlayer.h"

#include <QUrl>
#include <QApplication>

TupScreen::TupScreen(TupProject *work, const QSize viewSize, bool sizeChanged, QWidget *parent) : QFrame(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen()] - viewSize ->" << viewSize;
    #endif

    project = work;
    library = work->getLibrary();

    isScaled = sizeChanged;
    screenDimension = viewSize;

    playMode = OneScene;
    cyclicAnimation = false;
    fps = 24;
    sceneIndex = 0;
    currentFramePosition = 0;

    // PlayAll mode
    projectSceneIndex = 0;
    projectFramePosition = 0;

    playerIsActive = false;
    playDirection = Forward;
    mute = false;
    renderOn = false;

    timer = new QTimer(this);
    playBackTimer = new QTimer(this);

    connect(timer, SIGNAL(timeout()), this, SLOT(advance()));
    connect(playBackTimer, SIGNAL(timeout()), this, SLOT(back()));

    initAllPhotograms();

    updateSceneIndex(0);
    updateFirstFrame();
}

TupScreen::~TupScreen()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupScreen()]";
    #endif

    timer->stop();
    playBackTimer->stop();

    blankImagesList.clear();

    clearPhotograms();
    clearAllScenesPhotograms();

    sceneIsRendered.clear();

    delete timer;
    timer = nullptr;
    delete playBackTimer;
    playBackTimer = nullptr;
    delete renderer;
    renderer = nullptr;
}

void TupScreen::setPlayMode(PlayMode mode, int scene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::setPlayMode()] - playMode ->" << mode;
        qDebug() << "[TupScreen::setPlayMode()] - scene index ->" << scene;
    #endif

    playMode = mode;
    if (isPlaying())
        stop();

    updateSceneIndex(scene);
    initPlayerScreen();

    if (playMode == PlayAll) {
        calculateFramesTotal();
        renderAllScenes();
    }
}

void TupScreen::calculateFramesTotal()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::calculateFramesTotal()]";
    #endif

    projectFramesTotal = 0;
    int scenesTotal = animationList.count();
    for (int i=0; i<scenesTotal; i++)
        projectFramesTotal += project->sceneAt(i)->framesCount();
}

// Clean a photogram array if the scene has changed
void TupScreen::resetSceneFromList(int scene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::resetSceneFromList()]";
    #endif

    if (scene > -1) {
        if (sceneIsRendered.at(scene)) {
            sceneIsRendered.replace(scene, false);
            animationList.replace(scene, blankImagesList);
        }
    } else {
        initAllPhotograms();
    }

    resize(screenDimension);
}

void TupScreen::clearPhotograms()
{
    for (int i=0; i<photograms.count(); i++)
        photograms[i] = QImage();
    photograms.clear();
}

void TupScreen::clearAllScenesPhotograms()
{
    sceneIsRendered.clear();
    for (int i=0; i<animationList.count(); i++) {
        for (int j=0; j<animationList[i].count(); j++)
            animationList[i][j] = QImage();
    }
    animationList.clear();
    projectFramesTotal = 0;
}

void TupScreen::releaseAudioResources()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::releaseAudioResources()]";
    #endif

    soundRecords.clear();
    while(!soundPlayer.isEmpty()) {
        QMediaPlayer *player = soundPlayer.takeFirst();
        player->stop();
        player->setMedia(QMediaContent());
        delete player;
        player = nullptr;
    }
}

void TupScreen::initAllPhotograms()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::initAllPhotograms()]";
    #endif

    sceneIsRendered.clear();
    animationList.clear();

    for (int i=0; i < project->scenesCount(); i++) {
         sceneIsRendered.insert(i, false);
         animationList.insert(i, blankImagesList);
    }
}

void TupScreen::setFPS(int speed)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::setFPS()]";
    #endif

    fps = speed;

    if (playDirection == Forward) {
        if (timer->isActive()) {
            timer->stop();
            play();
        }
    } else { // Backward
        if (playBackTimer->isActive()) {
            playBackTimer->stop();
            playBack();
        }
    }
}

void TupScreen::paintEvent(QPaintEvent *)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::paintEvent()]";
    #endif
    */

    if (playMode == OneScene) {
        if (!mute && !renderOn) {
            if (photograms.count() > 1) {
                if (playerIsActive && (playDirection == Forward))
                    playSoundAt(currentFramePosition);
            }
        }

        if (!firstShoot) {
            if (currentFramePosition > -1 && currentFramePosition < photograms.count())
                currentPhotogram = photograms[currentFramePosition];
        } else {
            firstShoot = false;
        }
    } else { // PlayAll mode
        if (!firstShoot) {
            if (projectFramePosition > -1 && projectFramePosition < projectFramesTotal) {
                int sceneFramesTotal = photograms.count();
                if (currentFramePosition < sceneFramesTotal) {
                   currentPhotogram = photograms[currentFramePosition];
                } else { // Moving to next scene
                   if (projectSceneIndex < (animationList.size() - 1)) {
                       projectSceneIndex++;
                       photograms = animationList.at(projectSceneIndex);
                       currentFramePosition = 0;
                       currentPhotogram = photograms[0];
                   }
                }
            }
        } else {
            firstShoot = false;
        }
    }

    QPainter painter;
    if (painter.begin(this)) {
        if (!currentPhotogram.isNull()) {
            painter.drawImage(imagePos, currentPhotogram);
        } else {
            #ifdef TUP_DEBUG
                QString msg = "[TupScreen::paintEvent()] - Photogram is NULL (index: "
                              + QString::number(currentFramePosition) + "/"
                              + QString::number(photograms.count()) + ")";
                qWarning() << msg;
            #endif
        }
    }

    // SQA: Border for the player. Useful for some tests
    // painter.setPen(QPen(Qt::gray, 0.5, Qt::SolidLine));
    // painter.drawRect(x, y, currentPhotogram.size().width()-1, k->renderCamera.size().height()-1);
}

void TupScreen::play()
{
    #ifdef TUP_DEBUG
        qWarning() << "[TupScreen::play()] - Playing at " << fps << " FPS";
        qWarning() << "[TupScreen::play()] - playMode ->" << playMode;
    #endif

    if (playDirection == Backward) {
        playDirection = Forward;
        if (playBackTimer->isActive())
                playBackTimer->stop();
    }

    playerIsActive = true;
    currentFramePosition = 0;

    if (playMode == OneScene) {
        if (!timer->isActive()) {
            if (!sceneIsRendered.at(sceneIndex))
                renderScene(sceneIndex);

            // No frames to play
            if (photograms.count() == 1)
                return;
        }
    } else { // PlayAll mode
        renderAllScenes();
        projectSceneIndex = 0;
        photograms = animationList.at(0);
    }

    if (!timer->isActive())
        timer->start(1000 / fps);
}

void TupScreen::playBack()
{
    #ifdef TUP_DEBUG
        qWarning() << "[TupScreen::playBack()] - Starting procedure...";
    #endif

    if (playDirection == Forward) {
        stopSounds();

        playDirection = Backward;
        if (timer->isActive())
            timer->stop();
    }

    playerIsActive = true;

    if (playMode == OneScene) {
        if (photograms.count() == 1)
            return;

        currentFramePosition = photograms.count() - 1;

        if (!playBackTimer->isActive()) {
            if (!sceneIsRendered.at(sceneIndex)) {
                QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
                renderScene(sceneIndex);
                QApplication::restoreOverrideCursor();
            }
        }
    } else { // PlayAll mode
        renderAllScenes();
        projectSceneIndex = animationList.count() - 1;
        photograms = animationList.at(projectSceneIndex);
        currentFramePosition = photograms.count() - 1;
    }

    playBackTimer->start(1000 / fps);
}

bool TupScreen::isPlaying()
{
    return playerIsActive;
}

PlayDirection TupScreen::getPlayDirection()
{
    return playDirection;
}

void TupScreen::pause()
{
    if (playerIsActive) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupScreen::pause()] - Pausing player!";
        #endif
        stopAnimation();
    } else {
        if (playMode == OneScene) {
            if (photograms.isEmpty())
                renderScene(sceneIndex);

            // No frames to play
            if (photograms.count() == 1)
                return;

            playerIsActive = true;
            if (playDirection == Forward)
                timer->start(1000 / fps);
            else
                playBackTimer->start(1000 / fps);
        } else { // Play All
            playerIsActive = true;
            if (playDirection == Forward)
                timer->start(1000 / fps);
            else
                playBackTimer->start(1000 / fps);
        }        
        #ifdef TUP_DEBUG
            qWarning() << "[TupScreen::pause()] - Playing animation!";
        #endif
    }
}

void TupScreen::stop()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::stop()] - Stopping player!";
        qDebug() << "[TupScreen::stop()] - playMode -> " << playDirection;
    #endif

    stopAnimation();

    if (playMode == OneScene) {
        if (playDirection == Forward)
            currentFramePosition = 0;
        else
            currentFramePosition = photograms.count() - 1;

        if (currentFramePosition == 0)
            emit frameChanged(1);
        else
            emit frameChanged(currentFramePosition);
    } else { // Play All
        if (playDirection == Forward)
            projectFramePosition = 0;
        else
            projectFramePosition = projectFramesTotal - 1;

        if (projectFramePosition == 0)
            emit frameChanged(1);
        else
            emit frameChanged(projectFramePosition);
    }

    repaint();
}

void TupScreen::stopAnimation()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::stopAnimation()] - playMode -> " << playDirection;
    #endif

    playerIsActive = false;

    if (playDirection == Forward) {
        stopSounds();

        if (timer) {
            if (timer->isActive())
                timer->stop();
        }
    } else {
        if (playBackTimer) {
            if (playBackTimer->isActive())
                playBackTimer->stop();
        }
    }

    emit playerStopped();
}

void TupScreen::nextFrame()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::nextFrame()]";
    #endif

    if (playMode == OneScene) {
        if (playerIsActive)
            stopAnimation();

        if (!sceneIsRendered.at(sceneIndex))
            renderScene(sceneIndex);

        currentFramePosition += 1;

        if (currentFramePosition == photograms.count())
            currentFramePosition = 0;

        emit frameChanged(currentFramePosition + 1);

        repaint();
    } else { // Play All

    }
}

void TupScreen::previousFrame()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::previousFrame()]";
    #endif

    if (playMode == OneScene) {
        if (playerIsActive)
            stopAnimation();

        if (!sceneIsRendered.at(sceneIndex))
            renderScene(sceneIndex);

        currentFramePosition -= 1;

        if (currentFramePosition < 0)
            currentFramePosition = photograms.count() - 1;

        emit frameChanged(currentFramePosition + 1);

        repaint();
    } else { // Play All

    }
}

void TupScreen::advance()
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::advance()]";
    #endif
    */

    if (playMode == OneScene) {
        if (cyclicAnimation && currentFramePosition >= photograms.count()) {
            currentFramePosition = -1;
            stopSounds();
        }

        if (currentFramePosition < photograms.count()) {
            repaint();
            currentFramePosition++;
            emit frameChanged(currentFramePosition);
        } else if (!cyclicAnimation) {
            stop();
        }
    } else { // Play All
        if (cyclicAnimation && projectFramePosition >= projectFramesTotal) {
            projectFramePosition = -1;
            currentFramePosition = -1;
            projectSceneIndex = 0;
            photograms = animationList.at(0);

            stopSounds();
        }

        if (projectFramePosition < projectFramesTotal) {
            repaint();
            currentFramePosition++;
            projectFramePosition++;
            emit frameChanged(projectFramePosition);
        } else if (!cyclicAnimation) {
            stop();
        }
    }
}

void TupScreen::back()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::back()]";
    #endif

    if (playMode == OneScene) {
        if (cyclicAnimation && currentFramePosition < 0)
            currentFramePosition = photograms.count() - 1;

        if (currentFramePosition >= 0) {
            repaint();
            currentFramePosition--;
        } else if (!cyclicAnimation) {
            stop();
        }
    } else { // PlayAll mode
        if (cyclicAnimation && projectFramePosition < 0)
            projectFramePosition = projectFramesTotal - 1;

        if (projectFramePosition >= 0) {
            repaint();
            projectFramePosition--;
        } else if (!cyclicAnimation) {
            stop();
        }
    }
}

void TupScreen::frameResponse(TupFrameResponse *)
{
}

void TupScreen::layerResponse(TupLayerResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::layerResponse()] - response->getAction() -> " << response->getAction();
    #endif

    switch (response->getAction()) {
        case TupProjectRequest::AddLipSync:
        case TupProjectRequest::UpdateLipSync:
        {
            #ifdef TUP_DEBUG
                qDebug() << "[TupScreen::layerResponse()] - Lipsync call. Updating sound records...";
            #endif
            loadSoundRecords();
        }
        break;
    }
}

void TupScreen::sceneResponse(TupSceneResponse *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::sceneResponse()]";
    #endif

    int index = event->getSceneIndex();

    switch (event->getAction()) {
        case TupProjectRequest::Add:
          {
              addPhotogramsArray(index);
              calculateFramesTotal();
          }
        break;
        case TupProjectRequest::Remove:
          {
              if (index < 0)
                  break;

              sceneIsRendered.removeAt(index);
              animationList.removeAt(index);
              calculateFramesTotal();

              if (index == project->scenesCount())
                  index--;

              updateSceneIndex(index);
          }
        break;
        case TupProjectRequest::Reset:
          {
              sceneIsRendered.replace(index, false);
              animationList.replace(index, blankImagesList);

              clearPhotograms();
              photograms = blankImagesList;
          }
        break;
        case TupProjectRequest::Select:
          {
              updateSceneIndex(index);
          }
        break;
        default: 
        break;
    }
}

void TupScreen::projectResponse(TupProjectResponse *)
{
}

void TupScreen::itemResponse(TupItemResponse *)
{
}

void TupScreen::libraryResponse(TupLibraryResponse *response)
{
    #ifdef TUP_DEBUG
       qDebug() << "[TupScreen::libraryResponse()]";
    #endif

    Q_UNUSED(response)
}

void TupScreen::renderAllScenes()
{
    for (int i=0; i < project->scenesCount(); i++) {
        if (!sceneIsRendered.at(i))
            renderScene(i);
    }
}

void TupScreen::renderScene(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::renderScene(index)] - scene index ->" << index;
    #endif

    renderOn = true;
    emit isRendering(0);

    TupScene *scene = project->sceneAt(index);
    if (scene) {
        clearPhotograms();

        renderer = new TupAnimationRenderer(library);
        renderer->setScene(scene, project->getDimension(), scene->getBgColor());
        int i = 1;
        while (renderer->nextPhotogram()) {
            renderized = QImage(project->getDimension(), QImage::Format_RGB32);
            painter = new QPainter(&renderized);
            painter->setRenderHint(QPainter::Antialiasing);

            renderer->render(painter);
            painter->end();
            painter = nullptr;
            delete painter;

            if (isScaled)
                photograms << renderized.scaledToWidth(screenDimension.width(), Qt::SmoothTransformation);
            else
                photograms << renderized;

            emit isRendering(i);
            i++;
        }

        animationList.replace(index, photograms);
        sceneIsRendered.replace(index, true);

        renderer = nullptr;
        delete renderer;
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupScreen::renderScene()] - Fatal Error: Scene is NULL! - index ->" << index;
        #endif
    }

    emit isRendering(0); 
    renderOn = false;
}

QSize TupScreen::sizeHint() const
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::sizeHint()]";
    #endif

    return currentPhotogram.size();
}

void TupScreen::resizeEvent(QResizeEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::resizeEvent()]";
    #endif

    if (sceneIndex > -1) {
        currentFramePosition = 0;
        clearPhotograms();
        if (playMode == OneScene)
            photograms = animationList.at(sceneIndex);
        else
            qDebug() << "PlayAll mode...";
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupScreen::resizeEvent()] - "
                          "Error: Current index is invalid -> " << sceneIndex;
        #endif
    }

    QFrame::resizeEvent(event);
}

void TupScreen::setLoop(bool loop)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::setLoop()]";
    #endif

    cyclicAnimation = loop;
}

void TupScreen::updateSceneIndex(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::updateSceneIndex()]";
    #endif

    sceneIndex = index;
    if (sceneIndex > -1 && sceneIndex < animationList.count()) {
        currentFramePosition = 0;
        clearPhotograms();
        photograms = animationList.at(sceneIndex);
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupScreen::updateSceneIndex()] - "
                          "Error: Can't set current photogram array -> " << sceneIndex;
        #endif
    }
}

int TupScreen::getCurrentSceneIndex()
{
    return sceneIndex;
}

TupScene *TupScreen::getCurrentScene()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::currentScene()]";
    #endif

    if (sceneIndex > -1) {
        return project->sceneAt(sceneIndex);
    } else {
        if (project->scenesCount() == 1) {
            sceneIndex = 0;
            return project->sceneAt(0);
        } 
    }

    return nullptr;
}

int TupScreen::sceneTotalFrames()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::sceneTotalFrames()]";
    #endif

    TupScene *scene;
    if (sceneIndex > -1) {
        scene = project->sceneAt(sceneIndex);
        if (scene)
            return scene->photogramsTotal();
    } else {
        if (project->scenesCount() == 1) {
            sceneIndex = 0;
            scene = project->sceneAt(0);
            return scene->photogramsTotal();
        }
    }

    return 0;
}

// Update and paint the first image of the current scene
void TupScreen::initPlayerScreen()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::initPlayerScreen()]";
    #endif

    if (playMode == OneScene) {
        if (sceneIndex > -1 && sceneIndex < animationList.count()) {
            currentFramePosition = 0;
            clearPhotograms();
            photograms = animationList.at(sceneIndex);
            updateFirstFrame();
            update();
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupScreen::initPlayerScreen()] - "
                              "Fatal Error: Can't access to scene index ->" << sceneIndex;
            #endif
        }
    } else { // PlayAll mode
        sceneIndex = 0;
        currentFramePosition = 0;
        projectFramePosition = 0;
        projectSceneIndex = 0;

        clearPhotograms();
        photograms = animationList.at(sceneIndex);
        updateFirstFrame();
        update();
    }
}

// Prepare the first photogram of the current scene to be painted
void TupScreen::updateFirstFrame()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::updateFirstFrame()]";
    #endif

    if (sceneIndex > -1 && sceneIndex < animationList.count()) {
        TupScene *scene = project->sceneAt(sceneIndex);
        if (scene) {
            loadSoundRecords();

            renderer = new TupAnimationRenderer(library);
            renderer->setScene(scene, project->getDimension(), scene->getBgColor());
            renderer->renderPhotogram(0);

            renderized = QImage(project->getDimension(), QImage::Format_RGB32);

            QPainter *painter = new QPainter(&renderized);
            painter->setRenderHint(QPainter::Antialiasing);
            renderer->render(painter);

            if (isScaled)
                currentPhotogram = renderized.scaledToWidth(screenDimension.width(),
                                                            Qt::SmoothTransformation);
            else
                currentPhotogram = renderized;

            int x = (frameSize().width() - currentPhotogram.size().width()) / 2;
            int y = (frameSize().height() - currentPhotogram.size().height()) / 2;
            imagePos = QPoint(x, y);

            firstShoot = true;

            delete painter;
            painter = nullptr;

            delete renderer;
            renderer = nullptr;
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupScreen::updateFirstFrame()] - "
                              "Fatal Error: Null scene at index -> " << sceneIndex;
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupScreen::updateFirstFrame()] - "
                          "Fatal Error: Can't access to scene index -> " << sceneIndex;
        #endif
    }
}

void TupScreen::addPhotogramsArray(int scene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::addPhotogramsArray()]";
    #endif

    if (scene > -1) {
        sceneIsRendered.insert(scene, false);
        animationList.insert(scene, blankImagesList);
    }
}

void TupScreen::loadSoundRecords()
{
    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupScreen::loadSoundRecords()]";
    #endif

    releaseAudioResources();

    // Loading effect sounds
    QList<SoundResource> effectsList = project->soundResourcesList();
    int total = effectsList.count();

    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::loadSoundRecords()] - Loading sound effects...";
    #endif

    for (int i=0; i<total; i++)  {
        SoundResource sound = effectsList.at(i);
        soundRecords << sound;
        #ifdef TUP_DEBUG
            qDebug() << "[TupScreen::loadSoundRecords()] - Audio loaded! -> " << sound.path;
            qDebug() << "[TupScreen::loadSoundRecords()] - Audio frame -> " << sound.frame;
        #endif
        soundPlayer << new QMediaPlayer();
    }

    #ifdef TUP_DEBUG
        qDebug() << "*** Sound items total -> " << soundRecords.size();
        qDebug() << "---";
    #endif
}

void TupScreen::playSoundAt(int frame)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::playSoundAt()] - frame -> " << frame;
    #endif
    */

    if (playMode) {
        int size = soundRecords.count();
        for (int i=0; i<size; i++) {
            SoundResource soundRecord = soundRecords.at(i);
            if (!soundRecord.muted) {
                if (frame == (soundRecord.frame - 1)) {
                    if (i < soundPlayer.count()) {
                        if (soundPlayer.at(i)->state() != QMediaPlayer::PlayingState) {
                            #ifdef TUP_DEBUG
                                qWarning() << "[TupScreen::playSoundAt()] - Playing file -> " << soundRecord.path;
                                qWarning() << "[TupScreen::playSoundAt()] - frame -> " << frame;
                            #endif
                            soundPlayer.at(i)->setMedia(QUrl::fromLocalFile(soundRecord.path));
                            soundPlayer.at(i)->play();
                        }
                    } else {
                        #ifdef TUP_DEBUG
                            qWarning() << "[TupScreen::playSoundAt()] - Fatal Error: "
                            "No sound file was found at -> " << soundRecord.path;
                        #endif
                    }
                }
            } else {
                #ifdef TUP_DEBUG
                    qWarning() << "[TupScreen::playSoundAt()] - "
                                  "Sound file is muted -> " << soundRecord.path;
                #endif
            }
        }
    } else { // PlayAll mode

    }
}

void TupScreen::enableMute(bool flag)
{
    mute = flag;

    if (mute) {
        stopSounds();
    } else {
       if (playerIsActive) {
           stop();
           play();
       }
    }
}

void TupScreen::stopSounds()
{
    int size = soundRecords.count();
    for (int i=0; i<size; i++)
        soundPlayer.at(i)->stop();
}

void TupScreen::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    emit activePause();
}

bool TupScreen::removeSoundTrack(const QString &soundKey)
{
    int size = soundRecords.count();
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::removeSoundTrack()] - soundKey -> " << soundKey;
        qDebug() << "[TupScreen::removeSoundTrack()] - sounds list size -> " << size;
    #endif

    for (int i=0; i<size; i++) {
        SoundResource soundRecord = soundRecords.at(i);
        if (soundKey.compare(soundRecord.key) == 0) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupScreen::removeSoundTrack()] - "
                            "Found! Sound resource path -> " << soundRecord.path;
            #endif

            soundRecords.takeAt(i);
            QMediaPlayer *player = soundPlayer.takeAt(i);
            player->stop();
            player->setMedia(QMediaContent());
            delete player;
            player = nullptr;

            return true;
        }
    }

    return false;
}
