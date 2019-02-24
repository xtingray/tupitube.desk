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

#include "tupbackground.h"
#include "tupserializer.h"
#include "tupbackgroundscene.h"

#include <cmath>

TupBackground::TupBackground(TupScene *parent, const QSize size, const QColor color) : QObject(parent)
{
    dimension = size;
    bgColor = color;
    noRender = true;
    currentDynamicLayer = 0;
    currentStaticLayer = 0;

    for(int i=0; i<BG_LAYERS; i++) {
        dynamicFrames << new TupFrame(this, TupFrame::DynamicBg, i);
        staticFrames << new TupFrame(this, TupFrame::StaticBg, i);
    }
}

TupBackground::~TupBackground()
{
    dynamicFrames.clear();
    staticFrames.clear();
}

void TupBackground::setBgColor(const QColor color)
{
    bgColor = color;
}

void TupBackground::fromXml(const QString &xml)
{
    if (xml.contains("<dynamic_landscape>"))
        loadBgLayers(xml);
    else
        loadLegacyBgLayers(xml); // Source File < 0.2.14
}

void TupBackground::loadBgLayers(const QString &xml)
{
    QDomDocument document;
    if (!document.setContent(xml))
        return;

    dynamicFrames.clear();
    staticFrames.clear();

    QDomElement root = document.documentElement();
    QDomNode n = root.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (e.tagName() == "dynamic_landscape") {
            QDomNode n2 = e.firstChild();
            int index = 0;
            while (!n2.isNull()) {
                QDomElement e1 = n2.toElement();
                if (e1.tagName() == "frame") {
                    TupFrame *frame = new TupFrame(this, TupFrame::DynamicBg, index);
                    if (frame) {
                       QString newDoc;
                       {
                           QTextStream ts(&newDoc);
                           ts << n2;
                       }
                       frame->fromXml(newDoc);
                       dynamicFrames << frame;
                       index++;
                    }
                }
                n2 = n2.nextSibling();
            }
        }
        if (e.tagName() == "static_landscape") {
            QDomNode n2 = e.firstChild();
            int index = 0;
            while (!n2.isNull()) {
                QDomElement e1 = n2.toElement();
                if (e1.tagName() == "frame") {
                    TupFrame *frame = new TupFrame(this, TupFrame::StaticBg, index);
                    if (frame) {
                       QString newDoc;
                       {
                           QTextStream ts(&newDoc);
                           ts << n2;
                       }
                       frame->fromXml(newDoc);
                       staticFrames << frame;
                       index++;
                    }
                }
                n2 = n2.nextSibling();
            }
        }
        n = n.nextSibling();
    }

    // if (!dynamicBgIsEmpty())
    //     renderDynamicViews();
}

void TupBackground::loadLegacyBgLayers(const QString &xml)
{
    QDomDocument document;
    if (!document.setContent(xml))
        return;

    dynamicFrames.clear();
    staticFrames.clear();

    TupFrame *staticBg;
    TupFrame *dynamicBg;
    QDomElement root = document.documentElement();
    QDomNode n = root.firstChild();

    while (!n.isNull()) {
           QDomElement e = n.toElement();

           if (e.tagName() == "frame") {
               QString type = e.attribute("name", "none");
               if (type == "landscape_static") {
                   staticBg = new TupFrame(this, TupFrame::StaticBg, 0);
                   if (staticBg) {
                       QString newDoc;
                       {
                           QTextStream ts(&newDoc);
                           ts << n;
                       }

                       staticBg->fromXml(newDoc);
                   }
               } else if (type == "landscape_dynamic") {
                   dynamicBg = new TupFrame(this, TupFrame::DynamicBg, 0);
                   if (dynamicBg) {
                       QString newDoc;
                       {
                           QTextStream ts(&newDoc);
                           ts << n;
                       }

                       dynamicBg->fromXml(newDoc);
                   }
               } else {
                   #ifdef TUP_DEBUG
                       QString msg = "TupBackground::fromXml() - Error: The background input is invalid";
                       #ifdef Q_OS_WIN
                           qDebug() << msg;
                       #else
                           tError() << msg;
                       #endif
                   #endif
               }
           }

           n = n.nextSibling();
    }

    dynamicFrames << dynamicBg;
    staticFrames << staticBg;

    for(int i=1; i<BG_LAYERS; i++) {
        dynamicFrames << new TupFrame(this, TupFrame::DynamicBg, i);
        staticFrames << new TupFrame(this, TupFrame::StaticBg, i);
    }

    // if (!dynamicBgIsEmpty())
    //     renderDynamicViews();
}

