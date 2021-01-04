/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustav Gonzalez / xtingray                                           *
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

#ifndef TUPREFLEXRENDERAREA_H
#define TUPREFLEXRENDERAREA_H

#include "tglobal.h"

#include <QPen>
#include <QPixmap>
#include <QWidget>

class TUPITUBE_EXPORT TupReflexRenderArea : public QWidget
{
    Q_OBJECT

    public:
        TupReflexRenderArea(const QSize &size, QWidget *parent = nullptr);

        QSize minimumSizeHint() const override;
        QSize sizeHint() const override;

        void enableSafeArea(bool enabled);
        void enableGrid(bool enabled);
        void updateGridSpacing(int space);
        void updateGridColor(const QColor color);
        void showHistory(bool flag);
        void updateImagesOpacity(double factor);
        void updateImagesDepth(int depth);

        void addPixmap(const QString &path);

    protected:
        void paintEvent(QPaintEvent *event) override;

    private:
        QSize size;
        int width;
        int height;
        QList<QPixmap> stack;

        QPen gridAxesPen;
        QPen gridPen;
        QPen whitePen;
        QPen grayPen;
        QPen greenThickPen;
        QPen greenThinPen;

        bool isSafeAreaEnabled;
        bool isGridEnabled;
        int gridSpace;
        int historySize;

        bool showPrevious;
        double opacity;
};

#endif // TUPREFLEXRENDERAREA_H
