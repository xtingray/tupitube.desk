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
#include "tapplicationproperties.h"

#include <cmath>

TupBackground::TupBackground(TupScene *parent, int index, const QSize size,
                             const QColor color) : QObject(parent)
{
    sceneIndex = index;
    dimension = size;
    bgColor = color;

    noVectorRender = true;
    noRasterRender = true;

    vectorDynamicBgFrame = new TupFrame(this, "landscape_dynamic");
    vectorDynamicBgFrame->setDynamicDirection("0");
    vectorDynamicBgFrame->setDynamicShift("5");

    rasterDynamicBgFrame = new TupFrame(this, "landscape_raster_dynamic");
    rasterDynamicBgFrame->setDynamicDirection("0");
    rasterDynamicBgFrame->setDynamicShift("5");

    vectorStaticBgFrame = new TupFrame(this, "landscape_static");
    rasterStaticBgFrame = new TupFrame(this, "landscape_raster_static");

    bgLayerIndex << VectorDynamic << RasterDynamic << VectorStatic << RasterStatic;
}

TupBackground::~TupBackground()
{
}

void TupBackground::setBgColor(const QColor color)
{
    bgColor = color;
}

void TupBackground::fromXml(const QString &xml)
{
    QDomDocument document;
    if (! document.setContent(xml))
        return;

    QDomElement root = document.documentElement();
    bgLayerIndex.clear();
    QStringList layers = root.attribute("bgLayerIndexes", "0,1,2,3").split(",");
    for (int i=0; i < layers.count(); i++)
        bgLayerIndex << BgType(layers.at(i).toInt());

    QDomNode n = root.firstChild();

    while (!n.isNull()) {
        QDomElement e = n.toElement();

        if (e.tagName() == "frame") {
            QString type = e.attribute("name", "none");
            if (type == "landscape_dynamic") {
                vectorDynamicBgFrame = new TupFrame(this, "landscape_dynamic");

                if (vectorDynamicBgFrame) {
                    QString newDoc;
                    {
                        QTextStream ts(&newDoc);
                        ts << n;
                    }

                    vectorDynamicBgFrame->fromXml(newDoc);
                    if (!vectorDynamicBgFrame->isEmpty()) {
                        renderVectorDynamicView();
                    }
                }
            } else if (type == "landscape_raster_dynamic") {
                rasterDynamicBgFrame = new TupFrame(this, "landscape_raster_dynamic");

                if (rasterDynamicBgFrame) {
                    QString newDoc;
                    {
                        QTextStream ts(&newDoc);
                        ts << n;
                    }

                    rasterDynamicBgFrame->fromXml(newDoc);
                    if (!rasterDynamicBgFrame->isEmpty()) {
                        renderRasterDynamicView();
                    }
                }
            } else if (type == "landscape_static") {
                vectorStaticBgFrame = new TupFrame(this, "landscape_static");

                if (vectorStaticBgFrame) {
                    QString newDoc;
                    {
                        QTextStream ts(&newDoc);
                        ts << n;
                    }

                    vectorStaticBgFrame->fromXml(newDoc);
                }
             }  else if (type == "landscape_raster_static") {
                rasterStaticBgFrame = new TupFrame(this, "landscape_raster_static");

                if (rasterStaticBgFrame) {
                    QString newDoc;
                    {
                        QTextStream ts(&newDoc);
                        ts << n;
                    }

                    rasterStaticBgFrame->fromXml(newDoc);
                }
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "TupBackground::fromXml() - Error: The background input is invalid";
                #endif
            }
        }

        n = n.nextSibling();
    }

    // Loading raster backgrounds
    QString staticPath = RASTER_BG_DIR + QString::number(sceneIndex) + "/bg/static_bg.png";
    QString dynamicPath = RASTER_BG_DIR + QString::number(sceneIndex) + "/bg/dynamic_bg.png";

    if (QFile::exists(staticPath))
        rasterStaticBgPix = QPixmap(staticPath);
    if (QFile::exists(dynamicPath))
        rasterDynamicBgPix = QPixmap(dynamicPath);
}

QDomElement TupBackground::toXml(QDomDocument &doc) const
{
    QDomElement root = doc.createElement("background");

    QString indexes = "";
    for (int i=0; i < bgLayerIndex.count(); i++)
        indexes += QString::number(bgLayerIndex.at(i)) + ",";
    indexes.chop(1);
    root.setAttribute("bgLayerIndexes", indexes);

    doc.appendChild(root);

    root.appendChild(vectorDynamicBgFrame->toXml(doc));
    root.appendChild(rasterDynamicBgFrame->toXml(doc));
    root.appendChild(vectorStaticBgFrame->toXml(doc));
    root.appendChild(rasterStaticBgFrame->toXml(doc));

    return root;
}

