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

#ifndef SELECTIONTOOL_H
#define SELECTIONTOOL_H

#include "tglobal.h"
#include "tuptoolplugin.h"
#include "tuppathitem.h"
#include "tupproject.h"
#include "tupgraphicsscene.h"
#include "selectionsettings.h"
#include "tupprojectresponse.h"
#include "tupellipseitem.h"

#include <QObject>
#include <QPointF>
#include <QKeySequence>
#include <QList>
#include <QGraphicsView>
#include <QTimer>
#include <cmath>

class TupItemResponse;
class NodeManager;

class TUPITUBE_PLUGIN SelectionTool : public TupToolPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.maefloresta.tupi.TupToolInterface" FILE "selectiontool.json")
    
    public:
        SelectionTool();
        virtual ~SelectionTool();
        
        virtual void init(TupGraphicsScene *scene);
        virtual QList<TAction::ActionId> keys() const;
        virtual void press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);
        virtual void move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);
        virtual void release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);
        virtual void keyPressEvent(QKeyEvent *event);
        virtual void keyReleaseEvent(QKeyEvent *event);

        virtual void sceneResponse(const TupSceneResponse *event);

        virtual QMap<TAction::ActionId, TAction *> actions() const;
        TAction * getAction(TAction::ActionId toolId);

        int toolType() const;
        
        virtual QWidget *configurator();
        
        virtual void aboutToChangeTool();
        void aboutToChangeScene(TupGraphicsScene *scene);
        
        virtual void itemResponse(const TupItemResponse *response);
        void layerResponse(const TupLayerResponse *response);

        virtual void saveConfig();
        virtual QCursor cursor();

        void resizeNode(qreal scaleFactor);
        void updateZoomFactor(qreal scaleFactor);
        void clearSelection();
        virtual void setProjectSize(const QSize size);

        void updateColorOnSelection(TupProjectRequest::Action action, const QColor &color);
        void updatePenOnSelection(const QPen &pen);

    signals:
        void closeHugeCanvas();
        void callForPlugin(int menu, int index);

    private slots:
        void initItems(TupGraphicsScene *scene);
        void syncNodes();
        void applyAlignAction(SelectionSettings::Align align);
        void applyFlip(SelectionSettings::Flip flip);
        void applyOrderAction(SelectionSettings::Order order);
        void applyGroupAction(SelectionSettings::Group action);
        void setItemPosition(int x, int y);
        void setItemRotation(int angle);
        void setItemScale(double xFactor, double yFactor);
        void enableProportion(bool flag);
        void resetItemTransformations();

    private:
        void setupActions();
        bool selectionIsActive();
        void updateItemPosition();
        void updateItemRotation();
        void updateItemScale();
        void addTarget();
        void removeTarget();
        TupFrame* getCurrentFrame();
        TupFrame* frameAt(int sceneIndex, int layerIndex, int frameIndex);
        void requestTransformation(QGraphicsItem *item, TupFrame *frame);

        SelectionSettings *settingsPanel;
        QMap<TAction::ActionId, TAction *> selectActions;
        QList<QGraphicsItem *> selectedObjects;
        QList<NodeManager*> nodeManagers;
        TupGraphicsScene *scene;
        bool activeSelection;
        qreal realFactor;
        int nodeZValue;

        TupEllipseItem *center;
        QGraphicsLineItem *target1;
        QGraphicsLineItem *target2;
        QGraphicsLineItem *topLeftX;
        QGraphicsLineItem *topLeftY;
        QGraphicsLineItem *topRightX;
        QGraphicsLineItem *topRightY;
        QGraphicsLineItem *bottomLeftX;
        QGraphicsLineItem *bottomLeftY;
        QGraphicsLineItem *bottomRightX;
        QGraphicsLineItem *bottomRightY;

        bool targetIsIncluded;
        QString key;

        int currentLayer;
        int currentFrame;
        TupFrame *frame;
        QPointF wsCenter;
        QList<int> specialSelection;
};

#endif
