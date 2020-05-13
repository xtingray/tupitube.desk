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

#include "tupitemtweener.h"
#include "tupsvg2qt.h"

TupItemTweener::TupItemTweener()
{
    frames = 0;
    zLevel = 0;
    initialXScaleFactor = 1;
    initialYScaleFactor = 1;
}

TupItemTweener::~TupItemTweener()
{
    qDeleteAll(steps);
}

QString TupItemTweener::getTweenName()
{
    return tweenName;
}

TupItemTweener::Type TupItemTweener::getType()
{
    return tweenType;
}

void TupItemTweener::setZLevel(int level)
{
    zLevel = level;
}

int TupItemTweener::getZLevel()
{
    return zLevel;
}

void TupItemTweener::addStep(const TupTweenerStep &step)
{
    int counter = step.getIndex();

    if (step.has(TupTweenerStep::Position))
        setPosAt(counter, step.getPosition());

    if (step.has(TupTweenerStep::Rotation)) 
        setRotationAt(counter, step.getRotation());
    
    if (step.has(TupTweenerStep::Scale))
        setScaleAt(counter, step.horizontalScale(), step.verticalScale());
    
    if (step.has(TupTweenerStep::Shear))
        setShearAt(counter, step.horizontalShear(), step.verticalShear());
    
    if (step.has(TupTweenerStep::Opacity))
        setOpacityAt(counter, step.getOpacity());

    if (step.has(TupTweenerStep::Coloring))
        setColorAt(counter, step.getColor());
}

TupTweenerStep * TupItemTweener::stepAt(int index)
{
    return step(index);
}

TupTweenerStep * TupItemTweener::lastStep()
{
    int index = steps.size() - 1;
    return step(index);
}

void TupItemTweener::setPosAt(int index, const QPointF &pos)
{
    step(index)->setPosition(pos);
}

void TupItemTweener::setRotationAt(int index, double angle)
{
    step(index)->setRotation(angle);
}

void TupItemTweener::setScaleAt(int index, double sx, double sy)
{
    step(index)->setScale(sx, sy);
}

void TupItemTweener::setShearAt(int index, double sx, double sy)
{
    step(index)->setShear(sx, sy);
}

void TupItemTweener::setOpacityAt(int index, double opacity)
{
    step(index)->setOpacity(opacity);
}

void TupItemTweener::setColorAt(int index, const QColor &color)
{
    step(index)->setColor(color);
}

void TupItemTweener::setFrames(int length)
{
    frames = length;
}

int TupItemTweener::getFrames() const
{
    return frames;
}

int TupItemTweener::getInitFrame()
{
    return initFrame;
}

int TupItemTweener::getInitLayer()
{
    return initLayer;
}

int TupItemTweener::getInitScene()
{
    return initScene;
}

QPointF TupItemTweener::transformOriginPoint()
{
    return originPoint;
}

double TupItemTweener::initXScaleValue()
{
    return initialXScaleFactor;
}

double TupItemTweener::initYScaleValue()
{
    return initialYScaleFactor;
}

