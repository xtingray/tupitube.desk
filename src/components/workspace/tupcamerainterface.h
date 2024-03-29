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

#ifndef TUPCAMERAINTERFACE_H
#define TUPCAMERAINTERFACE_H

#include "tglobal.h"
#include "tupcamerawindow.h"
#include "tupcolorwidget.h"

#include <QFrame>
#include <QCloseEvent>
#include <QComboBox>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QCameraInfo>

class TUPITUBE_EXPORT TupCameraInterface : public QFrame
{
    Q_OBJECT

    public:
        TupCameraInterface(const QString &title, QList<QCameraInfo> cameraDevices, QComboBox *devicesCombo, int cameraIndex, 
                           const QSize cameraSize = QSize(), int counter = 1, QWidget *parent = nullptr);
        ~TupCameraInterface();

    protected:
        void closeEvent(QCloseEvent *event);

    signals:
        void pictureHasBeenSelected(int id, const QString path);
        void closed();

    private slots:
        void changeCameraDevice(int index);
        void takePicture();
        void drawGrid();
        void drawActionSafeArea();
        void showHistory();
        void updateImagesOpacity(double opacity);
        void updateImagesDepth(int depth);
        void updateGridSpacing(int space);
        void updateGridThickness(int thickness);
        void updateColour();
        void flipCamera();

    private:
        QSize setBestResolution(QList<QSize> resolutions, QSize cameraSize);
        QString randomPath();

        QStackedWidget *widgetStack;
        TupCameraWindow *currentCamera;
        QPushButton *gridButton;
        QPushButton *safeAreaButton;
        QPushButton *historyButton;
        QWidget *gridWidget;
        QWidget *historyWidget;
        int counter;
        QColor gridColor;
        TupColorWidget *colorCell;
        QLabel *counterLabel;
};

#endif
