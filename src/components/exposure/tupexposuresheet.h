/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez                                                     *
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

#ifndef TUPEXPOSURESHEET_H
#define TUPEXPOSURESHEET_H

#include "tglobal.h"
#include "tupmodulewidgetbase.h"
#include "tupexposuretable.h"
#include "tupexposurescenetabwidget.h"
#include "timagebutton.h"
#include "tupprojectactionbar.h"
#include "tupproject.h"
#include "tapplication.h"
#include "tupprojectrequest.h"
#include "tuprequestbuilder.h"
#include "tupscene.h"
#include "tuplayer.h"
#include "tupframe.h"

#include <QList>
#include <QMenu>

class TUPITUBE_EXPORT TupExposureSheet : public TupModuleWidgetBase
{
    Q_OBJECT

    public:
        TupExposureSheet(QWidget *parent = nullptr, TupProject *project = nullptr);
        ~TupExposureSheet();

        void updateFramesState();
        void updateLayerOpacity(int sceneIndex, int layerIndex);
        double getLayerOpacity(int sceneIndex, int layerIndex);
        void initLayerVisibility();

    private:
        void createMenuForAFrame();
        // void createMenuForSelection();
        void requestExtendCurrentFrame(int times);
        void insertFrames(int times);
        void removeBlock(TupExposureTable *table, int layerIndex, int frameIndex, 
                         int layersTotal, int framesTotal);
        void requestSceneRename(const QString &name);

    protected:
        virtual void sceneResponse(TupSceneResponse *response);
        virtual void layerResponse(TupLayerResponse *response);
        virtual void frameResponse(TupFrameResponse *response);
        virtual void itemResponse(TupItemResponse *response);
        virtual void libraryResponse(TupLibraryResponse *response);

    signals:
        void newPerspective(int);
        void sceneChanged(int sceneIndex);

    public slots:
        void updateSceneFramesState(int sceneIndex);
        void closeAllScenes();
        void applyAction(int action);
        void addScene(int sceneIndex, const QString &name);
        void renameScene(int sceneIndex, const QString &name);
        void setScene(int sceneIndex);
        void changeLayerVisibility(int, bool);
        void updateFPS(int fps);

    private slots: 
        void requestChangeScene(int index);
        void requestSceneMove(int from, int to);
        void requestCopyFrameSelection();
        void requestPasteSelectionInCurrentFrame();
        void requestUpdateLayerOpacity(double opacity);

        void insertFramesFromMenu(QAction *action);
        void clearFrame();

        void insertFrame(int layerIndex, int frameIndex);
        void renameFrame(int layerIndex, int frameIndex, const QString &name);
        void selectFrame(int layerIndex, int frameIndex);
        void removeFrame();
        void extendFrameForward(int layerIndex, int frameIndex);

        void requestRenameLayer(int layerIndex, const QString &name);
        void moveLayer(int oldIndex, int newIndex);
        void actionTriggered(QAction *action);
        void showRenameSceneDialog(int sceneIndex);

    private:
        TupProject *project;
        TupExposureSceneTabWidget *scenesContainer;
        TupExposureTable *currentTable;
        TupProjectActionBar *topActionBar;
        TupProjectActionBar *bottomActionBar;

        QMenu *singleMenu;
        // QMenu *multipleMenu;
        bool localRequest;
        int previousScene;
        int previousLayer;

        QList<QString> framesList;
        QList<TupExposureTable::FrameType> statesList;

        // bool movingTab;
};

#endif
