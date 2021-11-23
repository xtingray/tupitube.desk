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

#ifndef INKTOOL_H
#define INKTOOL_H

#include "tglobal.h"
#include "tuptoolplugin.h"
#include "inksettings.h"
#include "tuppathitem.h"

#include <QObject>
#include <QSpinBox>
#include <QTimer>
#include <QPointF>
#include <QKeySequence>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QGraphicsLineItem>
#include <QGraphicsView>
#include <QBrush>
#include <cmath>

class TUPITUBE_PLUGIN InkTool : public TupToolPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.maefloresta.tupi.TupToolInterface" FILE "inktool.json")
    
    public:
        enum Direction { None, Up, Down, Right, RightUp, RightDown, Left, LeftUp, LeftDown };
        InkTool();
        virtual ~InkTool();
        
        virtual void init(TupGraphicsScene *scene);
        virtual QList<TAction::ActionId> keys() const;
        virtual void press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);
        virtual void move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);
        virtual void release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene);
        virtual QMap<TAction::ActionId, TAction *> actions() const;
        TAction * getAction(TAction::ActionId toolId);
        int toolType() const;
        virtual QWidget *configurator();
        virtual void aboutToChangeTool();
        virtual void saveConfig();
        virtual void keyPressEvent(QKeyEvent *event);
        virtual QCursor toolCursor(); // const;
        void updatePressure(qreal pressure);

    signals:
        void closeHugeCanvas();
        void callForPlugin(int menu, int index);
        
    private slots:
        void setDevice(InkSettings::Device device);
        void updateBorderFeature(bool border);
        void updateFillFlag(bool fill);
        void updateBorderSize(int size);
        void updatePressure(int value);
        void updateSmoothness(double value);

    private:
        void setupActions();
        void processPoint(QPointF currentPoint, qreal strokeWidth);
        void smoothPath(QPainterPath &guidePainterPath, double smoothness, int from = 0,
                        int to = -1, bool closePath = false);
        void removeExtraPoints();

    private:
        QPointF firstPoint;
        QPointF oldPos;

        QPointF previousPoint;
        QPointF firstHalfPrevious;
        bool firstHalfOnTop;
        QPointF secondHalfPrevious;

        QPainterPath guidePainterPath;
        QPainterPath inkPath;
        QList<QPointF> guidePoints;
        QList<QPointF> shapePoints;
        QList<qreal> pointPress;

        InkSettings *settings;
        QMap<TAction::ActionId, TAction *> inkActions;

        TupPathItem *guidePath;

        int borderSize;
        qreal initPenWidth;
        qreal penWidth;
        qreal penPress;
        // qreal oldSlope;
        int arrowSize;
        int firstArrow;
        QCursor inkCursor;

        int sensibility;
        double smoothness;
        bool showBorder;
        bool showFill;

        Direction previousDirection;
        InkSettings::Device device;
};

#endif
