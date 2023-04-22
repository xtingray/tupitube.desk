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

#ifndef TUPITEMTWEENER_H
#define TUPITEMTWEENER_H

#include "tglobal.h"
#include "tuptweenerstep.h"

#include <QObject>
#include <QPointF>
#include <QColor>
#include <QGraphicsPathItem>

class TUPITUBE_EXPORT TupItemTweener : public QObject, public TupAbstractSerializable
{
    Q_OBJECT
 
    public:
        enum Type {
             None = -1,
             Motion = 0,
             Rotation = 1,
             Scale = 2,
             Shear = 3,
             Opacity = 4,
             Coloring = 5,
             Papagayo = 6
        };

        enum RotationType { Continuos = 0, Partial };
        enum RotateDirection { Clockwise = 0, Counterclockwise };
        enum TransformAxes { XY = 0, X, Y };
        enum FillType { NoFill = -1, Internal = 0, Line, FillAll };

        TupItemTweener();
        ~TupItemTweener();

        QString getTweenName();
        TupItemTweener::Type getType();
        
        void setZLevel(int level);
        int getZLevel();

        void setPosAt(int step, const QPointF & point);
        void setRotationAt(int step, double angle);
        void setScaleAt(int step, double sx, double sy);
        void setShearAt(int step, double sh, double sv);
        void setOpacityAt(int step, double opacity);
        void setColorAt(int step, const QColor &color);
        
        void addStep(const TupTweenerStep &step);
        TupTweenerStep *stepAt(int index);
        TupTweenerStep *lastStep();

        void setFrames(int getFrames);

        int getFrames() const;
        int getInitFrame();
        int getInitLayer();
        int getInitScene();
        QPointF transformOriginPoint();
        double initXScaleValue();
        double initYScaleValue();
        
        void setStep(int step);
        
        void fromXml(const QString &xml);
        QDomElement toXml(QDomDocument &doc) const;

        QGraphicsPathItem *graphicsPath() const;
        void setGraphicsPath(const QString &tweenPath);

        QList<int> getIntervals();
        QString tweenTypeToString();

        TupItemTweener::RotationType tweenRotationType();
        double tweenRotateSpeed();
        bool tweenRotateLoop();
        TupItemTweener::RotateDirection tweenRotateDirection();
        int tweenRotateStartDegree();
        int tweenRotateEndDegree();
        bool tweenRotateReverseLoop();

        TupItemTweener::TransformAxes tweenScaleAxes();
        double tweenScaleFactor();
        int tweenScaleIterations();
        int tweenScaleLoop();
        int tweenScaleReverseLoop();

        TupItemTweener::TransformAxes tweenShearAxes();
        double tweenShearFactor();
        int tweenShearIterations();
        int tweenShearLoop();
        int tweenShearReverseLoop();

        double tweenOpacityInitialFactor();
        double tweenOpacityEndingFactor();
        int tweenOpacityIterations();
        int tweenOpacityLoop();       
        int tweenOpacityReverseLoop();

        TupItemTweener::FillType tweenColorFillType();
        QColor tweenInitialColor();
        QColor tweenEndingColor();
        int tweenColorIterations();
        int tweenColorLoop();
        int tweenColorReverseLoop();

        bool contains(TupItemTweener::Type getType);
        
    private:
        QString tweenName;
        TupItemTweener::Type tweenType;
        int initFrame;
        int initLayer;
        int initScene;
        int zLevel;

        int frames;
        QPointF originPoint;

        // Position Tween
        QString tweenPath;
        QString intervals;

        // Rotation Tween
        TupItemTweener::RotationType rotationType;
        TupItemTweener::RotateDirection rotateDirection;
        double rotateSpeed;
        int rotateLoop;
        int rotateReverseLoop;
        int rotateStartDegree;
        int rotateEndDegree;

        // Scale Tween
        TupItemTweener::TransformAxes scaleAxes;
        double initialXScaleFactor;
        double initialYScaleFactor;
        double scaleFactor;
        int scaleIterations;
        int scaleLoop;
        int scaleReverseLoop;

        // Shear Tween
        TupItemTweener::TransformAxes shearAxes;
        double shearFactor;
        int shearIterations;
        int shearLoop;
        int shearReverseLoop;

        // Opacity Tween
        double initOpacityFactor;
        double endOpacityFactor;
        int opacityIterations;
        int opacityLoop;
        int opacityReverseLoop;

        // Color Tween
        FillType colorFillType;
        QColor initialColor;
        QColor endingColor;
        int colorIterations;
        int colorLoop;
        int colorReverseLoop;

        QList<TupItemTweener::Type> tweenList;

        QHash<int, TupTweenerStep *> steps; // TODO: remove when Qt 4.3

        inline TupTweenerStep *step(int step)
        {
            TupTweenerStep *currentStep = steps[step];
            if (!currentStep) {
                currentStep = new TupTweenerStep(step);
                steps.insert(step, currentStep);
            }

            return currentStep;
        }
};

#endif
