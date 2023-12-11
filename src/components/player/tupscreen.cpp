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

    loadSoundRecords();
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
    updateSceneIndex(scene);
    initPlayerScreen();

    if (playMode == PlayAll) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupScreen::setPlayMode()] - Enabling PlayAll mode..";
        #endif
        renderAllScenes();
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupScreen::setPlayMode()] - Enabling OneScene mode..";
        #endif
    }
    play();
}

// Update and paint the first image of the current scene
void TupScreen::initPlayerScreen()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::initPlayerScreen()]";
    #endif

    if (playMode == OneScene) {
        if (sceneIndex > -1 && sceneIndex < animationList.count()) {
            getSoundsForCurrentScene();

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
        getSoundsForProject();

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

void TupScreen::calculateFramesTotal()
{
    projectFramesTotal = 0;
    int scenesTotal = animationList.count();
    for (int i=0; i<scenesTotal; i++)
        projectFramesTotal += project->sceneAt(i)->framesCount();

    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::calculateFramesTotal()] - projectFramesTotal ->" << projectFramesTotal;
    #endif
}

// Clean a photogram array if the scene has changed
void TupScreen::resetSceneFromList(int scene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::resetSceneFromList()] - Resetting scene at index -> " << scene;
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
    soundIndexesForScene.clear();
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

void TupScreen::play()
{
    #ifdef TUP_DEBUG
        qWarning() << "---";
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
                renderOneScene(sceneIndex);

            // No frames to play
            if (photograms.count() == 1) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupScreen::play()] - Scene only has one scene. Leaving!";
                #endif

                return;
            }
        }
    } else { // PlayAll mode
        renderAllScenes();
        projectFramePosition = 0;
        projectSceneIndex = 0;
        photograms = animationList.at(0);
    }

    if (!timer->isActive())
        timer->start(1000 / fps); // Timer frequence: 1 second divided by the frame rate
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
                renderOneScene(sceneIndex);
                QApplication::restoreOverrideCursor();
            }
        }
    } else { // PlayAll mode
        renderAllScenes();

        projectSceneIndex = animationList.count() - 1;
        photograms = animationList.at(projectSceneIndex);
        currentFramePosition = photograms.count() - 1;
        projectFramePosition = projectFramesTotal - 1;
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
                renderOneScene(sceneIndex);

            // No frames to play
            if (photograms.count() == 1)
                return;
        }

        #ifdef TUP_DEBUG
            qWarning() << "[TupScreen::pause()] - Playing animation!";
        #endif

        playerIsActive = true;
        if (playDirection == Forward)
            timer->start(1000 / fps);
        else
            playBackTimer->start(1000 / fps);
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
        if (playDirection == Forward) {
            currentFramePosition = 0;
        } else { // Backward
            currentFramePosition = photograms.count();
            currentPhotogram = photograms.last();
        }

        if (currentFramePosition == 0)
            emit frameChanged(1);
        else
            emit frameChanged(currentFramePosition);
    } else { // Play All
        if (playDirection == Forward) {
            projectFramePosition = 0;
            currentFramePosition = 0;
            projectSceneIndex = 0;
            photograms = animationList.at(projectSceneIndex);
            currentPhotogram = photograms.last();
        } else { // Backward
            projectFramePosition = projectFramesTotal;
            currentFramePosition = projectFramesTotal;
            projectSceneIndex = animationList.count() - 1;
            photograms = animationList.at(projectSceneIndex);
            currentPhotogram = photograms.last();
        }

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

    stopAnimation();

    if (playDirection == Backward)
        playDirection = Forward;

    if (!sceneIsRendered.at(sceneIndex))
        renderOneScene(sceneIndex);

    currentFramePosition++;

    if (playMode == OneScene) {
        if (currentFramePosition == photograms.count())
            currentFramePosition = 0;

        emit frameChanged(currentFramePosition + 1);
    } else { // Play All
        projectFramePosition++;

        if (projectFramePosition >= projectFramesTotal) {
            currentFramePosition = 0;
            projectFramePosition = 0;
            projectSceneIndex = 0;

            photograms = animationList.at(projectSceneIndex);
        }

        emit frameChanged(projectFramePosition + 1);
    }

    repaint();
}

