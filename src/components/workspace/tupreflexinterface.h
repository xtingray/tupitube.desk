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

#ifndef TUPREFLEXINTERFACE_H
#define TUPREFLEXINTERFACE_H

#include "tglobal.h"

#include <QDialog>
#include <QCloseEvent>
#include <QCamera>

class TUPITUBE_EXPORT TupReflexInterface : public QDialog
{
    Q_OBJECT

    public:
        TupReflexInterface(const QString &cameraDesc, const QString &resolution, QByteArray cameraDevice, 
                           const QSize cameraSize = QSize(), int counter = 1, QWidget *parent = 0);
        ~TupReflexInterface();

    protected:
        void closeEvent(QCloseEvent *event);

    signals:
        void pictureHasBeenSelected(int id, const QString path);
        void closed();

    private slots:
        void takePicture();
        void reset();
        void imageSavedFromCamera(int id, const QString path);
        void error(QCamera::Error error);

    private:
        void randomPath();

        struct Private;
        Private *const k;
};

#endif
