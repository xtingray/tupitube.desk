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
#include "colorsettings.h"
#include "tweenmanager.h"
#include "buttonspanel.h"
#include "tupitemtweener.h"
#include "tosd.h"

#include <QFrame>
#include <QLabel>

class QGraphicsPathItem;
class QListWidgetItem;
// class TupItemTweener;

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

        void loadTweenList(QList<QString> tweenList);

        void initStartCombo(int framesCount, int currentFrame);
        void setStartFrame(int currentIndex);
        int startFrame();

        int totalSteps();
        void activateMode(TupToolPlugin::EditMode mode);
        void activeButtonsPanel(bool enable);
        void setCurrentTween(TupItemTweener *currentTween);
        QString currentTweenName() const;
        QString getTweenNameFromList() const;
        void notifySelection(bool flag);
        int startComboSize();
        void setInitialColor(QColor color);

        void closeSettingsPanel();
        TupToolPlugin::Mode mode();
        void resetUI();
        QString tweenToXml(int currentScene, int currentLayer, int currentFrame);
        
    private slots:
        void applyItem();
        void addTween(const QString &name);
        void editTween();
        void removeTween();
        void removeTween(const QString &name);
        void closeTweenProperties();
        void updateTweenData(const QString &name);
        
    signals:
        void startingPointChanged(int index);
        void clickedSelect();
        void clickedDefineProperties();
        void clickedRemoveTween(const QString &name);
        void setMode(TupToolPlugin::Mode mode);
        void clickedApplyTween();
        void clickedResetInterface();
        void getTweenData(const QString &name);
        
    private:
        void setPropertiesPanel();
        void activePropertiesPanel(bool enable);
        void setTweenManagerPanel();
        void activeTweenManagerPanel(bool enable);
        void setButtonsPanel();

        QBoxLayout *layout;
        QBoxLayout *settingsLayout;
        ColorSettings *settingsPanel;
        TweenManager *tweenManager;
        ButtonsPanel *controlPanel;

        TupItemTweener *currentTween;

        int framesCount;
        int currentFrame;

        TupToolPlugin::Mode toolMode;
        Configurator::GuiState state;
};

#endif
