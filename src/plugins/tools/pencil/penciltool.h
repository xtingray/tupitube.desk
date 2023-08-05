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

#ifndef PENCILTOOL_H
#define PENCILTOOL_H

#include "tglobal.h"
#include "tuptoolplugin.h"
#include "pencilsettings.h"
#include "tuppathitem.h"

#include "tupinputdeviceinformation.h"
#include "tupbrushmanager.h"
#include "tupgraphicsscene.h"
#include "tuplibraryobject.h"
#include "tupellipseitem.h"
#include "taction.h"
#include "tconfig.h"

#include <QPointF>
#include <QPainterPath>
#include <QGraphicsEllipseItem>

class TUPITUBE_PLUGIN PencilTool : public TupToolPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.maefloresta.tupi.TupToolInterface" FILE "penciltool.json")
    
    public:
        PencilTool();
        virtual ~PencilTool();
        
        virtual void init(TupGraphicsScene *gScene);
        virtual QList<TAction::ActionId> keys() const;
        virtual void press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene);
        virtual void move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene);
        virtual void release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene);
        virtual QMap<TAction::ActionId, TAction *>actions() const;
        TAction * getAction(TAction::ActionId toolId);

        int toolType() const;
        virtual QWidget *configurator();
        virtual void aboutToChangeTool();
        virtual void saveConfig();
        virtual QCursor toolCursor();
        virtual void frameResponse(const TupFrameResponse *event);
        virtual void sceneResponse(const TupSceneResponse *event);

        void addKeyPoints(TupPathItem *item);

    protected:
        virtual void keyPressEvent(QKeyEvent *event);
        virtual void keyReleaseEvent(QKeyEvent *event);

    private:
        void setupActions();
        void setZValueReferences();
        void reset(TupGraphicsScene *scene);

    signals:
        void closeHugeCanvas();
        void callForPlugin(int menu, int index);
        void penWidthChanged(int width);

    private slots:
        void updatePenTool(PenTool tool);
        void updateSmoothness(double value);
        void updateEraserSize(int value);

    private:
        void storePathItems();
        void activeEraser(const QPointF &point);
        TupFrame* getCurrentFrame();

        QPointF firstPoint;
        QPointF previousPos;
        QPainterPath path;
        PencilSettings *settings;
        QMap<TAction::ActionId, TAction *> penActions;
        TupPathItem *item;
        QCursor penCursor;
        TupGraphicsScene *scene;
        TupBrushManager *brushManager;
        TupInputDeviceInformation *input;

        bool resizeMode;
        QGraphicsEllipseItem *penCircle;
        int baseZValue;
        int topZValue;
        int circleZValue;
        QPointF penCirclePos;
        int penWidth;
        int eraserSize;
        double smoothness;
        PenTool currentTool;

        QList<TupPathItem *> lineItems;
        QList<QGraphicsItem *> graphicItems;

        int currentLayer;
        int currentFrame;

        QPen eraserPen;
        QGraphicsEllipseItem *eraserCircle;
        QPointF eraserDistance;

        QList<TupEllipseItem *> route;
        TupPathItem *lineItem;
        bool lineAdded;
};

#endif