void TupScreen::previousFrame()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::previousFrame()]";
    #endif

    stopAnimation();

    if (playDirection == Forward)
        playDirection = Backward;

    if (!sceneIsRendered.at(sceneIndex))
        renderOneScene(sceneIndex);

    if (playMode == OneScene) {
        currentFramePosition--;

        if (currentFramePosition < 0)
            currentFramePosition = photograms.count() - 1;

        emit frameChanged(currentFramePosition + 1);
    } else { // Play All
        currentFramePosition--;
        projectFramePosition--;

        if (projectFramePosition <= -1) {
            photograms = animationList.last();
            currentFramePosition = photograms.size() - 1;
            projectFramePosition = projectFramesTotal - 1;
            projectSceneIndex = animationList.count() - 1;
        }

        emit frameChanged(projectFramePosition + 1);
    }

    repaint();
}

void TupScreen::advance()
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::advance()]";
    #endif
    */

    if (playMode == OneScene) {
        // If the animation is done and the loop is on, restart the frame position
        if (cyclicAnimation && currentFramePosition == photograms.count() - 1) {
            currentFramePosition = -1; // -1 ensures to repaint last photogram
            stopSounds();
        }

        // If current frame is part of the animation array, paint it
        if (currentFramePosition < photograms.count()) {
            emit frameChanged(currentFramePosition + 2);
            repaint();
            currentFramePosition++;
        } else if (!cyclicAnimation) {
            stop(); // If loop is off, then stop the player
        }
    } else { // Play All
        if (cyclicAnimation && projectFramePosition == projectFramesTotal - 1) {
            projectFramePosition = -1;
            currentFramePosition = -1;
            projectSceneIndex = 0;
            photograms = animationList.at(0);

            stopSounds();
        }

        if (projectFramePosition < projectFramesTotal) {
            emit frameChanged(projectFramePosition + 2);
            repaint();
            currentFramePosition++;
            projectFramePosition++;
        } else if (!cyclicAnimation) {
            stop();
        }
    }
}

void TupScreen::back()
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::back()]";
    #endif
    */

    if (playMode == OneScene) {
        if (cyclicAnimation && currentFramePosition <= 0)
            currentFramePosition = photograms.count();

        if (currentFramePosition >= 0) {
            emit frameChanged(currentFramePosition);
            repaint();
            currentFramePosition--;
        } else if (!cyclicAnimation) {
            stop();
        }
    } else { // PlayAll mode
        if (cyclicAnimation && projectFramePosition < 0) { // Loop must be restarted
            if (projectSceneIndex == 0) { // Start again from the last scene
                projectSceneIndex = animationList.size() - 1;
                photograms = animationList.at(projectSceneIndex);
                currentFramePosition = photograms.count() - 1;
                projectFramePosition = projectFramesTotal - 1;
            }
        }

        if (projectFramePosition < 0) { // Move to the previous scene
            if (!cyclicAnimation) {
                stop();
            } else {
                if (projectSceneIndex > 0)
                    projectSceneIndex--;
                else
                    projectSceneIndex = animationList.size() - 1;

                currentFramePosition = animationList.at(projectSceneIndex).count() - 1;

                photograms = animationList.at(projectSceneIndex);
                currentFramePosition = photograms.size() - 1;
                currentPhotogram = photograms[currentFramePosition];

                repaint();
                currentFramePosition--;
                projectFramePosition--;
            }
        } else {
            emit frameChanged(projectFramePosition + 1);
            repaint();
            currentFramePosition--;
            projectFramePosition--;
        }
    }
}

