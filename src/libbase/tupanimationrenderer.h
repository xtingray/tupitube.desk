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

#ifndef TUPANIMATIONRENDERER_H
#define TUPANIMATIONRENDERER_H

#include "tglobal.h"
#include "tupgraphicsscene.h"
#include "tupscene.h"
#include "tuplibrary.h"

#include <QColor>
#include <QSize>
#include <QPainter>

class TUPITUBE_EXPORT TupAnimationRenderer
{
    public:
        TupAnimationRenderer(const QColor color, TupLibrary *library = nullptr);
        ~TupAnimationRenderer();

        void setScene(TupScene *scene, QSize dimension);

        bool nextPhotogram();
        void renderPhotogram(int index);
        void render(QPainter *painter);

        int getCurrentPhotogram() const;
        int getTotalPhotograms() const;

    private:
        int calculateTotalPhotograms(TupScene *scene);

        TupGraphicsScene *gScene;
        int totalPhotograms;
        int currentPhotogram;
        QColor bgColor;
};

#endif
