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

#include "tupscene.h"
#include "tupgraphicobject.h"
#include "tupsvgitem.h"
#include "tupsoundlayer.h"
#include "tupitemgroup.h"
#include "tupprojectloader.h"
#include "tupitemfactory.h"

TupScene::TupScene(TupProject *parent, int index, const QSize size, const QColor color) : QObject(parent)
{
    sceneIndex = index;
    dimension = size;
    bgColor = color;
    isLocked = false;
    layerCount = 0;
    layers = Layers();
    isVisible = true;
    storyboard = new TupStoryboard(parent->getAuthor());
    background = new TupBackground(this, sceneIndex, size, color);
}

TupScene::~TupScene()
{
    delete storyboard;
    delete background;

    tweeningGraphicObjects.clear();
    tweeningSvgObjects.clear();
    layers.clear();
    undoLayers.clear();
    soundLayers.clear();
}

void TupScene::setSceneName(const QString &name)
{
    sceneName = name;
}

void TupScene::setBgColor(const QColor color)
{
    bgColor = color;
    background->setBgColor(color);
}

void TupScene::setSceneLocked(bool locked)
{
    isLocked = locked;
}

QString TupScene::getSceneName() const
{
    return sceneName;
}

bool TupScene::isSceneLocked() const
{
    return isLocked;
}

void TupScene::setVisibility(bool visible)
{
    isVisible = visible;
}

bool TupScene::isSceneVisible() const
{
    return isVisible;
}

Layers TupScene::getLayers() const
{
    return layers;
}

int TupScene::layersCount() const
{
    return layerCount;
}

SoundLayers TupScene::getSoundLayers() const
{
    return soundLayers;
}

void TupScene::setBasicStructure()
{
    layerCount++;
    TupLayer *layer = new TupLayer(this, layerCount-1);
    layer->setLayerName(tr("Layer 1"));
    layer->createFrame(tr("Frame"), 0, false);

    layers.insert(0, layer);
}

/**
 * Set the layers list, this function overwrites the old layers
 */
void TupScene::setLayers(const Layers &sLayers)
{
    layers = sLayers;
}

TupLayer *TupScene::createLayer(QString name, int position, bool loaded)
{
    if (position < 0 || position > layers.count()) {
        #ifdef TUP_DEBUG
            QString msg = "TupScene::createLayer() - Invalid index -> " + QString::number(position);
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif    
        
        return nullptr;
    }

    layerCount++;
    TupLayer *layer = new TupLayer(this, layerCount - 1);
    layer->setLayerName(name);
    layers.insert(position, layer);

    if (loaded)
        TupProjectLoader::createLayer(project()->visualIndexOf(this), position, layer->getLayerName(), project());

    return layer;
}

void TupScene::addLayer(const QString &xml)
{
    TupLayer *layer = new TupLayer(this, layers.count());
    layer->fromXml(xml);
    layers << layer;
}

TupSoundLayer *TupScene::createSoundLayer(int position, bool loaded)
{
    #ifdef TUP_DEBUG
        qDebug() << "[createSoundLayer()] - position: " << position;
    #endif    
    
    if (position < 0 || position > soundLayers.count()) {
        #ifdef TUP_DEBUG
            qDebug() << "TupScene::createSoundLayer() - [ Fatal Error ] - Index incorrect!";
        #endif        
        return nullptr;
    }

    TupSoundLayer *layer = new TupSoundLayer(this);
    layerCount++;

    layer->setLayerName(tr("Sound layer %1").arg(layerCount));

    soundLayers.insert(position, layer);

    if (loaded)
        TupProjectLoader::createSoundLayer(objectIndex(), position, layer->getLayerName(), project());

    return layer;
}

bool TupScene::restoreLayer(int index)
{
    if (undoLayers.count() > 0) {
        TupLayer *layer = undoLayers.takeLast();
        if (layer) {
            layers.insert(index, layer);
            layerCount++;
            return true;
        }
        return false;
    }

    return false;
}

bool TupScene::removeLayer(int position)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScene::removeLayer()] - position: " << position;
    #endif

    TupLayer *layer = this->layerAt(position);
    if (layer) {
        removeTweensFromLayer(position + 1);
        undoLayers << layers.takeAt(position);
        layerCount--;

        return true;
    }

    return false;
}

/**
 * Retorna el layer que se encuentra en la posicion indicada
 * @param position 
 * @return 
 */
