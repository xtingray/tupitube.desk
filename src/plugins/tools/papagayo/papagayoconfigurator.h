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

#ifndef PAPAGAYOCONFIGURATOR_H
#define PAPAGAYOCONFIGURATOR_H

#include "tglobal.h"
#include "papagayosettings.h"
#include "lipsyncmanager.h"
#include "tuplipsync.h"

#include <QFrame>
#include <QBoxLayout>

/**
 * @author Gustav Gonzalez 
*/

class TUPITUBE_PLUGIN PapagayoConfigurator : public QFrame
{
    Q_OBJECT

    public:
        enum GuiState { Manager = 1, Properties };

        PapagayoConfigurator(QWidget *parent = nullptr);
        ~PapagayoConfigurator();

        void loadLipSyncList(QList<QString> list);
        void openLipSyncProperties(TupLipSync *lipsync);

        void addLipSyncRecord(const QString &name);
        void removeLipSyncRecord(const QString &name);
        void updateInterfaceRecords();

        void resetUI();
        void closePanels();

        void setPhoneme(const TupPhoneme *phoneme);
        void setTransformations(const QDomElement &dom);
        void setTransformations(const TupTransformation::Parameters parameters);

        void updatePositionCoords(int x, int y);
        void updateRotationAngle(int angle);
        void updateScaleFactor(double x, double y);

        void setProportionState(bool flag);

    signals:
        void lipsyncCreatorRequested();
        void lipsyncEditionRequested(const QString &lipSyncName);
        void mouthEditionRequested(const QString &lipSyncName);
        void selectMouth(const QString &name, int index);
        void closeLipSyncProperties();
        void initFrameHasChanged(int index);
        void currentLipsyncRemoved(const QString &name);

        void xPosChanged(int x);
        void yPosChanged(int y);
        void rotationChanged(int angle);
        void scaleChanged(double xFactor, double yFactor);

        void objectHasBeenReset();
        void proportionActivated(bool flag);

    private slots:
        void editCurrentLipSync(const QString &name);
        void closeSettingsPanel();
        void openMouthsDialog();

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
