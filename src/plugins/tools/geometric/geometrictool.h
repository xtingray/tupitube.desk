/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustav Gonzalez / xtingray                                           *
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

#ifndef GEOMETRICTOOLPLUGIN_H
#define GEOMETRICTOOLPLUGIN_H

#include "tglobal.h"
#include "tuptoolplugin.h"
#include "tuprectitem.h"
#include "tupellipseitem.h"
#include "tuppathitem.h"
#include "tuplineitem.h"
#include "geometricsettings.h"

#include <QObject>
#include <QLabel>
#include <QKeyEvent>
#include <cmath>
#include <QKeySequence>
#include <QImage>
#include <QPaintDevice>
#include <QGraphicsView>

class TUPITUBE_PLUGIN GeometricTool : public TupToolPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.maefloresta.tupi.TupToolInterface" FILE "geometrictool.json")
    
    public:
        GeometricTool();
        ~GeometricTool();

        virtual QList<TAction::ActionId> keys() const;
        virtual void init(TupGraphicsScene *scene);
        virtual void press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);
        virtual void move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);
        virtual void release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);
        virtual void keyPressEvent(QKeyEvent *event);
        virtual void keyReleaseEvent(QKeyEvent *event);
        // virtual void doubleClick(const TupInputDeviceInformation *input, TupGraphicsScene *scene);

        virtual void sceneResponse(const TupSceneResponse *event);
        virtual void layerResponse(const TupLayerResponse *event);
        virtual void frameResponse(const TupFrameResponse *event);

        virtual QMap<TAction::ActionId, TAction *> actions() const;

        int toolType() const;
        
        virtual QWidget *configurator();
        void aboutToChangeScene(TupGraphicsScene *scene);
        virtual void aboutToChangeTool();
        virtual void saveConfig();
        virtual QCursor polyCursor(); // const;
        void updatePos(QPointF pos);

    public slots:
        void endItem();

    signals:
        void closeHugeCanvas();
        void callForPlugin(int menu, int index);
        
    private:
        void setupActions();

    private:
        QBrush setLiteBrush(QColor c, Qt::BrushStyle style);

        QBrush fillBrush;
        TupRectItem *rect;
        TupEllipseItem *ellipse;
        TupLineItem *line;
        TupPathItem *path;
        TupGraphicsScene *scene;
        GeometricSettings *configPanel;
        bool added;
        QPointF currentPoint;
        QPointF lastPoint;
        QMap<TAction::ActionId, TAction *> geoActions;

        bool proportion;
        bool side;
        int xSideLength;
        int ySideLength;

        QGraphicsItem *item;
        QCursor squareCursor;
        QCursor circleCursor;
        QCursor lineCursor;
};

#endif
