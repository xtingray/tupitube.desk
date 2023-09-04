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

#ifndef BUCKETTOOLPLUGIN_H
#define BUCKETTOOLPLUGIN_H

#include "tglobal.h"
#include "tuptoolplugin.h"
#include "tuppathitem.h"
#include "tcolorcell.h"

#include <QObject>
#include <QLabel>
#include <QKeySequence>
#include <QImage>
#include <QPaintDevice>
#include <QGraphicsView>

class TUPITUBE_PLUGIN BucketTool : public TupToolPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.maefloresta.tupi.TupToolInterface" FILE "buckettool.json")
    
    public:
        BucketTool();
        ~BucketTool();

        virtual QList<TAction::ActionId> keys() const;

        void init(TupGraphicsScene *scene);
        
        virtual void press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, 
                           TupGraphicsScene *gScene);
        virtual void move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, 
                          TupGraphicsScene *gScene);

        virtual void release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, 
                    TupGraphicsScene *gScene);
        
        TupPathItem *itemPressed(QGraphicsItem *item, const TupBrushManager *brush);
        
        virtual QMap<TAction::ActionId, TAction *> actions() const;
        TAction * getAction(TAction::ActionId toolId);
        
        int toolType() const;
        
        virtual QWidget *configurator();
        void aboutToChangeScene(TupGraphicsScene *scene);
        virtual void aboutToChangeTool();
        
        QPainterPath mapPath(const QPainterPath &path, const QPointF &pos);
        QPainterPath mapPath(const QGraphicsPathItem *item);

        virtual void saveConfig();
        virtual void keyPressEvent(QKeyEvent *event);
        virtual QCursor toolCursor(); // const;

        void setColorMode(TColorCell::FillType colorMode);

    signals:
        void closeHugeCanvas();
        void callForPlugin(int, int);
 
    private:
        void setupActions();

    private:
        QMap<TAction::ActionId, TAction *> bucketActions;
        TupGraphicsScene *scene;
        QCursor fillCursor;
        QCursor borderCursor;
        TColorCell::FillType mode;
};

#endif
