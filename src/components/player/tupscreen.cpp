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
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen()]";
        #else
            TINIT;
        #endif
    #endif

    project = work;
    library = work->library();
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
        #ifdef Q_OS_WIN
            qDebug() << "[~TupScreen()]";
        #else
            TEND;
        #endif
    #endif

    timer->stop();
    playBackTimer->stop();

    newList.clear();
    photograms.clear();
    animationList.clear();
    // k->sounds.clear();
    renderControl.clear();

    // delete soundPlayer;
    soundPlayer.clear();

    delete timer;
    timer = nullptr;
    delete playBackTimer;
    playBackTimer = nullptr;
    delete renderer;
    renderer = nullptr;

    // delete k;
}

// Clean a photogram array if the scene has changed
void TupScreen::resetPhotograms(int scene)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen::resetPhotograms()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (scene > -1) {
        if (!renderControl.isEmpty()) {
            if (renderControl.at(scene)) {
                renderControl.replace(scene, false);
                animationList.replace(scene, newList);
            }
        }
    } else {
        initPhotogramsArray();
    }

    resize(screenDimension);
}

void TupScreen::initPhotogramsArray()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen::initPhotogramsArray()]";
        #else
            T_FUNCINFO;
        #endif
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
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen::setFPS()]";
        #else
            T_FUNCINFO;
        #endif
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
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen::paintEvent()]";
        #else
            T_FUNCINFO;
        #endif
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
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
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
        QString msg = "TupScreen::play() - Playing at " + QString::number(fps) + " FPS";
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning("camera") << msg;
        #endif
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
        QString msg = "TupScreen::playBack() - Starting procedure...";
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning("camera") << msg;
        #endif
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
        QString msg = "TupScreen::pause() - Pausing player!";
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning("camera") << msg;
        #endif
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
        QString msg = "TupScreen::stop() - Stopping player!";
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning("camera") << msg;
        #endif
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
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen::nextFrame()]";
        #else
            T_FUNCINFO;
        #endif
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
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen::previousFrame()]";
        #else
            T_FUNCINFO;
        #endif
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
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen::advance()]";
        #else
            T_FUNCINFO;
        #endif
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
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen::back()]";
        #else
            T_FUNCINFO;
        #endif
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
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen::sceneResponse()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    int index = event->sceneIndex();

    switch (event->action()) {
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
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen::render()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    emit isRendering(0);

    TupScene *scene = project->sceneAt(sceneIndex);
    if (!scene) {
        #ifdef TUP_DEBUG
            QString msg = "TupScreen::render() - [ Fatal Error ] - Scene is NULL! -> index: " + QString::number(sceneIndex);
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
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

    photograms.clear();

    renderer = new TupAnimationRenderer(project->bgColor(), library);
    renderer->setScene(scene, project->dimension());
    int i = 1;
    while (renderer->nextPhotogram()) {
        renderized = QImage(project->dimension(), QImage::Format_RGB32);
        painter = new QPainter(&renderized);
        painter->setRenderHint(QPainter::Antialiasing);

        renderer->render(painter);
        delete painter;
        painter = nullptr;

        if (isScaled)
            photograms << renderized.scaledToWidth(screenDimension.width(), Qt::SmoothTransformation);
        else
            photograms << renderized;

        emit isRendering(i); 
        i++;
    }

    animationList.replace(sceneIndex, photograms);
    renderControl.replace(sceneIndex, true);

    delete renderer;
    renderer = nullptr;

    emit isRendering(0); 
}

QSize TupScreen::sizeHint() const
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen::sizeHint()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    return currentPhotogram.size();
}

void TupScreen::resizeEvent(QResizeEvent *event)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen::resizeEvent()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (sceneIndex > -1) {
        currentFramePosition = 0;
        photograms = animationList.at(sceneIndex);
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupScreen::resizeEvent() - [ Error ] - Current index is invalid -> " + QString::number(sceneIndex);
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }

    QFrame::resizeEvent(event);
}

void TupScreen::setLoop(bool loop)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen::setLoop()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    cyclicAnimation = loop;
}

void TupScreen::updateSceneIndex(int index)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen::updateSceneIndex()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    sceneIndex = index;
    if (sceneIndex > -1 && sceneIndex < animationList.count()) {
        currentFramePosition = 0;
        photograms = animationList.at(sceneIndex);
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupScreen::updateSceneIndex() - [ Error ] - Can't set current photogram array -> " + QString::number(sceneIndex);
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }
}

