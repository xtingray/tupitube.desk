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

#ifndef TUPCAMERAWINDOW_H
#define TUPCAMERAWINDOW_H

#include "tglobal.h"
#include "tupvideosurface.h"

#include <QtGui>
#include <QWidget>
#include <QCamera>
#include <QCameraImageCapture>

class TUPITUBE_EXPORT TupCameraWindow: public QWidget, public VideoIF
{
    Q_OBJECT

    public:
        TupCameraWindow(QCamera *input, const QSize &camSize, const QSize &displaySize, 
                        QCameraImageCapture *imageCapture, const QString &path, QWidget *parent = nullptr);
        ~TupCameraWindow();

        void startCamera();
        void stopCamera();

        void paintEvent(QPaintEvent *event);
        void updateVideo();
        void reset();
        void drawGrid(bool flag);
        void drawActionSafeArea(bool flag);
        void showHistory(bool flag);
        void updateImagesOpacity(double opacity);
        void updateImagesDepth(int depth);
        void updateGridSpacing(int space);
        void updateGridLineThickness(int thickness);
        void updateGridColor(const QColor color);
        void flipCamera();

    signals:
        void pictureHasBeenSelected(int id, const QString path);

    public slots:
        void takePicture(int counter);
   
    private slots: 
        void imageSavedFromCamera(int id, const QString path);
        void error(QCamera::Error error);

    private:
        QCamera *camera;
        QCameraImageCapture *imageCapture;
        TupVideoSurface *videoSurface;
        QString picturesPath;
        int counter;
};

#endif // TUPCAMERAWINDOW_H 
