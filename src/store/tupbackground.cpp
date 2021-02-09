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
#include <QTimer>

TupBackground::TupBackground(TupScene *parent, int index, const QSize size,
                             const QColor color) : QObject(parent)
{
    sceneIndex = index;
    dimension = size;
    bgColor = color;

    rasterStaticUpdateRequired = true;
    vectorDynamicRenderRequired = true;
    rasterDynamicRenderRequired = true;

    vectorDynamicBgFrame = new TupFrame(this, "landscape_dynamic");
    vectorDynamicBgFrame->setDynamicDirection("0");
    vectorDynamicBgFrame->setDynamicShift("5");

    rasterDynamicBgFrame = new TupFrame(this, "landscape_raster_dynamic");
    rasterDynamicBgFrame->setDynamicDirection("0");
    rasterDynamicBgFrame->setDynamicShift("5");

    vectorStaticBgFrame = new TupFrame(this, "landscape_static");
    rasterStaticBgFrame = new TupFrame(this, "landscape_raster_static");

    vectorFgFrame = new TupFrame(this, "landscape_vector_foreground");

    bgLayerIndexes << VectorDynamic << RasterDynamic << VectorStatic << RasterStatic << VectorForeground;
    bgVisibilityList << true << true << true << true << true;
}

TupBackground::~TupBackground()
{
}

void TupBackground::setBgColor(const QColor color)
{
    bgColor = color;
}

QColor TupBackground::getBgColor() const
{
    return bgColor;
}

void TupBackground::setProjectSize(const QSize size)
{
    dimension = size;
}

QSize TupBackground::getProjectSize() const
{
    return dimension;
}

void TupBackground::fromXml(const QString &xml)
{
    QDomDocument document;
    if (! document.setContent(xml))
        return;

    QDomElement root = document.documentElement();
    bgLayerIndexes.clear();
    QStringList layers = root.attribute("bgLayerIndexes", "0,1,2,3,4").split(",");
    for (int i=0; i < layers.count(); i++)
        bgLayerIndexes << BgType(layers.at(i).toInt());

    if (bgLayerIndexes.count() == BG_LAYERS)
        bgLayerIndexes << VectorForeground;

    bgVisibilityList.clear();
    QStringList visibility = root.attribute("bgLayerVisibility", "1,1,1,1,1").split(",");
    for (int i=0; i < visibility.count(); i++)
        bgVisibilityList << visibility.at(i).toInt();

    if (bgVisibilityList.count() == BG_LAYERS)
        bgVisibilityList << true;

    QDomNode n = root.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();

        if (e.tagName() == "frame") {
            QString type = e.attribute("name", "none");
            if (type == "landscape_dynamic") {
                vectorDynamicBgFrame = new TupFrame(this, "landscape_dynamic");

                QString newDoc;
                {
                    QTextStream ts(&newDoc);
                    ts << n;
                }
                vectorDynamicBgFrame->fromXml(newDoc);

                // Loading raster dynamic background image
                QString imgPath = VECTOR_BG_DIR + QString::number(sceneIndex) + "/bg/dynamic_bg.png";
                if (QFile::exists(imgPath)) {
                    vectorDynamicBgExpanded = QPixmap(imgPath);
                    vectorDynamicRenderRequired = false;

                    #ifdef TUP_DEBUG
                        qDebug() << "[TupBackground::fromXml()] - Vector dynamic image loaded -> " << imgPath;
                    #endif
                } else {
                    #ifdef TUP_DEBUG
                        qWarning() << "[TupBackground::fromXml()] - Vector dynamic image wasn't loaded -> " << imgPath;
                    #endif
                }
            } else if (type == "landscape_raster_dynamic") {
                rasterDynamicBgFrame = new TupFrame(this, "landscape_raster_dynamic");

                QString newDoc;
                {
                    QTextStream ts(&newDoc);
                    ts << n;
                }

                rasterDynamicBgFrame->fromXml(newDoc);

                // Loading raster dynamic background image
                QString imgPath = RASTER_BG_DIR + QString::number(sceneIndex) + "/bg/dynamic_bg.png";
                if (QFile::exists(imgPath)) {
                    rasterDynamicBgPix = QPixmap(imgPath);
                    renderRasterDynamicView();

                    #ifdef TUP_DEBUG
                        qDebug() << "[TupBackground::fromXml()] - Raster dynamic image loaded -> " << imgPath;
                    #endif
                } else {
                    #ifdef TUP_DEBUG
                        qWarning() << "[TupBackground::fromXml()] - Raster dynamic bg wasn't loaded -> " << imgPath;
                    #endif
                }
            } else if (type == "landscape_static") {
                vectorStaticBgFrame = new TupFrame(this, "landscape_static");

                QString newDoc;
                {
                    QTextStream ts(&newDoc);
                    ts << n;
                }
                vectorStaticBgFrame->fromXml(newDoc);
             }  else if (type == "landscape_raster_static") {
                rasterStaticBgFrame = new TupFrame(this, "landscape_raster_static");

                QString newDoc;
                {
                    QTextStream ts(&newDoc);
                    ts << n;
                }
                rasterStaticBgFrame->fromXml(newDoc);

                // Loading raster static background image
                QString imgPath = RASTER_BG_DIR + QString::number(sceneIndex) + "/bg/static_bg.png";
                if (QFile::exists(imgPath)) {
                    rasterStaticBgPix = QPixmap(imgPath);
                    rasterStaticUpdateRequired = true;
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupBackground::fromXml()] - Raster static image loaded -> " << imgPath;
                    #endif
                } else {
                    #ifdef TUP_DEBUG
                        qWarning() << "[TupBackground::fromXml()] - Raster static bg wasn't loaded -> " << imgPath;
                    #endif
                }                
            } else if (type == "landscape_vector_foreground") {
                vectorFgFrame = new TupFrame(this, "landscape_vector_foreground");

                QString newDoc;
                {
                    QTextStream ts(&newDoc);
                    ts << n;
                }
                vectorFgFrame->fromXml(newDoc);
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupBackground::fromXml()] - Error: The background input is invalid";
                #endif
            }
        }

        n = n.nextSibling();
    }
}

