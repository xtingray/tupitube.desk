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
        qDebug() << "[TupScreen()] - viewSize: " << viewSize;
    #endif

    project = work;
    library = work->getLibrary();
    QList<QPair<int, QString> > effectsList = library->soundEffectList();

    isScaled = sizeChanged;
    screenDimension = viewSize;

    cyclicAnimation = false;
    fps = 24;
    sceneIndex = 0;
    currentFramePosition = 0;

    playerIsActive = false;
    playFlag = true;
    playBackFlag = false;
    mute = false;

    timer = new QTimer(this);
    playBackTimer = new QTimer(this);

    connect(timer, SIGNAL(timeout()), this, SLOT(advance()));
    connect(playBackTimer, SIGNAL(timeout()), this, SLOT(back()));

    initPhotogramsArray();

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

    newList.clear();

    clearPhotograms();
    clearScenesArrays();

    renderControl.clear();
    soundPlayer.clear();

    delete timer;
    timer = nullptr;
    delete playBackTimer;
    playBackTimer = nullptr;
    delete renderer;
    renderer = nullptr;
}

// Clean a photogram array if the scene has changed
void TupScreen::resetSceneFromList(int scene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::resetPhotograms()]";
    #endif

    if (scene > -1) {
        if (renderControl.at(scene)) {
            renderControl.replace(scene, false);
            animationList.replace(scene, newList);
        }
    } else {
        initPhotogramsArray();
    }

    resize(screenDimension);
}

void TupScreen::clearPhotograms()
{
    for (int i=0; i<photograms.count(); i++)
        photograms[i] = QImage();
    photograms.clear();
}

void TupScreen::clearScenesArrays()
{
    renderControl.clear();
    for (int i=0; i<animationList.count(); i++) {
        for (int j=0; j<animationList[i].count(); j++)
            animationList[i][j] = QImage();
    }
    animationList.clear();
}

void TupScreen::initPhotogramsArray()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::initPhotogramsArray()]";
    #endif

    renderControl.clear();
    animationList.clear();

    for (int i=0; i < project->scenesCount(); i++) {
         renderControl.insert(i, false);
         animationList.insert(i, newList);
    }
}

void TupScreen::setFPS(int speed)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::setFPS()]";
    #endif

    fps = speed;

    if (playFlag) {
        if (timer->isActive()) {
            timer->stop();
            play();
        }
    } else {
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

    if (!mute) {
        if (playerIsActive && playFlag)
            playSoundAt(currentFramePosition);
    }

    if (!firstShoot) {
        if (currentFramePosition > -1 && currentFramePosition < photograms.count())
            currentPhotogram = photograms[currentFramePosition];
    } else {
        firstShoot = false;
    }

    QPainter painter;
    if (painter.begin(this)) {
        if (!currentPhotogram.isNull()) {
            painter.drawImage(imagePos, currentPhotogram);
        } else {
            #ifdef TUP_DEBUG
                QString msg = "TupScreen::paintEvent() - Photogram is NULL (index: "
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
        qWarning() << "TupScreen::play() - Playing at " << fps << " FPS";
    #endif

    if (playBackFlag) {
        playBackFlag = false;
        if (playBackTimer->isActive())
            playBackTimer->stop();
    }

    playerIsActive = true;
    playFlag = true;
    currentFramePosition = 0;

    if (!timer->isActive()) {
        if (!renderControl.at(sceneIndex))
            render();

        // No frames to play
        if (photograms.count() == 1)
            return;

        if (renderControl.at(sceneIndex))
            timer->start(1000/fps);
    }
}

void TupScreen::playBack()
{
    #ifdef TUP_DEBUG
        qWarning() << "TupScreen::playBack() - Starting procedure...";
    #endif

    if (photograms.count() == 1)
        return;

    if (playFlag) {
        stopSounds();

        /*
        foreach (TupSoundLayer *sound, k->sounds)
            sound->stop();
        */

        playFlag = false;
        if (timer->isActive())
            timer->stop();
    }

    playerIsActive = true;
    playBackFlag = true;
    currentFramePosition = photograms.count() - 1;

    if (!playBackTimer->isActive()) {
        if (!renderControl.at(sceneIndex)) {
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            render();
            QApplication::restoreOverrideCursor();
        }

        if (renderControl.at(sceneIndex))
            playBackTimer->start(1000 / fps);
    }
}

bool TupScreen::isPlaying()
{
    return playerIsActive;
}

void TupScreen::pause()
{
    #ifdef TUP_DEBUG
        qWarning() << "TupScreen::pause() - Pausing player!";
    #endif

    if (playerIsActive) {
        stopAnimation();
    } else {
        if (photograms.isEmpty())
            render();

        // No frames to play
        if (photograms.count() == 1)
            return;

        playerIsActive = true;
        if (playFlag)
            timer->start(1000 / fps);
        else
            playBackTimer->start(1000 / fps);
    }
}

void TupScreen::stop()
{
    #ifdef TUP_DEBUG
        qWarning() << "TupScreen::stop() - Stopping player!";
    #endif

    stopAnimation();

    if (playFlag)
        currentFramePosition = 0;
    else
        currentFramePosition = photograms.count() - 1;

    if (currentFramePosition == 0)
        emit frameChanged(1);
    else
        emit frameChanged(currentFramePosition);

    repaint();
}

void TupScreen::stopAnimation()
{
    playerIsActive = false;

    if (playFlag) {
        stopSounds();

        if (timer) {
            if (timer->isActive())
                timer->stop();
        }

        // foreach (TupSoundLayer *sound, k->sounds)
        //     sound->stop();
    } else {
        if (playBackTimer) {
            if (playBackTimer->isActive())
                playBackTimer->stop();
        }
    }
}

void TupScreen::nextFrame()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::nextFrame()]";
    #endif

    if (playerIsActive)
        stopAnimation();

    if (!renderControl.at(sceneIndex))
        render();

    currentFramePosition += 1;

    if (currentFramePosition == photograms.count())
        currentFramePosition = 0;

    emit frameChanged(currentFramePosition + 1);

    repaint();
}

