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

#include "tuplayer.h"
#include "tupscene.h"
#include "tupprojectloader.h"

TupLayer::TupLayer(TupScene *parentScene, int pos)
{
    scene = parentScene;
    index = pos;
    isVisible = true;
    layerName = tr("Layer");
    framesCounter = 0;
    isLocked = false;
    opacity = 1.0;
}

TupLayer::~TupLayer()
{
    frames.clear();
    lipsyncList.clear();
    tweeningGraphicObjects.clear();
    tweeningSvgObjects.clear();
}

Frames TupLayer::getFrames()
{
    return frames;
}

void TupLayer::setFrames(const Frames &array)
{
    frames = array;
    framesCounter = array.count();
}

void TupLayer::setFrame(int index, TupFrame *frame)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupLayer::setFrame()]";
        #else
            T_FUNCINFO;
            tWarning() << "At index -> " << index;
        #endif
    #endif

    frames.insert(index, frame);
    framesCounter++;
}

void TupLayer::setLayerName(const QString &name)
{
    layerName = name;
}

QString TupLayer::getLayerName() const
{
    return layerName;
}

void TupLayer::setLocked(bool locked)
{
    isLocked = locked;
}

bool TupLayer::isLayerLocked() const
{
    return isLocked;
}

void TupLayer::setLayerVisibility(bool visible)
{
    isVisible = visible;
}

bool TupLayer::isLayerVisible() const
{
    return isVisible;
}

void TupLayer::setOpacity(qreal factor)
{
    opacity = factor;
}

qreal TupLayer::getOpacity()
{
    return opacity;
}

TupFrame *TupLayer::createFrame(QString name, int position, bool loaded)
{
    /*
    if (position < 0 || position > frames.count()) {
        tFatal() << "TupLayer::createFrame -> index is out of range: " << position << " - frames.count(): " << frames.count();
        return 0;
    }
    */

    if (position < 0)
        return 0;

    TupFrame *frame = new TupFrame(this);
    framesCounter++;
    frame->setFrameName(name);
    frames.insert(position, frame);

    if (loaded)
        TupProjectLoader::createFrame(parentScene()->objectIndex(), objectIndex(), position, name, parentProject());

    return frame;
}

TupLipSync *TupLayer::createLipSync(const QString &name, const QString &soundFile, int initFrame)
{
    TupLipSync *lipsync = new TupLipSync(name, soundFile, initFrame);
    lipsyncList << lipsync;

    return lipsync;
}

void TupLayer::addLipSync(TupLipSync *lipsync)
{
    if (lipsync)
        lipsyncList << lipsync;
}

int TupLayer::lipSyncCount()
{
     return lipsyncList.count();
}

Mouths TupLayer::getLipSyncList()
{
     return lipsyncList;
}

bool TupLayer::restoreFrame(int index)
{
    if (undoFrames.count() > 0) {
        TupFrame *frame = undoFrames.takeLast();
        if (frame) {
            frames.insert(index, frame);
            framesCounter++;
            return true;
        }
        return false;
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupLayer::restoreFrame() - Fatal Error: "
            "No available frames to restore index -> " + QString::number(index);
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }

    return false;
}

bool TupLayer::removeFrame(int pos)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupLayer::removeFrame()]";
        #else
            T_FUNCINFO << "pos -> " << pos;
        #endif
    #endif

    TupFrame *toRemove = frameAt(pos);
    if (toRemove) {
        undoFrames << frames.takeAt(pos);
        framesCounter--;

        return true;
    }

    return false;
}

bool TupLayer::removeLipSync(const QString &name)
{
    int size = lipsyncList.size();
    for (int i = 0; i < size; i++) {
         TupLipSync *lipsync = lipsyncList.at(i);
         if (lipsync->getLipSyncName().compare(name) == 0) {
             lipsyncList.removeAt(i);
             return true;
         }
    }

    return false;
}

bool TupLayer::resetFrame(int pos)
{
    TupFrame *toReset = frameAt(pos);

    if (toReset) {
        resettedFrames << frames.takeAt(pos);
        TupFrame *frame = new TupFrame(this); 
        frame->setFrameName(tr("Frame"));
        frames.insert(pos, frame);

        return true;
    }

    return false;
}

bool TupLayer::restoreResettedFrame(int pos)
{
    if (resettedFrames.count() > 0) {
        TupFrame *frame = resettedFrames.takeLast();
        if (frame) {
            frames.removeAt(pos);
            frames.insert(pos, frame);
            return true;
        }
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupLayer::restoreResettedFrame() - Fatal Error: "
            "No available resetted frames to restore -> " + QString::number(resettedFrames.count());
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }

    return false;
}

