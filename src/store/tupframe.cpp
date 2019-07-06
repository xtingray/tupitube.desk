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

#include "tupframe.h"
#include "tuplayer.h"
#include "tupsvg2qt.h"
#include "tupitemfactory.h"
#include "tupserializer.h"
#include "tupgraphicobject.h"
#include "tuppixmapitem.h"
#include "tuplibrary.h"
#include "tupitemgroup.h"
#include "tupitemtweener.h"
#include "tupprojectloader.h"
#include "tupscene.h"
#include "tuplayer.h"

#include <QApplication>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QCursor>
#include <QSvgRenderer>

TupFrame::TupFrame()
{
    type = Regular;
    isLocked = false;
    isVisible = true;
}

TupFrame::TupFrame(TupLayer *parent) : QObject(parent)
{
    layer = parent;
    frameName = "Frame";
    type = Regular;

    isLocked = false;
    isVisible = true;

    direction = "-1";
    shift = "0";

    zLevelIndex = (layer->layerIndex() + 2)*ZLAYER_LIMIT; // Layers levels starts from 2
}

TupFrame::TupFrame(TupBackground *bg, const QString &label) : QObject(bg)
{
    frameName = label;
    isLocked = false;
    isVisible = true;
    opacity = 1.0;

    direction = "-1";
    shift = "0";

    if (frameName.compare("landscape_dynamic") == 0) {
        zLevelIndex = 0;
        type = DynamicBg;
    } else {
        zLevelIndex = ZLAYER_LIMIT;
        type = StaticBg;
    }
}

TupFrame::~TupFrame()
{
    /*
    objectIndexes.clear();
    svgIndexes.clear();
    graphics.clear();
    svg.clear();
    */
}

void TupFrame::reset()
{
    objectIndexes.clear();
    svgIndexes.clear();

    graphics.clear();
    svg.clear();
}

void TupFrame::clear()
{
    for (int i=0; i<graphics.count(); i++) {
         TupGraphicObject *object = graphics.takeAt(i);
         if (object) {
             delete object;
             object = nullptr;
         }
    }

    reset();
}

void TupFrame::setFrameName(const QString &name)
{
    frameName = name;
}

QString TupFrame::getFrameName() const
{
    return frameName;
}

void TupFrame::setDynamicDirection(const QString &orientation)
{
    direction = orientation;
}

void TupFrame::setDynamicShift(const QString &pixels)
{
    shift = pixels;
}

TupBackground::Direction TupFrame::dynamicDirection() const
{
    qDebug() << "TupFrame::dynamicDirection() - direction: " << direction;
    return TupBackground::Direction(direction.toInt());
}

int TupFrame::dynamicShift() const
{
    qDebug() << "TupFrame::dynamicShift() - shift: " << shift;
    return shift.toInt();
}

void TupFrame::setLocked(bool locked)
{
    isLocked = locked;
}

bool TupFrame::isFrameLocked() const
{
    return isLocked;
}

TupFrame::FrameType TupFrame::frameType()
{
   return type;
}

void TupFrame::setVisible(bool visible)
{
    isVisible = visible;
}

bool TupFrame::isFrameVisible() const
{
    return isVisible;
}

void TupFrame::setFrameOpacity(double factor)
{
    opacity = factor;
}

double TupFrame::frameOpacity()
{
    return opacity;
}

void TupFrame::fromXml(const QString &xml)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupFrame::fromXml()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    QDomDocument document;
    if (!document.setContent(xml)) {    
        #ifdef TUP_DEBUG
            QString msg = "TupFrame::fromXml() - File corrupted!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
                qWarning() << "Content:";
                qWarning() << xml;
            #else
                tError() << msg;
                tWarning() << "Content:";
                tWarning() << xml;            
            #endif
        #endif
        
        return;
    }

    QDomElement root = document.documentElement();
    setFrameName(root.attribute("name", tr("Frame")));

    if (type == DynamicBg) {
        setDynamicDirection(root.attribute("direction", "0"));
        setDynamicShift(root.attribute("shift", "0"));
        setFrameOpacity(root.attribute("opacity", "1.0").toDouble());
    }

    if (type == StaticBg)
        setFrameOpacity(root.attribute("opacity", "1.0").toDouble());

    QDomNode n = root.firstChild();
    while (!n.isNull()) {
           QDomElement e = n.toElement();
           if (!e.isNull()) {
               if (e.tagName() == "object") {
                   QDomNode n2 = e.firstChild();

                   TupGraphicObject *last = nullptr; // This variable contains the object in case of tweening
                   while (!n2.isNull()) {
                          QDomElement e2 = n2.toElement();
                          if (e2.tagName() == "tweening" && last) {
                              TupItemTweener *tweener = new TupItemTweener(); 
                              QString newDoc;
                              {
                                QTextStream ts(&newDoc);
                                ts << n2;
                              }

                              tweener->fromXml(newDoc);
                              last->addTween(tweener);
                              parentScene()->addTweenObject(layer->layerIndex(), last);
                          } else {
                              QString newDoc;
                              {
                                QTextStream ts(&newDoc);
                                ts << n2;
                              }

                              QPointF point = QPointF();
                              QDomNode n3 = n2.firstChild();
                              while (!n3.isNull()) {
                                     QDomElement e3 = n3.toElement();
                                     if (e3.tagName() == "properties") {
                                         TupSvg2Qt::parsePointF(e3.attribute("pos"), point);
                                         break;
                                     }
                                     n3 = n3.nextSibling();
                              }

                              createItem(point, newDoc);
                              last = graphics.at(graphics.size()-1);
                          }
                          n2 = n2.nextSibling();
                   }
               } else if (e.tagName() == "svg") {
                          QString symbol = e.attribute("id");
                          if (symbol.length() > 0) {
                              TupLibraryObject *object = parentProject()->getLibrary()->getObject(symbol);

                              if (object) {
                                  QString path(object->getDataPath());
                                  QDomNode n2 = e.firstChild();
                                  TupSvgItem *svg = new TupSvgItem();

                                  while (!n2.isNull()) {
                                      QDomElement e2 = n2.toElement();
                                      if (e2.tagName() == "properties") {
                                          svg = new TupSvgItem(path, this);
                                          svg->setSymbolName(symbol);
                                          TupSerializer::loadProperties(svg, e2);

                                          addSvgItem(symbol, svg);
                                      } else if (e2.tagName() == "tweening") {
                                          TupItemTweener *tweener = new TupItemTweener();
                                          QString newDoc;
                                          {
                                              QTextStream ts(&newDoc);
                                              ts << n2;
                                          }
                                          tweener->fromXml(newDoc);
                                          svg->addTween(tweener);
                                          parentScene()->addTweenObject(layer->layerIndex(), svg);
                                      }
                                      n2 = n2.nextSibling(); 
                                  }
                              } else {
                                  #ifdef TUP_DEBUG
                                      QString msg = "TupFrame::fromXml() - Fatal Error: Object is NULL -> " + symbol;
                                      #ifdef Q_OS_WIN
                                          qDebug() << msg;
                                      #else
                                          tError() << msg;
                                      #endif
                                  #endif
                              }
                          } else {
                              #ifdef TUP_DEBUG
                                  QString msg = "TupFrame::fromXml() - Fatal Error: Object id is NULL!";
                                  #ifdef Q_OS_WIN
                                      qDebug() << msg;
                                  #else
                                      tError() << msg;
                                  #endif
                              #endif    
                          }
               } 
           }
           n = n.nextSibling();
    }
}