TupLayer *TupScene::layerAt(int position) const
{
    if (position < 0 || position >= layers.count()) {
        #ifdef TUP_DEBUG
            QString msg1 = "TupScene::layerAt() - FATAL ERROR: LAYERS TOTAL: " + QString::number(layers.count());
            QString msg2 = "TupScene::layerAt() - FATAL ERROR: index out of bound -> " + QString::number(position);
            QString msg3 = "TupScene::layerAt() - FATAL ERROR: The layer requested doesn't exist anymore";
            #ifdef Q_OS_WIN
                qDebug() << msg1;
                qDebug() << msg2;
                qDebug() << msg3;
            #else
                tError() << msg1;
                tError() << msg2;
                tError() << msg3;
            #endif
        #endif
        return nullptr;
    }

    return layers.value(position);
}

TupSoundLayer *TupScene::soundLayer(int position) const
{
    if (position < 0 || position >= soundLayers.count()) {
        #ifdef TUP_DEBUG
            qDebug() << "TupScene::fromXml() - FATAL ERROR: index out of bound " + QString::number(position);
        #endif
        return nullptr;
    }

    return soundLayers.value(position);
}

void TupScene::fromXml(const QString &xml)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScene::fromXml()]";
    #endif

    QDomDocument document;
    if (!document.setContent(xml)) {
		#ifdef TUP_DEBUG
            qDebug() << "TupScene::fromXml() - Error while processing XML file";
        #endif  
        return;
    }

    QDomElement root = document.documentElement();
    setSceneName(root.attribute("name", getSceneName()));
    QDomNode n = root.firstChild();

    while (!n.isNull()) {
           QDomElement e = n.toElement();

           if (!e.isNull()) {
               if (e.tagName() == "layer") {
                   int layerIndex = layers.count();
                   TupLayer *layer = createLayer(e.attribute("name"), layerIndex, true);

                   if (layer) {
                       QString newDoc;
                       {
                         QTextStream ts(&newDoc);
                         ts << n;
                       }
                       layer->fromXml(newDoc);
                   }
               } else if (e.tagName() == "background") {
                   QString newDoc;
                   {
                     QTextStream ts(&newDoc);
                     ts << n;
                   }
                   background->fromXml(newDoc);
               } else if (e.tagName() == "soundlayer") {
                   int pos = soundLayers.count();
                   TupSoundLayer *layer = createSoundLayer(pos, true);

                   if (layer) {
                       QString newDoc;
                        {
                          QTextStream ts(&newDoc);
                          ts << n;
                        }
                       layer->fromXml(newDoc);
                   }
               } else if (e.tagName() == "storyboard") {
                   QString newDoc;
                   {
                     QTextStream ts(&newDoc);
                     ts << n;
                   }
                   storyboard->fromXml(newDoc);
               }
           }

           n = n.nextSibling();

    } // end while
}

QDomElement TupScene::toXml(QDomDocument &doc) const
{
    QDomElement root = doc.createElement("scene");
    root.setAttribute("name", sceneName);

    root.appendChild(storyboard->toXml(doc));
    root.appendChild(background->toXml(doc));

    int total = layers.size();
    for (int i = 0; i < total; ++i)
        root.appendChild(layers.at(i)->toXml(doc));

    total = soundLayers.size();
    for (int i = 0; i < total; ++i)
        root.appendChild(soundLayers.at(i)->toXml(doc));

    return root;
}

bool TupScene::moveLayer(int from, int to)
{
    if (from < 0 || from >= layers.count() || to < 0 || to >= layers.count()) {
        #ifdef TUP_DEBUG
            qDebug() << "TupScene::moveLayer() - FATAL ERROR: Layer index out of bound " + QString::number(to);
        #endif
        return false;
    }

    TupLayer *sourceLayer = layers[from];
    sourceLayer->updateLayerIndex(to + 1);
    TupLayer *targetLayer = layers[to];
    targetLayer->updateLayerIndex(from + 1);

    Frames frames = sourceLayer->getFrames(); 
    int totalFrames = frames.size();
    int zLevelIndex = (to + 2) * ZLAYER_LIMIT;
    for (int i = 0; i < totalFrames; i++) {
         TupFrame *frame = frames.at(i);
         frame->updateZLevel(zLevelIndex);
    }

    frames = targetLayer->getFrames();
    totalFrames = frames.size();
    zLevelIndex = (from + 2)*ZLAYER_LIMIT;
    for (int i = 0; i < totalFrames; i++) {
         TupFrame *frame = frames.at(i);
         frame->updateZLevel(zLevelIndex);
    }
    // layers.swap(from, to);
    layers.swapItemsAt(from, to);

    return true;
}

int TupScene::objectIndex() const
{
    if (TupProject *project = dynamic_cast<TupProject *>(parent()))
        return project->visualIndexOf(const_cast<TupScene *>(this));

    return -1;
}

