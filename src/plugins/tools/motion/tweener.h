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

#ifndef POSITIONTWEENER_H
#define POSITIONTWEENER_H

#include "tglobal.h"
#include "tuptoolplugin.h"
// #include "motionsettings.h"
#include "tupprojectresponse.h"
#include "tnodegroup.h"
#include "configurator.h"
#include "tupellipseitem.h"
#include "tuplineitem.h"
#include "taction.h"

#include <QPointF>
#include <QKeySequence>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QGraphicsLineItem>
#include <QGraphicsView>
#include <QDomDocument>
#include <QDir>

class TUPITUBE_PLUGIN Tweener : public TupToolPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.maefloresta.tupi.TupToolInterface" FILE "motiontool.json")

    public:
        Tweener();
        virtual ~Tweener();
        virtual void init(TupGraphicsScene *scene);

        virtual QList<TAction::ActionId> keys() const;
        virtual void press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);
        virtual void move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);
        virtual void release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);
        virtual void updateScene(TupGraphicsScene *scene);
        virtual QMap<TAction::ActionId, TAction *>actions() const;
        TAction * getAction(TAction::ActionId toolId);
        virtual QWidget *configurator();
        virtual void aboutToChangeTool();
        virtual void saveConfig();

        int toolType() const;
        void aboutToChangeScene(TupGraphicsScene *scene);

        virtual void sceneResponse(const TupSceneResponse *event);
        virtual void layerResponse(const TupLayerResponse *event);
        virtual void frameResponse(const TupFrameResponse *event);
        virtual void itemResponse(const TupItemResponse *event);

        virtual TupToolPlugin::Mode currentMode();
        virtual TupToolPlugin::EditMode currentEditMode();

        void resizeNode(qreal scaleFactor);
        void updateZoomFactor(qreal scaleFactor);
        void updatePos(QPointF pos);

    protected:
        virtual void keyPressEvent(QKeyEvent *event);
        virtual void keyReleaseEvent(QKeyEvent *event);

    signals:
        void tweenRemoved();

    private slots:
        void applyReset();
        void applyTween();
        void removeTween(const QString &name);
        void setTweenPath();
        void setSelection();
        void setEditEnv();
        void updateMode(TupToolPlugin::Mode mode);
        void updateStartFrame(int index);
        void setCurrentTween(const QString &name);
        void updateTweenPoints();

        void updatePathThickness(int thickness);
        void updatePathColor(const QColor &color);

    public slots:
        void updatePath();

    private:
        int framesCount();
        void setupActions();
        QString pathToCoords();
        void clearSelection();
        void disableSelection();
        void removeTweenFromProject(const QString &name);
        void resetGUI();
        void removeTweenPoints();
        void paintTweenPoints();
        void updateTweenPath();

        QMap<TAction::ActionId, TAction *> posActions;
        Configurator *configPanel;

        TupGraphicsScene *scene;
        QGraphicsPathItem *linePath;
        TupLineItem *guideLine;

        QList<QPainterPath> doList;
        QList<QPainterPath> undoList;

        QList<QGraphicsItem *> objects;

        TupItemTweener *currentTween;
        TNodeGroup *nodesGroup;
        qreal realFactor;

        bool isPathInScene;
        int initFrame;
        int initLayer;
        int initScene;

        TupToolPlugin::Mode mode;
        TupToolPlugin::EditMode editMode;

        QPointF itemObjectReference;
        QPointF pathOffset;
        QPointF firstNode;
        QPointF objectPos;
        QList<TupEllipseItem *> dots;

        int baseZValue;
        bool lineStraightMode;
        QPointF currentPoint;
        QPointF lastPoint;
};

#endif