QDomElement TupBackground::toXml(QDomDocument &doc) const
{
    QDomElement root = doc.createElement("background");
    doc.appendChild(root);

    QDomElement dynamicRoot = doc.createElement("dynamic_landscape");
    for(int i=0; i<BG_LAYERS; i++) // Dynamic Layers
        dynamicRoot.appendChild(dynamicFrames.at(i)->toXml(doc));
    root.appendChild(dynamicRoot);

    QDomElement staticRoot = doc.createElement("static_landscape");
    for(int i=0; i<BG_LAYERS; i++) // Static Layers
        staticRoot.appendChild(staticFrames.at(i)->toXml(doc));
    root.appendChild(staticRoot);

    return root;
}

TupFrame * TupBackground::getCurrentDynamicFrame()
{
    return dynamicFrames.at(currentDynamicLayer);
}

void TupBackground::setCurrentDynamicLayer(int layer)
{
    currentDynamicLayer = layer;
}

int TupBackground::getCurrentDynamicLayer()
{
    return currentDynamicLayer;
}

TupFrame * TupBackground::getCurrentStaticFrame()
{
    return staticFrames.at(currentStaticLayer);
}

void TupBackground::setCurrentStaticLayer(int layer)
{
    currentStaticLayer = layer;
}

bool TupBackground::dynamicBgIsEmpty()
{
    foreach(TupFrame *frame, dynamicFrames) {
        if (!frame->isEmpty())
            return false;
    }
    return true;
}

bool TupBackground::dynamicLayerIsEmpty(int layer)
{
    return dynamicFrames.at(layer)->isEmpty();
}

bool TupBackground::staticBgIsEmpty()
{
    foreach(TupFrame *frame, staticFrames) {
        if (!frame->isEmpty())
            return false;
    }
    return true;
}

TupFrame *TupBackground::staticFrame(int layer)
{
    return staticFrames.at(layer);
}

TupFrame* TupBackground::dynamicFrame(int layer)
{
    return dynamicFrames.at(layer);
}

void TupBackground::clear()
{
    for(int i=0; i<BG_LAYERS; i++) // Static Layers
        staticFrames.at(i)->clear();

    for(int i=0; i<BG_LAYERS; i++) // Dynamic Layers
        dynamicFrames.at(i)->clear();

    rasterBg.clear();
    noRender = true;
}

void TupBackground::setDynamicOpacity(int layer, double opacity)
{
    dynamicFrames.at(layer)->setFrameOpacity(opacity);
}

double TupBackground::dynamicOpacity(int layer)
{
    return dynamicFrames.at(layer)->frameOpacity();
}

void TupBackground::setStaticOpacity(int layer, double opacity)
{
    staticFrames.at(layer)->setFrameOpacity(opacity);
}

double TupBackground::staticOpacity(int layer)
{
    return staticFrames.at(layer)->frameOpacity();
}

