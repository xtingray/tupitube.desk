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

#include "tupsvgitem.h"
#include "tupserializer.h"
#include "tuplayer.h"

TupSvgItem::TupSvgItem(QGraphicsItem * parent) : QGraphicsSvgItem(parent)
{
    setAcceptHoverEvents(true);
}

TupSvgItem::TupSvgItem(const QString &file, TupFrame *currentFrame) : QGraphicsSvgItem(file)
{
    setAcceptHoverEvents(true);
    path = file;
    tupFrame = currentFrame;
}

TupSvgItem::~TupSvgItem()
{
    setAcceptHoverEvents(false);
}

void TupSvgItem::setSymbolName(const QString &symbolName)
{
    name = symbolName;
}

QString TupSvgItem::symbolName() const
{
    return name;
}

QString TupSvgItem::itemPath() const
{
    return path;
}

TupFrame *TupSvgItem::frame() const
{
    return tupFrame;
}

int TupSvgItem::frameIndex()
{
    return tupFrame->index();
}

bool TupSvgItem::layerIsVisible()
{
    TupLayer *layer = tupFrame->parentLayer();
    if (layer->isLayerVisible())
        return true;

    return false;
}

TupItemTweener* TupSvgItem::tween(const QString &id) const
{
    int total = tweens.count();
    for(int i=0; i < total; i++) {
        TupItemTweener *tween = tweens.at(i);
        if (tween->getTweenName().compare(id) == 0)
            return tween;
    }

    return NULL;
}

QList<TupItemTweener *> TupSvgItem::tweensList() const
{
    return tweens;
}

void TupSvgItem::rendering()
{
    QByteArray stream = data.toLocal8Bit();
    renderer()->load(stream);
}

void TupSvgItem::fromXml(const QString &xml)
{
    Q_UNUSED(xml);
}

QDomElement TupSvgItem::toXml(QDomDocument &doc) const
{
    if (name.length() == 0) {
        #ifdef TUP_DEBUG
            qDebug() << "TupFrame::fromXml() - Error: Object ID is null!";
        #endif
    }

    QDomElement root = doc.createElement("svg");
    root.setAttribute("id", name);
    root.appendChild(TupSerializer::properties(this, doc));

    int total = tweens.count();
    for(int i=0; i < total; i++)
        root.appendChild(tweens.at(i)->toXml(doc));

    return root;
}

void TupSvgItem::addTween(TupItemTweener *itemTween)
{
    tweens << itemTween;
}

bool TupSvgItem::hasTweens()
{
    return !tweens.isEmpty();
}

void TupSvgItem::removeTween(int index)
{
    tweens.removeAt(index);
}

void TupSvgItem::removeAllTweens()
{
    tweens.clear();
}

void TupSvgItem::setLastTweenPos(QPointF point)
{
    lastTweenPosition = point;
}

QPointF TupSvgItem::lastTweenPos()
{
    return lastTweenPosition;
}

void TupSvgItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    /*
    QRectF rect = sceneBoundingRect();
    int x = event->pos().x();
    int y = event->pos().y();
    
    int xmin = 0; 
    int xmax = rect.width();
    int ymin = 0;
    int ymax = rect.height();

    if (rect.width() >= 100) {
        xmin = (rect.width() - 100)/2;
        xmax = xmin + 100;
        ymin = (rect.height() - 100)/2;
        ymax = ymin + 100;
    }

    if (x >= xmin && x <= xmax) {
        if (y >= ymin && y <= ymax) {
            emit enabledChanged();
        }
    }
    */

    emit enabledChanged();
    QGraphicsSvgItem::hoverEnterEvent(event);
}

void TupSvgItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsSvgItem::hoverLeaveEvent(event);
}

bool TupSvgItem::transformationIsNotEdited()
{
    return doList.isEmpty() && undoList.isEmpty();
}

void TupSvgItem::saveInitTransformation()
{
    QDomDocument doc;
    doc.appendChild(TupSerializer::properties(this, doc));
    doList << doc.toString();
}

void TupSvgItem::storeItemTransformation(const QString &properties)
{
    doList << properties;
}

void TupSvgItem::undoTransformation()
{
    if (doList.count() > 1) {
        undoList << doList.takeLast();
        if (!doList.isEmpty()) {
            QString properties = doList.last();
            QDomDocument doc;
            doc.setContent(properties);
            TupSerializer::loadProperties(this, doc.documentElement());
        }
    }
}

void TupSvgItem::redoTransformation()
{
    if (!undoList.isEmpty()) {
        QString properties = undoList.takeLast();
        doList << properties;
        QDomDocument doc;
        doc.setContent(properties);
        TupSerializer::loadProperties(this, doc.documentElement());
    }
}