QDomElement TupFrame::toXml(QDomDocument &doc) const
{
    QDomElement root = doc.createElement("frame");
    root.setAttribute("name", frameName);

    if (type == DynamicBg) {
        root.setAttribute("direction", direction);
        root.setAttribute("shift", shift);
        root.setAttribute("opacity", QString::number(opacity));
    }

    if (type == StaticBg)
        root.setAttribute("opacity", QString::number(opacity));

    doc.appendChild(root);

    int objectsCount = graphics.count();
    int svgCount = svg.count();

    if (objectsCount == 0 && svgCount == 0)
        return root;

    if (objectsCount > 0 && svgCount == 0) {
        foreach (TupGraphicObject *object, graphics)
                 root.appendChild(object->toXml(doc));
        return root;
    }

    if (svgCount > 0 && objectsCount == 0) {
        foreach (TupSvgItem *svg, svg)
                 root.appendChild(svg->toXml(doc));
        return root;
    }

    GraphicObjects itemList = graphics;
    SvgObjects svgList = svg;

    do {
           int objectZValue = itemList.at(0)->itemZValue();
           int svgZValue = static_cast<int> (svgList.at(0)->zValue());

           if (objectZValue < svgZValue) {
               TupGraphicObject *object = itemList.takeFirst();
               root.appendChild(object->toXml(doc));
           } else { 
               TupSvgItem *svg = svgList.takeFirst();
               root.appendChild(svg->toXml(doc));
           }

           if (itemList.isEmpty()) {
               foreach (TupSvgItem *svg, svgList) 
                        root.appendChild(svg->toXml(doc));
               break;
           } else {
               if (svgList.isEmpty()) {
                   foreach (TupGraphicObject *object, itemList)
                            root.appendChild(object->toXml(doc));
                   break;
               }
           }
    } while (true);

    return root;
}

void TupFrame::addLibraryItem(const QString &id, TupGraphicLibraryItem *libraryItem)
{
    QGraphicsItem *item = libraryItem->item();
    QDomDocument dom;
    TupItemFactory itemFactory;

    if (TupItemGroup *group = qgraphicsitem_cast<TupItemGroup *>(item)) {
        dom.appendChild(dynamic_cast<TupAbstractSerializable *>(group)->toXml(dom));
        item = itemFactory.create(dom.toString());
    } else {
        if (TupPathItem *path = qgraphicsitem_cast<TupPathItem *>(item)) {
            dom.appendChild(dynamic_cast<TupAbstractSerializable *>(path)->toXml(dom));
            item = itemFactory.create(dom.toString());
        }
    } 

    addItem(id, item);
}

void TupFrame::addItem(const QString &id, QGraphicsItem *item)
{
    /*
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupFrame::addItem()]";
        #else
            T_FUNCINFO;
        #endif
    #endif
    */

    item->setZValue(zLevelIndex);
    zLevelIndex++;

    TupGraphicObject *object = new TupGraphicObject(item, this);
    object->setObjectName(id);

    graphics.append(object);
    objectIndexes.append(id);
}

bool TupFrame::removeImageItemFromFrame(const QString &id)
{
    bool found = false;

    for (int i=0; i<objectIndexes.size(); i++) {
        if (objectIndexes[i].compare(id) == 0) {
            removeGraphicAt(i);
            found = true;
        }
    }

    return found;
}

void TupFrame::updateIdFromFrame(const QString &oldId, const QString &newId)
{
    for (int i=0; i<objectIndexes.size(); i++) {
        if (objectIndexes.at(i).compare(oldId) == 0) {
            objectIndexes[i] = newId;

            TupGraphicObject *object = graphics.at(i);
            TupGraphicLibraryItem *libraryItem = static_cast<TupGraphicLibraryItem *>(object->item());
            libraryItem->setSymbolName(newId);

            object->setObjectName(newId);
            object->setItem(libraryItem);

            graphics[i] = object;
        }
    }
}

void TupFrame::addSvgItem(const QString &id, TupSvgItem *item)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupFrame::addSvgItem()] - id: " << id;
        #else
            T_FUNCINFO << id;
        #endif
    #endif
    
    svgIndexes.append(id);

    item->setZValue(zLevelIndex);
    zLevelIndex++;

    svg.append(item);
}

bool TupFrame::removeSvgItemFromFrame(const QString &id)
{
    bool found = false;

    for (int i = 0; i < svgIndexes.size(); ++i) {
         if (svgIndexes.at(i).compare(id) == 0) {
             removeSvgAt(i); 
             found = true;
         }
    }

    return found;
}

