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

#include "tupgraphicobject.h"
#include "tuplayer.h"
#include "tupscene.h"
#include "tupserializer.h"

TupGraphicObject::TupGraphicObject(QGraphicsItem *graphic, TupFrame *parent) : QObject(parent)
{
    /*
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupGraphicObject()]";
        #else
            TINIT;
        #endif
    #endif	
    */

    graphicItem = graphic;
    tupFrame = parent;

    initItemData();
}

TupGraphicObject::~TupGraphicObject()
{
    /*
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[~TupGraphicObject()]";
        #else
            TEND;
        #endif
    #endif	
    */
}

void TupGraphicObject::fromXml(const QString &xml)
{
    Q_UNUSED(xml);
}

QDomElement TupGraphicObject::toXml(QDomDocument &doc) const
{
    QDomElement object = doc.createElement("object");

    if (TupAbstractSerializable *serialData = dynamic_cast<TupAbstractSerializable *>(graphicItem))
        object.appendChild(serialData->toXml(doc));

    int total = tweens.count();
    for(int i=0; i < total; i++)
        object.appendChild(tweens.at(i)->toXml(doc));

    return object;
}

void TupGraphicObject::setItem(QGraphicsItem *graphic)
{
    if (graphic) {
        graphicItem = graphic;
        initItemData();
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupGraphicObject::setItem() - Fatal Error: QGraphicsItem is null!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    } 
}

QGraphicsItem *TupGraphicObject::item() const
{
    return graphicItem;
}

void TupGraphicObject::setObjectName(const QString &objectName)
{
    name = objectName;
}

QString TupGraphicObject::objectName() const
{
    return name;
}

void TupGraphicObject::initItemData()
{
    if (! graphicItem->data(ScaleX).isValid())
        graphicItem->setData(ScaleX, 1.0);

    if (! graphicItem->data(ScaleY).isValid())
        graphicItem->setData(ScaleY, 1.0);

    if (! graphicItem->data(Rotate).isValid())
        graphicItem->setData(Rotate, 0.0);

    if (! graphicItem->data(TranslateX).isValid())
        graphicItem->setData(TranslateX, 0.0);

    if (! graphicItem->data(TranslateY).isValid())
        graphicItem->setData(TranslateY, 0.0);
}

void TupGraphicObject::addTween(TupItemTweener *itemTween)
{
    int total = tweens.count();
    for(int i=0; i < total; i++) {
        TupItemTweener *tween = tweens.at(i);
        if (tween->type() == itemTween->type()) {
            tweens[i] = itemTween;
            return;
        }
    }

    tweens << itemTween;
}

bool TupGraphicObject::hasTweens()
{
    return !tweens.isEmpty();
}

void TupGraphicObject::removeTween(int index)
{
    tweens.removeAt(index);
}

void TupGraphicObject::removeAllTweens()
{
    tweens.clear();
}

TupItemTweener *TupGraphicObject::tween(const QString &id) const
{
    int total = tweens.count();
    for(int i=0; i < total; i++) {
        TupItemTweener *tween = tweens.at(i);
        if (tween->name().compare(id) == 0)
            return tween;
    }

    return NULL;
}

QList<TupItemTweener *> TupGraphicObject::tweensList() const
{
    return tweens;
}

TupFrame *TupGraphicObject::frame() const
{
    return tupFrame;
}

void TupGraphicObject::setFrame(TupFrame *currentFrame)
{
    tupFrame = currentFrame;
}

int TupGraphicObject::frameIndex()
{
    return tupFrame->index();
}

bool TupGraphicObject::layerIsVisible()
{
    TupLayer *layer = tupFrame->layer();
    if (layer->isVisible())
        return true;

    return false;
}

int TupGraphicObject::objectIndex() const
{
    return tupFrame->indexOf(const_cast<TupGraphicObject *>(this));
}

void TupGraphicObject::setLastTweenPos(QPointF point)
{
    lastTweenPosition = point;
}

QPointF TupGraphicObject::lastTweenPos()
{   
    return lastTweenPosition;
}

void TupGraphicObject::setItemZValue(int value)
{
    graphicItem->setZValue(value);
}

int TupGraphicObject::itemZValue()
{
    int value = static_cast<int> (graphicItem->zValue());
    return value;
}

bool TupGraphicObject::transformationIsNotEdited()
{
    return transformDoList.isEmpty() && transformUndoList.isEmpty();
}

void TupGraphicObject::saveInitTransformation()
{
    QDomDocument doc;
    doc.appendChild(TupSerializer::properties(graphicItem, doc));
    transformDoList << doc.toString();
}

void TupGraphicObject::storeItemTransformation(const QString &properties)
{
    transformDoList << properties;
}

