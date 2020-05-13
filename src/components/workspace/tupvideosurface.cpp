/***************************************************************************
 *   Project TUPITUBE DESK                                                 *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez                                                     *
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

#include "tupvideosurface.h"

#include <QScreen>
#include <QGuiApplication>

TupVideoSurface::TupVideoSurface(QWidget *widget, VideoIF *target, const QSize &size, bool scaleFlag,
                                 int orientation, QObject *parent) : QAbstractVideoSurface(parent)
{
    setNativeResolution(size);

    isScaled = scaleFlag;
    targetWidget = widget;
    videoIF = target;
    imageFormat = QImage::Format_Invalid;
    displaySize = size;
    safeArea = false;
    grid = false;
    showPrevious = false;
    opacity = 127;
    historySize = 1;
    gridSpace = 10;
    historyInit = 0;
    historyEnd = 0;

    gridPen = QPen(QColor(0, 0, 180, 50), 1);
    gridAxesPen = QPen(QColor(0, 135, 0, 150), 1);
    whitePen = QPen(QColor(255, 255, 255, 255), 1);
    grayPen = QPen(QColor(150, 150, 150, 255), 1);
    greenThickPen = QPen(QColor(0, 135, 0, 255), 3);
    greenThinPen = QPen(QColor(0, 135, 0, 255), 1);

    QRect rect = targetWidget->rect();
    widgetWidth = rect.size().width();
    widgetHeight = rect.size().height();

    const QScreen *screen = QGuiApplication::primaryScreen();

    #ifdef TUP_DEBUG
        qWarning() << "TupVideoSurface() - isScaled: " + QString::number(isScaled);
        qWarning() << "TupVideoSurface() - displaySize: " + QString::number(size.width()) + ", " + QString::number(size.height());
        qWarning() << "TupVideoSurface() - widgetSize: " + QString::number(widgetWidth) + ", " + QString::number(widgetHeight);
        qWarning() << "TupVideoSurface() - Screen Orientation: " + QString::number(screen->nativeOrientation());
    #endif

    const int screenAngle = screen->angleBetween(screen->nativeOrientation(), screen->orientation());
    rotation = (360 - orientation + screenAngle) % 360;
}

TupVideoSurface::~TupVideoSurface()
{
}

bool TupVideoSurface::start(const QVideoSurfaceFormat &format)
{
    const QImage::Format imgFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
    const QSize size = format.frameSize();

    if (imgFormat != QImage::Format_Invalid && !size.isEmpty()) {
        imageFormat = imgFormat;
        QAbstractVideoSurface::start(format);
        return true;
    } else {
        return false;
    }
}

bool TupVideoSurface::present(const QVideoFrame &vFrame)
{
    frame = vFrame;
    if (surfaceFormat().pixelFormat() != frame.pixelFormat() ||
        surfaceFormat().frameSize() != frame.size()) {
        stop();
        return false;
    } else {
        videoIF->updateVideo();
        return true;
    }
}

void TupVideoSurface::paint(QPainter *painter)
{
    if (frame.map(QAbstractVideoBuffer::ReadOnly)) {
        int width = frame.width();
        int height = frame.height();
        QImage image(frame.bits(), width, height, frame.bytesPerLine(), imageFormat);

        if (isScaled) {
            width = (displaySize.width() * height) / displaySize.height();
            int posX = (image.width() - width)/2;
            int posY = 0;
            if (width > image.width()) {
                width = image.width();
                height = (displaySize.height() * width) / displaySize.width();
                posX = 0;
                posY = (image.height() - height)/2;
            }
            QImage mask = image.copy(posX, posY, width, height);
            image = mask.scaledToWidth(displaySize.width(), Qt::SmoothTransformation);

            width = image.width();
            height = image.height();
        }

        QPoint leftTop((qAbs(widgetWidth - width))/2, (qAbs(widgetHeight - height))/2);

        if (!image.isNull()) {
            if (rotation != 0)
                image = image.mirrored();
                // image = image.transformed(QTransform().rotate(rotation));

            painter->drawImage(leftTop, image);
        }

        if (showPrevious && !history.empty() && historySize > 0) {
            for (int i=historyInit; i <= historyEnd; i++) {
                 QImage image = history.at(i);
                 image = image.scaledToWidth(width, Qt::SmoothTransformation);

                 QPixmap transparent(QSize(width, height));
                 transparent.fill(Qt::transparent);

                 QPainter p;
                 p.begin(&transparent);
                 p.setCompositionMode(QPainter::CompositionMode_Source);
                 p.drawPixmap(0, 0, QPixmap::fromImage(image));
                 p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                 p.fillRect(transparent.rect(), QColor(0, 0, 0, opacity));
                 p.end();

                 transparent = transparent.scaledToWidth(width, Qt::SmoothTransformation);
                 painter->drawPixmap(leftTop, transparent);
            }
        }

        int midX = displaySize.width() / 2;
        int midY = displaySize.height() / 2;

        int minX = midX - (width/2);
        int maxX = midX + (width/2);
        int minY = midY - (height/2);
        int maxY = midY + (height/2);

        if (grid) {
            painter->setPen(gridPen);

            int initX = midX - gridSpace;
            for (int i=initX; i > minX; i -= gridSpace)
                 painter->drawLine(i, minY, i, maxY);

            initX = midX + gridSpace;
            for (int i=initX; i < maxX; i += gridSpace)
                 painter->drawLine(i, minY, i, maxY);

            int initY = midY - gridSpace;
            for (int i=initY; i > minY; i -= gridSpace)
                 painter->drawLine(minX, i, maxX, i);

            initY = midY + gridSpace;
            for (int i=initY; i < maxY; i += gridSpace)
                 painter->drawLine(minX, i, maxX, i);

            painter->setPen(gridAxesPen);
            painter->drawLine(midX, minY, midX, maxY);
            painter->drawLine(minX, midY, maxX, midY);
        }

        if (safeArea) {
            painter->setPen(whitePen);
            int outerBorder = width/19;
            int innerBorder = width/6;

            int hSpace = width/3;
            int vSpace = height/3;
            int hSpace2 = hSpace*2;
            int vSpace2 = vSpace*2;

            QPoint rectLeft(minX, minY);
            QPoint rectRight(maxX, maxY);

            QPointF left = rectLeft + QPointF(outerBorder, outerBorder);
            QPointF right = rectRight - QPointF(outerBorder, outerBorder);
            QRectF outerRect(left, right);

            painter->setPen(grayPen);
            painter->drawRect(outerRect);

            int leftY = left.y();
            int leftX = left.x();
            int rightY = right.y();
            int rightX = right.x();

            painter->setPen(greenThickPen);
            painter->drawLine(QPoint(hSpace, leftY - 8), QPoint(hSpace, leftY + 8));
            painter->drawLine(QPoint(hSpace - 5, leftY), QPoint(hSpace + 5, leftY));
            painter->drawLine(QPoint(hSpace2, leftY - 8), QPoint(hSpace2, leftY + 8));
            painter->drawLine(QPoint(hSpace2 - 5, leftY), QPoint(hSpace2 + 5, leftY));

            painter->drawLine(QPoint(hSpace, rightY - 8), QPoint(hSpace, rightY + 8));
            painter->drawLine(QPoint(hSpace - 5, rightY), QPoint(hSpace + 5, rightY));
            painter->drawLine(QPoint(hSpace2, rightY - 8), QPoint(hSpace2, rightY + 8));
            painter->drawLine(QPoint(hSpace2 - 5, rightY), QPoint(hSpace2 + 5, rightY));

            painter->drawLine(QPoint(leftX - 8, vSpace), QPoint(leftX + 8, vSpace));
            painter->drawLine(QPoint(leftX, vSpace - 5), QPoint(leftX, vSpace + 5));
            painter->drawLine(QPoint(leftX - 8, vSpace2), QPoint(leftX + 8, vSpace2));
            painter->drawLine(QPoint(leftX, vSpace2 - 5), QPoint(leftX, vSpace2 + 5));

            painter->drawLine(QPoint(rightX - 8, vSpace), QPoint(rightX + 8, vSpace));
            painter->drawLine(QPoint(rightX, vSpace - 5), QPoint(rightX, vSpace + 5));
            painter->drawLine(QPoint(rightX - 8, vSpace2), QPoint(rightX + 8, vSpace2));
            painter->drawLine(QPoint(rightX, vSpace2 - 5), QPoint(rightX, vSpace2 + 5));

            painter->setPen(greenThinPen);

            left = rectLeft + QPointF(innerBorder, innerBorder);
            right = rectRight - QPointF(innerBorder, innerBorder);
            QRectF innerRect(left, right);

            painter->drawRect(innerRect);
        }

        frame.unmap();
    }
}

QList<QVideoFrame::PixelFormat> TupVideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    if (handleType == QAbstractVideoBuffer::NoHandle) {
        return QList<QVideoFrame::PixelFormat>()
                << QVideoFrame::Format_RGB32
                << QVideoFrame::Format_ARGB32
                << QVideoFrame::Format_ARGB32_Premultiplied
                << QVideoFrame::Format_RGB565
                << QVideoFrame::Format_RGB555;
    } else {
        return QList<QVideoFrame::PixelFormat>();
    }
}

void TupVideoSurface::drawGrid(bool flag)
{
    grid = flag;
    videoIF->updateVideo();
}

void TupVideoSurface::drawActionSafeArea(bool flag)
{
    safeArea = flag;
    videoIF->updateVideo();
}

void TupVideoSurface::setLastImage(const QImage &image)
{
    if (isScaled) {
        int height = image.height();
        int width = (displaySize.width() * height) / displaySize.height();
        int posX = (image.width() - width)/2;
        int posY = 0;
        if (width > image.width()) {
            width = image.width();
            height = (displaySize.height() * width) / displaySize.width();
            posX = 0;
            posY = (image.height() - height)/2;
        }
        QImage mask = image.copy(posX, posY, width, height);
        QImage pic = mask.scaledToWidth(displaySize.width(), Qt::SmoothTransformation);
        history << pic;
    } else {
        history << image;
    }

    if (history.count() > 5)
        history.removeFirst();

    calculateImageDepth();
}

void TupVideoSurface::showHistory(bool flag)
{
    showPrevious = flag;
    videoIF->updateVideo();
}

void TupVideoSurface::updateImagesOpacity(double factor)
{
    opacity = static_cast<int> (255 * factor);
    videoIF->updateVideo();
}

void TupVideoSurface::updateImagesDepth(int depth)
{
    historySize = depth;
    calculateImageDepth();
    videoIF->updateVideo();
}

void TupVideoSurface::updateGridSpacing(int space)
{
    gridSpace = space;
    videoIF->updateVideo();
}

void TupVideoSurface::calculateImageDepth()
{
    int times = historySize;
    int limit = history.count();
    if (times > limit)
        times = limit;
    historyInit = limit - times;
    historyEnd = limit - 1;
}

void TupVideoSurface::updateGridColor(const QColor color)
{
    QColor gridColor = color;
    gridColor.setAlpha(50);
    gridPen = QPen(gridColor);
    videoIF->updateVideo();
}

void TupVideoSurface::flipSurface()
{
    if (rotation == 0)
        rotation = 180;
    else
        rotation = 0;
}