int TupScreen::currentSceneIndex()
{
    return sceneIndex;
}

// TupScene *TupScreen::currentScene() const
TupScene *TupScreen::currentScene()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen::currentScene()]";
        #else
            T_FUNCINFO;
        #endif
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
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen::updateAnimationArea()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (sceneIndex > -1 && sceneIndex < animationList.count()) {
        currentFramePosition = 0;
        photograms = animationList.at(sceneIndex);
        updateFirstFrame();
        update();
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupScreen::updateAnimationArea() - [ Fatal Error ] - Can't access to scene index: "
                          + QString::number(sceneIndex);
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }
}

// Prepare the first photogram of the current scene to be painted
void TupScreen::updateFirstFrame()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen::updateFirstFrame()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (sceneIndex > -1 && sceneIndex < animationList.count()) {
        TupScene *scene = project->sceneAt(sceneIndex);
        if (scene) { 
            loadSoundRecords();

            renderer = new TupAnimationRenderer(project->bgColor(), library);
            renderer->setScene(scene, project->dimension());
            renderer->renderPhotogram(0);

            renderized = QImage(project->dimension(), QImage::Format_RGB32);

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
                QString msg = "TupScreen::updateFirstFrame() - [ Fatal Error ] - Null scene at index: " + QString::number(sceneIndex);
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupScreen::updateFirstFrame() - [ Fatal Error ] - Can't access to scene index: " + QString::number(sceneIndex);
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }
}

void TupScreen::addPhotogramsArray(int scene)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen::addPhotogramsArray()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (scene > -1) {
        renderControl.insert(scene, false);
        animationList.insert(scene, newList);
    }
}

void TupScreen::loadSoundRecords()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupScreen::loadSoundRecords()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    soundRecords.clear();
    soundPlayer.clear();

    TupScene *scene = project->sceneAt(sceneIndex);
    // Loading lipSync sounds
    if (scene) {
        #ifdef TUP_DEBUG
            QString msg1 = "TupScreen::loadSoundRecords() - Loading lip-sync files...";
            QString msg2 = "Total -> " + QString::number(scene->lipSyncTotal());
            #ifdef Q_OS_WIN
                qDebug() << msg1;
                qDebug() << msg2;
            #else
                tWarning() << msg1;
                tWarning() << msg2;
            #endif
        #endif
        if (scene->lipSyncTotal() > 0) {
            soundRecords.clear();
            Mouths mouths = scene->getLipSyncList();
            int i = 0;
            foreach(TupLipSync *lipsync, mouths) {
                TupLibraryFolder *folder = library->getFolder(lipsync->name());
                if (folder) {
                    TupLibraryObject *sound = folder->getObject(lipsync->soundFile());
                    if (sound) {
                        QPair<int, QString> soundRecord;
                        soundRecord.first = lipsync->initFrame();
                        soundRecord.second = sound->dataPath();

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
    QString msg1 = "TupScreen::loadSoundRecords() - Loading sound effects...";
    QString msg2 = "Files Total -> " + QString::number(size);
    #ifdef Q_OS_WIN
        qDebug() << msg1;
        qDebug() << msg2;
    #else
        tWarning() << msg1;
        tWarning() << msg2;
    #endif
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
    #ifdef Q_OS_WIN
        qDebug() << "[TupScreen::playSoundAt()]";
    #else
        T_FUNCINFO << "frame -> " << frame;
    #endif
#endif
*/
    int size = soundRecords.count();
    for (int i=0; i<size; i++) {
        QPair<int, QString> soundRecord = soundRecords.at(i);
        if (frame == soundRecord.first) {
            if (i < soundPlayer.count()) {
                #ifdef TUP_DEBUG
                    QString msg = "TupScreen::playSoundAt() - Playing file -> " + soundRecord.second;
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tWarning() << msg;
                    #endif
                #endif
                soundPlayer.at(i)->setMedia(QUrl::fromLocalFile(soundRecord.second));
                soundPlayer.at(i)->play();
            } else { 
                #ifdef TUP_DEBUG
                    QString msg = "TupScreen::playSoundAt() -  Fatal Error: "
					"Not sound file was found at -> " + soundRecord.second;
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
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