void TupFrame::updateSvgIdFromFrame(const QString &oldId, const QString &newId)
{
    for (int i = 0; i < svgIndexes.size(); ++i) {
         if (svgIndexes.at(i).compare(oldId) == 0) {
             svgIndexes[i] = newId;
             TupSvgItem *svgItem = svg.at(i);
             svgItem->setSymbolName(newId);
             svg[i] = svgItem;
         }
    }
}

void TupFrame::insertObject(int position, TupGraphicObject *object, const QString &label)
{
    graphics.insert(position, object);
    objectIndexes.insert(position, label);

    for (int i=position+1; i < graphics.size(); ++i) {
         int zLevel = graphics.at(i)->itemZValue();
         graphics.at(i)->setItemZValue(zLevel + 1);
    }

    QGraphicsItem *item = object->item();
    int itemLevel =  static_cast<int> (item->zValue());

    for (int i=0; i < svg.size(); ++i) {
         int zLevel = static_cast<int> (svg.at(i)->zValue());
         if (zLevel < itemLevel)
             svg.at(i)->setZValue(zLevel + 1);
    }

    zLevelIndex++;
}

void TupFrame::insertItem(int position, QGraphicsItem *item, const QString &label)
{
    TupGraphicObject *object = new TupGraphicObject(item, this);
    insertObject(position, object, label);
}

void TupFrame::insertSvg(int position, TupSvgItem *item, const QString &label)
{
    svg.insert(position, item);
    svgIndexes.insert(position, label);

    for (int i=position+1; i < svg.size(); ++i) {
         int zLevel = static_cast<int> (svg.at(i)->zValue());
         svg.at(i)->setZValue(zLevel + 1);
    }

    int itemLevel = static_cast<int> (item->zValue());

    for (int i=0; i < graphics.size(); ++i) {
         int zLevel = graphics.at(i)->itemZValue();
         if (zLevel < itemLevel)
             graphics.at(i)->setItemZValue(zLevel + 1);
    }

    zLevelIndex++;
}

int TupFrame::createItemGroup(int position, QList<int> group)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupFrame::createItemGroup()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    int zBase = static_cast<int> (this->item(position)->zValue());
    TupItemGroup *itemGroup = new TupItemGroup;

    foreach (int index, group) {
        QGraphicsItem *item = this->item(index);
        item->setOpacity(1.0);
        itemGroup->addToGroup(item);
    }

    int size = group.size()-1;
    for (int i=size;i>=0;i--)
         removeGraphicAt(group.at(i));

    QGraphicsItem *block = qgraphicsitem_cast<QGraphicsItem *>(itemGroup);
    block->setZValue(zBase);
    insertItem(position, block, "group");

    return position;
}

QList<QGraphicsItem *> TupFrame::splitGroup(int position)
{
    QList<QGraphicsItem *> items;
    QGraphicsItem *object = qgraphicsitem_cast<TupItemGroup *>(item(position));

    if (object) {
        if (TupItemGroup *group = qgraphicsitem_cast<TupItemGroup *>(item(position))) {
            removeGraphicAt(position);
            items = group->childItems();
            foreach (QGraphicsItem *child, group->childItems()) {
                     group->removeFromGroup(child);
                     addItem("path", child);
            }
        }
    }

    return items;
}

void TupFrame::replaceItem(int position, QGraphicsItem *item)
{
    TupGraphicObject *toReplace = this->graphicAt(position);

    if (toReplace)
        toReplace->setItem(item);
}