void TupScreen::previousFrame()
{
    /* 
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::previousFrame()]";
    #endif
    */

    if (playerIsActive)
        stopAnimation();

    if (!renderControl.at(sceneIndex))
        render();

    currentFramePosition -= 1;

    if (currentFramePosition < 0)
        currentFramePosition = photograms.count() - 1;

    emit frameChanged(currentFramePosition + 1);

    repaint();
}

void TupScreen::advance()
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::advance()]";
    #endif
    */

    if (cyclicAnimation && currentFramePosition >= photograms.count()) {
        currentFramePosition = -1;
        stopSounds();
    }

    /*
    if (currentFramePosition == 0) {
        foreach (TupSoundLayer *sound, k->sounds)
            sound->play();
    }
    */

    if (currentFramePosition < photograms.count()) {
        repaint();
        currentFramePosition++;
        emit frameChanged(currentFramePosition);
    } else if (!cyclicAnimation) {
        stop();
    }
}

void TupScreen::back()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::back()]";
    #endif

    if (cyclicAnimation && currentFramePosition < 0)
        currentFramePosition = photograms.count() - 1;

    if (currentFramePosition >= 0) {
        repaint();
        currentFramePosition--;
    } else if (!cyclicAnimation) {
        stop();
    }
}

void TupScreen::frameResponse(TupFrameResponse *)
{
}

void TupScreen::layerResponse(TupLayerResponse *)
{
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
          }
        break;
        case TupProjectRequest::Remove:
          {
              if (index < 0)
                  break;

              renderControl.removeAt(index);
              animationList.removeAt(index);

              if (index == project->scenesCount())
                  index--;

              updateSceneIndex(index);
          }
        break;
        case TupProjectRequest::Reset:
          {
              renderControl.replace(index, false);
              animationList.replace(index, newList);

              clearPhotograms();
              photograms = newList;
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

void TupScreen::libraryResponse(TupLibraryResponse *)
{
}

void TupScreen::render()
{
    #ifdef TUP_DEBUG
       qDebug() << "[TupScreen::render()]";
    #endif

    emit isRendering(0);

    // TupScene *scene = project->sceneAt(sceneIndex);
    if (!project->sceneAt(sceneIndex)) {
        #ifdef TUP_DEBUG
            QString msg = "TupScreen::render() - [ Fatal Error ] - Scene is NULL! -> index: "
                          + QString::number(sceneIndex);
            qWarning() << msg;
        #endif
        return;
    }

    /* SQA: This code will be required for the sound feature. Do not remove. 
    k->sounds.clear();
    int soundLayersTotal = scene->soundLayers().size();
    for (int i=0; i<soundLayersTotal; i++) {
         TupSoundLayer *layer = scene->soundLayers().at(i);
         k->sounds << layer;
    }
    */

    clearPhotograms();

    renderer = new TupAnimationRenderer(project->getBgColor(), library);
    renderer->setScene(project->sceneAt(sceneIndex), project->getDimension());
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

    animationList.replace(sceneIndex, photograms);
    renderControl.replace(sceneIndex, true);

    renderer = nullptr;
    delete renderer;

    emit isRendering(0); 
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
            qWarning() << "TupScreen::resizeEvent() - [ Error ] - Current index is invalid -> " << sceneIndex;
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
            qWarning() << "TupScreen::updateSceneIndex() - [ Error ] - Can't set current photogram array -> " << sceneIndex;
        #endif
    }
}