QList<TupBackground::BgType> TupBackground::layerIndexes()
{
    return bgLayerIndex;
}

bool TupBackground::vectorDynamicBgIsEmpty()
{
    return vectorDynamicBgFrame->isEmpty();
}

bool TupBackground::vectorStaticBgIsEmpty()
{
    return vectorStaticBgFrame->isEmpty();
}

TupFrame *TupBackground::vectorStaticFrame()
{
    return vectorStaticBgFrame;
}

TupFrame* TupBackground::vectorDynamicFrame()
{
    return vectorDynamicBgFrame;
}

void TupBackground::clearBackground()
{
    if (vectorDynamicBgFrame)
        vectorDynamicBgFrame->clear();

    if (vectorStaticBgFrame)
        vectorStaticBgFrame->clear();

    // SQA: Implement the UNDO action for this case
    rasterDynamicBgPix = QPixmap();
    rasterStaticBgPix = QPixmap();
}

void TupBackground::setVectorDynamicOpacity(double opacity)
{
    vectorDynamicBgFrame->setFrameOpacity(opacity);
}

double TupBackground::vectorDynamicOpacity()
{
    return vectorDynamicBgFrame->frameOpacity();
}

void TupBackground::setVectorStaticOpacity(double opacity)
{
    vectorStaticBgFrame->setFrameOpacity(opacity);
}

double TupBackground::vectorStaticOpacity()
{
    return vectorStaticBgFrame->frameOpacity();
}

void TupBackground::setRasterDynamicOpacity(double opacity)
{
    rasterDynamicBgFrame->setFrameOpacity(opacity);
}

double TupBackground::rasterDynamicOpacity()
{
    return rasterDynamicBgFrame->frameOpacity();
}

void TupBackground::setRasterStaticOpacity(double opacity)
{
    rasterStaticBgFrame->setFrameOpacity(opacity);
}

double TupBackground::rasterStaticOpacity()
{
    return rasterStaticBgFrame->frameOpacity();
}

// Creating expanded vector dynamic image
void TupBackground::renderVectorDynamicView()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupBackground::renderVectorDynamicView()]";
    #endif 
	
    TupBackgroundScene *bgScene = new TupBackgroundScene(dimension, bgColor, vectorDynamicBgFrame);
    QImage image(dimension, QImage::Format_ARGB32);
    {
        QPainter *painter = new QPainter(&image);
        painter->setRenderHint(QPainter::Antialiasing, true);
        bgScene->renderView(painter);

        painter = nullptr;
        delete painter;
    }

    int width = dimension.width();
    int height = dimension.height();
    QImage bgView(width * 2, height * 2, QImage::Format_ARGB32);

    QPainter *canvas = new QPainter(&bgView);
    canvas->drawImage(0, 0, image);
    canvas->drawImage(width, 0, image);
    canvas->drawImage(0, height, image);

    setVectorDynamicViewImage(bgView);
    noVectorRender = false;

    bgScene = nullptr;
    delete bgScene;
    canvas = nullptr;
    delete canvas;
}

// Creating expanded raster dynamic image
void TupBackground::renderRasterDynamicView()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupBackground::renderRasterDynamicView()]";
    #endif

    int width = dimension.width();
    int height = dimension.height();

    QImage image = rasterDynamicBgPix.toImage();
    QImage bgView(width * 2, height * 2, QImage::Format_ARGB32);

    QPainter *canvas = new QPainter(&bgView);
    canvas->drawImage(0, 0, image);
    canvas->drawImage(width, 0, image);
    canvas->drawImage(0, height, image);

    setRasterDynamicViewImage(bgView);
    noRasterRender = false;

    canvas = nullptr;
    delete canvas;
}

QPixmap TupBackground::vectorDynamicView(int frameIndex)
{
    int posX = 0;
    int posY = 0;
    int shift = vectorDynamicShift();
    if (shift == 0)
        shift = 5;

    #ifdef TUP_DEBUG
        qDebug() << "[TupBackground::vectorDynamicView()] - shift: " << shift;
        qDebug() << "[TupBackground::vectorDynamicView()] - frameIndex: " << frameIndex;
    #endif

    TupBackground::Direction direction = vectorDynamicBgFrame->dynamicDirection();
    switch (direction) {
        case TupBackground::Right:
        {
            int delta = dimension.width() / shift;
            if (delta > frameIndex) {
                posX = dimension.width() - frameIndex * shift;
            } else {
                int mod = static_cast<int> (fmod(frameIndex, delta));
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
                int mod = static_cast<int> (fmod(frameIndex, delta));
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
                int mod = static_cast<int> (fmod(frameIndex, delta));
                posY = mod * shift;
            }
        }
        break;
        case TupBackground::Bottom:
        {
            int delta = dimension.height() / shift;
            if (delta > frameIndex) {
                posY = dimension.height() - frameIndex*shift;
            } else {
                int mod = static_cast<int> (fmod(frameIndex, delta));
                posY = dimension.height() - (mod * shift);
            }
        }
        break;
    }

    return QPixmap::fromImage(vectorDynamicViewImg.copy(posX, posY, dimension.width(), dimension.height()));
}

