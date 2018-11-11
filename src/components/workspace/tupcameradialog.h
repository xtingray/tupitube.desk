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

#ifndef TUPCAMERADIALOG_H
#define TUPCAMERADIALOG_H

#include "tglobal.h"
#include "tapplicationproperties.h"
#include "tconfig.h"

#include <QDialog>
#include <QCamera>
#include <QComboBox>
#include <QCheckBox>

class TUPITUBE_EXPORT TupCameraDialog : public QDialog
{
    Q_OBJECT

    public:
        TupCameraDialog(QComboBox *devices, const QSize dimension, QList<QSize> resList, QWidget *parent=0);
        ~TupCameraDialog();

        int cameraIndex();
        QSize cameraResolution() const;
        bool changeProjectSize();
        bool useBasicCamera();
        bool isWebcam();

    private slots:
        void changeCameraDevice(const QString &camera);
        void setCameraResolution(int index);
        void projectSizeHasChanged(bool flag);
        void enableBasicCamera(bool flag);

    private:
        void setCamera(const QString &reference);
        void updateCameraType();

        QSize projectSize;
        QSize cameraSize;
        bool resizeProject;

        QCamera *camera;
        QComboBox *devicesCombo;
        QString cameraReference;

        QComboBox *resolutionCombo;
        QList<QSize> resolutions;
        int deviceIndex;
        bool useBasicInterface;
        QCheckBox *lowCheck;
        bool webcamFlag;
};

#endif