void TupBackground::renderDynamicViews()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupBackground::renderDynamicView()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    rasterBg.clear();
    /*
    for(int i=0; i<BG_LAYERS; i++) {
        tError() << "TupBackground::renderDynamicView() - Blank image at " << i;
        tError() << "Frame Dynamic Direction -> " << dynamicFrames.at(i)->dynamicDirection();
        tError() << "Frame is Empty? -> " << dynamicFrames.at(i)->isEmpty();
        rasterBg << QImage();
    }
    */

    for(int i=0; i<BG_LAYERS; i++) {
        tError() << "Frame Dynamic Direction -> " << dynamicFrames.at(i)->dynamicDirection();
        bool isEmpty = dynamicFrames.at(i)->isEmpty();
        if (!isEmpty) {
            tError() << "TupBackground::renderDynamicView() - Rendering view at " << i;
            QColor bg = Qt::transparent;
            if (i == 0)
                bg = bgColor;
            TupBackgroundScene *bgScene = new TupBackgroundScene(dimension, bg, dynamicFrames.at(i));
            QImage image(dimension, QImage::Format_ARGB32);
            {
                QPainter *painter = new QPainter(&image);
                painter->setRenderHint(QPainter::Antialiasing, true);
                bgScene->renderView(painter);

                delete painter;
                painter = NULL;
            }

            int width = dimension.width();
            int height = dimension.height();

            QImage background(width*2, height*2, QImage::Format_ARGB32);
            QPainter *canvas = new QPainter(&background);
            canvas->drawImage(0, 0, image);
            canvas->drawImage(width, 0, image);
            canvas->drawImage(0, height, image);
            rasterBg << background;

            delete bgScene;
            bgScene = NULL;
            delete canvas;
            canvas = NULL;
        } else {
            tError() << "TupBackground::renderDynamicView() - Blank image at " << i;
            rasterBg << QImage();
        }
    }

    noRender = false;
}

QPixmap TupBackground::dynamicView(int layer, int frameIndex)
{
    if (!dynamicFrames.at(layer))
        tError() << "TupBackground::dynamicView() - Frame is NULL at layer -> " << layer;
    else
        tError() << "TupBackground::dynamicView() - Frame is OK -> " << layer;

    tError() << "TupBackground::dynamicView() - layer: " << layer;
    tError() << "TupBackground::dynamicView() - layers total: " << dynamicFrames.size();

    if (dynamicFrames.at(layer)->isEmpty())
        return QPixmap();

    int posX = 0;
    int posY = 0;
    int shift = dynamicShift(layer);

    TupBackground::Direction direction = dynamicFrames.at(layer)->dynamicDirection();
    switch (direction) {
            case TupBackground::Right:
            {
                int delta = dimension.width() / shift;
                if (delta > frameIndex) {
                    posX = dimension.width() - (frameIndex * shift);
                } else {
                    int mod = fmod(frameIndex, delta);
                    posX = dimension.width() - (mod * shift);
                }
            }
            break;
            case TupBackground::Left:
            {
                int delta = dimension.width() / shift;
                if (delta > frameIndex) {
                    posX = frameIndex * shift;
                } else {
                    int mod = fmod(frameIndex, delta);
                    posX = mod * shift;
                }
            }
            break;
            case TupBackground::Top:
            {
                int delta = dimension.height() / shift;
                if (delta > frameIndex) {
                    posY = frameIndex * shift;
                } else {
                    int mod = fmod(frameIndex, delta);
                    posY = mod * shift;
                }
            }
            break;
            case TupBackground::Bottom:
            {
                int delta = dimension.height() / shift;
                if (delta > frameIndex) {
                    posY = dimension.height() - (frameIndex * shift);
                } else {
                    int mod = fmod(frameIndex, delta);
                    posY = dimension.height() - (mod * shift);
                }
            }
            break;
    }

    QImage view = rasterBg.at(layer).copy(posX, posY, dimension.width(), dimension.height());

    return QPixmap::fromImage(view);
}

bool TupBackground::rasterRenderIsPending()
{
    return noRender;
}

void TupBackground::scheduleRender(bool status)
{
    noRender = status;
}

QImage TupBackground::dynamicRaster(int layer)
{
    return rasterBg[layer];
}

void TupBackground::setDynamicDirection(int layer, int direction)
{
    dynamicFrames.at(layer)->setDynamicDirection(QString::number(direction));
}

void TupBackground::setDynamicShift(int layer, int shift)
{
    dynamicFrames.at(layer)->setDynamicShift(QString::number(shift));
}

TupBackground::Direction TupBackground::dynamicDirection(int layer)
{
    return dynamicFrames.at(layer)->dynamicDirection();
}

int TupBackground::dynamicShift(int layer)
{
    return dynamicFrames.at(layer)->dynamicShift();
}

TupScene * TupBackground::scene()
{
    return static_cast<TupScene *>(parent());
}

TupProject * TupBackground::project()
{
    return scene()->project();
}
