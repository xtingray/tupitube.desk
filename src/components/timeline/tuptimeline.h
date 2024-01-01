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

#ifndef TUPTIMELINE_H
#define TUPTIMELINE_H

#include "tglobal.h"
#include "tupmodulewidgetbase.h"
#include "tuptimelinescenecontainer.h"
#include "tupproject.h"
#include "tuptimelinetable.h"
#include "tuprequestbuilder.h"

#include <QDoubleSpinBox>

class TUPITUBE_EXPORT TupTimeLine : public TupModuleWidgetBase
{
    Q_OBJECT

    public:
        TupTimeLine(TupProject *project, QWidget *parent = nullptr);
        ~TupTimeLine();

        void initLayerVisibility();
        void closeAllScenes();
        void updateFramesState();

    private:
        TupTimeLineTable *framesTable(int sceneIndex);
        void requestRemoveFrame(bool flag);
 
    protected:
        void sceneResponse(TupSceneResponse *response);
        void layerResponse(TupLayerResponse *response);
        void frameResponse(TupFrameResponse *response);
        void itemResponse(TupItemResponse *response);
        void libraryResponse(TupLibraryResponse *response);

    signals:
        void newPerspective(int);
        void sceneChanged(int sceneIndex);

    public slots:
        void addScene(int sceneIndex, const QString &name);
        void removeScene(int sceneIndex);
        void updateFPS(int fps);

    private slots:
        void requestCommand(int action);
        bool requestFrameAction(int action, int frameIndex = -1, int layerIndex = -1, int sceneIndex = -1,
                                const QVariant &arg = QVariant());
        bool requestLayerAction(int action, int layerIndex = -1, int sceneIndex = -1, const QVariant &arg = QVariant());
        bool requestSceneAction(int action, int sceneIndex = -1, const QVariant &arg = QVariant());

        void requestFrameSelection(int indexLayer, int indexFrame);
        void requestCopyFrameSelection();
        void requestPasteSelectionInCurrentFrame();
        void removeFrameSelection();

        void extendFrameForward(int layerIndex, int frameIndex);
        void requestLayerMove(int oldLayerIndex, int newLayerIndex);

        void requestLayerVisibilityAction(int layerIndex, bool isVisible);
        void requestLayerRenameAction(int layerIndex, const QString &name);

        void requestSceneSelection(int sceneIndex);
        void requestSceneMove(int from, int to);

        void requestUpdateLayerOpacity(double opacity);
        void showRenameSceneDialog(int sceneIndex);        

    private:
        void requestReverseFrameSelection();
        double getLayerOpacity(int sceneIndex, int layerIndex);
        void updateLayerOpacity(int sceneIndex, int layerIndex);
        void requestSceneRename(const QString &name);

        TupTimelineSceneContainer *scenesContainer;
        TupTimeLineTable *currentTable;
        TupProjectActionBar *actionBar;
        int selectedLayer;
        TupProject *project;
        TupLibrary *library;
        QString frameSelection;
        bool doSelection;
        QDoubleSpinBox *opacitySpinBox;

        bool localSceneMove;
};

#endif