QDomElement TupBackground::toXml(QDomDocument &doc) const
{
    QDomElement root = doc.createElement("background");

    QString indexes = "";
    for (int i=0; i < bgLayerIndexes.count(); i++)
        indexes += QString::number(bgLayerIndexes.at(i)) + ",";
    indexes.chop(1);
    root.setAttribute("bgLayerIndexes", indexes);

    indexes = "";
    QString visibility = "";
    for (int i=0; i < bgVisibilityList.count(); i++)
        indexes += QString::number(bgVisibilityList.at(i)) + ",";
    indexes.chop(1);
    root.setAttribute("bgLayerVisibility", indexes);

    doc.appendChild(root);

    root.appendChild(vectorDynamicBgFrame->toXml(doc));
    root.appendChild(rasterDynamicBgFrame->toXml(doc));
    root.appendChild(vectorStaticBgFrame->toXml(doc));
    root.appendChild(rasterStaticBgFrame->toXml(doc));
    root.appendChild(vectorFgFrame->toXml(doc));

    return root;
}

QList<TupBackground::BgType> TupBackground::layerIndexes()
{
    return bgLayerIndexes;
}

void TupBackground::updateLayerIndexes(QList<TupBackground::BgType> indexes)
{
    if (bgLayerIndexes != indexes)
        bgLayerIndexes = indexes;
}

QList<bool> TupBackground::layersVisibility()
{
    return bgVisibilityList;
}

void TupBackground::updateLayersVisibility(QList<bool> viewFlags)
{
    if (bgVisibilityList != viewFlags)
        bgVisibilityList = viewFlags;
}

bool TupBackground::isLayerVisible(BgType bgId)
{
    int i;
    for (i = 0;i < bgLayerIndexes.count(); i++) {
        if (bgLayerIndexes.at(i) == bgId)
            break;
    }

    return bgVisibilityList.at(i);
}

bool TupBackground::vectorDynamicBgIsEmpty()
{
    return vectorDynamicBgFrame->isEmpty();
}

bool TupBackground::vectorStaticBgIsEmpty()
{
    return vectorStaticBgFrame->isEmpty();
}

bool TupBackground::vectorFgIsEmpty()
{
    return vectorFgFrame->isEmpty();
}

TupFrame *TupBackground::vectorStaticFrame()
{
    return vectorStaticBgFrame;
}