void TupScreen::paintEvent(QPaintEvent *)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::paintEvent()] - currentFramePosition ->" << currentFramePosition;
    #endif
    */

    if (playMode == OneScene) {
        if (!mute && !renderOn) {
            if ((photograms.count() > 1) && currentSceneGotSounds()) {
                if (playerIsActive && (playDirection == Forward))
                    playSoundsAt(currentFramePosition);
            }
        }

        if (!firstFrameRendered) {
            if (currentFramePosition > -1 && currentFramePosition < photograms.count()) {
                // Assign photogram from array to currentPhotogram
                currentPhotogram = photograms[currentFramePosition];
            }
            // If currentFramePosition == -1 then paint the last photogram once again
        } else {
            // Case 1: First photogram was already rendered and stored in currentPhotogram.
            // It's ready for drawing
            firstFrameRendered = false;
        }
    } else { // PlayAll mode
        if (!mute && !renderOn) {
            if ((photograms.count()) > 1 && !soundRecords.isEmpty()) {
                if (playerIsActive && (playDirection == Forward))
                    playSoundsAt(currentFramePosition);
            }
        }

        if (!firstFrameRendered) {
            if (projectFramePosition > -1 && projectFramePosition < projectFramesTotal) {
                if (playDirection == Forward) {
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
                } else { // Backward
                    if (currentFramePosition >= photograms.count()) {
                        currentPhotogram = photograms.last();
                    } else if (currentFramePosition > -1) {
                        currentPhotogram = photograms[currentFramePosition];
                    } else { // Moving to previous scene
                        if (projectSceneIndex > 0) {
                            projectSceneIndex--;
                            photograms = animationList.at(projectSceneIndex);
                            currentFramePosition = photograms.count() - 1;
                            currentPhotogram = photograms[currentFramePosition];
                        }
                    }
                }
            }
        } else {
            firstFrameRendered = false;
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
    if (index < 0) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupScreen::sceneResponse()] - Invalid scene index ->" << index;
        #endif
        return;
    }

    emit sceneResponseActivated(event->getAction(), index);

    switch (event->getAction()) {
        case TupProjectRequest::Add:
          {
              sceneIndex = index;
              addPhotogramsEmptyArray(index);
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

   switch (response->getAction()) {
       case TupProjectRequest::Add:
       {
            if (response->symbolType() == TupLibraryObject::Folder)
                return;

            QString id = response->getArg().toString();
            #ifdef TUP_DEBUG
                qDebug() << "[TupScreen::libraryResponse()] - response->getArg() ->" << id;
            #endif

            TupLibraryObject *object = library->getObject(id);
            if (object) {
                switch (object->getObjectType()) {
                    case TupLibraryObject::Audio:
                    {
                        loadSoundRecords();
                    }
                    break;
                    default:
                    break;
                }
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupScreen::libraryResponse()] - Fatal Error: Can't find library object ->" << id;
                #endif
            }
       }
       break;
       case TupProjectRequest::Remove:
       {
            loadSoundRecords();
       }
       break;
       default:
       break;
   }
}

void TupScreen::renderAllScenes()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::renderAllScenes()]";
    #endif

    calculateFramesTotal();

    renderOn = true;
    emit isRendering(0);

    progressCounter = 1;
    for (int i=0; i < project->scenesCount(); i++) {
        if (!sceneIsRendered.at(i))
            renderScene(i);
    }

    emit isRendering(0);
    renderOn = false;
}

void TupScreen::renderOneScene(int index) {
    renderOn = true;
    emit isRendering(0);

    progressCounter = 1;
    renderScene(index);

    emit isRendering(0);
    renderOn = false;
}

