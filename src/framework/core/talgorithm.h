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

#ifndef TALGORITHM_H
#define TALGORITHM_H

#include "tglobal.h"

#include <QColor>

class T_CORE_EXPORT TAlgorithm
{
    public:
        static int random();
        static QString randomString(int length);
        static QColor randomColor(bool withAlpha = false);
        static int randomNumber(int limit);
        static void storeData(const QString &data);
        static bool cacheIDChanged(const QString &data);
        static void resetCacheID();
        static QString windowCacheID();
        static QStringList header(const QString &input);
        static bool isKeyRandomic(const QString &id);
        static bool copyFolder(const QString &src, const QString &dst);
        static QStringList naturalSort(QStringList elements);

        static float distance(const QPointF &p1, const QPointF &p2);
        static float slope(const QPointF &p1, const QPointF &p2);
        static float inverseSlope(const QPointF &p1, const QPointF &p2);
        static float calculateBFromLine(const QPointF &point, float slope);
        static float calculateYFromLine(float x, float m, float b);
        static float distanceFromLine(QPointF linePoint1, QPointF linePoint2, QPointF point);
};

#endif