TupFrame* TupBackground::vectorDynamicFrame()
{
    return vectorDynamicBgFrame;
}

TupFrame *TupBackground::vectorForegroundFrame()
{
    return vectorFgFrame;
}

void TupBackground::clearBackground()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupBackground::clearBackground()]";
    #endif

    // SQA: Implement the UNDO action for this case

    if (vectorDynamicBgFrame) {
        vectorDynamicBgFrame->clear();
        vectorDynamicBgExpanded = QPixmap();
    }

    if (vectorStaticBgFrame)
        vectorStaticBgFrame->clear();

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

void TupBackground::renderVectorDynamicView()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupBackground::renderVectorDynamicView()] - Rendering view...";
    #endif

    TupBackgroundScene *bgScene = new TupBackgroundScene(dimension, QBrush(Qt::transparent),
                                                         vectorDynamicBgFrame);
    QImage vectorDynamicImg = QImage(dimension, QImage::Format_ARGB32);
    vectorDynamicImg.fill(Qt::transparent);

    QPainter *painter = new QPainter(&vectorDynamicImg);
    painter->setRenderHint(QPainter::Antialiasing, true);
    bgScene->renderView(painter);
    painter->end();

    int width = dimension.width();
    int height = dimension.height();

    bool isHorizontal = true;
    TupBackground::Direction direction = vectorDynamicBgFrame->dynamicDirection();
    switch (direction) {
        case TupBackground::Right:
        case TupBackground::Left:
        {
            width *= 2;
        }
        break;
        case TupBackground::Top:
        case TupBackground::Bottom:
        {
            height *= 2;
            isHorizontal = false;
        }
        break;
    }

    QImage bgView(width, height, QImage::Format_ARGB32);
    bgView.fill(Qt::transparent);
    QPainter *canvas = new QPainter(&bgView);
    canvas->drawImage(0, 0, vectorDynamicImg);
    if (isHorizontal)
        canvas->drawImage(dimension.width(), 0, vectorDynamicImg);
    else
        canvas->drawImage(0, dimension.height(), vectorDynamicImg);
    canvas->end();

    QString dirPath = VECTOR_BG_DIR + QString::number(sceneIndex) + "/bg/";
    QDir imgDir(dirPath);
    if (!imgDir.exists()) {
        if (!imgDir.mkpath(dirPath)) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupBackground::renderVectorDynamicView()] - Error creating image path -> " << dirPath;
            #endif
            return;
        }
    }

    if (!bgView.save(dirPath + "dynamic_bg.png", "PNG", 100)) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupBackground::renderVectorDynamicView()] - Error: can't save bg image at -> " << dirPath;
        #endif
        return;
    }

    vectorDynamicBgExpanded = QPixmap::fromImage(bgView);
    vectorDynamicRenderRequired = false;

    free(painter);
    free(canvas);
}

// Creating expanded raster dynamic image
void TupBackground::renderRasterDynamicView()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupBackground::renderRasterDynamicView()]";
    #endif

    int width = dimension.width();
    int height = dimension.height();

    bool isHorizontal = true;
    TupBackground::Direction direction = rasterDynamicBgFrame->dynamicDirection();
    switch (direction) {
        case TupBackground::Right:
        case TupBackground::Left:
        {
            width *= 2;
        }
        break;
        case TupBackground::Top:
        case TupBackground::Bottom:
        {
            height *= 2;
            isHorizontal = false;
        }
        break;
    }

    QImage image = rasterDynamicBgPix.toImage();
    QImage bgView(width, height, QImage::Format_ARGB32);
    bgView.fill(Qt::transparent);

    QPainter *canvas = new QPainter(&bgView);
    canvas->drawImage(0, 0, image);
    if (isHorizontal)
        canvas->drawImage(dimension.width(), 0, image);
    else
        canvas->drawImage(0, dimension.height(), image);
    canvas->end();

    QString dirPath = RASTER_BG_DIR + QString::number(sceneIndex) + "/bg/";
    QDir imgDir(dirPath);
    if (!imgDir.exists()) {
        if (!imgDir.mkpath(dirPath)) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupBackground::renderRasterDynamicView()] - Error creating image path -> " << dirPath;
            #endif
            return;
        }
    }

    if (!bgView.save(dirPath + "dynamic_bg.png", "PNG", 100)) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupBackground::renderRasterDynamicView()] - Error: can't save bg image at -> " << dirPath;
        #endif
        return;
    }

    rasterDynamicBgExpanded = QPixmap::fromImage(bgView);
    rasterDynamicRenderRequired = false;
}