void TupScreen::renderScene(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::renderScene(index)] - scene index ->" << index;
    #endif

    TupScene *scene = project->sceneAt(index);
    if (scene) {
        clearPhotograms();

        renderer = new TupAnimationRenderer(library);
        renderer->setScene(scene, project->getDimension(), scene->getBgColor());
        while (renderer->nextPhotogram()) {
            renderedImg = QImage(project->getDimension(), QImage::Format_RGB32);
            painter = new QPainter(&renderedImg);
            painter->setRenderHint(QPainter::Antialiasing);

            renderer->render(painter);
            painter->end();
            painter = nullptr;
            delete painter;

            if (isScaled)
                photograms << renderedImg.scaledToWidth(screenDimension.width(), Qt::SmoothTransformation);
            else
                photograms << renderedImg;

            emit isRendering(progressCounter);
            progressCounter++;
        }

        #ifdef TUP_DEBUG
            qDebug() << "[TupScreen::renderScene(index)] - Replacing scene at index ->" << index;
            qDebug() << "[TupScreen::renderScene(index)] - animationList size ->" << animationList.count();
        #endif

        if (index < animationList.count()) {
            animationList.replace(index, photograms);
            sceneIsRendered.replace(index, true);
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupScreen::renderScene(index)] - "
                            "Fatal Error: Can't replace scene at animationList - Invalid index! ->" << index;
            #endif
        }

        renderer = nullptr;
        delete renderer;
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupScreen::renderScene()] - Fatal Error: Scene is NULL! - index ->" << index;
        #endif
    }
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
        photograms = animationList.at(sceneIndex);
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
        qDebug() << "[TupScreen::updateSceneIndex()] - index ->" << index;
    #endif

    sceneIndex = index;
    if (sceneIndex > -1 && sceneIndex < animationList.count()) {
        currentFramePosition = 0;
        clearPhotograms();
        photograms = animationList.at(sceneIndex);
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupScreen::updateSceneIndex()] - "
                          "Error: Can't set current photogram array ->" << sceneIndex;
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

int TupScreen::sceneFramesTotal()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::sceneFramesTotal()]";
    #endif

    if (playMode == OneScene) {
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
    } else { // PlayAll mode
        int framesTotal = 0;
        foreach(TupScene *scene, project->getScenes())
            framesTotal += scene->photogramsTotal();

        return framesTotal;
    }

    return 0;
}

// Prepare the first photogram of the current scene to be painted
void TupScreen::updateFirstFrame()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::updateFirstFrame()] - sceneIndex ->" << sceneIndex;
    #endif

    if (sceneIndex > -1 && sceneIndex < animationList.count()) {
        TupScene *scene = project->sceneAt(sceneIndex);
        if (scene) {
            renderer = new TupAnimationRenderer(library);
            renderer->setScene(scene, project->getDimension(), scene->getBgColor());
            renderer->renderPhotogram(0);

            renderedImg = QImage(project->getDimension(), QImage::Format_RGB32);

            QPainter *painter = new QPainter(&renderedImg);
            painter->setRenderHint(QPainter::Antialiasing);
            renderer->render(painter);

            if (isScaled)
                currentPhotogram = renderedImg.scaledToWidth(screenDimension.width(),
                                                             Qt::SmoothTransformation);
            else
                currentPhotogram = renderedImg;

            int x = (frameSize().width() - currentPhotogram.size().width()) / 2;
            int y = (frameSize().height() - currentPhotogram.size().height()) / 2;
            imagePos = QPoint(x, y);

            firstFrameRendered = true;

            delete painter;
            painter = nullptr;

            delete renderer;
            renderer = nullptr;
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupScreen::updateFirstFrame()] - "
                              "Fatal Error: Null scene at index ->" << sceneIndex;
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupScreen::updateFirstFrame()] - "
                          "Fatal Error: Can't access to scene index ->" << sceneIndex;
        #endif
    }
}

void TupScreen::addPhotogramsEmptyArray(int scene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::addPhotogramsEmptyArray()] - Adding empty scene at index ->" << scene;
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
    QList<SoundResource> effectsList = project->getSoundResourcesList();
    int total = effectsList.count();

    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::loadSoundRecords()] - Loading sound effects ->" << total;
    #endif

    for (int i=0; i<total; i++)  {
        SoundResource sound = effectsList.at(i);
        soundRecords << sound;
        #ifdef TUP_DEBUG
            qDebug() << "---";
            qDebug() << "[TupScreen::loadSoundRecords()] - Audio key ->" <<sound.key;
            qDebug() << "[TupScreen::loadSoundRecords()] - Audio path ->" << sound.path;
            qDebug() << "[TupScreen::loadSoundRecords()] - isBackgroundTrack ->" << sound.isBackgroundTrack;
            qDebug() << "[TupScreen::loadSoundRecords()] - scenes count ->" << sound.scenes.count();
            foreach(SoundScene scene, sound.scenes) {
                qDebug() << "[TupScreen::loadSoundRecords()] - scene index ->" << scene.sceneIndex;
                foreach(int frameIndex, scene.frames)
                    qDebug() << "[TupScreen::loadSoundRecords()] - frame index ->" << frameIndex;
            }
        #endif
        soundPlayer << new QMediaPlayer();
    }

    #ifdef TUP_DEBUG
        qDebug() << "*** Sound items total - soundRecords.size() ->" << soundRecords.size();
        qDebug() << "---";
    #endif
}