bool TupFrame::moveItem(TupLibraryObject::Type objectType, int currentIndex, int action)
{
    if ((svg.size() + graphics.size()) == 1)
        return true;

    int layerIndex = 0;
    if (type == Regular)
        layerIndex = layer->layerIndex();

    MoveItemType move = MoveItemType(action); 
    switch(move) {
           case MoveBack :
             {
                int zMin = (layerIndex + 1)*ZLAYER_LIMIT;

                if (objectType == TupLibraryObject::Svg) {
                    int zLimit = static_cast<int> (svg.at(currentIndex)->zValue());
                    if (zLimit == zMin) {
                        #ifdef TUP_DEBUG
                            QString msg = "TupFrame::moveItem() - MoveBack: Minimum level has been reached! (SVG)";
                            #ifdef Q_OS_WIN
                                qWarning() << msg;
                            #else
                                tWarning() << msg;
                            #endif
                        #endif
                        return true;
                    }

                    if (svg.size() > 1) {
                        TupSvgItem *object = svg.takeAt(currentIndex);
                        QString id = svgIndexes.takeAt(currentIndex);

                        object->setZValue(zMin);
                        svg.insert(0, object);
                        svgIndexes.insert(0, id);

                        for (int i=1; i <= currentIndex; ++i) {
                             int zLevel = static_cast<int> (svg.at(i)->zValue());
                             if (zLevel < zLimit)
                                 svg.at(i)->setZValue(zLevel + 1);
                        } 
                    } else {
                        svg.at(currentIndex)->setZValue(zMin);
                    }

                    for (int i=0; i < graphics.size(); ++i) {
                         int zLevel = graphics.at(i)->itemZValue();
                         if (zLevel < zLimit)
                             graphics.at(i)->setItemZValue(zLevel + 1);
                    }

                    return true;
                } else {
                    int zLimit = graphics.at(currentIndex)->itemZValue();
                    if (zLimit == zMin) {
                        #ifdef TUP_DEBUG
                            QString msg = "TupFrame::moveItem() - MoveBack: Minimum level has been reached! (VECTOR/RASTER)";
                            #ifdef Q_OS_WIN
                                qWarning() << msg; 
                            #else
                                tWarning() << msg;
                            #endif
                        #endif
                        return true;
                    }

                    if (graphics.size() > 1) {
                        TupGraphicObject *object = graphics.takeAt(currentIndex);
                        QString id = objectIndexes.takeAt(currentIndex);

                        object->setItemZValue(zMin);
                        graphics.insert(0, object);
                        objectIndexes.insert(0, id);

                        for (int i=1; i < graphics.size(); ++i) {
                             int zLevel = graphics.at(i)->itemZValue();
                             if (zLevel < zLimit)
                                 graphics.at(i)->setItemZValue(zLevel + 1);
                        }

                    } else {
                        graphics.at(currentIndex)->setItemZValue(zMin);
                    }
            
                    for (int i=0; i < svg.size(); ++i) {
                         int zLevel = static_cast<int> (svg.at(i)->zValue());
                         if (zLevel < zLimit)
                             svg.at(i)->setZValue(zLevel + 1);
                    }

                    return true;
                }
             }
           case MoveToFront :
             {
                int zMax = zLevelIndex - 1;
                if (objectType == TupLibraryObject::Svg) {
                    int zLimit = static_cast<int> (svg.at(currentIndex)->zValue());
                    if (zLimit == zMax) {
                        #ifdef TUP_DEBUG
                            QString msg = "TupFrame::moveItem() - MoveToFront: Maximum level has been reached! (SVG)";
                            #ifdef Q_OS_WIN
                                qWarning() << msg;
                            #else
                                tWarning() << msg;
                            #endif
                        #endif
                        return true;
                    }

                    if (svg.size() > 1) {
                        TupSvgItem *object = svg.takeAt(currentIndex);
                        QString id = svgIndexes.takeAt(currentIndex);

                        for (int i=currentIndex; i < svg.size(); ++i) {
                             int zLevel = static_cast<int> (svg.at(i)->zValue());
                             svg.at(i)->setZValue(zLevel - 1);
                        }

                        object->setZValue(zMax);
                        svg.append(object);
                        svgIndexes.append(id);
                    } else {
                        svg.at(currentIndex)->setZValue(zMax);
                    }

                    for (int i=0; i < graphics.size(); ++i) {
                         int zLevel = graphics.at(i)->itemZValue();
                         if (zLevel > zLimit)
                             graphics.at(i)->setItemZValue(zLevel - 1);
                    }

                    return true;
                } else {
                    int zLimit = graphics.at(currentIndex)->itemZValue();
                    if (zLimit == zMax) {
                        #ifdef TUP_DEBUG
                            QString msg = "TupFrame::moveItem() - MoveToFront: Maximum level has been reached! (VECTOR/RASTER)";
                            #ifdef Q_OS_WIN
                                qWarning() << msg;
                            #else
                                tWarning() << msg;
                            #endif
                        #endif
                        return true;
                    }

                    if (graphics.size() > 1) {
                        TupGraphicObject *object = graphics.takeAt(currentIndex);
                        QString id = objectIndexes.takeAt(currentIndex);

                        for (int i=currentIndex; i < graphics.size(); ++i) {
                             int zLevel = graphics.at(i)->itemZValue();
                             graphics.at(i)->setItemZValue(zLevel - 1);
                        }

                        object->setItemZValue(zMax);
                        graphics.append(object);
                        objectIndexes.append(id);
                    } else {
                        graphics.at(currentIndex)->setItemZValue(zMax);
                    }

                    for (int i=0; i < svg.size(); ++i) {
                         int zLevel = static_cast<int> (svg.at(i)->zValue());
                         if (zLevel > zLimit)
                             svg.at(i)->setZValue(zLevel - 1);
                    }

                    return true;
                }
             }
           case MoveOneLevelBack :
             {
                // int zMin = (layerIndex + 1)*ZLAYER_LIMIT;
                int zMin = (layerIndex + 1)*ZLAYER_LIMIT;

                if (objectType == TupLibraryObject::Svg) {
                    int zLevel = static_cast<int> (svg.at(currentIndex)->zValue());
                    if (zLevel == zMin) {
                        #ifdef TUP_DEBUG
                            QString msg = "TupFrame::moveItem() - MoveOneLevelBack: Minimum level has been reached! (SVG)";
                            #ifdef Q_OS_WIN
                                qWarning() << msg;
                            #else
                                tWarning() << msg;
                            #endif
                        #endif
                        return true;
                    }

                    TupSvgItem *object = svg.at(currentIndex);
                    int zLimit = static_cast<int> (object->zValue());

                    if ((svg.size() > 1) && (currentIndex > 0)) {
                        object = svg.at(currentIndex - 1);
                        int downzValue = static_cast<int> (object->zValue());
                        if (downzValue == (zLimit - 1)) {
                            svg.at(currentIndex)->setZValue(downzValue);
                            svg.at(currentIndex - 1)->setZValue(zLimit);
                            svg.swap(currentIndex, currentIndex -1);
                            svgIndexes.swap(currentIndex, currentIndex -1);
                            return true;
                        } else {
                            for (int i=0; i < graphics.size(); ++i) {
                                 int zLevel = graphics.at(i)->itemZValue();
                                 if (zLevel == (zLimit - 1)) {
                                     svg.at(currentIndex)->setZValue(zLevel);
                                     graphics.at(i)->setItemZValue(zLimit);
                                     return true;
                                 }
                            }
                        } 
                    } else {
                        if (!graphics.isEmpty()) {
                            for (int i=0; i < graphics.size(); ++i) {
                                 int zLevel = graphics.at(i)->itemZValue();
                                 if (zLevel == (zLimit - 1)) {
                                     svg.at(currentIndex)->setZValue(zLevel);
                                     graphics.at(i)->setItemZValue(zLimit);
                                     return true;
                                 }
                            }
                        } else {                            
                            #ifdef TUP_DEBUG
                                QString msg = "TupFrame::moveItem() - Fatal Error: Something went wrong [ case MoveOneLevelBack/Svg ]";
                                #ifdef Q_OS_WIN
                                    qDebug() << msg;
                                #else
                                    tError() << msg;
                                #endif
                            #endif

                            return false;
                        }
                    }
                } else {
                    if (graphics.at(currentIndex)->itemZValue() == zMin) {
                        #ifdef TUP_DEBUG
                            QString msg = "TupFrame::moveItem() - MoveOneLevelBack: Minimum level has been reached! (VECTOR/RASTER)";
                            #ifdef Q_OS_WIN
                                qWarning() << msg;
                            #else
                                tWarning() << msg;
                            #endif
                        #endif
                        return true;
                    }

                    TupGraphicObject *object = graphics.at(currentIndex);
                    int zLimit = object->itemZValue();

                    if ((graphics.size() > 1) && (currentIndex > 0)) {
                        object = graphics.at(currentIndex - 1);
                        int downzValue = object->itemZValue();
                        if (downzValue == (zLimit - 1)) {
                            graphics.at(currentIndex)->setItemZValue(downzValue);
                            graphics.at(currentIndex - 1)->setItemZValue(zLimit);
                            graphics.swap(currentIndex, currentIndex - 1);
                            objectIndexes.swap(currentIndex, currentIndex - 1);
                            return true;
                        } else {
                            for (int i=0; i < svg.size(); ++i) {
                                 int zLevel = static_cast<int> (svg.at(i)->zValue());
                                 if (zLevel == (zLimit - 1)) {
                                     graphics.at(currentIndex)->setItemZValue(zLevel);
                                     svg.at(i)->setZValue(zLimit);
                                     return true;
                                 }
                            }
                        }
                    } else {
                        if (!svg.isEmpty()) {
                            for (int i=0; i < svg.size(); ++i) {
                                 int zLevel = static_cast<int> (svg.at(i)->zValue());
                                 if (zLevel == (zLimit - 1)) {
                                     graphics.at(currentIndex)->setItemZValue(zLevel);
                                     svg.at(i)->setZValue(zLimit);
                                     return true;
                                 }
                            }
                        } else {                            
                            #ifdef TUP_DEBUG
                                QString msg = "TupFrame::moveItem() - Fatal Error: Something went wrong [ case MoveOneLevelBack/Items ]";
                                #ifdef Q_OS_WIN
                                    qDebug() << msg;
                                #else
                                    tError() << msg;
                                #endif
                            #endif
                            
                            return false;
                        }
                    }
                }
             }
           break;
           case MoveOneLevelToFront :
             {
                int zMax = zLevelIndex - 1;

                if (objectType == TupLibraryObject::Svg) {
                    int zLevel = static_cast<int> (svg.at(currentIndex)->zValue());
                    if (zLevel == zMax) {
                        #ifdef TUP_DEBUG
                            QString msg = "TupFrame::moveItem() - MoveOneLevelToFront: Maximum level has been reached! (SVG)";
                            #ifdef Q_OS_WIN
                                qWarning() << msg;
                            #else
                                tWarning() << msg;
                            #endif
                        #endif
                        return true;
                    }

                    TupSvgItem *object = svg.at(currentIndex);
                    int zLimit = static_cast<int> (object->zValue());

                    if (currentIndex < (svg.size() - 1)) {
                        object = svg.at(currentIndex + 1);
                        int upZValue = static_cast<int> (object->zValue());
                        if (upZValue == (zLimit + 1)) {
                            svg.at(currentIndex)->setZValue(upZValue);
                            svg.at(currentIndex + 1)->setZValue(zLimit);
                            svg.swap(currentIndex, currentIndex + 1);
                            svgIndexes.swap(currentIndex, currentIndex + 1);
                            return true;
                        } else {
                            for (int i=0; i < graphics.size(); ++i) {
                                 int zLevel = graphics.at(i)->itemZValue();
                                 if (zLevel == (zLimit + 1)) {
                                     svg.at(currentIndex)->setZValue(zLevel);
                                     graphics.at(i)->setItemZValue(zLimit);
                                     return true;
                                 }
                            }
                        }
                    } else {
                        if (!graphics.isEmpty()) {
                            for (int i=0; i < graphics.size(); ++i) {
                                 int zLevel = graphics.at(i)->itemZValue();
                                 if (zLevel == (zLimit + 1)) {
                                     svg.at(currentIndex)->setZValue(zLevel);
                                     graphics.at(i)->setItemZValue(zLimit);
                                     return true;
                                 }
                            }
                        } else {                            
                            #ifdef TUP_DEBUG
                                QString msg = "TupFrame::moveItem() - Fatal Error: Something went wrong [ case MoveOneLevelToFront/Svg ]";
                                #ifdef Q_OS_WIN
                                    qDebug() << msg;
                                #else
                                    tError() << msg;
                                #endif
                            #endif
                            
                            return false;
                        }
                    }
                } else {
                    if (graphics.at(currentIndex)->itemZValue() == zMax) {
                        #ifdef TUP_DEBUG
                            QString msg = "TupFrame::moveItem() - MoveOneLevelToFront: Maximum level has been reached! (VECTOR/RASTER)";
                            #ifdef Q_OS_WIN
                                qWarning() << msg;
                            #else
                                tWarning() << msg;
                            #endif
                        #endif
                        return true;
                    }

                    TupGraphicObject *object = graphics.at(currentIndex);
                    int zLimit = object->itemZValue();

                    if (currentIndex < (graphics.size() - 1)) {
                        object = graphics.at(currentIndex + 1);
                        int upZValue = object->itemZValue();
                        if (upZValue == (zLimit + 1)) {
                            graphics.at(currentIndex)->setItemZValue(upZValue);
                            graphics.at(currentIndex + 1)->setItemZValue(zLimit);
                            graphics.swap(currentIndex, currentIndex + 1);
                            objectIndexes.swap(currentIndex, currentIndex + 1);
                            return true;
                        } else {
                            for (int i=0; i < svg.size(); ++i) {
                                 int zLevel = static_cast<int> (svg.at(i)->zValue());
                                 if (zLevel == (zLimit + 1)) {
                                     graphics.at(currentIndex)->setItemZValue(zLevel);
                                     svg.at(i)->setZValue(zLimit);
                                     return true;
                                 }
                            }
                        }
                    } else {
                        if (!svg.isEmpty()) {
                            for (int i=0; i < svg.size(); ++i) {
                                 int zLevel = static_cast<int> (svg.at(i)->zValue());
                                 if (zLevel == (zLimit + 1)) {
                                     graphics.at(currentIndex)->setItemZValue(zLevel);
                                     svg.at(i)->setZValue(zLimit);
                                     return true;
                                 }
                            }
                        } else {
                            #ifdef TUP_DEBUG
                                QString msg = "TupFrame::moveItem() - Fatal Error: Something went wrong [ case MoveOneLevelToFront/Items ]";
                                #ifdef Q_OS_WIN
                                    qDebug() << msg;
                                #else
                                    tError() << msg;
                                #endif
                            #endif
                            return false;
                        }
                    }
                }
             }
           break;
    }

    #ifdef TUP_DEBUG
        QString msg = "TupFrame::moveItem() - Fatal Error: Something went wrong!";
        #ifdef Q_OS_WIN
            qDebug() << msg;
        #else
            tError() << msg;
        #endif
    #endif
    
    return false;
}

