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

#ifndef COLORSETTINGS_H
#define COLORSETTINGS_H

#include "tglobal.h"
#include "tuptoolplugin.h"
#include "tupitemtweener.h"
#include "timagebutton.h"
#include "tradiobuttongroup.h"

#include <QComboBox>
#include <QSpinBox>
#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

/**
 * @author Gustav Gonzalez 
*/

class TUPITUBE_PLUGIN ColorSettings : public QWidget
{
    Q_OBJECT

    public:
        ColorSettings(QWidget *parent = nullptr);
        ~ColorSettings();

        void setParameters(const QString &name, int framesCount, int startFrame);
        void setParameters(TupItemTweener *currentTween);
        void initStartCombo(int totalFrames, int currentIndex);
        void setStartFrame(int currentIndex);
        int startFrame();

        int totalSteps();

        QString currentTweenName() const;
        void activatePropertiesMode(TupToolPlugin::EditMode mode);
        void notifySelection(bool flag);
        int startComboSize();
        void setInitialColor(QColor color);
		void activateMode(TupToolPlugin::EditMode mode);
        QString tweenToXml(int currentScene, int currentLayer, int currentFrame);

    private slots:
        void applyTween();
        void emitOptionChanged(int option);
        void updateLoopCheckbox(int state);
        void updateReverseCheckbox(int state);
        void setInitialColor();
        void setEndingColor();
        void updateRangeFromInit(int begin);
        void updateRangeFromEnd(int end);

    signals:
        void clickedSelect();
        void clickedDefineProperties();
        void clickedApplyTween();
        void clickedResetTween();
        void startingPointChanged(int index);
        
    private:
        void setInnerForm();
        void activeInnerForm(bool enable);
        void setEditMode();
        void checkFramesRange();
        void updateColor(QColor color, QPushButton *colorButton);
        QString labelColor(QColor color) const;

        QWidget *innerPanel;
        QBoxLayout *layout;
        TupToolPlugin::Mode mode;

        QLineEdit *input;

        QSpinBox *initFrame;
        QSpinBox *endFrame;

        TRadioButtonGroup *options;

        QComboBox *fillTypeCombo;
        QPushButton *initColorButton;
        QColor initialColor;
        QPushButton *endColorButton;
        QColor endingColor;

        QSpinBox *iterationsCombo;

        QCheckBox *loopBox;
        QCheckBox *reverseLoopBox;

        QLabel *totalLabel;
        int totalStepsCount;

        bool selectionDone;
        bool propertiesDone;

        TImageButton *apply;
        TImageButton *remove;

        TupItemTweener::FillType fillType;
        QMargins margins;
};

#endif