int TupScene::visualIndexOf(TupLayer *layer) const
{
    return layers.indexOf(layer);
}

TupProject *TupScene::project() const
{
    return static_cast<TupProject *>(parent());
}

void TupScene::addTweenObject(int layerIndex, TupGraphicObject *object)
{
    TupLayer *layer = layerAt(layerIndex);
    if (layer)
        layer->addTweenObject(object);
}

void TupScene::addTweenObject(int layerIndex, TupSvgItem *object)
{
    TupLayer *layer = layerAt(layerIndex);
    if (layer)
        layer->addTweenObject(object);
}

void TupScene::updateTweenObject(int layerIndex, int objectIndex, TupGraphicObject *object)
{
    TupLayer *layer = layerAt(layerIndex);
    if (layer)
        layer->updateTweenObject(objectIndex, object);
}

void TupScene::updateTweenObject(int layerIndex, int objectIndex, TupSvgItem *object)
{
    TupLayer *layer = layerAt(layerIndex);
    if (layer)
        layer->updateTweenObject(objectIndex, object);
}

void TupScene::removeTweenObject(int layerIndex, TupGraphicObject *object)
{
    TupLayer *layer = layerAt(layerIndex);
    if (layer)
        layer->removeTweenObject(object);
}

void TupScene::removeTweenObject(int layerIndex, TupSvgItem *object)
{
    TupLayer *layer = layerAt(layerIndex);
    if (layer)
        layer->removeTweenObject(object);
}

/*
QList<TupGraphicObject *> TupScene::tweeningGraphicObjects() const
{
    QList<TupGraphicObject *> list;
    foreach(TupLayer *layer, layers)
            list += layer->tweeningGraphicObjects();

    return list;
}
*/

QList<TupGraphicObject *> TupScene::getTweeningGraphicObjects(int layerIndex) const
{
    QList<TupGraphicObject *> list;
    TupLayer *layer = layerAt(layerIndex);
    if (layer)
        list = layer->getTweeningGraphicObjects();

    return list;
}

/*
QList<TupSvgItem *> TupScene::tweeningSvgObjects() const
{
    QList<TupSvgItem *> list;
    foreach(TupLayer *layer, layers)
            list += layer->tweeningSvgObjects();

    return list;
}
*/

QList<TupSvgItem *> TupScene::getTweeningSvgObjects(int layerIndex) const
{
    QList<TupSvgItem *> list;
    TupLayer *layer = layerAt(layerIndex);
    if (layer)
        list = layer->getTweeningSvgObjects();

    return list;
}

bool TupScene::tweenExists(const QString &name, TupItemTweener::Type type)
{
    if (layers.count()) {
        foreach(TupLayer *layer, layers) {
            QList<TupGraphicObject *> objectList = layer->getTweeningGraphicObjects();
            foreach (TupGraphicObject *object, objectList) {
                QList<TupItemTweener *> list = object->tweensList();
                foreach(TupItemTweener *tween, list) {
                    if ((tween->getTweenName().compare(name) == 0) && (tween->getType() == type))
                        return true;
                }
            }

            QList<TupSvgItem *> svgList = layer->getTweeningSvgObjects();
            foreach (TupSvgItem *object, svgList) {
                QList<TupItemTweener *> list = object->tweensList();
                foreach(TupItemTweener *tween, list) {
                    if ((tween->getTweenName().compare(name) == 0) && (tween->getType() == type))
                        return true;
                }
            }
        }
    }

    return false;
}

bool TupScene::removeTween(const QString &name, TupItemTweener::Type type)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupScene::removeTween()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (layers.count()) {
        foreach(TupLayer *layer, layers) {
            QList<TupGraphicObject *> objectList = layer->getTweeningGraphicObjects();
            foreach (TupGraphicObject *object, objectList) {
                QList<TupItemTweener *> list = object->tweensList();
                int total = list.count();
                for (int i=0; i < total; i++) {
                     TupItemTweener *tween = list.at(i);
                     if ((tween->getTweenName().compare(name) == 0) && (tween->getType() == type)) {
                         object->removeTween(i);
                         if (total == 1)
                             removeTweenObject(layer->layerIndex(), object);
                         return true;
                     }
                }
            }

            QList<TupSvgItem *> svgList = layer->getTweeningSvgObjects();
            foreach (TupSvgItem *object, svgList) {
                QList<TupItemTweener *> list = object->tweensList();
                int total = list.count();
                for (int i=0; i < total; i++) {
                     TupItemTweener *tween = list.at(i);
                     if ((tween->getTweenName().compare(name) == 0) && (tween->getType() == type)) {
                         object->removeTween(i);
                         if (total == 1)
                             removeTweenObject(layer->layerIndex(), object);
                         return true;
                     }
                }
            }
        }
    }

    return false;
}