bool TupFrame::removeGraphic(int position)
{
    TupGraphicObject *object = graphics.at(position);
    itemsUndoList << object;
    QString index = objectIndexes.at(position);
    objectUndoIndexes << index;
    objectUndoPos << position;

    return removeGraphicAt(position);
}

void TupFrame::restoreGraphic()
{
    if (!objectUndoPos.isEmpty()) {
        int position = objectUndoPos.takeLast();
        TupGraphicObject *object = itemsUndoList.takeLast();
        QString index = objectUndoIndexes.takeLast();

        insertObject(position, object, index);
    }
}

bool TupFrame::removeGraphicAt(int position)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupFrame::removeGrahpicAt()]";
        #else
            T_FUNCINFO << position;
        #endif
    #endif

    if ((position < 0) || (position >= graphics.size())) {
        #ifdef TUP_DEBUG
            QString msg = "TupFrame::removeGraphicAt() - Fatal Error: invalid object index! [ " + QString::number(position) + " ]";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return false;
    }

    TupGraphicObject *object = graphics.at(position);
    if (object) {
        if (object->hasTweens())
            this->parentScene()->removeTweenObject(layer->layerIndex(), object);

        int zLimit = graphics.at(position)->itemZValue();
        objectIndexes.removeAt(position);
        graphics.removeAt(position);

        for (int i=position; i < graphics.size(); ++i) {
             int zLevel = graphics.at(i)->itemZValue();
             graphics.at(i)->setItemZValue(zLevel - 1);
        }
        for (int i=0; i < svg.size(); ++i) {
             int zLevel = static_cast<int> (svg.at(i)->zValue());
             if (zLevel > zLimit)
                 svg.at(i)->setZValue(zLevel-1);
        }
        zLevelIndex--;

        return true;
    } 

    #ifdef TUP_DEBUG
        QString msg = "TupFrame::removeGraphicAt() - Error: Object at position " + QString::number(position) + " is NULL!";
        #ifdef Q_OS_WIN
            qDebug() << msg;
        #else
            tError() << msg;
        #endif
    #endif

    return false;
}