void TupItemTweener::fromXml(const QString &xml)
{
    #ifdef TUP_DEBUG
        qDebug() << "TupItemTweener::fromXml() - Tween content: ";
        qWarning() << xml;
    #endif
    
    QDomDocument doc;

    if (doc.setContent(xml)) {
        QDomElement root = doc.documentElement();

        tweenName = root.attribute("name");
        tweenType = TupItemTweener::Type(root.attribute("type").toInt());

        initFrame = root.attribute("initFrame").toInt();
        initLayer = root.attribute("initLayer").toInt();
        initScene = root.attribute("initScene").toInt();

        frames = root.attribute("frames").toInt();

        QString origin = root.attribute("origin"); // [x,y]
        QStringList list = origin.split(",");
        double x = list.first().toDouble();
        double y = list.last().toDouble();

        originPoint = QPointF(x, y);

        if (tweenType == TupItemTweener::Position) {
            tweenPath = root.attribute("coords");
            intervals = root.attribute("intervals");
        } else if (tweenType == TupItemTweener::Rotation) {
            rotationType = TupItemTweener::RotationType(root.attribute("rotationType").toInt());
            rotateSpeed = root.attribute("rotateSpeed").toDouble();
            rotateDirection = TupItemTweener::RotateDirection(root.attribute("rotateDirection").toInt());

            if (rotationType == TupItemTweener::Partial) {
                rotateSpeed = root.attribute("rotateLoop").toInt();
                rotateStartDegree = root.attribute("rotateStartDegree").toInt();
                rotateEndDegree = root.attribute("rotateEndDegree").toInt();
                rotateReverseLoop = root.attribute("rotateReverseLoop").toInt();
            }
        } else if (tweenType == TupItemTweener::Scale) {
            scaleAxes = TupItemTweener::TransformAxes(root.attribute("scaleAxes").toInt());
            initialXScaleFactor = root.attribute("initXScaleFactor").toDouble();
            initialYScaleFactor = root.attribute("initYScaleFactor").toDouble();
            scaleFactor = root.attribute("scaleFactor").toDouble();
            scaleIterations = root.attribute("scaleIterations").toInt();
            scaleLoop = root.attribute("scaleLoop").toInt();
            scaleReverseLoop = root.attribute("scaleReverseLoop").toInt();
        } else if (tweenType == TupItemTweener::Shear) {
            shearAxes = TupItemTweener::TransformAxes(root.attribute("shearAxes").toInt());
            shearFactor = root.attribute("shearFactor").toDouble();
            shearIterations = root.attribute("shearIterations").toInt();
            shearLoop = root.attribute("shearLoop").toInt();
            shearReverseLoop = root.attribute("shearReverseLoop").toInt();
        } else if (tweenType == TupItemTweener::Opacity) {
            initOpacityFactor = root.attribute("initOpacityFactor").toDouble();
            endOpacityFactor = root.attribute("endOpacityFactor").toDouble();
            opacityIterations = root.attribute("opacityIterations").toInt();
            opacityLoop = root.attribute("opacityLoop").toInt();
            opacityReverseLoop = root.attribute("opacityReverseLoop").toInt();
        } else if (tweenType == TupItemTweener::Coloring) {
            colorFillType = FillType(root.attribute("fillType").toInt());
            QString colorText = root.attribute("initialColor");
            QStringList list = colorText.split(",");
            int red = list.at(0).toInt();
            int green = list.at(1).toInt();
            int blue = list.at(2).toInt();
            initialColor = QColor(red, green, blue);

            colorText = root.attribute("endingColor");
            list = colorText.split(",");
            red = list.at(0).toInt();
            green = list.at(1).toInt();
            blue = list.at(2).toInt();
            endingColor = QColor(red, green, blue);

            colorIterations = root.attribute("colorIterations").toInt();
            colorLoop = root.attribute("colorLoop").toInt();
            colorReverseLoop = root.attribute("colorReverseLoop").toInt();
        }

        QDomNode node = root.firstChildElement("step");
        while (!node.isNull()) {
               QDomElement e = node.toElement();

               if (!e.isNull()) {
                   if (e.tagName() == "step") {
                       QString stepDoc;
                       {
                           QTextStream ts(&stepDoc);
                           ts << node;
                       }

                       TupTweenerStep *step = new TupTweenerStep(0);
                       step->fromXml(stepDoc);
                       addStep(*step);

                       delete step;
                    }
                }

                node = node.nextSibling();
        }
    }
}