int TupLayer::resettedFramesCount()
{
    return resettedFrames.count();
}

void TupLayer::clear()
{
    for (int i=0; i<frames.count(); i++) {
         TupFrame *frame = frames.takeAt(i);
         frame->clear();
         delete frame;
         frame = NULL;
    }
    
    layerName = "";
    framesCounter = 0;
    lipsyncList.clear();
    tweeningGraphicObjects.clear();
    tweeningSvgObjects.clear();
}

bool TupLayer::moveFrame(int from, int to)
{
    if (from < 0 || from >= frames.count() || to < 0 || to > frames.count())
        return false;
 
    TupFrame *origin = frames.value(from);
    if (origin) {
        QString label = origin->getFrameName();
        TupFrame *frame = new TupFrame(this);
        frame->setFrameName(label);

        TupFrame *target = frames.value(to);
        if (target) {
            QString targetLabel = target->getFrameName();
            origin->setFrameName(targetLabel);
            frames.insert(to, origin);
            frames.insert(from, frame);
            return true;
        }
    }
    
    return false;
}

bool TupLayer::exchangeFrame(int from, int to)
{
    if (from < 0 || from >= frames.count() || to < 0 || to >= frames.count()) {
        #ifdef TUP_DEBUG
            QString msg = "TupLayer::exchangeFrame() - Fatal Error: frame indexes are invalid -> from: " + QString::number(from) + " / to: " + QString::number(to);
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return false;
    }

    frames.swap(from, to);

    return true;
}

bool TupLayer::extendFrame(int pos, int times)
{
    if (pos < 0 || pos >= frames.count())
        return false;

    TupFrame *toExpand = frameAt(pos);
    if (toExpand) {
        QDomDocument doc;
        doc.appendChild(toExpand->toXml(doc));
        QString data = doc.toString();

        for (int i=1; i<=times; i++) {
            TupFrame *frame = new TupFrame(this);
            frame->fromXml(data);
            frames.insert(pos + i, frame);
            framesCounter++;
        }

        return true;
    }

    return false;
}


TupFrame *TupLayer::frameAt(int position) const
{
    if (position < 0 || position >= frames.count()) {
        #ifdef TUP_DEBUG
            QString msg1 = "TupLayer::frameAt() - Fatal Error: frame index out of bound : " + QString::number(position);
            QString msg2 = "TupLayer::frameAt() - Fatal Error: index limit at layer(" + QString::number(index) + ") : " + QString::number(frames.count()-1);
            #ifdef Q_OS_WIN
                qDebug() << msg1;
                qDebug() << msg2;
            #else
                tError() << msg1;
                tError() << msg2;
            #endif
        #endif    

        return 0;
    }

    return frames.value(position);
}


void TupLayer::fromXml(const QString &xml)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupLayer::fromXml()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    QDomDocument document;
    if (!document.setContent(xml))
        return;

    QDomElement root = document.documentElement();
    setLayerName(root.attribute("name", getLayerName()));
    setOpacity(root.attribute("opacity", "1.0").toDouble());
    setLayerVisibility(root.attribute("visible", "1").toInt());
    QDomNode n = root.firstChild();

    while (!n.isNull()) {
           QDomElement e = n.toElement();
           if (!e.isNull()) {
               if (e.tagName() == "frame") {
                   TupFrame *frame = createFrame(e.attribute("name"), frames.count(), true);
                   if (frame) {
                       QString newDoc;
                       {
                         QTextStream ts(&newDoc);
                         ts << n;
                       }
                       frame->fromXml(newDoc);
                   }
               } else if (e.tagName() == "lipsync") {
                   TupLipSync *lipsync = createLipSync(e.attribute("name"), e.attribute("soundFile"), e.attribute("initFrame").toInt());
                   if (lipsync) {
                       QString newDoc;
                       {
                           QTextStream ts(&newDoc);
                           ts << n;
                       }
                       lipsync->fromXml(newDoc);
                   }
               }
           }
           n = n.nextSibling();
    }
}

QDomElement TupLayer::toXml(QDomDocument &doc) const
{
    QDomElement root = doc.createElement("layer");
    root.setAttribute("name", layerName);
    root.setAttribute("opacity", QString::number(opacity));
    root.setAttribute("visible", QString::number(isVisible));

    doc.appendChild(root);
    int framesCounter = frames.size();
    for (int i = 0; i < framesCounter; i++) {
         TupFrame *frame = frames.at(i);
         root.appendChild(frame->toXml(doc));
    }

    int lipsyncTotal = lipsyncList.size();
    for (int i = 0; i < lipsyncTotal; i++) {
         TupLipSync *lipSync = lipsyncList.at(i);
         root.appendChild(lipSync->toXml(doc));
    }

    return root;
}