bool TupFrame::removeSvg(int position)
{
    TupSvgItem *item = svg.at(position);
    svgUndoList << item;
    QString index = svgIndexes.at(position);
    svgUndoIndexes << index;
    svgUndoPos << position;

    return removeSvgAt(position);
}

void TupFrame::restoreSvg()
{
    if (!svgUndoPos.isEmpty()) {
        int position = svgUndoPos.takeLast();
        TupSvgItem *item = svgUndoList.takeLast();
        QString index = svgUndoIndexes.takeLast();

        insertSvg(position, item, index);
    }
}

bool TupFrame::removeSvgAt(int position)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupFrame::removeSvgAt()]";
        #else
            T_FUNCINFO;
        #endif
    #endif
    
    if ((position < 0) || (position >= svg.size())) {
        #ifdef TUP_DEBUG
            QString msg = "TupFrame::removeSvgAt() - Fatal Error: invalid object index! [ " + QString::number(position) + " ]";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return false;
    }

    TupSvgItem *item = svg.at(position);

    if (item) {
        QGraphicsScene *scene = item->scene();
        if (scene)
            scene->removeItem(item);

        int zLimit = static_cast<int> (svg.at(position)->zValue());
        svgIndexes.removeAt(position);
        svg.removeAt(position);

        for (int i=position; i < svg.size(); ++i) {
             int zLevel = static_cast<int> (svg.at(i)->zValue());
             svg.at(i)->setZValue(zLevel-1);
        }
        for (int i=0; i < graphics.size(); ++i) {
             int zLevel = graphics.at(i)->itemZValue();
             if (zLevel > zLimit)
                 graphics.at(i)->setItemZValue(zLevel - 1);
        }
        zLevelIndex--;

        #ifdef TUP_DEBUG
            QString msg = "TupFrame::removeSvgAt() - SVG object has been removed (" + QString::number(position) + ")";
            #ifdef Q_OS_WIN
                qWarning() << msg;
            #else
                tWarning() << msg;
            #endif
        #endif

        return true;
    }

    #ifdef TUP_DEBUG
        QString msg = "TupFrame::removeSvgAt() - Error: Couldn't find SVG object (" + QString::number(position) + ")";
        #ifdef Q_OS_WIN
            qDebug() << msg;
        #else
            tError() << msg;
        #endif
    #endif

    return false;
}

QGraphicsItem *TupFrame::createItem(QPointF coords, const QString &xml, bool loaded)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupFrame::createItem()]";
        #else
            tWarning() << "coords: " << coords;
            T_FUNCINFO << xml;
        #endif
    #endif

    TupItemFactory itemFactory;
    // SQA: Refactor the code related to the library variable within this class

    TupLibrary *library = parentProject()->getLibrary();
    if (library)
        itemFactory.setLibrary(library);

    QGraphicsItem *graphicItem = itemFactory.create(xml);
    if (graphicItem) {
        graphicItem->setPos(coords);
        QString id = "path";
        if (library) {
            if (itemFactory.getType() == TupItemFactory::Library)
                id = itemFactory.itemID(xml);
        }

        addItem(id, graphicItem);

        if (type == Regular) {
            if (loaded)
                TupProjectLoader::createItem(parentScene()->objectIndex(), parentLayer()->objectIndex(), index(), graphics.size() - 1,
                                             coords, TupLibraryObject::Item, xml, parentProject());
        }

        return graphicItem;
    }

    #ifdef TUP_DEBUG
        QString msg1 = "TupFrame::createItem() - Fatal Error: Couldn't create QGraphicsItem object";
        QString msg2 = "TupFrame::createItem() - xml: ";
        #ifdef Q_OS_WIN
            qDebug() << msg1;
            qDebug() << msg2;
            qDebug() << xml;
        #else
            tError() << msg1;
            tError() << msg2;    
            tError() << xml;
        #endif
    #endif

    return nullptr;
}

