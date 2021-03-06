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

#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include "tglobal.h"
#include "papagayosettings.h"
#include "lipsyncmanager.h"

#include <QFrame>
#include <QLabel>
#include <QBoxLayout>
#include <QListWidgetItem>

/**
 * @author Gustav Gonzalez 
*/

class TUPITUBE_PLUGIN Configurator : public QFrame
{
    Q_OBJECT

    public:
        enum GuiState { Manager = 1, Properties };

        Configurator(QWidget *parent = nullptr);
        ~Configurator();

        void loadLipSyncList(QList<QString> list);
        void openLipSyncProperties(TupLipSync *lipsync);

        void addLipSyncRecord(const QString &name);
        void updateInterfaceRecords();

        void resetUI();
        void closePanels();

        void setPhoneme(const QString &phoneme);
        void setPos(const QPointF &point);

    private slots:
        void editCurrentLipSync(const QString &name);
        void closeSettingsPanel();
        void openMouthsDialog();

    signals:
        void importLipSync();
        void selectMouth(const QString &name, int index);
        void closeLipSyncProperties();
        void editLipSyncSelection(const QString &lipSyncName);
        void initFrameHasChanged(int index);
        void removeCurrentLipSync(const QString &name);
        void xPosChanged(int x);
        void yPosChanged(int y);
        
    private:
        void setPropertiesPanel();
        void activePropertiesPanel(bool enable);
        void setLipSyncManagerPanel();
        void activeLipSyncManagerPanel(bool enable);

        QBoxLayout *settingsLayout;
        PapagayoSettings *settingsPanel;
        LipSyncManager *manager;
};

#endif
