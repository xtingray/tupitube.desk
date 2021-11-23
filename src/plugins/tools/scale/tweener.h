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

#ifndef SCALETWEENER_H
#define SCALETWEENER_H

#include "tglobal.h"
#include "tuptoolplugin.h"
#include "scalesettings.h"
#include "tupprojectresponse.h"
#include "configurator.h"

#include <QPointF>
#include <QKeySequence>
#include <QGraphicsView>
#include <QDomDocument>
#include <QDir>

/**
 * @author Gustav Gonzalez 
 * 
*/

class TUPITUBE_PLUGIN Tweener : public TupToolPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.maefloresta.tupi.TupToolInterface" FILE "scaletool.json")

    public:
        Tweener();
        virtual ~Tweener();

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

    signals:
        void tweenRemoved();

    private slots:
        void setSelection();
        void setPropertiesMode();
        void updateMode(TupToolPlugin::Mode mode);
        void applyReset();
        void applyTween();
        void removeTween(const QString &name);
        void updateStartPoint(int index);
        void setCurrentTween(const QString &name);
        void updateOriginPoint(const QPointF &point);

    private:
        void setupActions();
        int framesCount();
        void clearSelection();
        void disableSelection();
        void removeTweenFromProject(const QString &name);
        QTransform initialStep();

        QMap<TAction::ActionId, TAction *> scaleActions;
        Configurator *configPanel;

        TupGraphicsScene *scene;
        QList<QGraphicsItem *> objects;

        TupItemTweener *currentTween;
        int initFrame;
        int initLayer;
        int initScene;

        QPointF origin;
        double initialXScaleFactor;
        double initialYScaleFactor;

        TupToolPlugin::Mode mode;
        TupToolPlugin::EditMode editMode;

        int baseZValue;
};

#endif
