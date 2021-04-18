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

#ifndef TEXTTOOL_H
#define TEXTTOOL_H

#include "tglobal.h"
#include "tuptoolplugin.h"
#include "tuptextitem.h"
#include "textconfigurator.h"
#include "tupbrushmanager.h"
#include "node.h"
#include "nodemanager.h"

#include <QPointF>
#include <QFontMetrics>
#include <QKeySequence>
#include <QGraphicsView>

class TUPITUBE_PLUGIN TextTool : public TupToolPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.maefloresta.tupi.TupToolInterface" FILE "texttool.json")

    public:
        TextTool();
        ~TextTool();
        
        virtual void init(TupGraphicsScene *gScene);
        virtual QList<TAction::ActionId> keys() const;
        virtual QCursor cursor();

        virtual void press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);
        // virtual void doubleClick(const TupInputDeviceInformation *input, TupGraphicsScene *scene);
        virtual void move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);
        virtual void release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);
        // virtual void doubleClick(const TupInputDeviceInformation *input, TupGraphicsScene *scene);

        virtual QMap<TAction::ActionId, TAction *>actions() const; 
        virtual QWidget *configurator();
        virtual void saveConfig();
        virtual void keyPressEvent(QKeyEvent *event);

        virtual void sceneResponse(const TupSceneResponse *event);
        virtual void layerResponse(const TupLayerResponse *response);
        virtual void itemResponse(const TupItemResponse *response);

        int toolType() const;
        void aboutToChangeScene(TupGraphicsScene *scene);
        void aboutToChangeTool();
        void resizeNode(qreal scaleFactor);
        void updateZoomFactor(qreal scaleFactor);
        void clearSelection();
        void updateTextColor(const QColor &color);

    signals:
        void closeHugeCanvas();
        void callForPlugin(int menu, int index);

    private slots:
        void initItems(TupGraphicsScene *scene);
        void insertText();
        void updateText();
        void syncNodes();

    private:
        void setupActions();
        TupFrame* getCurrentFrame();
        void requestTransformation(QGraphicsItem *item, TupFrame *frame);
        TupFrame* frameAt(int sceneIndex, int layerIndex, int frameIndex);

        TupGraphicsScene *scene;
        // TupTextItem *textItem;
        TextConfigurator *config;
        QMap<TAction::ActionId, TAction *> textActions;
        QColor currentColor;

        int currentLayer;
        int currentFrame;
        TupFrame *frame;

        int nodeZValue;
        qreal realFactor;
        NodeManager *manager;
        bool activeSelection;
};

#endif
