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
#include "tconfig.h"

#include <QScreen>
#include <QGuiApplication>

TupVideoSurface::TupVideoSurface(QWidget *widget, VideoIF *target, const QSize &size, bool scaleFlag,
                                 int orientation, QObject *parent) : QAbstractVideoSurface(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupVideoSurface()]";
    #endif

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
    historyInit = 0;
    historyEnd = 0;

    loadGridParameters();
    loadSafeParameters();
    loadROTParameters();

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

void TupVideoSurface::loadGridParameters()
{
    TCONFIG->beginGroup("PaintArea");
    QString colorName = TCONFIG->value("GridColor", "#0000b4").toString();
    QColor gridColor(colorName);
    gridColor.setAlpha(50);

    int thickness = TCONFIG->value("GridLineThickness", "1").toInt();
    gridPen = QPen(gridColor, thickness);
    gridAxesPen = QPen(Qt::black, thickness);
    gridSeparation = TCONFIG->value("GridSeparation", "10").toInt();
}

void TupVideoSurface::loadSafeParameters()
{
    TCONFIG->beginGroup("PaintArea");
    QString rectColorName = TCONFIG->value("SafeAreaRectColor", "#008700").toString();
    QString lineColorName = TCONFIG->value("SafeAreaLineColor", "#969696").toString();
    gridLineThickness = TCONFIG->value("SafeLineThickness", 1).toInt();

    QColor safeRectColor = QColor(rectColorName);
    safeRectPen = QPen(safeRectColor, gridLineThickness);
    QColor safeLineColor = QColor(lineColorName);
    safeLinePen = QPen(safeLineColor, gridLineThickness);
}

void TupVideoSurface::loadROTParameters()
{
    TCONFIG->beginGroup("PaintArea");
    QString colorName = TCONFIG->value("ROTColor", "#000000").toString();
    int thickness = TCONFIG->value("ROTLineThickness", "1").toInt();

    rotColor = QColor(colorName);
    rotPen = QPen(rotColor, thickness);
    rotColor.setAlpha(20);
}

bool TupVideoSurface::start(const QVideoSurfaceFormat &format)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupVideoSurface::start()]";
    #endif

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
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupVideoSurface::present()]";
    #endif
    */

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
            /*
            painter->setPen(gridPen);
            int maxX = static_cast<int> (width + 100);
            int maxY = static_cast<int> (height + 100);
            for (int i = -100; i <= maxX; i += gridSeparation)
                 painter->drawLine(i, -100, i, maxY);
            for (int i = -100; i <= maxY; i += gridSeparation)
                 painter->drawLine(-100, i, maxX, i);
            */

            painter->setPen(gridPen);

            int initX = midX - gridSeparation;
            for (int i=initX; i > minX; i -= gridSeparation)
                 painter->drawLine(i, minY, i, maxY);

            initX = midX + gridSeparation;
            for (int i=initX; i < maxX; i += gridSeparation)
                 painter->drawLine(i, minY, i, maxY);

            int initY = midY - gridSeparation;
            for (int i=initY; i > minY; i -= gridSeparation)
                 painter->drawLine(minX, i, maxX, i);

            initY = midY + gridSeparation;
            for (int i=initY; i < maxY; i += gridSeparation)
                 painter->drawLine(minX, i, maxX, i);

            painter->setPen(gridAxesPen);
            painter->drawLine(midX, minY, midX, maxY);
            painter->drawLine(minX, midY, maxX, midY);
        }

        if (safeArea) {
            painter->setPen(safeRectPen);
            painter->setBrush(QBrush());
            QRectF drawingRect = QRectF(QPointF(0, 0), displaySize); // QSize(width, height));
            painter->drawRect(drawingRect);

            int w = static_cast<int> (width);
            int h = static_cast<int> (height);
            int outerBorder = w / 19;
            int innerBorder = w / 6;

            QPointF left = drawingRect.topLeft() + QPointF(outerBorder, outerBorder);
            QPointF right = drawingRect.bottomRight() - QPointF(outerBorder, outerBorder);

            QRectF outerRect(left, right);
            painter->drawRect(outerRect);

            left = drawingRect.topLeft() + QPointF(innerBorder, innerBorder);
            right = drawingRect.bottomRight() - QPointF(innerBorder, innerBorder);
            QRectF innerRect(left, right);
            painter->drawRect(innerRect);

            painter->setPen(safeLinePen);
            int middleX = w/2;
            int middleY = h/2;
            painter->drawLine(QPoint(0, middleY), QPoint(w, middleY));
            painter->drawLine(QPoint(middleX, 0), QPoint(middleX, h));
            int target = static_cast<int> (drawingRect.width() * (0.02));
            QRect rect(QPoint(middleX - target, middleY - target),
                       QPoint(middleX + target, middleY + target));
            painter->drawRect(rect);

            painter->setPen(rotPen);
            int horizontalSpace = static_cast<int> (width / 3);
            int verticalSpace = static_cast<int> (height / 3);

            painter->drawLine(0, verticalSpace, width, verticalSpace);
            painter->drawLine(0, verticalSpace*2, width, verticalSpace*2);
            painter->drawLine(horizontalSpace, 0, horizontalSpace, height);
            painter->drawLine(horizontalSpace*2, 0, horizontalSpace*2, height);

            /*
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
            */
        }

        frame.unmap();
    }
}

QList<QVideoFrame::PixelFormat> TupVideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupVideoSurface::supportedPixelFormats()]";
    #endif

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
    #ifdef TUP_DEBUG
        qDebug() << "[TupVideoSurface::setLastImage()]";
    #endif

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
    gridSeparation = space;
    videoIF->updateVideo();
}

void TupVideoSurface::updateGridLineThickness(int thickness)
{
    gridLineThickness = thickness;
    gridPen.setWidth(thickness);
    gridAxesPen.setWidth(thickness);
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
    gridPen = QPen(gridColor, gridLineThickness);
    videoIF->updateVideo();
}

void TupVideoSurface::flipSurface()
{
    if (rotation == 0)
        rotation = 180;
    else
        rotation = 0;
}