bool TupScreen::currentSceneGotSounds()
{
    return !soundIndexesForScene.isEmpty();
}

void TupScreen::getSoundsForCurrentScene()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::getSoundsForCurrentScene()] - soundRecords.size() ->" << soundRecords.size();
        qDebug() << "[TupScreen::getSoundsForCurrentScene()] - sceneIndex ->" << sceneIndex;
    #endif

    soundIndexesForScene.clear();
    soundFramesForScene.clear();
    if (!soundRecords.isEmpty()) {
        for(int i=0; i<soundRecords.size(); i++) {
            SoundResource resource = soundRecords.at(i);
            qDebug() << "---";
            qDebug() << "resource.key ->" << resource.key;
            qDebug() << "resource.path ->" << resource.path;
            if (!resource.scenes.isEmpty() && !resource.muted) {
                foreach(SoundScene scene, resource.scenes) {
                    qDebug() << "   scene.sceneIndex ->" << scene.sceneIndex;
                    qDebug() << "   scene.frames.size() ->" << scene.frames.size();
                    qDebug() << "   scene.frames ->" << scene.frames;
                    if ((scene.sceneIndex == sceneIndex) && (!scene.frames.isEmpty())) {
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupScreen::getSoundsForCurrentScene()] - Found sound at index ->" << i;
                        #endif
                        soundIndexesForScene << i;
                        soundFramesForScene << scene.frames;
                        break;
                    }
                }
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupScreen::getSoundsForCurrentScene()] - Warning: No scenes available for the sound resource!";
                    qDebug() << "[TupScreen::getSoundsForCurrentScene()] - Scenes count ->" << resource.scenes.count();
                    qDebug() << "[TupScreen::getSoundsForCurrentScene()] - resource.muted? ->" << resource.muted;
                #endif
            }
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupScreen::getSoundsForCurrentScene()] - Warning: No sound records!";
        #endif
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::getSoundsForCurrentScene()] - sceneSoundIndexes ->" << soundIndexesForScene;
    #endif
}

void TupScreen::getSoundsForProject()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::getSoundsForProject()] - soundRecords.size() ->" << soundRecords.size();
    #endif

    projectSoundsList.clear();

    if (!soundRecords.isEmpty()) {
        for(int i=0; i<soundRecords.size(); i++) {
            SoundResource resource = soundRecords.at(i);
            qDebug() << "---";
            qDebug() << "resource.key ->" << resource.key;
            qDebug() << "resource.path ->" << resource.path;
            if (!resource.scenes.isEmpty() && !resource.muted) {
                foreach(SoundScene scene, resource.scenes) {
                    if (!scene.frames.isEmpty()) {
                        bool found = false;
                        if (!projectSoundsList.isEmpty()) {
                            for(int j=0; j<projectSoundsList.count(); j++) {
                                ProjectSoundList list = projectSoundsList.at(j);
                                if (list.sceneIndex == scene.sceneIndex) {
                                    qDebug() << "*** Adding item to existing sound list...";
                                    ProjectSoundItem item;
                                    item.playerIndex = i;
                                    item.frames = scene.frames;

                                    qDebug() << "*** scene.frames ->" << scene.frames;
                                    qDebug() << "---";

                                    list.soundItems << item;
                                    projectSoundsList.replace(j, list);

                                    found = true;
                                    break;
                                }
                            }
                        }

                        if (!found) {
                            qDebug() << "*** Adding a new project sound list...";
                            qDebug() << "*** scene.sceneIndex ->" << scene.sceneIndex;
                            ProjectSoundList newList;
                            newList.sceneIndex = scene.sceneIndex;

                            ProjectSoundItem item;
                            item.playerIndex = i;
                            item.frames = scene.frames;

                            qDebug() << "*** scene.frames ->" << scene.frames;
                            qDebug() << "---";

                            newList.soundItems << item;
                            projectSoundsList << newList;
                        }
                    }
                }
            }
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupScreen::getSoundsForProject()] - Warning: No sound records!";
        #endif
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::getSoundsForProject()] - sceneSoundIndexes ->" << soundIndexesForScene;
    #endif
}

