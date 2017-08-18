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

struct TupReflexRenderArea::Private
{
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

    bool safeArea;
    bool grid;
    int gridSpace;
    int historySize;

    bool showPrevious;
    double opacity;
};

TupReflexRenderArea::TupReflexRenderArea(const QSize &size, QWidget *parent) : QWidget(parent), k(new Private) 
{
    k->size = size;
    k->width = size.width();
    k->height = size.height();
    k->safeArea = false;
    k->grid = false;
    k->gridSpace = 10;
    k->showPrevious = false;
    k->opacity = 0.5;
    k->historySize = 1;

    k->gridPen = QPen(QColor(0, 0, 180, 50), 1);
    k->gridAxesPen = QPen(QColor(0, 135, 0, 150), 1);
    k->whitePen = QPen(QColor(255, 255, 255, 255), 1);
    k->grayPen = QPen(QColor(150, 150, 150, 255), 1);
    k->greenThickPen = QPen(QColor(0, 135, 0, 255), 3);
    k->greenThinPen = QPen(QColor(0, 135, 0, 255), 1);
}

QSize TupReflexRenderArea::minimumSizeHint() const
{
    return k->size;
}

QSize TupReflexRenderArea::sizeHint() const
{
    return k->size;
}

void TupReflexRenderArea::enableSafeArea(bool enabled)
{
    k->safeArea = enabled;
    update();
}

void TupReflexRenderArea::enableGrid(bool enabled)
{
    k->grid = enabled;
    update();
}

void TupReflexRenderArea::updateGridSpacing(int space)
{
    k->gridSpace = space;
    update();
}

void TupReflexRenderArea::updateGridColor(const QColor color)
{
    QColor gridColor = color;
    gridColor.setAlpha(50);
    k->gridPen = QPen(gridColor);
    update();
}

void TupReflexRenderArea::showHistory(bool flag)
{
    k->showPrevious = flag;
    update();
}

void TupReflexRenderArea::updateImagesOpacity(double opacity)
{
    k->opacity = opacity;
    update();
}

void TupReflexRenderArea::updateImagesDepth(int depth)
{
    k->historySize = depth;
    update();
}

void TupReflexRenderArea::addPixmap(const QString &path)
{
    QPixmap pic;
    pic.load(path);
    k->stack << pic;
    update();
}

void TupReflexRenderArea::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPixmap frame;
    int size = k->stack.count();

    if (k->showPrevious) {
        if (size > 1) {
            if (k->historySize > 0)  {
                int range = k->historySize + 1;
                if (range > size)
                    range = size;

                int start = size - range;

                for (int i=start; i < size; i++) {
                    painter.setOpacity(k->opacity);
                    frame = k->stack.at(i);
                    painter.drawPixmap(0, 0, frame);
                }
            } else {
                frame = k->stack.last();
                painter.setOpacity(1.0);
                painter.drawPixmap(0, 0, frame);
            }
        } else {
            frame = k->stack.at(0);
            painter.setOpacity(1.0);
            painter.drawPixmap(0, 0, frame);
        }
    } else {
        if (size > 0) {
            frame = k->stack.last();
            painter.setOpacity(1.0);
            painter.drawPixmap(0, 0, frame);
        } else {
            frame = QPixmap(k->size);
            frame.fill(Qt::gray);
            painter.drawPixmap(0, 0, frame);
        }
    }

    if (k->grid) {
        int midX = k->width/2;
        int midY = k->height/2;
        painter.setPen(k->gridPen);
        int initX = midX - k->gridSpace;
        for (int i=initX; i > 0; i -= k->gridSpace)
             painter.drawLine(i, 0, i, k->height);
        initX = midX + k->gridSpace;
        for (int i=initX; i < k->width; i += k->gridSpace)
             painter.drawLine(i, 0, i, k->height);

        int initY = midY - k->gridSpace;
        for (int i=initY; i > 0; i -= k->gridSpace)
             painter.drawLine(0, i, k->width, i);
        initY = midY + k->gridSpace;
        for (int i=initY; i < k->height; i += k->gridSpace)
             painter.drawLine(0, i, k->width, i);

        painter.setPen(k->gridAxesPen);
        painter.drawLine(midX, 0, midX, k->height);
        painter.drawLine(0, midY, k->width, midY);
    }

    if (k->safeArea) {
        painter.setPen(k->whitePen);
        int outerBorder = k->width/19;
        int innerBorder = k->width/6;

        int hSpace = k->width/3;
        int vSpace = k->height/3;
        int hSpace2 = hSpace*2;
        int vSpace2 = vSpace*2;

        QPoint rectLeft = frame.rect().topLeft();
        QPoint rectRight = frame.rect().bottomRight();

        QPointF left = rectLeft + QPointF(outerBorder, outerBorder);
        QPointF right = rectRight - QPointF(outerBorder, outerBorder);
        QRectF outerRect(left, right);

        painter.setPen(k->grayPen);
        painter.drawRect(outerRect);

        int leftY = left.y();
        int leftX = left.x();
        int rightY = right.y();
        int rightX = right.x();

        painter.setPen(k->greenThickPen);
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

        painter.setPen(k->greenThinPen);

        left = rectLeft + QPointF(innerBorder, innerBorder);
        right = rectRight - QPointF(innerBorder, innerBorder);
        QRectF innerRect(left, right);

        painter.drawRect(innerRect);
    } 
}
