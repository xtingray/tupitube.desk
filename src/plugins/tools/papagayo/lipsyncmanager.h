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

#ifndef LIPSYNCMANAGER_H
#define LIPSYNCMANAGER_H

#include "tglobal.h"
#include "timagebutton.h"
#include "tosd.h"

#include <QWidget>
#include <QListWidget>

/**
 * @author Gustav Gonzalez 
*/

class TUPITUBE_PLUGIN LipSyncManager: public QWidget 
{
    Q_OBJECT

    public:
        LipSyncManager(QWidget *parent = nullptr);
        ~LipSyncManager();

        void loadLipSyncList(QList<QString> list);
        void resetUI();
        QString currentLipSyncName() const;
        int listSize(); 
        void addNewRecord(const QString &name);
        void removeRecordFromList(const QString &name);

    signals:
        void lipsyncCreatorRequested();
        void lipsyncEditionRequested(const QString &name);
        void mouthEditionRequested(const QString &name);
        void currentLipSyncRemoved(const QString &name);

    private slots:
        void editLipSync();
        void editMouth();
        void removeLipSync();

    private:
        bool itemExists(const QString &name);

        QListWidget *lipSyncList;
        TImageButton *openButton;
        TImageButton *editPgoButton;
        TImageButton *editMouthButton;
        TImageButton *deleteButton;
        QString target;
};

#endif