void TupScreen::playSoundsAt(int frameIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::playSoundsAt()] - current frame ->" << frameIndex;
    #endif

    if (playMode == OneScene) {
        for(int i=0; i< soundIndexesForScene.size(); i++) {
            int soundIndex = soundIndexesForScene.at(i);
            SoundResource soundRecord = soundRecords.at(soundIndex);
            QList<int> frameIndexes = soundFramesForScene.at(i);
            for(int i=0; i<frameIndexes.count(); i++) {
                int value = frameIndexes.at(i);
                value--;
                frameIndexes.replace(i, value);
            }

            foreach(int frame, frameIndexes) {
                if (frameIndex == frame) {
                    if (i < soundPlayer.count()) {
                        playAudioFile(soundIndex, soundRecord.path);
                    } else {
                        #ifdef TUP_DEBUG
                            qWarning() << "[TupScreen::playSoundsAt()] - Fatal Error: "
                                          "No sound file was found at ->" << soundRecord.path;
                        #endif
                    }
                }
            }
        }
    } else { // PlayAll mode
        qDebug() << "---";
        qDebug() << "[TupScreen::playSoundsAt()] - PlayAll Mode...";
        qDebug() << "[TupScreen::playSoundsAt()] - projectFramePosition ->" << projectFramePosition;
        qDebug() << "[TupScreen::playSoundsAt()] - projectSceneIndex ->" << projectSceneIndex;
        qDebug() << "[TupScreen::playSoundsAt()] - scene frames total ->" << animationList.at(projectSceneIndex).count();

        for(int i=0; i<projectSoundsList.size(); i++) {
            ProjectSoundList list = projectSoundsList.at(i);
            if (list.sceneIndex == projectSceneIndex) {
                if (!list.soundItems.isEmpty()) {
                    QList<ProjectSoundItem> items = list.soundItems;
                    for (int j=0; j<items.size(); j++) {
                         ProjectSoundItem item = items.at(j);
                         QList<int> frameIndexes = item.frames;
                         for(int i=0; i<frameIndexes.count(); i++) {
                            int value = frameIndexes.at(i);
                            value--;
                            frameIndexes.replace(i, value);
                         }
                         foreach(int frame, frameIndexes) {
                            if (frame == frameIndex) {
                                SoundResource resource = soundRecords.at(item.playerIndex);

                                qDebug() << "[TupScreen::playSoundsAt()] - key frame ->" << frame;
                                qDebug() << "[TupScreen::playSoundsAt()] - current frame ->" << frameIndex;
                                qDebug() << "[TupScreen::playSoundsAt()] - playing file ->" << resource.path;

                                playAudioFile(item.playerIndex, resource.path);
                            }
                         }
                    }
                    break;
                }
            }
        }
    }
}

void TupScreen::playAudioFile(int index, QString audioPath)
{
    if (soundPlayer.at(index)->state() != QMediaPlayer::PlayingState) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupScreen::playAudioFile()] - Playing file ->" << audioPath;
        #endif
        soundPlayer.at(index)->setMedia(QUrl::fromLocalFile(audioPath));
        soundPlayer.at(index)->play();
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
        qDebug() << "[TupScreen::removeSoundTrack()] - soundKey ->" << soundKey;
        qDebug() << "[TupScreen::removeSoundTrack()] - sounds list size ->" << size;
    #endif

    for (int i=0; i<size; i++) {
        SoundResource soundRecord = soundRecords.at(i);
        if (soundKey.compare(soundRecord.key) == 0) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupScreen::removeSoundTrack()] - "
                            "Found! Sound resource path ->" << soundRecord.path;
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