TupSvgItem *TupFrame::createSvgItem(QPointF coords, const QString &xml, bool loaded)
{
    QDomDocument document;
    if (!document.setContent(xml)) {
        #ifdef TUP_DEBUG
            QString msg1 = "TupFrame::createSvgItem() - Fatal Error: Svg xml content is invalid!";
            QString msg2 = "TupFrame::createSvgItem() - xml: ";
            #ifdef Q_OS_WIN
                qDebug() << msg1;
                qDebug() << msg2;
                qDebug() << xml;
            #else
                tError() << msg1;
                tError() << msg2;    
                tError() << xml;
            #endif
        #endif
        return nullptr;
    }

    QDomElement root = document.documentElement();
    QString id = root.attribute("id");
    TupLibraryObject *object = parentProject()->getLibrary()->getObject(id);
    if (object) {
        QString path = object->getDataPath();
        TupSvgItem *item = new TupSvgItem(path, this);
        if (item) {
            item->setSymbolName(id);
            item->moveBy(coords.x(), coords.y()); 
            addSvgItem(id, item);
            if (loaded)
                TupProjectLoader::createItem(parentScene()->objectIndex(), parentLayer()->objectIndex(), index(),
                                             svg.size() - 1, coords, TupLibraryObject::Svg, xml, parentProject());
            return item;
        } else {        
            #ifdef TUP_DEBUG
                QString msg = "TupFrame::createSvgItem() - Fatal Error: Svg object is invalid!";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif
            return nullptr;
        }
    }

    #ifdef TUP_DEBUG
        QString msg = "TupFrame::createSvgItem() - Fatal Error: TupLibraryObject variable is NULL!";
        #ifdef Q_OS_WIN
            qDebug() << msg;
        #else
            tError() << msg;
        #endif
    #endif
    
    return nullptr;
}

void TupFrame::setGraphics(GraphicObjects objects)
{
    graphics = objects;
}

void TupFrame::setSvgObjects(SvgObjects items)
{
    svg = items;
}

GraphicObjects TupFrame::graphicItems() const
{
    return graphics;
}

SvgObjects TupFrame::svgItems() const
{
    return svg;
}

TupGraphicObject *TupFrame::graphicAt(int position) const
{
    if ((position < 0) || (position >= graphics.count())) {
        #ifdef TUP_DEBUG
            QString msg = "TupFrame::graphicAt() - Fatal Error: index out of bound [ " + QString::number(position) + " ] /  Total items: " + QString::number(graphics.count());
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        
        return nullptr;
    } 

    return graphics.at(position);
}

TupSvgItem *TupFrame::svgAt(int position) const
{
    if ((position < 0) || (position >= svg.count())) {
        #ifdef TUP_DEBUG
            QString msg = "TupFrame::svgAt() -  Fatal Error: index out of bound [ " + QString::number(position) + " ] / Total items: " + QString::number(svg.count());
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        
        return nullptr;
    }

    return svg.at(position);
}

QGraphicsItem *TupFrame::item(int position) const
{
    if ((position < 0) || (position >= graphics.count())) {
        #ifdef TUP_DEBUG
            QString msg = "TupFrame::item() -  Fatal Error: index out of bound [ " + QString::number(position) + " ] / Total items: " + QString::number(graphics.count());
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif

        return nullptr;
    }

    TupGraphicObject *object = graphics.at(position);
    if (object) {
        QGraphicsItem *item = object->item();
        if (item) {
            return item;
        } else {
            #ifdef TUP_DEBUG
                QString msg = "TupFrame::item() -  Fatal Error: QGraphicsItem object is NULL!";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif
            return nullptr;
        }
    }

    #ifdef TUP_DEBUG
        QString msg = "TupFrame::item() -  Fatal Error: TupGraphicObject is NULL!";
        #ifdef Q_OS_WIN
            qDebug() << msg;
        #else
            tError() << msg;
        #endif
    #endif

    return nullptr;
}

// SQA: Verify if this method is used by something... anything! 

int TupFrame::indexOf(TupGraphicObject *object) const
{
    return graphics.indexOf(object);
}

int TupFrame::indexOf(TupSvgItem *object) const
{
    return svg.indexOf(object);
}

int TupFrame::indexOf(QGraphicsItem *item) const
{
    if (item) {
        for (int i = 0; i < graphics.size(); i++) {
             TupGraphicObject *object = graphics.at(i);
             if (static_cast<int>(object->item()->zValue()) == static_cast<int>(item->zValue()))
                 return graphics.indexOf(object);
        }
    }

    return -1;
}

/*
void TupFrame::setRepeat(int repeat)
{
    k->repeat = repeat;
}

int TupFrame::repeat() const
{
    return k->repeat;
}
*/

int TupFrame::index() const
{
    return parentLayer()->visualIndexOf(const_cast<TupFrame *>(this));
}

TupLayer *TupFrame::parentLayer() const
{
    return static_cast<TupLayer *>(parent());
}

TupScene *TupFrame::parentScene() const
{
    return parentLayer()->parentScene();
}

TupProject *TupFrame::parentProject() const
{
    if (type == Regular)
        return parentLayer()->parentProject();

    TupBackground *bg = static_cast<TupBackground *>(parent());
    return bg->project();
}

int TupFrame::graphicsCount()
{
    return graphics.count();
}

int TupFrame::svgItemsCount()
{
    return svg.count();
}

int TupFrame::itemsTotalCount()
{
    return graphics.count() + svg.count();
}

int TupFrame::getTopZLevel()
{
    return zLevelIndex;
}

bool TupFrame::isEmpty()
{
    if (graphicsCount() + svgItemsCount() > 0)
        return false;

    return true;
}

