/***************************************************************************
 *   Project TUPITUBE DESK                                                *
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

#ifndef TUPVIDEOSURFACE_H
#define TUPVIDEOSURFACE_H

#include "tglobal.h"

#include <QtGui>
#include <QWidget>
#include <QAbstractVideoSurface>
#include <QVideoRendererControl>
#include <QVideoSurfaceFormat>

class TUPITUBE_EXPORT VideoIF {
    public:
        virtual void updateVideo() = 0;
};

class TUPITUBE_EXPORT TupVideoSurface: public QAbstractVideoSurface
{
    Q_OBJECT

    public:
        TupVideoSurface(QWidget *widget, VideoIF *target, const QSize &displaySize, bool isScaled, int orientation, QObject *parent = 0);
        ~TupVideoSurface();

        bool start(const QVideoSurfaceFormat &format);
        bool present(const QVideoFrame &frame);
        QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;
        void paint(QPainter *painter);
        void drawGrid(bool flag);
        void drawActionSafeArea(bool flag);
        void setLastImage(const QImage &image);
        void showHistory(bool flag);
        void updateImagesOpacity(double transparency);
        void updateImagesDepth(int depth);
        void updateGridSpacing(int space);
        void updateGridColor(const QColor color);
        void flipSurface();

    private:
        void calculateImageDepth();

        QWidget* targetWidget;
        VideoIF* videoIF;
        QVideoFrame frame;
        QImage::Format imageFormat;
        QSize displaySize;
        QList<QImage> history;
        int widgetWidth;
        int widgetHeight;

        bool isScaled;
        bool showPrevious;
        bool safeArea;
        bool grid;
        int opacity;
        int historySize;
        int gridSpace;
        int historyInit;
        int historyEnd;
        qreal rotation;

        QPen gridAxesPen;
        QPen gridPen;
        QPen whitePen;
        QPen grayPen;
        QPen greenThickPen;
        QPen greenThinPen;
};

#endif // TUPVIDEOSURFACE_H