QDomElement TupItemTweener::toXml(QDomDocument &doc) const
{
    #ifdef TUP_DEBUG
        qWarning() << "TupItemTweener::toXml() - Saving tween: " + tweenName;
        qWarning() << "TupItemTweener::toXml() - Type: " + QString::number(tweenType);
    #endif

    QDomElement root = doc.createElement("tweening");
    root.setAttribute("name", tweenName);
    root.setAttribute("type", tweenType);

    root.setAttribute("initFrame", QString::number(initFrame));
    root.setAttribute("initLayer", QString::number(initLayer));
    root.setAttribute("initScene", QString::number(initScene));
    root.setAttribute("frames", QString::number(frames));

    root.setAttribute("origin", QString::number(originPoint.x()) + "," + QString::number(originPoint.y()));

    if (tweenType == TupItemTweener::Position) {
        root.setAttribute("coords", tweenPath);
        root.setAttribute("intervals", intervals);
    } else if (tweenType == TupItemTweener::Rotation) {
        root.setAttribute("rotationType", rotationType);
        root.setAttribute("rotateSpeed", QString::number(rotateSpeed));
        root.setAttribute("rotateDirection", rotateDirection);

        if (rotationType == TupItemTweener::Partial) {
            root.setAttribute("rotateLoop", QString::number(rotateSpeed));
            root.setAttribute("rotateStartDegree", QString::number(rotateStartDegree));
            root.setAttribute("rotateEndDegree", QString::number(rotateEndDegree));
            root.setAttribute("rotateReverseLoop", QString::number(rotateReverseLoop));
        }
    } else if (tweenType == TupItemTweener::Scale) {
        root.setAttribute("scaleAxes", QString::number(scaleAxes));
        root.setAttribute("initXScaleFactor", QString::number(initialXScaleFactor));
        root.setAttribute("initYScaleFactor", QString::number(initialYScaleFactor));
        root.setAttribute("scaleFactor", QString::number(scaleFactor));
        root.setAttribute("scaleIterations", QString::number(scaleIterations));
        root.setAttribute("scaleLoop", QString::number(scaleLoop));
        root.setAttribute("scaleReverseLoop", QString::number(scaleReverseLoop));
    } else if (tweenType == TupItemTweener::Shear) {
        root.setAttribute("shearAxes", QString::number(shearAxes));
        root.setAttribute("shearFactor", QString::number(shearFactor));
        root.setAttribute("shearIterations", QString::number(shearIterations));
        root.setAttribute("shearLoop", QString::number(shearLoop));
        root.setAttribute("shearReverseLoop", QString::number(shearReverseLoop));
    } else if (tweenType == TupItemTweener::Opacity) {
        root.setAttribute("initOpacityFactor", QString::number(initOpacityFactor));
        root.setAttribute("endOpacityFactor", QString::number(endOpacityFactor));
        root.setAttribute("opacityIterations", QString::number(opacityIterations));
        root.setAttribute("opacityLoop", QString::number(opacityLoop));
        root.setAttribute("opacityReverseLoop", QString::number(opacityReverseLoop));
    } else if (tweenType == TupItemTweener::Coloring) {
        root.setAttribute("fillType", colorFillType);
        QString colorText = QString::number(initialColor.red()) + "," + QString::number(initialColor.green())
                            + "," + QString::number(initialColor.blue());
        root.setAttribute("initialColor", colorText);
        colorText = QString::number(endingColor.red()) + "," + QString::number(endingColor.green())
                    + "," + QString::number(endingColor.blue());
        root.setAttribute("endingColor", colorText);
        root.setAttribute("colorIterations", QString::number(colorIterations));
        root.setAttribute("colorLoop", QString::number(colorLoop));
        root.setAttribute("colorReverseLoop", QString::number(colorReverseLoop));
    }
 
    foreach (TupTweenerStep *step, steps.values())
        root.appendChild(step->toXml(doc));
    
    return root;
}

QGraphicsPathItem *TupItemTweener::graphicsPath() const
{
    QGraphicsPathItem *item = new QGraphicsPathItem();
    QPainterPath route;
    TupSvg2Qt::svgpath2qtpath(tweenPath, route);
    item->setPath(route);

    return item;
}