void TupFrame::reloadGraphicItem(const QString &id, const QString &path)
{
    for (int i = 0; i < objectIndexes.size(); ++i) {
         if (objectIndexes.at(i).compare(id) == 0) {
             TupGraphicObject *old = graphics.at(i);
             QGraphicsItem *oldItem = old->item();

             QPixmap pixmap(path);
             TupPixmapItem *image = new TupPixmapItem;
             image->setPixmap(pixmap);

             TupGraphicLibraryItem *item = new TupGraphicLibraryItem;
             item->setSymbolName(id);
             item->setItem(image);
             item->setTransform(oldItem->transform());
             item->setPos(oldItem->pos());
             item->setEnabled(true);
             item->setFlags(oldItem->flags());
             item->setZValue(oldItem->zValue());

             TupGraphicObject *object = new TupGraphicObject(item, this);
             graphics[i] = object;
         }
    }
}

void TupFrame::reloadSVGItem(const QString &id, TupLibraryObject *object)
{
    if (object) {
        for (int i = 0; i < svgIndexes.size(); ++i) {
            if (svgIndexes.at(i).compare(id) == 0) {
                TupSvgItem *oldItem = svg.value(i);
                if (oldItem) {
                    QString path = object->getDataPath();

                    TupSvgItem *item = new TupSvgItem(path, this);
                    item->setSymbolName(object->getSymbolName());
                    item->setTransform(oldItem->transform());
                    item->setPos(oldItem->pos());
                    item->setEnabled(true);
                    item->setFlags(oldItem->flags());
                    item->setZValue(oldItem->zValue());

                    svg[i] = item;
                }

                return;
            }
        }
    }
}

void TupFrame::updateZLevel(int zLevelIndex)
{
    int max = 0;
    int graphicsSize = graphics.size();
    for (int i = 0; i < graphicsSize; ++i) {
         TupGraphicObject *object = graphics.at(i);
         if (object) {  
             int currentZValue = object->itemZValue();
             int zLevel = zLevelIndex + (currentZValue % ZLAYER_LIMIT);
             object->setItemZValue(zLevel);
             if (i == (graphicsSize-1)) {
                 if (zLevel > max)
                     max = zLevel;
             }
         }
    }

    graphicsSize = svgIndexes.size();
    for (int i = 0; i < graphicsSize; ++i) {
         TupSvgItem *item = svg.value(i);
         if (item) {
             int currentZValue = static_cast<int> (item->zValue());
             int zLevel = zLevelIndex + (currentZValue % ZLAYER_LIMIT);
             item->setZValue(zLevel);
             if (i == (graphicsSize-1)) {
                 if (zLevel > max)
                     max = zLevel;
             }
         }
    }

    if (max > 0) {
        zLevelIndex = max;
        zLevelIndex++;
    } else {
        zLevelIndex = (layer->layerIndex() + 1)*ZLAYER_LIMIT;
    }
}

void TupFrame::checkTransformationStatus(TupLibraryObject::Type itemType, int index)
{
    if (itemType == TupLibraryObject::Svg) {
        TupSvgItem *item = svg.at(index);
        if (item) {
            if (item->transformationIsNotEdited())
                item->saveInitTransformation();
        }
        return;
    }
    
    TupGraphicObject *object = graphics.at(index);
    if (object) {
        if (object->transformationIsNotEdited())
            object->saveInitTransformation();
    }
}

void TupFrame::storeItemTransformation(TupLibraryObject::Type itemType, int index, const QString &properties)
{
    if (itemType == TupLibraryObject::Svg) {
        TupSvgItem *item = svg.at(index);
        if (item)
            item->storeItemTransformation(properties);
    } else {
        TupGraphicObject *object = graphics.at(index);
        if (object)
            object->storeItemTransformation(properties);
    }
}

void TupFrame::undoTransformation(TupLibraryObject::Type itemType, int index)
{
    if (itemType == TupLibraryObject::Svg) {
        TupSvgItem *item = svg.at(index);
        if (item)
            item->undoTransformation(); 
    } else {
        TupGraphicObject *object = graphics.at(index);
        if (object)
            object->undoTransformation();
    }
}

void TupFrame::redoTransformation(TupLibraryObject::Type itemType, int index)
{
    if (itemType == TupLibraryObject::Svg) {
        TupSvgItem *item = svg.at(index);
        if (item)
            item->redoTransformation();
    } else {
        TupGraphicObject *object = graphics.at(index);
        if (object)
            object->redoTransformation();
    }
}

void TupFrame::checkBrushStatus(int itemIndex)
{
     TupGraphicObject *object = graphics.at(itemIndex);
     if (object->brushIsNotEdited())
         object->saveInitBrush();
}

void TupFrame::setBrushAtItem(int itemIndex, const QString &xml)
{
#ifdef TUP_DEBUG
    #ifdef Q_OS_WIN
        qDebug() << "[TupFrame::setBrushAtItem()]";
    #else
        T_FUNCINFO;
    #endif
#endif

    TupGraphicObject *object = graphics.at(itemIndex);
    object->setBrush(xml);
}

void TupFrame::redoBrushAction(int itemIndex)
{
    TupGraphicObject *object = graphics.at(itemIndex);
    object->redoBrushAction();
}

void TupFrame::undoBrushAction(int itemIndex)
{
    TupGraphicObject *object = graphics.at(itemIndex);
    object->undoBrushAction();
}

void TupFrame::checkPenStatus(int itemIndex)
{
     TupGraphicObject *object = graphics.at(itemIndex);
     if (object->penIsNotEdited())
         object->saveInitPen();
}

void TupFrame::setPenAtItem(int itemIndex, const QString &xml)
{
    TupGraphicObject *object = graphics.at(itemIndex);
    object->setPen(xml);
}

void TupFrame::redoPenAction(int itemIndex)
{
    TupGraphicObject *object = graphics.at(itemIndex);
    object->redoPenAction();
}

void TupFrame::undoPenAction(int itemIndex)
{
    TupGraphicObject *object = graphics.at(itemIndex);
    object->undoPenAction();
}
