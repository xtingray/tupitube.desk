/***************************************************************************
 *   Project TUPITUBE DESK                                                 *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *                                                                         *
 *   Developers:                                                           *
 *   2012:                                                                 *
 *    Gustavo Gonzalez / @xtingray                                         *
 *    Andres Calderon / @andresfcalderon                                   *
 *    Antonio Vanegas / @hpsaturn                                          *
 *                                                                         *
 *   TupiTube Desk is a fork of the KTooN project                          *
 *   KTooN's versions:                                                     *
 *   2006:                                                                 *
 *    David Cuadrado                                                       *
 *    Jorge Cuadrado                                                       *
 *   2003:                                                                 *
 *    Fernado Roldan                                                       *
 *    Simena Dinas                                                         *
 *                                                                         *
 *   Copyright (C) 2012 Mae Floresta - http://www.maefloresta.com          *
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

#include "tslider.h"
#include "tapplicationproperties.h" 

#include <QDesktopWidget>

TSlider::TSlider(Qt::Orientation orientation, TSlider::Mode mode, const QColor &start, const QColor &end, QWidget *parent) : QGraphicsView(parent)
{
    setStyleSheet("* { background-color: rgba(255,255,255,0); border: 1px solid rgb(170,170,170); }");

    sliderOrientation = orientation;
    sliderMode = mode;
    startColor = start;
    endColor = end;
    value = 0;
    enabled = true;

    if (sliderOrientation == Qt::Vertical) {
        image = new QImage(THEME_DIR + "icons/slider_vertical.png");
        imageW = image->width();
        imageH = image->height();
        setFixedWidth(imageW + 2);
    } else {
        image = new QImage(THEME_DIR + "icons/slider_horizontal.png");
        imageW = image->width();
        imageH = image->height();
        setFixedHeight(imageH + 2);
    }

    setUpdatesEnabled(true);

    if (sliderOrientation == Qt::Vertical)
        currentBase = viewport()->height();
    else
        currentBase = viewport()->width();
}

TSlider::~TSlider()
{
    delete image;
}

void TSlider::setBrushSettings(Qt::BrushStyle style, double opacity)
{
    sliderStyle = style;
    sliderOpacity = opacity;
}

void TSlider::setRange(int min, int max)
{
    minRange = min;
    maxRange = max;
}

void TSlider::setValue(int value)
{
    if (sliderOrientation == Qt::Vertical) {
        int height = viewport()->height();
        if (value == maxRange) {
            value = height - image->height()/2;
        } else if (value == minRange) {
            value = 0;
        } else {
            value = height*value/maxRange;
        }
    } else {
        int width = viewport()->width();
        if (value == maxRange) {
            value = width - image->width()/2;
        } else if (value == minRange) {
            value = 0;
        } else {
            value = width*value/maxRange;
        }
    }

    this->update();
}

void TSlider::setEnabled(bool flag)
{
    enabled = flag;
    this->update();
}

bool TSlider::isEnabled()
{
    return enabled;
}

void TSlider::setColors(const QColor &start, const QColor &end)
{
    startColor = start;
    endColor = end;

    this->update();
}

void TSlider::mousePressEvent(QMouseEvent *event)
{
    if (!enabled)
        return;

    int pos = -1;
    if (sliderOrientation == Qt::Vertical)
        pos = event->y();
    else
        pos = event->x();

    calculateNewPosition(pos);
}

void TSlider::mouseMoveEvent(QMouseEvent *event)
{
    if (!enabled)
        return;

    int pos = -1;
    if (sliderOrientation == Qt::Vertical)
        pos = event->y();
    else
        pos = event->x();

    calculateNewPosition(pos);
}

void TSlider::calculateNewPosition(int pos)
{
    int length = -1;
    value = pos;

    if (sliderOrientation == Qt::Vertical) {
        length = viewport()->height();
        if (pos > (length - image->height())) {
            this->update();
            if (sliderMode == Color)
                calculateColor(minRange);
            else
                emit valueChanged(minRange);
            return;
        }
    } else {
        length = viewport()->width();
        if (pos > (length - image->width())) {
            this->update();
            if (sliderMode == Color)
                calculateColor(maxRange);
            else
                emit valueChanged(maxRange);
            return;
        }
    }

    int value = -1;
    if (sliderOrientation == Qt::Vertical)
        value = minRange + (maxRange - minRange) * (1.0 - float(pos)/float(length));
    else 
        value = minRange + (maxRange - minRange) * (float(pos)/float(length));

    if (value < 0)
        value = 0;

    if (value < minRange)
        value = minRange;

    this->update();

    if (sliderMode == Color)
        calculateColor(value);
    else
        emit valueChanged(value);
}

void TSlider::calculateColor(int value)
{
    int r = endColor.red();
    int g = endColor.green();
    int b = endColor.blue();

    r = (r*value)/100;
    g = (g*value)/100;
    b = (b*value)/100;

    QColor color = QColor(r, g, b);
    emit colorChanged(color);
}

void TSlider::paintScales()
{
    QPainter painter(viewport());

    if (!enabled) {
        QColor color(232, 232, 232);
        painter.setPen(color);
        painter.setBrush(color);
        painter.drawRect(0, 0, viewport()->width(), viewport()->height());
        return;
    }

    int width = viewport()->width();
    int height = viewport()->height();
    int length = viewport()->width();
    if (sliderOrientation == Qt::Vertical)
        length = viewport()->height();
    int segments = 32;
    int delta = length/(segments-1);

    if (sliderMode == Color) {
        for (int section=0; section<=segments; section++) {
             QColor color;
             int r;
             int g;
             int b;
             r = section*(endColor.red() - startColor.red()) / segments + startColor.red();
             g = section*(endColor.green() - startColor.green()) / segments + startColor.green();
             b = section*(endColor.blue() - startColor.blue()) / segments + startColor.blue();

             if ((r > -1 && r < 256) && (g > -1 && g < 256) && (b > -1 && b < 256)) {
                 color.setRed(r);
                 color.setGreen(g);
                 color.setBlue(b);

                 painter.setPen(color);
                 painter.setBrush(color);

                 if (sliderOrientation == Qt::Vertical)
                     painter.drawRect((width - imageW)/2, section*delta, imageW, delta);
                 else 
                     painter.drawRect(section*delta, (height - imageH)/2, delta, imageH);
            }
        }
    } else if (sliderMode == Size) {
               painter.setPen(QColor(232, 232, 232));
               painter.setBrush(QBrush(endColor, sliderStyle));
               painter.setOpacity(sliderOpacity);
               painter.setRenderHint(QPainter::Antialiasing);

               QPainterPath path;

               if (sliderOrientation == Qt::Vertical) {
                   path = QPainterPath(QPointF(0, 0));
                   path.lineTo(QPointF(0, height));
                   path.lineTo(QPointF(width, height));
                   path.lineTo(QPointF(0, 0));
               } else {
                   path = QPainterPath(QPointF(0, height));
                   path.lineTo(QPointF(width, 0)); 
                   path.lineTo(QPointF(width, height));
                   path.lineTo(QPointF(0, height));
               }

               painter.drawPath(path);
               painter.setOpacity(1.0);
    } else if (sliderMode == Opacity) {
               double opacityDelta = 1.0/32; 
               double opacity = 0;
               for (int section=0; section<=segments; section++) {
                    painter.setPen(QColor(232, 232, 232));
                    painter.setBrush(endColor);
                    painter.setOpacity(opacity);
                    opacity += opacityDelta;

                    if (sliderOrientation == Qt::Vertical) {
                        painter.drawRect((width - imageW)/2, section*delta, imageW, delta);
                     } else {
                        painter.drawRect(section*delta, (height - imageH)/2, delta, imageH);
                     }
               }
               painter.setOpacity(1.0);
    } else if (sliderMode == FPS) {
        for (int section=0; section<=segments; section++) {
             painter.setPen(QColor(232, 232, 232));
             painter.setBrush(endColor);
             painter.setOpacity(1.0);
             if (sliderOrientation == Qt::Vertical) {
                 painter.drawRect((width - imageW)/2, section*delta, imageW, delta);
              } else {
                 painter.drawRect(section*delta, (height - imageH)/2, delta, imageH);
              }
        }
    }

    // Drawing selector image
    if (sliderOrientation == Qt::Vertical) {
        int h = viewport()->height();

        if (value >= h || currentBase != h) {
            value = (value * h)/currentBase;
            currentBase = h;
        }

        painter.drawImage((width/2)-(imageW/2), value, *image);

        int x = (width/2)-(imageW/2);
        int middle = imageH/2;
        if (value <= middle) {
            painter.drawImage(x, value, *image);
        } else if (value >= (h - middle)) {
            painter.drawImage(x, h - imageH, *image);
        } else {
            painter.drawImage(x, value - middle, *image);
        }
    } else {
        int w = viewport()->width();

        if (value >= w || currentBase != w) {
            value = (value * w)/currentBase;
            currentBase = w;
        }

        int y = (height/2)-(imageH/2);
        int middle = imageW/2;
        if (value <= middle) {
            painter.drawImage(value, y, *image);
        } else if (value >= (w - middle)) {
            painter.drawImage(w - imageW, y, *image);
        } else {
            painter.drawImage(value - middle, y, *image);
        }
    }
}

void TSlider::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    paintScales();
}