void TupScene::removeTweensFromLayer(int layerIndex)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupScene::removeTweensFromLayer()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    TupLayer *layer = layerAt(layerIndex);
    if (layer)
        layer->removeAllTweens();
}

void TupScene::removeTweensFromFrame(int layerIndex, int frameIndex)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupScene::removeTweensFromFrame()]" << frameIndex;
        #else
            T_FUNCINFO << frameIndex;
        #endif
    #endif

    TupLayer *layer = layerAt(layerIndex);
    if (layer)
        layer->removeTweensFromFrame(frameIndex);
}

TupItemTweener *TupScene::tween(const QString &name, TupItemTweener::Type type)
{
    if (!layers.isEmpty()) {
        foreach(TupLayer *layer, layers) {
            QList<TupGraphicObject *> objectList = layer->getTweeningGraphicObjects();
            foreach (TupGraphicObject *object, objectList) {
                QList<TupItemTweener *> list = object->tweensList();
                foreach(TupItemTweener *tween, list) {
                    if ((tween->getTweenName().compare(name) == 0) && (tween->getType() == type))
                        return tween;
                }
            }

            QList<TupSvgItem *> svgList = layer->getTweeningSvgObjects();
            foreach (TupSvgItem *object, svgList) {
                QList<TupItemTweener *> list = object->tweensList();
                foreach(TupItemTweener *tween, list) {
                    if ((tween->getTweenName().compare(name) == 0) && (tween->getType() == type))
                        return tween;
                }
            }
        }
    }

    return nullptr;
}

QList<QString> TupScene::getTweenNames(TupItemTweener::Type type)
{
    QList<QString> names;
    if (!layers.isEmpty()) {
        foreach(TupLayer *layer, layers) {
            QList<TupGraphicObject *> objectList = layer->getTweeningGraphicObjects();
            foreach (TupGraphicObject *object, objectList) {
                QList<TupItemTweener *> list = object->tweensList();
                foreach(TupItemTweener *tween, list) {
                    if (tween->getType() == type && !names.contains(tween->getTweenName()))
                        names.append(tween->getTweenName());
                }
            }

            QList<TupSvgItem *> svgList = layer->getTweeningSvgObjects();
            foreach (TupSvgItem *object, svgList) {
                QList<TupItemTweener *> list = object->tweensList();
                foreach(TupItemTweener *tween, list) {
                    if (tween->getType() == type && !names.contains(tween->getTweenName()))
                         names.append(tween->getTweenName());
                }
            }
        }
    }

    return names;
}

QList<QGraphicsItem *> TupScene::getItemsFromTween(const QString &name, TupItemTweener::Type type)
{
    QList<QGraphicsItem *> items;
    if (layers.count()) {
        foreach(TupLayer *layer, layers) {
            QList<TupGraphicObject *> objectList = layer->getTweeningGraphicObjects();
            foreach (TupGraphicObject *object, objectList) {
                QList<TupItemTweener *> list = object->tweensList();
                foreach(TupItemTweener *tween, list) {
                    if ((tween->getTweenName().compare(name) == 0) && (tween->getType() == type))
                        items.append(object->item());
                }
            }

            QList<TupSvgItem *> svgList = layer->getTweeningSvgObjects();
            foreach (TupSvgItem *object, svgList) {
                QList<TupItemTweener *> list = object->tweensList();
                foreach(TupItemTweener *tween, list) {
                    if ((tween->getTweenName().compare(name) == 0) && (tween->getType() == type))
                        items.append(object);
                }
            }
        }
    }

    return items;
}

/*
int TupScene::getTotalTweens()
{
    int total = 0;

    foreach(TupLayer *layer, layers) {
            total += layer->tweensCount();

    return total;
}
*/

int TupScene::framesCount()
{
    int total = 0;
    if (layers.count()) {
        foreach (TupLayer *layer, layers) {
            int frames = layer->framesCount();
            if (frames > total)
                total = frames;
        }
    }

    return total;
}

TupBackground* TupScene::sceneBackground()
{
    return background;
}

void TupScene::reset(const QString &name)
{
    sceneName = name;
    background = new TupBackground(this, sceneIndex, dimension, bgColor);
    layers.clear();
    tweeningGraphicObjects.clear();
    tweeningSvgObjects.clear();

    layerCount = 1;
    TupLayer *layer = new TupLayer(this, 0);
    layer->setLayerName(tr("Layer %1").arg(1));
    layer->createFrame(tr("Frame"), 0, false);

    layers.insert(0, layer);
}

