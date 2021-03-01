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

#ifndef NODESTOOL_H
#define NODESTOOL_H

#include "tglobal.h"
// #include "tuppathitem.h"
#include "tcontrolnode.h"
#include "tnodegroup.h"
#include "tuptoolplugin.h"
#include "taction.h"
#include "talgorithm.h"
#include "tnodegroup.h"
#include "tupgraphicalgorithm.h"
#include "tupscene.h"
#include "tuplayer.h"
#include "tupserializer.h"
#include "tupinputdeviceinformation.h"
#include "tupgraphicsscene.h"
#include "tuplibraryobject.h"
#include "tupitemgroup.h"
#include "tosd.h"
#include "tupgraphiclibraryitem.h"
#include "tuppixmapitem.h"

#include <QObject>
#include <QSpinBox>
#include <QGraphicsView>
#include <QPointF>
#include <QKeySequence>
#include <QList>
#include <QTimer>
#include <QDir>

class TupItemResponse;
class TupGraphicsScene;

class TUPITUBE_PLUGIN NodesTool : public TupToolPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.maefloresta.tupi.TupToolInterface" FILE "nodestool.json")
    
    public:
        NodesTool();
        virtual ~NodesTool();
        
        virtual void init(TupGraphicsScene *scene);
        virtual QList<TAction::ActionId> keys() const;
        virtual void press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);
        virtual void move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);
        virtual void release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);
        
        virtual QMap<TAction::ActionId, TAction *>actions() const;
        int toolType() const;
        virtual QWidget *configurator();
        void aboutToChangeScene(TupGraphicsScene *scene);
        virtual void aboutToChangeTool();
        
        virtual void itemResponse(const TupItemResponse *event);
        virtual void layerResponse(const TupLayerResponse *response);
        virtual void keyPressEvent(QKeyEvent *event);
        virtual void keyReleaseEvent(QKeyEvent *event);
        virtual void saveConfig();
        virtual QCursor polyCursor(); // const;

        void resizeNode(qreal scaleFactor);
        void updateZoomFactor(qreal scaleFactor);
        void clearSelection();

    signals:
        void closeHugeCanvas();
        void callForPlugin(int menu, int index);

    private:
        // void reset(TupGraphicsScene *scene);
        void setupActions();
        TupFrame* getCurrentFrame();
        void requestTransformation(QGraphicsItem *item, TupFrame *frame);

        QMap<TAction::ActionId, TAction *> nodesActions;
        TNodeGroup *nodeGroup;
        TupGraphicsScene *scene;
        int baseZValue;
        qreal realFactor;
        bool activeSelection;
        int currentLayer;
        int currentFrame;
};

#endif
