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

#include "tuptweenerstep.h"

#include <QVector>

TupTweenerStep::TupTweenerStep(int idx): TupAbstractSerializable()
{
    index = idx;
    flags = None;
}

TupTweenerStep::~TupTweenerStep()
{
}

void TupTweenerStep::setPosition(const QPointF &pos)
{
    position = pos;
    flags |= Motion;
}

void TupTweenerStep::setRotation(double angle)
{
    rotation = angle;
    flags |= Rotation;
}

void TupTweenerStep::setScale(double sx, double sy)
{
    scale.x = sx;
    scale.y = sy;
    flags |= Scale;
}

void TupTweenerStep::setShear(double sh, double sv)
{
    shear.x = sh;
    shear.y = sv;
    flags |= Shear;
}

void TupTweenerStep::setOpacity(double factor)
{
    opacity = factor;
    flags |= Opacity;
}

void TupTweenerStep::setColor(const QColor &c)
{
    color = c;
    flags |= Coloring;
}

bool TupTweenerStep::has(Type type) const
{
    return flags & type;
}

int TupTweenerStep::getIndex() const
{
    return index;
}

QPointF TupTweenerStep::getPosition() const
{
    return position;
}

double TupTweenerStep::horizontalScale() const
{
    return scale.x;
}

double TupTweenerStep::verticalScale() const
{
    return scale.y;
}

double TupTweenerStep::horizontalShear() const
{
    return shear.x;
}

double TupTweenerStep::verticalShear() const
{
    return shear.y;
}

double TupTweenerStep::getRotation() const
{
    return rotation;
}

double TupTweenerStep::getOpacity() const
{
    return opacity;
}

QColor TupTweenerStep::getColor() const
{
    return color;
}

QDomElement TupTweenerStep::toXml(QDomDocument& doc) const
{
    QDomElement step = doc.createElement("step");
    step.setAttribute("value", index);
    
    if (this->has(TupTweenerStep::Motion)) {
        QDomElement e = doc.createElement("position");
        e.setAttribute("x", QString::number(position.x()));
        e.setAttribute("y", QString::number(position.y()));

        step.appendChild(e);
    }

    if (this->has(TupTweenerStep::Rotation)) {
        QDomElement e = doc.createElement("rotation");
        e.setAttribute("angle", QString::number(rotation));
    
        step.appendChild(e);
    }
    
    if (this->has(TupTweenerStep::Scale)) {
        QDomElement e = doc.createElement("scale");
        e.setAttribute("sx", QString::number(scale.x));
        e.setAttribute("sy", QString::number(scale.y));
        
        step.appendChild(e);
    }
    
    if (this->has(TupTweenerStep::Shear)) {
        QDomElement e = doc.createElement("shear");
        e.setAttribute("sh", QString::number(shear.x));
        e.setAttribute("sv", QString::number(shear.y));
        
        step.appendChild(e);
    }
    
    if (this->has(TupTweenerStep::Opacity)) {
        QDomElement e = doc.createElement("opacity");
        e.setAttribute("opacity", QString::number(opacity));

        step.appendChild(e);
    }

    if (this->has(TupTweenerStep::Coloring)) {
        QDomElement e = doc.createElement("color");
        QString red = QString::number(color.red());
        QString green = QString::number(color.green());
        QString blue = QString::number(color.blue());

        e.setAttribute("red", red);
        e.setAttribute("green", green);
        e.setAttribute("blue", blue);

        step.appendChild(e);
    }
    
    return step;
}

void TupTweenerStep::fromXml(const QString& xml)
{
    QDomDocument doc;
    
    if (doc.setContent(xml)) {
        QDomElement root = doc.documentElement();
        QDomNode node = root.firstChild();
        index = root.attribute("value").toInt();
        
        while (!node.isNull()) {
            QDomElement e = node.toElement();
            if (!e.isNull()) {
                if (e.tagName() == "position") {
                    setPosition(QPointF(e.attribute("x").toDouble(), e.attribute("y").toDouble()));
                } else if (e.tagName() == "rotation") {
                    setRotation(e.attribute("angle").toDouble());
                } else if (e.tagName() == "scale") {
                    setScale(e.attribute("sx").toDouble(), e.attribute("sy").toDouble());
                } else if (e.tagName() == "shear") {
                    setShear(e.attribute("sh").toDouble(), e.attribute("sv").toDouble());
                } else if (e.tagName() == "opacity") {
                    setOpacity(e.attribute("opacity").toDouble());
                } else if (e.tagName() == "color") {
                    int red = e.attribute("red").toInt();
                    int green = e.attribute("green").toInt();
                    int blue = e.attribute("blue").toInt();
                    setColor(QColor(red, green, blue));
                }
            }
            node = node.nextSibling();
        }
    }
}