TupScene *TupLayer::parentScene() const
{
    return scene;
    // return static_cast<TupScene *>(parent());
}

TupProject *TupLayer::parentProject() const
{
    return parentScene()->project();
}

void TupLayer::updateLayerIndex(int pos)
{
    index = pos;
}

int TupLayer::layerIndex()
{
    return index;
}

int TupLayer::visualIndexOf(TupFrame *frame) const
{
    return frames.indexOf(frame);
}

int TupLayer::objectIndex() const
{
    return parentScene()->visualIndexOf(const_cast<TupLayer *>(this));
}

int TupLayer::framesCount() const
{
    return framesCounter;
}

void TupLayer::addTweenObject(TupGraphicObject *object)
{
    tweeningGraphicObjects << object;
}

void TupLayer::addTweenObject(TupSvgItem *object)
{
    tweeningSvgObjects << object;
}

void TupLayer::updateTweenObject(int index, TupGraphicObject *object)
{
    tweeningGraphicObjects.replace(index, object);
}

void TupLayer::updateTweenObject(int index, TupSvgItem *object)
{
    tweeningSvgObjects.replace(index, object);
}

void TupLayer::removeTweenObject(TupGraphicObject *object)
{
    if (tweeningGraphicObjects.size() > 0)
        tweeningGraphicObjects.removeAll(object);
}

void TupLayer::removeTweenObject(TupSvgItem *object)
{
    if (tweeningSvgObjects.size() > 0)
        tweeningSvgObjects.removeAll(object);
}

QList<TupGraphicObject *> TupLayer::getTweeningGraphicObjects() const
{
    return tweeningGraphicObjects;
}

QList<TupSvgItem *> TupLayer::getTweeningSvgObjects() const
{
    return tweeningSvgObjects;
}

bool TupLayer::tweenExists(const QString &name, TupItemTweener::Type type)
{
    foreach (TupGraphicObject *object, tweeningGraphicObjects) {
        QList<TupItemTweener *> list = object->tweensList();
        foreach(TupItemTweener *tween, list) {
            if ((tween->getTweenName().compare(name) == 0) && (tween->getType() == type))
                return true;
        }
    }

    foreach (TupSvgItem *object, tweeningSvgObjects) {
        QList<TupItemTweener *> list = object->tweensList();
        foreach(TupItemTweener *tween, list) {
            if ((tween->getTweenName().compare(name) == 0) && (tween->getType() == type))
                return true;
        }
    }

    return false;
}

bool TupLayer::removeTween(const QString &name, TupItemTweener::Type type)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupLayer::removeTween()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    foreach (TupGraphicObject *object, tweeningGraphicObjects) {
        QList<TupItemTweener *> list = object->tweensList();
        int total = list.count();
        for (int i=0; i < total; i++) {
            TupItemTweener *tween = list.at(i);
            if ((tween->getTweenName().compare(name) == 0) && (tween->getType() == type)) {
                object->removeTween(i);
                if (total == 1)
                    removeTweenObject(object);
                return true;
            }
        }
    }

    foreach (TupSvgItem *object, tweeningSvgObjects) {
        QList<TupItemTweener *> list = object->tweensList();
        int total = list.count();
        for (int i=0; i < total; i++) {
            TupItemTweener *tween = list.at(i);
            if ((tween->getTweenName().compare(name) == 0) && (tween->getType() == type)) {
                object->removeTween(i);
                if (total == 1)
                    removeTweenObject(object);
                return true;
            }
        }
    }

    return false;
}

void TupLayer::removeAllTweens()
{
    foreach (TupGraphicObject *object, tweeningGraphicObjects) {
        object->removeAllTweens();
        removeTweenObject(object);
    }

    foreach (TupSvgItem *object, tweeningSvgObjects) {
        object->removeAllTweens();
        removeTweenObject(object);
    }
}

void TupLayer::removeTweensFromFrame(int frameIndex)
{
    foreach (TupGraphicObject *object, tweeningGraphicObjects) {
        if (object->frame()->index() == frameIndex) {
            object->removeAllTweens();
            removeTweenObject(object);
        }
    }

    foreach (TupSvgItem *object, tweeningSvgObjects) {
        if (object->frame()->index() == frameIndex) {
            object->removeAllTweens();
            removeTweenObject(object);
        }
    }
}

/*
int TupLayer::tweensCount()
{
    return tweeningGraphicObjects.count() + tweeningSvgObjects.count();
}
*/
