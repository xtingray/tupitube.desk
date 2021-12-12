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

#ifndef PAPAGAYOSETTINGS_H
#define PAPAGAYOSETTINGS_H

#include "tglobal.h"
#include "tuptoolplugin.h"
#include "tuplipsync.h"

#include <QWidget>
#include <QLabel>
#include <QBoxLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QTextEdit>

/**
 * @author Gustav Gonzalez 
*/

class TUPITUBE_PLUGIN PapagayoSettings : public QWidget 
{
    Q_OBJECT

    public:
        PapagayoSettings(QWidget *parent = nullptr);
        ~PapagayoSettings();

        void openLipSyncProperties(TupLipSync *lipsync);
        void updateInterfaceRecords();

        void setPhoneme(const TupPhoneme *phoneme);
        void setTransformations(const TupTransformation::Parameters parameters);
        void setTransformations(const QDomElement &dom);

        void updatePositionCoords(int x, int y);
        void updateRotationAngle(int angle);
        void updateScaleFactor(double x, double y);

        void setProportionState(int flag);

    signals:
        void initFrameHasChanged(int index);
        void selectMouth(const QString &id, int index);
        void closeLipSyncProperties();

        void xPosChanged(int x);
        void yPosChanged(int y);
        void rotationChanged(int angle);
        void scaleChanged(double xFactor, double yFactor);

        void objectHasBeenReset();
        void proportionActivated(bool flag);

    private slots:
        void updateInitFrame(int index);

        void notifyRotation(int angle);
        void notifyXScale(double factor);
        void notifyYScale(double factor);
        void enableProportion(int flag);
        void resetTransformations();

    private:
        void setInnerForm();

        QWidget *innerPanel;
        QBoxLayout *layout;

        QLabel *lipSyncName;
        QSpinBox *comboInit;

        QLabel *endingLabel;
        QLabel *totalLabel;

        QLabel *phonemeLabel;
        QSpinBox *xPosField;
        QSpinBox *yPosField;

        QSpinBox *angleField;
        QDoubleSpinBox *factorXField;
        QDoubleSpinBox *factorYField;
        QCheckBox *propCheck;

        QString name;
        int initFrame;
        int framesCount;

        int currentX;
        int currentY;
        double currentXFactor;
        double currentYFactor;

        const TupPhoneme *phoneme;
};

#endif
