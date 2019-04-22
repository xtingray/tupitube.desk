/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustav Gonzalez / xtingray                                           *
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

#ifndef SETTINGS_H
#define SETTINGS_H

#include "tglobal.h"
#include "tuptoolplugin.h"
#include "tupitemtweener.h"
#include "tradiobuttongroup.h"
#include "tuptweenerstep.h"
#include "timagebutton.h"

#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>

/**
 * @author Gustav Gonzalez 
*/

class TUPITUBE_PLUGIN Settings : public QWidget 
{
    Q_OBJECT

    public:
        Settings(QWidget *parent = nullptr);
        ~Settings();

        void setParameters(const QString &name, int framesCount, int startFrame);
        void setParameters(TupItemTweener *currentTween);
        void initStartCombo(int totalFrames, int currentIndex);
        void setStartFrame(int currentIndex);

        int totalSteps();

        void notifySelection(bool flag);
        int startFrame();
        int startComboSize();
        QString currentTweenName() const;
        void activateMode(TupToolPlugin::EditMode mode);
        QString tweenToXml(int currentScene, int currentLayer, int currentFrame, QPointF point);

    private slots:
        void applyTween();
        void emitOptionChanged(int option);
        void updateLoopCheckbox(int state);
        void updateReverseCheckbox(int state);
        void updateRangeFromInit(int begin);
        void updateRangeFromEnd(int end);

    signals:
        void clickedSelect();
        void clickedDefineProperties();
        void clickedApplyTween();
        void clickedResetTween();
        void initFrameChanged(int index);
        
    private:
        void setInnerForm();
        void activeInnerForm(bool enable);
        void setEditMode();
        void checkFramesRange();

        QWidget *innerPanel;
        QWidget *rangePanel;
        QWidget *clockPanel;

        QBoxLayout *layout;
        TupToolPlugin::Mode mode;
        QLineEdit *input;
        TRadioButtonGroup *options;

        QSpinBox *initFrameSpin;
        QSpinBox *endFrameSpin;

        QLabel *totalLabel;
        int stepsCounter;

        TupItemTweener::TransformAxes shearAxes;
        QComboBox *comboAxes;

        QDoubleSpinBox *comboFactor;
        QSpinBox *iterationsCombo;

        QCheckBox *loopBox;
        QCheckBox *reverseLoopBox;

        bool selectionDone;
        bool propertiesDone;

        TImageButton *apply;
        TImageButton *remove;
};

#endif
