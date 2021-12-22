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

#ifndef PAPAGAYOTOOL_H
#define PAPAGAYOTOOL_H

#include "tglobal.h"
#include "tuptoolplugin.h"
#include "papagayosettings.h"
#include "tupprojectresponse.h"
#include "papagayoconfigurator.h"
#include "nodemanager.h"

#include <QPointF>
#include <QKeySequence>
#include <QGraphicsView>
#include <QDomDocument>

/**
 * @author Gustav Gonzalez 
 * 
*/

class TUPITUBE_PLUGIN PapagayoTool : public TupToolPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.maefloresta.tupi.TupToolInterface" FILE "papagayotool.json")

    public:
        PapagayoTool();
        virtual ~PapagayoTool();

        virtual void init(TupGraphicsScene *scene);
        virtual QList<TAction::ActionId> keys() const;
        virtual void press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);
        virtual void move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);
        virtual void release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);

        virtual QMap<TAction::ActionId, TAction *>actions() const;
        TAction * getAction(TAction::ActionId toolId);
        int toolType() const;
        virtual QWidget *configurator();

        void aboutToChangeScene(TupGraphicsScene *scene);
        virtual void aboutToChangeTool();

        virtual void updateScene(TupGraphicsScene *scene);
        virtual void saveConfig();

        virtual void sceneResponse(const TupSceneResponse *event);
        virtual void layerResponse(const TupLayerResponse *event);
        virtual void frameResponse(const TupFrameResponse *event);

        virtual void updateWorkSpaceContext();

        virtual TupToolPlugin::Mode currentMode();

        void resizeNode(qreal factor);
        void updateZoomFactor(qreal factor);

        void setNodesManagerEnvironment();
        void setProportionState(int flag);

    protected:
        virtual void keyPressEvent(QKeyEvent *event);
        virtual void keyReleaseEvent(QKeyEvent *event);

    signals:
        void lipsyncCreatorRequested();
        void lipsyncEditionRequested(const QString &lipsyncName);
        void callForPlugin(int menu, int index);

    private slots:
        // void saveMouthTransformations();
        void editLipsyncMouth(const QString &name);
        void removeCurrentLipSync(const QString &name);

        void resetCanvas();
        void addNodesManager();

        void updateInitFrame(int index);
        void updateXMouthPositionInScene(int x);
        void updateYMouthPositionInScene(int y);
        void updateRotationInScene(int angle);
        void updateScaleInScene(double xFactor, double yFactor);

        void updatePositionRecord(const QPointF &point);
        void updateRotationAngleRecord(int angle);
        void updateScaleFactorRecord(double x, double y);

        void resetMouthTransformations();
        void enableProportion(bool flag);

    private:
        void setupActions();
        void removeNodesManager();
        void updateMouthTransformation();

        QMap<TAction::ActionId, TAction *> pgActions;
        PapagayoConfigurator *configPanel;

        TupGraphicsScene *scene;

        TupLipSync *currentLipSync;
        int sceneIndex;

        TupToolPlugin::Mode mode;

        qreal realFactor;
        QGraphicsItem *mouth;

        int nodeZValue;
        NodeManager *nodesManager;
        bool managerIncluded;
        QString key;
};

#endif
