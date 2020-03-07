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

#ifndef TUPGRAPHICSSCENE_H
#define TUPGRAPHICSSCENE_H

#include "tglobal.h"
#include "tuptoolplugin.h"
#include "tupsvgitem.h"
#include "tupproject.h"
#include "tupprojectresponse.h"
#include "tupinputdeviceinformation.h"
#include "tupguideline.h"
#include "tupbrushmanager.h"
#include "tupscene.h"
#include "tuplayer.h"
#include "tupframe.h"
#include "tupgraphicobject.h"
#include "tuplibrary.h"

#include <QGraphicsScene>
#include <QGraphicsItem>

class TUPITUBE_EXPORT TupGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

    public:
        enum Context { Current = 1, Previous, Next };

        TupGraphicsScene();
        ~TupGraphicsScene();

        void setCurrentFrame(int layer, int frame);
        
        void setCurrentScene(TupScene *currentScene);
        void drawCurrentPhotogram();
        
        void drawPhotogram(int photogram, bool drawContext);
        void drawSceneBackground(int photogram);
        void drawVectorFg();

        void cleanWorkSpace();
        void removeScene();
        int currentFrameIndex() const;
        int currentLayerIndex() const;
        int currentSceneIndex() const;
        
        void setNextOnionSkinCount(int n);
        void setPreviousOnionSkinCount(int n);

        void updateLayerVisibility(int layerIndex, bool visible);
        
        TupScene *currentScene() const;
        
        TupFrame *currentFrame();
        
        void setTool(TupToolPlugin *tool);
        TupToolPlugin *currentTool() const;
        
        bool userIsDrawing() const;
        
        TupBrushManager *getBrushManager() const;
        
        void itemResponse(TupItemResponse *event);
        void frameResponse(TupFrameResponse *event);
        void layerResponse(TupLayerResponse *event);
        void sceneResponse(TupSceneResponse *event);
        
        void mouseMoved(QGraphicsSceneMouseEvent *event);
        void mouseReleased(QGraphicsSceneMouseEvent *event);

        void setSelectionRange();
        void enableItemsForSelection();

        void includeObject(QGraphicsItem *object, bool isPolyLine = false);

        TupProject::Mode getSpaceContext();
        void setSpaceMode(TupProject::Mode mode);

        void setOnionFactor(double getOpacity);
        double getOpacity();

        int getFramesCount();

        void setLibrary(TupLibrary *library);
        void resetCurrentTool(); 
        TupInputDeviceInformation * inputDeviceInformation();

        void updateLoadingFlag(bool flag);

    // private slots:
    //  void updateObjectInformation(const QString &value);
    //  void showInfoWidget();
    //  void updateAliveObject();

    signals:
        void showInfoWidget();

    private:
        void addFrame(TupFrame *frame, double opacity = 1.0, Context mode = Current);
        void addGraphicObject(TupGraphicObject *object, TupFrame::FrameType frameType, double opacity = 1.0, bool tweenInAdvance = false);
        void processNativeObject(TupGraphicObject *object, TupFrame::FrameType frameType, double opacity, Context mode);
        void processSVGObject(TupSvgItem *svg, TupFrame::FrameType frameType, double opacity, Context mode);

        void addSvgObject(TupSvgItem *svgItem, TupFrame::FrameType frameType, double opacity = 1.0, bool tweenInAdvance = false);
        void addTweeningObjects(int indexLayer, int photogram, double opacity = 1, bool onProcess = true);
        void addSvgTweeningObjects(int indexLayer, int photogram, double opacity = 1, bool onProcess = true);
        void addLipSyncObjects(TupLayer *layer, int photogram, int zLevel);

        void drawVectorStaticBg(int index);
        void drawVectorDynamicBg();
        void drawVectorDynamicBgOnMovement(int index, int photogram);

        void drawRasterStaticBg(int index);
        // void drawRasterDynamicBg(int index);
        void drawRasterDynamicBgOnMovement(int index, int photogram);

    protected:
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void keyPressEvent(QKeyEvent *keyEvent);
        virtual void keyReleaseEvent(QKeyEvent *keyEvent);
       
        /* 
        virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent * event);
        virtual void dragEnterEvent(QGraphicsSceneDragDropEvent * event);
        virtual void dragMoveEvent(QGraphicsSceneDragDropEvent * event);
        virtual void dropEvent(QGraphicsSceneDragDropEvent * event);
        */
        
        virtual bool event(QEvent *event);

    private:
        TupToolPlugin *gTool;
        TupScene *tupScene;
        TupBackground *background;
        // QPixmap dynamicPixmap;
        double gOpacity;

        struct OnionSkin
         {
            int previous;
            int next;
            QHash<QGraphicsItem *, bool> accessMap;
         } onionSkin;

        struct FramePosition
         {
            int layer;
            int frame;
         } framePosition;

        TupBrushManager *brushManager;
        TupInputDeviceInformation *inputInformation;

        bool isDrawing;

        QList<TupLineGuide *> lines;
        TupProject::Mode spaceContext;
        TupLibrary *library;
        double layerOnProcessOpacity;
        int frameOnProcess;
        int layerOnProcess;

        int zLevel;

        bool loadingProject;
        QGraphicsPixmapItem *vectorDynamicBg;
        QGraphicsPixmapItem *rasterStaticBg;
        QGraphicsPixmapItem *rasterDynamicBg;
};

#endif