int TupScreen::currentSceneIndex()
{
    return sceneIndex;
}

TupScene *TupScreen::currentScene()
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

// Update and paint the first image of the current scene
void TupScreen::updateAnimationArea()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::updateAnimationArea()]";
    #endif

    if (sceneIndex > -1 && sceneIndex < animationList.count()) {
        currentFramePosition = 0;
        clearPhotograms();
        photograms = animationList.at(sceneIndex);
        updateFirstFrame();
        update();
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "TupScreen::updateAnimationArea() - [ Fatal Error ] - Can't access to scene index: "
                       << sceneIndex;
        #endif
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

            renderer = new TupAnimationRenderer(project->getBgColor(), library);
            renderer->setScene(scene, project->getDimension());
            renderer->renderPhotogram(0);

            renderized = QImage(project->getDimension(), QImage::Format_RGB32);

            QPainter *painter = new QPainter(&renderized);
            painter->setRenderHint(QPainter::Antialiasing);
            renderer->render(painter);

            if (isScaled)
                currentPhotogram = renderized.scaledToWidth(screenDimension.width(), Qt::SmoothTransformation);
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
                qWarning() << "TupScreen::updateFirstFrame() - [ Fatal Error ] - Null scene at index: " << sceneIndex;
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "TupScreen::updateFirstFrame() - [ Fatal Error ] - Can't access to scene index: " << sceneIndex;
        #endif
    }
}

void TupScreen::addPhotogramsArray(int scene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::addPhotogramsArray()]";
    #endif

    if (scene > -1) {
        renderControl.insert(scene, false);
        animationList.insert(scene, newList);
    }
}

void TupScreen::loadSoundRecords()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::loadSoundRecords()]";
    #endif

    soundRecords.clear();
    soundPlayer.clear();

    // Loading lipSync sounds
    if (project->sceneAt(sceneIndex)) {
        int total = project->sceneAt(sceneIndex)->lipSyncTotal();
        #ifdef TUP_DEBUG
            qWarning() << "TupScreen::loadSoundRecords() - Loading lip-sync files...";
            qWarning() << "Total -> " << total;
        #endif
        if (total > 0) {
            soundRecords.clear();
            Mouths mouths = project->sceneAt(sceneIndex)->getLipSyncList();
            int i = 0;
            foreach(TupLipSync *lipsync, mouths) {
                TupLibraryFolder *folder = library->getFolder(lipsync->getLipSyncName());
                if (folder) {
                    TupLibraryObject *sound = folder->getObject(lipsync->getSoundFile());
                    if (sound) {
                        QPair<int, QString> soundRecord;
                        soundRecord.first = lipsync->getInitFrame();
                        soundRecord.second = sound->getDataPath();

                        soundRecords << soundRecord;
                        soundPlayer << new QMediaPlayer();
                        i++;
                    }
                }
            }
        }
    }

    QList<QPair<int, QString> > effectsList = library->soundEffectList();
    int size = effectsList.count();

    #ifdef TUP_DEBUG
        qWarning() << "TupScreen::loadSoundRecords() - Loading sound effects...";
        qWarning() << "Files Total -> " << size;
    #endif

    for (int i=0; i<size; i++)  {
        QPair<int, QString> sound = effectsList.at(i);
        soundRecords << sound;
        soundPlayer << new QMediaPlayer();
    }
}

void TupScreen::playSoundAt(int frame)
{
/*
#ifdef TUP_DEBUG
    qDebug() << "[TupScreen::playSoundAt()]";
#endif
*/
    int size = soundRecords.count();
    for (int i=0; i<size; i++) {
        QPair<int, QString> soundRecord = soundRecords.at(i);
        if (frame == soundRecord.first) {
            if (i < soundPlayer.count()) {
                #ifdef TUP_DEBUG
                    qWarning() << "TupScreen::playSoundAt() - Playing file -> " << soundRecord.second;
                #endif
                soundPlayer.at(i)->setMedia(QUrl::fromLocalFile(soundRecord.second));
                soundPlayer.at(i)->play();
            } else { 
                #ifdef TUP_DEBUG
                    qWarning() << "TupScreen::playSoundAt() -  Fatal Error: "
                    "Not sound file was found at -> " << soundRecord.second;
                #endif
            }
        }
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

int TupScreen::currentSceneFrames()
{
    return photograms.count();
}
