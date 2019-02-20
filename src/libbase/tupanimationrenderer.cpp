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

#include "tupanimationrenderer.h"
#include "tuplayer.h"
#include "tupframe.h"
#include "tupgraphicobject.h"

TupAnimationRenderer::TupAnimationRenderer(const QColor color, TupLibrary *library)
{
    bgColor = color;
    gScene = new TupGraphicsScene;
    gScene->setLibrary(library);
    gScene->setBackgroundBrush(bgColor);
}

TupAnimationRenderer::~TupAnimationRenderer()
{
}

int TupAnimationRenderer::calculateTotalPhotograms(TupScene *scene)
{
    Layers layers = scene->getLayers();

    int total = 0;

    int totalLayers = layers.size();
    for (int i = 0; i < totalLayers; i++) {
         TupLayer *layer = layers.at(i);
         if (layer)
             total = qMax(total, layer->frames().count());
    }

    return total;
}

void TupAnimationRenderer::setScene(TupScene *scene, QSize dimension)
{
    gScene->setCurrentScene(scene);
    gScene->setSceneRect(QRectF(QPointF(0,0), dimension));

    // k->scene->drawPhotogram(0, false); // ### SQA: Why whithout this doesn't work?
    currentPhotogram = -1;

    totalPhotograms = calculateTotalPhotograms(scene);
}

bool TupAnimationRenderer::nextPhotogram()
{
    if (totalPhotograms < 0)
        return false;

    currentPhotogram++;

    if (currentPhotogram == totalPhotograms)
        return false;

    gScene->drawPhotogram(currentPhotogram, false);

    return true;
}

void TupAnimationRenderer::renderPhotogram(int index) 
{
#ifdef TUP_DEBUG
    #ifdef Q_OS_WIN
        qDebug() << "[TupAnimationRenderer::renderPhotogram()] - index -> " << index;
    #else
        T_FUNCINFO << " index -> " << index;
    #endif
#endif

    gScene->drawPhotogram(index, false);
}

void TupAnimationRenderer::render(QPainter *painter)
{
#ifdef TUP_DEBUG
    #ifdef Q_OS_WIN
        qDebug() << "[TupAnimationRenderer::render()]";
    #else
        T_FUNCINFO;
    #endif
#endif

    gScene->render(painter, gScene->sceneRect().toRect(),
                     gScene->sceneRect().toRect(), Qt::IgnoreAspectRatio);
}

int TupAnimationRenderer::getCurrentPhotogram() const
{
    return currentPhotogram;
}

int TupAnimationRenderer::getTotalPhotograms() const
{
    return totalPhotograms;
}