void TupItemTweener::setGraphicsPath(const QString &route)
{
    tweenPath = route;
}

QList<int> TupItemTweener::getIntervals()
{
    QList<int> sections;
    QStringList list = intervals.split(",");
    foreach (QString section, list)
        sections << section.toInt();

    return sections;
}

QString TupItemTweener::tweenTypeToString()
{
    QString label = "";
    switch (tweenType) {
        case TupItemTweener::Position :
            label = QString(tr("Position"));
            break;
        case TupItemTweener::Rotation :
            label = QString(tr("Rotation"));
            break;
        case TupItemTweener::Scale :
            label = QString(tr("Scale"));
            break;
        case TupItemTweener::Shear :
            label = QString(tr("Shear"));
            break;
        case TupItemTweener::Opacity :
            label = QString(tr("Opacity"));
            break;
        case TupItemTweener::Coloring :
            label = QString(tr("Coloring"));
            break;
        case TupItemTweener::Papagayo :
            label = QString(tr("Papagayo Lip-sync"));
            break;
    }

    return label;
}

TupItemTweener::RotationType TupItemTweener::tweenRotationType()
{
    return rotationType;
}

double TupItemTweener::tweenRotateSpeed()
{
    return rotateSpeed;
}

bool TupItemTweener::tweenRotateLoop()
{
    return rotateSpeed;
}

TupItemTweener::RotateDirection TupItemTweener::tweenRotateDirection()
{
    return rotateDirection;
}

int TupItemTweener::tweenRotateStartDegree()
{
    return rotateStartDegree;
}

int TupItemTweener::tweenRotateEndDegree()
{
    return rotateEndDegree;
}

bool TupItemTweener::tweenRotateReverseLoop()
{
    return rotateReverseLoop;
}

TupItemTweener::TransformAxes TupItemTweener::tweenScaleAxes()
{
    return scaleAxes;
}

double TupItemTweener::tweenScaleFactor()
{
    return scaleFactor;
}

int TupItemTweener::tweenScaleIterations()
{
    return scaleIterations;
}

int TupItemTweener::tweenScaleLoop()
{
    return scaleLoop;
}

int TupItemTweener::tweenScaleReverseLoop()
{
    return scaleReverseLoop;
}

TupItemTweener::TransformAxes TupItemTweener::tweenShearAxes()
{
    return shearAxes;
}

double TupItemTweener::tweenShearFactor()
{
    return shearFactor;
}

int TupItemTweener::tweenShearIterations()
{
    return shearIterations;
}

int TupItemTweener::tweenShearLoop()
{
    return shearLoop;
}

int TupItemTweener::tweenShearReverseLoop()
{
    return shearReverseLoop;
}

double TupItemTweener::tweenOpacityInitialFactor()
{
    return initOpacityFactor;
}

double TupItemTweener::tweenOpacityEndingFactor()
{
    return endOpacityFactor;
}

int TupItemTweener::tweenOpacityIterations()
{
    return opacityIterations;
}

int TupItemTweener::tweenOpacityLoop() 
{
    return opacityLoop;
}

int TupItemTweener::tweenOpacityReverseLoop()
{
    return opacityReverseLoop;
}

TupItemTweener::FillType TupItemTweener::tweenColorFillType()
{
    return colorFillType;
}

QColor TupItemTweener::tweenInitialColor()
{
    return initialColor;
}

QColor TupItemTweener::tweenEndingColor()
{
    return endingColor;
}

int TupItemTweener::tweenColorIterations()
{
    return colorIterations;
}

int TupItemTweener::tweenColorLoop()
{
    return colorLoop;
}

int TupItemTweener::tweenColorReverseLoop()
{
    return colorReverseLoop;
}

bool TupItemTweener::contains(TupItemTweener::Type type)
{
    for (int i=0; i < tweenList.size(); i++) {
         if (tweenList.at(i) == type)
             return true;
    }

    return false; 
}
