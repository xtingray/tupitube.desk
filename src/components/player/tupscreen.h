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

#ifndef TUPSCREEN_H
#define TUPSCREEN_H

#include "tglobal.h"
#include "tupscene.h"
#include "tupabstractprojectresponsehandler.h"
#include "tupprojectresponse.h"
#include "tupanimationrenderer.h"

#include <QImage>
#include <QPainter>
#include <QPaintEvent>
#include <QFrame>
#include <QTimer>
#include <QMediaPlayer>

class TUPITUBE_EXPORT TupScreen : public QFrame, public TupAbstractProjectResponseHandler
{
    Q_OBJECT

    public:    
        TupScreen(TupProject *project, const QSize viewSize = QSize(),
                  bool isScaled = false, QWidget *parent = nullptr);
        ~TupScreen();

        QSize sizeHint() const;
        void setLoop(bool l);
        void updateSceneIndex(int index);
        TupScene *getCurrentScene();
        int getCurrentSceneIndex();
        int sceneFramesTotal();
        void setFPS(int fps);
        void resetSceneFromList(int sceneIndex);
        void initPlayerScreen();
        void loadSoundRecords();
        bool isPlaying();
        void clearPhotograms();
        void clearAllScenesPhotograms();
        void releaseAudioResources();

        void setPlayMode(PlayMode mode, int sceneIndex);
        PlayDirection getPlayDirection();
        bool removeSoundTrack(const QString &soundKey);

    public slots:
        void play();
        void playBack();
        void pause();
        void stop();
        void nextFrame();
        void previousFrame();

    private slots:
        void renderOneScene(int index);
        void renderScene(int index);
        void advance();
        void back();
        void enableMute(bool flag);

    protected:
        void frameResponse(TupFrameResponse *event);
        void layerResponse(TupLayerResponse *event);
        void sceneResponse(TupSceneResponse *event);
        void projectResponse(TupProjectResponse *event);
        void itemResponse(TupItemResponse *event);
        void libraryResponse(TupLibraryResponse *request);
        void mousePressEvent(QMouseEvent *event);

    signals:
        void progressStep(int, int);
        void toStatusBar(const QString &, int);
        void sceneChanged(const TupScene *newScene);
        void requestTriggered(const TupProjectRequest *event);
        void isRendering(int advance);
        void frameChanged(int frame);
        void activePause();
        void playerStopped();

    protected:
        void paintEvent(QPaintEvent *event);
        void resizeEvent(QResizeEvent *event);

    private:
        void stopAnimation();
        void initAllPhotograms();
        void addPhotogramsEmptyArray(int index);
        void updateFirstFrame();
        void playSoundAt(int frame);
        void stopSounds();
        void renderAllScenes();
        void calculateFramesTotal();

        TupProject *project;
        int currentFramePosition;

        int projectFramePosition;
        int projectFramesTotal;
        int projectSceneIndex;

        int sceneIndex;
        int fps;

        QTimer *timer;
        QTimer *playBackTimer;

        TupAnimationRenderer *renderer;
        QList<QImage> photograms;
        QList<QImage> blankImagesList;

        typedef QList<QImage> photoArray;
        QList<photoArray> animationList;

        QList<bool> sceneIsRendered;
        bool renderOn;
        int progressCounter;
        QSize screenDimension;

        TupLibrary *library;
        QList<SoundResource> soundRecords;
        QList<QMediaPlayer *> soundPlayer;

        bool playerIsActive;
        PlayDirection playDirection;
        PlayMode playMode;

        bool mute;
        bool cyclicAnimation;
        bool isScaled;
        bool firstFrameRendered;

        QPoint imagePos;
        QPainter *painter;
        QImage renderedImg;
        QImage currentPhotogram;
};

#endif
