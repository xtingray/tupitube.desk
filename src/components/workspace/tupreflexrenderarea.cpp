/***************************************************************************
 *   Project TUPITUBE DESK                                                 *
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

#include "tupreflexrenderarea.h"

#include <QPainter>
#include <QBrush>

TupReflexRenderArea::TupReflexRenderArea(const QSize &areaSize, QWidget *parent) : QWidget(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupReflexRenderArea()] - areaSize -> " << areaSize;
    #endif

    size = areaSize;
    width = areaSize.width();
    height = areaSize.height();
    isSafeAreaEnabled = false;
    isGridEnabled = false;
    gridSpace = 10;
    showPrevious = false;
    opacity = 0.5;
    historySize = 1;

    gridPen = QPen(QColor(0, 0, 180, 50), 1);
    gridAxesPen = QPen(QColor(0, 135, 0, 150), 1);
    whitePen = QPen(QColor(255, 255, 255, 255), 1);
    grayPen = QPen(QColor(150, 150, 150, 255), 1);
    greenThickPen = QPen(QColor(0, 135, 0, 255), 3);
    greenThinPen = QPen(QColor(0, 135, 0, 255), 1);
}

QSize TupReflexRenderArea::minimumSizeHint() const
{
    return size;
}

QSize TupReflexRenderArea::sizeHint() const
{
    return size;
}

void TupReflexRenderArea::enableSafeArea(bool enabled)
{
    isSafeAreaEnabled = enabled;
    update();
}

void TupReflexRenderArea::enableGrid(bool enabled)
{
    isGridEnabled = enabled;
    update();
}

void TupReflexRenderArea::updateGridSpacing(int space)
{
    gridSpace = space;
    update();
}

void TupReflexRenderArea::updateGridColor(const QColor color)
{
    QColor gridColor = color;
    gridColor.setAlpha(50);
    gridPen = QPen(gridColor);
    update();
}

void TupReflexRenderArea::showHistory(bool flag)
{
    showPrevious = flag;
    update();
}

void TupReflexRenderArea::updateImagesOpacity(double factor)
{
    opacity = factor;
    update();
}

void TupReflexRenderArea::updateImagesDepth(int depth)
{
    historySize = depth;
    update();
}

void TupReflexRenderArea::addPixmap(const QString &path)
{
    QPixmap pic;
    pic.load(path);
    stack << pic;
    update();
}

void TupReflexRenderArea::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPixmap frame;
    int total = stack.count();

    if (showPrevious) {
        if (total > 1) {
            if (historySize > 0)  {
                int range = historySize + 1;
                if (range > total)
                    range = total;

                int start = total - range;

                for (int i = start; i < total; i++) {
                    painter.setOpacity(opacity);
                    frame = stack.at(i);
                    painter.drawPixmap(0, 0, frame);
                }
            } else {
                frame = stack.last();
                painter.setOpacity(1.0);
                painter.drawPixmap(0, 0, frame);
            }
        } else {
            frame = stack.at(0);
            painter.setOpacity(1.0);
            painter.drawPixmap(0, 0, frame);
        }
    } else {
        if (total > 0) {
            frame = stack.last();
            painter.setOpacity(1.0);
            painter.drawPixmap(0, 0, frame);
        } else {
            frame = QPixmap(size);
            frame.fill(Qt::gray);
            painter.drawPixmap(0, 0, frame);
        }
    }

    if (isGridEnabled) {
        int midX = width/2;
        int midY = height/2;
        painter.setPen(gridPen);
        int initX = midX - gridSpace;
        for (int i=initX; i > 0; i -= gridSpace)
             painter.drawLine(i, 0, i, height);
        initX = midX + gridSpace;
        for (int i=initX; i < width; i += gridSpace)
             painter.drawLine(i, 0, i, height);

        int initY = midY - gridSpace;
        for (int i=initY; i > 0; i -= gridSpace)
             painter.drawLine(0, i, width, i);
        initY = midY + gridSpace;
        for (int i=initY; i < height; i += gridSpace)
             painter.drawLine(0, i, width, i);

        painter.setPen(gridAxesPen);
        painter.drawLine(midX, 0, midX, height);
        painter.drawLine(0, midY, width, midY);
    }

    if (isSafeAreaEnabled) {
        painter.setPen(whitePen);
        int outerBorder = width/19;
        int innerBorder = width/6;

        int hSpace = width/3;
        int vSpace = height/3;
        int hSpace2 = hSpace*2;
        int vSpace2 = vSpace*2;

        QPoint rectLeft = frame.rect().topLeft();
        QPoint rectRight = frame.rect().bottomRight();

        QPointF left = rectLeft + QPointF(outerBorder, outerBorder);
        QPointF right = rectRight - QPointF(outerBorder, outerBorder);
        QRectF outerRect(left, right);

        painter.setPen(grayPen);
        painter.drawRect(outerRect);

        int leftY = left.y();
        int leftX = left.x();
        int rightY = right.y();
        int rightX = right.x();

        painter.setPen(greenThickPen);
        painter.drawLine(QPoint(hSpace, leftY - 8), QPoint(hSpace, leftY + 8));
        painter.drawLine(QPoint(hSpace - 5, leftY), QPoint(hSpace + 5, leftY));
        painter.drawLine(QPoint(hSpace2, leftY - 8), QPoint(hSpace2, leftY + 8));
        painter.drawLine(QPoint(hSpace2 - 5, leftY), QPoint(hSpace2 + 5, leftY));

        painter.drawLine(QPoint(hSpace, rightY - 8), QPoint(hSpace, rightY + 8));
        painter.drawLine(QPoint(hSpace - 5, rightY), QPoint(hSpace + 5, rightY));
        painter.drawLine(QPoint(hSpace2, rightY - 8), QPoint(hSpace2, rightY + 8));
        painter.drawLine(QPoint(hSpace2 - 5, rightY), QPoint(hSpace2 + 5, rightY));

        painter.drawLine(QPoint(leftX - 8, vSpace), QPoint(leftX + 8, vSpace));
        painter.drawLine(QPoint(leftX, vSpace - 5), QPoint(leftX, vSpace + 5));
        painter.drawLine(QPoint(leftX - 8, vSpace2), QPoint(leftX + 8, vSpace2));
        painter.drawLine(QPoint(leftX, vSpace2 - 5), QPoint(leftX, vSpace2 + 5));

        painter.drawLine(QPoint(rightX - 8, vSpace), QPoint(rightX + 8, vSpace));
        painter.drawLine(QPoint(rightX, vSpace - 5), QPoint(rightX, vSpace + 5));
        painter.drawLine(QPoint(rightX - 8, vSpace2), QPoint(rightX + 8, vSpace2));
        painter.drawLine(QPoint(rightX, vSpace2 - 5), QPoint(rightX, vSpace2 + 5));

        painter.setPen(greenThinPen);

        left = rectLeft + QPointF(innerBorder, innerBorder);
        right = rectRight - QPointF(innerBorder, innerBorder);
        QRectF innerRect(left, right);

        painter.drawRect(innerRect);
    } 
}