QPoint TupBackground::calculatePoint(TupBackground::Direction direction, int frameIndex, int shift)
{
    int posX = 0;
    int posY = 0;
    switch (direction) {
        case TupBackground::Right:
        {
            posX = - dimension.width();
            int delta = dimension.width() / shift;

            if (frameIndex < delta) { // Position is inside the workspace
                posX += (frameIndex * shift);
            } else { // Position is outside the workspace
                int mod = static_cast<int> (fmod(frameIndex, delta));
                posX += (mod * shift);
            }
        }
        break;
        case TupBackground::Left:
        {
            posX = 0;
            int delta = dimension.width() / shift;

            if (delta > frameIndex) {
                posX -= (frameIndex * shift);
            } else {
                int mod = static_cast<int> (fmod(frameIndex, delta));
                posX -= (mod * shift);
            }
        }
        break;
        case TupBackground::Top:
        {
            posY = 0;
            int delta = dimension.height() / shift;

            if (delta > frameIndex) {
                posY -= (frameIndex * shift);
            } else {
                int mod = static_cast<int> (fmod(frameIndex, delta));
                posY -= (mod * shift);
            }
        }
        break;
        case TupBackground::Bottom:
        {
            posY = - dimension.height();
            int delta = dimension.height() / shift;

            if (delta > frameIndex) {
                posY += (frameIndex * shift);
            } else {
                int mod = static_cast<int> (fmod(frameIndex, delta));
                posY += (mod * shift);
            }
        }
        break;
    }

    return QPoint(posX, posY);
}

QPoint TupBackground::vectorDynamicPos(int frameIndex)
{
    int shift = vectorDynamicShift();
    if (shift == 0)
        shift = 5;

    #ifdef TUP_DEBUG
        qDebug() << "[TupBackground::vectorDynamicPos()] - shift: " << shift;
        qDebug() << "[TupBackground::vectorDynamicPos()] - frameIndex: " << frameIndex;
    #endif

    return calculatePoint(vectorDynamicBgFrame->dynamicDirection(), frameIndex, shift);
}

QPoint TupBackground::rasterDynamicPos(int frameIndex)
{
    int shift = rasterDynamicShift();
    if (shift == 0)
        shift = 5;

    #ifdef TUP_DEBUG
        qDebug() << "[TupBackground::rasterDynamicPos()] - shift: " << shift;
        qDebug() << "[TupBackground::rasterDynamicPos()] - frameIndex: " << frameIndex;
    #endif

    return calculatePoint(rasterDynamicBgFrame->dynamicDirection(), frameIndex, shift);
}

bool TupBackground::vectorRenderIsPending()
{
    return vectorDynamicRenderRequired;
}

bool TupBackground::rasterStaticUpdateIsPending()
{
    return rasterStaticUpdateRequired;
}

void TupBackground::updateRasterStaticStatus(bool flag)
{
    rasterStaticUpdateRequired = flag;
}

bool TupBackground::rasterDynamicRenderIsPending()
{
    return rasterDynamicRenderRequired;
}

void TupBackground::scheduleVectorRender(bool status)
{
    vectorDynamicRenderRequired = status;
}

void TupBackground::scheduleRasterRender(bool status)
{
    rasterDynamicRenderRequired = status;
}

QPixmap TupBackground::vectorDynamicExpandedImage()
{
    return vectorDynamicBgExpanded;
}

QPixmap TupBackground::rasterDynamicExpandedImage()
{
    return rasterDynamicBgExpanded;
}

void TupBackground::setVectorDynamicDirection(int direction)
{
    vectorDynamicBgFrame->setDynamicDirection(QString::number(direction));
    vectorDynamicRenderRequired = true;
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
            rasterDynamicRenderRequired = true;
        } else {
            rasterDynamicBgPix = QPixmap();
        }
    } else {
        if (QFile::exists(imgPath)) {
            rasterStaticBgPix = QPixmap(imgPath);
            rasterStaticUpdateRequired = true;
        } else {
            rasterStaticBgPix = QPixmap();
        }
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
