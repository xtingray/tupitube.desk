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

#ifndef TUPBASICCAMERAINTERFACE_H
#define TUPBASICCAMERAINTERFACE_H

#include "tglobal.h"
#include "tupapplication.h"
#include "tapplicationproperties.h"

#include <QFrame>
#include <QComboBox>
#include <QCloseEvent>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>

class TUPITUBE_EXPORT TupBasicCameraInterface : public QFrame
{
    Q_OBJECT

    public:
        TupBasicCameraInterface(const QString &title, QList<QByteArray> cameraDevices,
                                QComboBox *devicesCombo, int cameraIndex, const QSize cameraSize = QSize(),
                                int counter = 1, QWidget *parent = 0);
        ~TupBasicCameraInterface();

    protected:
        void closeEvent(QCloseEvent *event);

    signals:
        void projectSizeHasChanged(const QSize size);
        void pictureHasBeenSelected(int id, const QString folder);
        void closed();

    private slots:
        void changeCameraDevice(int index);
        void takePicture();
        void imageSavedFromCamera(int id, const QString folder);

    private:
        QString randomPath();

        QStackedWidget *widgetStack;
        QList <QCamera *> cameras;
        QCamera *currentCamera;
        QList <QCameraImageCapture *> imageCaptors;
        QCameraImageCapture *currentImageCaptor;

        QString path;
        int counter;
};

#endif