QPixmap TupBackground::rasterDynamicView(int frameIndex)
{
    int posX = 0;
    int posY = 0;
    int shift = rasterDynamicShift();
    if (shift == 0)
        shift = 5;

    #ifdef TUP_DEBUG
        qDebug() << "[TupBackground::rasterDynamicView()] - shift: " << shift;
        qDebug() << "[TupBackground::rasterDynamicView()] - frameIndex: " << frameIndex;
    #endif

    TupBackground::Direction direction = rasterDynamicBgFrame->dynamicDirection();
    switch (direction) {
        case TupBackground::Right:
        {
            int delta = dimension.width() / shift;
            if (delta > frameIndex) {
                posX = dimension.width() - frameIndex * shift;
            } else {
                int mod = static_cast<int> (fmod(frameIndex, delta));
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
                int mod = static_cast<int> (fmod(frameIndex, delta));
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
                int mod = static_cast<int> (fmod(frameIndex, delta));
                posY = mod * shift;
            }
        }
        break;
        case TupBackground::Bottom:
        {
            int delta = dimension.height() / shift;
            if (delta > frameIndex) {
                posY = dimension.height() - frameIndex*shift;
            } else {
                int mod = static_cast<int> (fmod(frameIndex, delta));
                posY = dimension.height() - (mod * shift);
            }
        }
        break;
    }

    return QPixmap::fromImage(rasterDynamicViewImg.copy(posX, posY, dimension.width(), dimension.height()));
}

bool TupBackground::vectorRenderIsPending()
{
    return noVectorRender;
}

bool TupBackground::rasterRenderIsPending()
{
    return noRasterRender;
}

void TupBackground::scheduleVectorRender(bool status)
{
    noVectorRender = status;
}

void TupBackground::setVectorDynamicViewImage(QImage bg)
{
    vectorDynamicViewImg = bg;
}

void TupBackground::setRasterDynamicViewImage(QImage bg)
{
    rasterDynamicViewImg = bg;
}

void TupBackground::setVectorDynamicDirection(int direction)
{
    vectorDynamicBgFrame->setDynamicDirection(QString::number(direction));
}

void TupBackground::setRasterDynamicDirection(int direction)
{
    rasterDynamicBgFrame->setDynamicDirection(QString::number(direction));
}

void TupBackground::setVectorDynamicShift(int shift)
{
    vectorDynamicBgFrame->setDynamicShift(QString::number(shift));
}

void TupBackground::setRasterDynamicShift(int shift)
{
    rasterDynamicBgFrame->setDynamicShift(QString::number(shift));
}

TupBackground::Direction TupBackground::vectorDynamicDirection()
{
    return vectorDynamicBgFrame->dynamicDirection();
}

TupBackground::Direction TupBackground::rasterDynamicDirection()
{
    return rasterDynamicBgFrame->dynamicDirection();
}

int TupBackground::vectorDynamicShift()
{
    return vectorDynamicBgFrame->dynamicShift();
}

int TupBackground::rasterDynamicShift()
{
    return rasterDynamicBgFrame->dynamicShift();
}

TupScene * TupBackground::scene()
{
    return static_cast<TupScene *>(parent());
}

TupProject * TupBackground::project()
{
    return scene()->project();
}

void TupBackground::updateRasterBgImage(TupProject::Mode spaceContext, const QString &imgPath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupBackground::updateRasterBgImage()] - Space Context: " << spaceContext;
        qDebug() << "[TupBackground::updateRasterBgImage()] - Image Path: " << imgPath;
    #endif

    if (spaceContext == TupProject::RASTER_DYNAMIC_BG_MODE) {
        if (QFile::exists(imgPath)) {
            rasterDynamicBgPix = QPixmap(imgPath);
            noRasterRender = true;
        } else {
            rasterDynamicBgPix = QPixmap();
        }
    } else {
        if (QFile::exists(imgPath))
            rasterStaticBgPix = QPixmap(imgPath);
        else
            rasterStaticBgPix = QPixmap();
    }
}

bool TupBackground::rasterStaticBgIsNull()
{
    return rasterStaticBgPix.isNull();
}

bool TupBackground::rasterDynamicBgIsNull()
{
    return rasterDynamicBgPix.isNull();
}

QPixmap TupBackground::rasterStaticBackground()
{
    return rasterStaticBgPix;
}

QPixmap TupBackground::rasterDynamicBackground()
{
    return rasterDynamicBgPix;
}