void TupGraphicObject::undoTransformation()
{
    if (transformDoList.count() > 1) {
        transformUndoList << transformDoList.takeLast();
        if (!transformDoList.isEmpty()) {
            QString properties = transformDoList.last();
            QDomDocument doc;
            doc.setContent(properties);
            TupSerializer::loadProperties(graphicItem, doc.documentElement());
        }
    }
}

void TupGraphicObject::redoTransformation()
{
    if (!transformUndoList.isEmpty()) {
        QString properties = transformUndoList.takeLast();
        transformDoList << properties;
        QDomDocument doc;
        doc.setContent(properties);
        TupSerializer::loadProperties(graphicItem, doc.documentElement());
    }
}

bool TupGraphicObject::brushIsNotEdited()
{
    return brushDoList.isEmpty() && brushUndoList.isEmpty();
}

void TupGraphicObject::saveInitBrush()
{
    if (QAbstractGraphicsShapeItem *shape = qgraphicsitem_cast<QAbstractGraphicsShapeItem *>(graphicItem)) {
        QBrush brush = shape->brush();
        QDomDocument doc; 
        doc.appendChild(TupSerializer::brush(&brush, doc));
        brushDoList << doc.toString();
    }
}

void TupGraphicObject::setBrush(const QString &xml)
{
    if (QAbstractGraphicsShapeItem *shape = qgraphicsitem_cast<QAbstractGraphicsShapeItem *>(graphicItem)) {
        brushDoList << xml;
        QBrush brush;
        QDomDocument doc;
        doc.setContent(xml);

        TupSerializer::loadBrush(brush, doc.documentElement());
        shape->setBrush(brush);
    }
}

void TupGraphicObject::redoBrushAction()
{
    if (!brushUndoList.isEmpty()) {
        if (QAbstractGraphicsShapeItem *shape = qgraphicsitem_cast<QAbstractGraphicsShapeItem *>(graphicItem)) {
            QString xml = brushUndoList.takeLast();
            brushDoList << xml;
            QBrush brush;
            QDomDocument doc;
            doc.setContent(xml);

            TupSerializer::loadBrush(brush, doc.documentElement());
            shape->setBrush(brush);
        }
    }
}

void TupGraphicObject::undoBrushAction()
{
    if (brushDoList.count() > 1) {
        if (QAbstractGraphicsShapeItem *shape = qgraphicsitem_cast<QAbstractGraphicsShapeItem *>(graphicItem)) {
            brushUndoList << brushDoList.takeLast();
            if (!brushDoList.isEmpty()) {
                QString xml = brushDoList.last();
                QBrush brush;
                QDomDocument doc;
                doc.setContent(xml);

                TupSerializer::loadBrush(brush, doc.documentElement());
                shape->setBrush(brush);
            }
        }
    }
}

bool TupGraphicObject::penIsNotEdited()
{
    return penDoList.isEmpty() && penUndoList.isEmpty();
}

void TupGraphicObject::saveInitPen()
{
    if (QAbstractGraphicsShapeItem *shape = qgraphicsitem_cast<QAbstractGraphicsShapeItem *>(graphicItem)) {
        QPen pen = shape->pen();
        QDomDocument doc;
        doc.appendChild(TupSerializer::pen(&pen, doc));
        penDoList << doc.toString();
    }
}

void TupGraphicObject::setPen(const QString &xml)
{
    if (QAbstractGraphicsShapeItem *shape = qgraphicsitem_cast<QAbstractGraphicsShapeItem *>(graphicItem)) {
        QPen pen;
        QDomDocument doc;
        doc.setContent(xml);

        TupSerializer::loadPen(pen, doc.documentElement());
        shape->setPen(pen);
        penDoList << xml;
    }
}

void TupGraphicObject::redoPenAction()
{
    if (!penUndoList.isEmpty()) {
        if (QAbstractGraphicsShapeItem *shape = qgraphicsitem_cast<QAbstractGraphicsShapeItem *>(graphicItem)) {
            QString xml = penUndoList.takeLast();
            penDoList << xml;
            QPen pen;
            QDomDocument doc;
            doc.setContent(xml);

            TupSerializer::loadPen(pen, doc.documentElement());
            shape->setPen(pen);
        }
    }
}

void TupGraphicObject::undoPenAction()
{
    if (penDoList.count() > 1) {
        if (QAbstractGraphicsShapeItem *shape = qgraphicsitem_cast<QAbstractGraphicsShapeItem *>(graphicItem)) {
            penUndoList << penDoList.takeLast();
            if (!penDoList.isEmpty()) {
                QString xml = penDoList.last();
                QPen pen;
                QDomDocument doc;
                doc.setContent(xml);

                TupSerializer::loadPen(pen, doc.documentElement());
                shape->setPen(pen);
            }
        }
    }
}

QString TupGraphicObject::toString() const
{
    QString data;
    QTextStream stream(&data);
    QDomDocument doc;
    QDomElement e = this->toXml(doc);
    e.save(stream, 4);

    return data;
}
