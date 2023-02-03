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

#ifndef TUPPAINTAREA_H
#define TUPPAINTAREA_H

#include "tglobal.h"
#include "tuppaintareabase.h"
#include "tupabstractprojectresponsehandler.h"
#include "tupgraphicsscene.h"
#include "tupbrushmanager.h"
#include "tupinputdeviceinformation.h"
#include "tuppaintarearotator.h"
#include "tupimagedevice.h"
#include "tupgraphicsscene.h"
#include "tconfig.h"
#include "tapplication.h"
#include "tuptextitem.h"
#include "tuplibrarydialog.h"
#include "tuplibraryobject.h"
#include "tuprequestbuilder.h"
#include "tupprojectrequest.h"
#include "tupprojectresponse.h"
#include "tupscene.h"
#include "tuplayer.h"
#include "tupsvgitem.h"
#include "node.h"
#include "tcontrolnode.h"
#include "tupproject.h"

#include <QGraphicsScene>
#include <QMouseEvent>
#include <QGraphicsRectItem>
#include <QPolygon>
#include <QApplication>
#include <QStyleOptionGraphicsItem>
#include <QClipboard>
#include <QMenu>
#include <QNetworkReply>

class TupBrushManager;
class TupInputDeviceInformation;
class TupProject;
class TupPaintAreaRotator;

class TUPITUBE_EXPORT TupPaintArea : public TupPaintAreaBase, public TupAbstractProjectResponseHandler
{
    Q_OBJECT

    public:
        TupPaintArea(TupProject *project, QWidget *parent = nullptr);
        ~TupPaintArea();

        void setCurrentScene(int index);
        void setCurrentTool(TAction::ActionId tool);
        void paintBackground();
        void paintForeground();
        void updateSpaceContext();
        virtual void keyPressEvent(QKeyEvent *event);
        // virtual void keyReleaseEvent(QKeyEvent *event);
        void goOneFrameBack();
        void goOneFrameForward();
        void goToFrame(int frameIndex, int layerIndex, int sceneIndex);
        void goToScene(int sceneIndex);
        void copyCurrentFrame();
        void pasteCurrentFrame();
        void copyFrameForward();
        void removeCurrentFrame();
        int currentSceneIndex();
        int currentLayerIndex();
        int currentFrameIndex();
        void resetWorkSpaceCenter(const QSize projectSize);
        void updateLoadingFlag(bool flag);
        void goOneLayerBack();
        void goOneLayerForward();
        void getLocalAsset(const QString &path);
        void getWebAsset(const QString &webPath);
        void importLocalProject(const QString &path);

    public slots:
        void updatePaintArea();
        void setNextFramesOnionSkinCount(int n);
        void setPreviousFramesOnionSkinCount(int n);
        void setOnionFactor(double value);

    signals:
        void requestTriggered(const TupProjectRequest *event);
        void localRequestTriggered(const TupProjectRequest *event);
        void itemAddedOnSelection(TupGraphicsScene *);
        void frameChanged(int);
        void closePolyLine();
        void closeLine();
        void zoomIn();
        void zoomOut();
        void newPerspective(int index);
        void eyeDropperLaunched();
        void localAssetDropped(const QString &path, TupLibraryObject::ObjectType type);
        void webAssetDropped(const QString &name, const QString &extension,
                             TupLibraryObject::ObjectType type, QByteArray data);
        void libraryAssetDragged();
        void sceneCreated(int index);

    public slots:
        void deleteItems();
        // void ungroupItems();
        void copyItems();
        void pasteItems();
        void pasteNextFive();
        void pasteNextTen();
        void pasteNextTwenty();
        void pasteNextFifty();
        void pasteNextHundred();

        void cutItems();

        void addSelectedItemsToLibrary();
        void requestItemMovement(QAction *action);
        void goToFrame(int index);

    private slots:
        void processWebAsset(QNetworkReply *reply);
        void slotError(QNetworkReply::NetworkError error);

    protected:
        void mousePressEvent(QMouseEvent *event);
        void tabletEvent(QTabletEvent *event);
        void dragEnterEvent(QDragEnterEvent *event);
        void dragMoveEvent(QDragMoveEvent *event);
        void dropEvent(QDropEvent *event);

        void frameResponse(TupFrameResponse *response);
        void layerResponse(TupLayerResponse *response);
        void sceneResponse(TupSceneResponse *response);
        void itemResponse(TupItemResponse *response);
        void libraryResponse(TupLibraryResponse *response);
        void projectResponse(TupProjectResponse *response);

        bool canPaint() const;

    private:
        void multipasteObject(int pasteTotal);

        TupProject *project;
        int globalSceneIndex;
        QStringList copiesXml;
        TAction::ActionId currentToolID;
        bool deleteMode;
        TupProject::Mode spaceMode;
        QPointF copyPosition;
        QList<QPointF> copyCoords;

        QPointF centerCoord;
        QPointF position;
        bool menuOn;
        QString copyFrameName;
        bool copyIsValid;
        bool canvasEnabled;

        QString webAssetName;
        bool webLock;
};

#endif