void TupScene::clear()
{
    if (background) {
        background->clearBackground();

        background = nullptr;
        delete background;
    }

    for (int i=0; i<layers.count(); i++) {
         TupLayer *layer = layers.takeAt(i);
         layer->clear();

         layer = nullptr;
         delete layer;
    }

    layerCount = 1;
    layers.clear();
    tweeningGraphicObjects.clear();
    tweeningSvgObjects.clear();
}

void TupScene::setStoryboard(TupStoryboard *sb)
{
    storyboard = sb;
}

TupStoryboard * TupScene::storyboardStructure()
{
    return storyboard;
}

void TupScene::insertStoryBoardScene(int index)
{
    storyboard->insertScene(index);
}

void TupScene::appendStoryBoardScene()
{
    storyboard->appendScene();
}

void TupScene::moveStoryBoardScene(int oldIndex, int newIndex)
{
    storyboard->moveScene(oldIndex, newIndex);
}

void TupScene::resetStoryBoardScene(int index)
{
    storyboard->resetScene(index);
}

void TupScene::removeStoryBoardScene(int index)
{
    storyboard->removeScene(index);
}

QList<QString> TupScene::getLipSyncNames()
{
    QList<QString> names;

    if (layers.count()) {
        foreach (TupLayer *layer, layers) {
            if (layer->lipSyncCount() > 0) {
                Mouths mouths = layer->getLipSyncList();
                foreach (TupLipSync *lipsync, mouths)
                    names << lipsync->getLipSyncName();
            }
        }
    }

    return names;
}

bool TupScene::lipSyncExists(const QString &name)
{
    if (layers.count()) {
        foreach (TupLayer *layer, layers) {
            if (layer->lipSyncCount() > 0) {
                Mouths mouths = layer->getLipSyncList();
                foreach (TupLipSync *lipsync, mouths) {
                    if (lipsync->getLipSyncName().compare(name) == 0)
                        return true;
                }
            }
        }
    }

    return false;
}

int TupScene::getLipSyncLayerIndex(const QString &name)
{
    int index = 0;
    if (layers.count()) {
        foreach (TupLayer *layer, layers) {
            if (layer->lipSyncCount() > 0) {
                Mouths mouths = layer->getLipSyncList();
                foreach (TupLipSync *lipsync, mouths) {
                    if (lipsync->getLipSyncName().compare(name) == 0)
                        break;
                    index++;
                }
            }
        }
    }

    return index;
}

TupLipSync * TupScene::getLipSync(const QString &name)
{
    TupLipSync *project = nullptr;

    if (layers.count()) {
        foreach (TupLayer *layer, layers) {
            if (layer->lipSyncCount() > 0) {
                Mouths mouths = layer->getLipSyncList();
                foreach (TupLipSync *lipsync, mouths) {
                    if (lipsync->getLipSyncName().compare(name) == 0)
                        return lipsync;
                }
            }
        }
    }

    return project;
}

bool TupScene::updateLipSync(TupLipSync *lipsync)
{
    QString name = lipsync->getLipSyncName();

    if (layers.count()) {
        foreach (TupLayer *layer, layers) {
            if (layer->lipSyncCount() > 0) {
                Mouths mouths = layer->getLipSyncList();
                foreach (TupLipSync *record, mouths) {
                    if (record->getLipSyncName().compare(name) == 0) {
                        record = lipsync;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool TupScene::removeLipSync(const QString &name)
{
    if (layers.count()) {
        foreach (TupLayer *layer, layers) {
            if (layer->removeLipSync(name))
                return true;
        }
    }

    return false;
}

int TupScene::lipSyncTotal()
{
    int total = 0;
    if (layers.count()) {
        foreach (TupLayer *layer, layers)
            total += layer->lipSyncCount();
    }
    return total;
}

Mouths TupScene::getLipSyncList()
{
    Mouths list;
    if (layers.count()) {
        foreach (TupLayer *layer, layers) {
            if (layer->lipSyncCount() > 0) {
                Mouths mouths = layer->getLipSyncList();
                list.append(mouths);
            }
        }
    }

    return list;
}

int TupScene::totalPhotograms()
{
    int total = 0;
    int totalLayers = layers.size();
    for (int i = 0; i < totalLayers; i++)
        total = qMax(total, layers.at(i)->getFrames().count());

    return total;
}

void TupScene::updateRasterBackground(TupProject::Mode spaceContext, const QString &imgPath)
{
    background->updateRasterBgImage(spaceContext, imgPath);
}
