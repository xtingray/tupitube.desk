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
#include "tuptoolplugin.h"
#include "motionsettings.h"
#include "tweenmanager.h"
#include "buttonspanel.h"
#include "tupitemtweener.h"
#include "timagebutton.h"

#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QComboBox>
#include <QBoxLayout>
#include <QHeaderView>
#include <QGraphicsPathItem>
#include <QMenu>
#include <QAction>

// class TupItemTweener;

class TUPITUBE_PLUGIN Configurator : public QFrame
{
    Q_OBJECT

    public:
        enum GuiState { Manager = 1, Properties };

        Configurator(QWidget *parent = nullptr);
        ~Configurator();

        void loadTweenList(QList<QString> tweenList);

        void setPropertiesPanel();
        void activePropertiesPanel(bool enable);

        void setTweenManagerPanel();
        void activeTweenManagerPanel(bool enable);

        void setButtonsPanel();
        void activeButtonsPanel(bool enable);

        void initStartCombo(int framesCount, int currentFrame);
        void setStartFrame(int currentIndex);
        int startFrame();

        void updateSteps(const QGraphicsPathItem *path);
        QString tweenToXml(int currentScene, int currentLayer, int currentFrame, QPointF point, QString &path);
        int totalSteps();
        QList<QPointF> tweenPoints();
        void activateMode(TupToolPlugin::EditMode mode);
        void clearData();
        QString currentTweenName() const;
        QString getTweenNameFromList() const;
        void notifySelection(bool flag);
        int startComboSize();
        void closeSettingsPanel();
        TupToolPlugin::Mode mode();
        void resetUI();
        void setCurrentTween(TupItemTweener *currentTween);

        void undoSegment(const QPainterPath path);
        void redoSegment(const QPainterPath path);
        int stepsTotal();
        void updateSegments(const QPainterPath path);

        void enableSaveOption(bool flag);
        int getPathThickness();
        QColor getPathColor() const;

    public slots:
        void editTween();
        void closeTweenProperties();
        
    private slots:
        void applyItem();
        void addTween(const QString &name);
        void removeTween();
        void removeTween(const QString &name);
        void updateTweenData(const QString &name);
        
    signals:
        void clickedCreatePath();
        void clickedSelect();
        void clickedRemoveTween(QString);
        void setMode(TupToolPlugin::Mode mode);
        void clickedResetInterface();
        void clickedApplyTween();
        void startingFrameChanged(int);
        void tweenDataRequested(QString);
        void framesTotalChanged();

        void pathThicknessChanged(int);
        void pathColorUpdated(QColor);
        
    private:
        QBoxLayout *layout;
        QBoxLayout *settingsLayout;
        MotionSettings *settingsPanel;
        TweenManager *tweenManager;
        ButtonsPanel *controlPanel;

        bool selectionDone;

        TImageButton *removeButton;
        TImageButton *editButton;

        TupToolPlugin::Mode currentMode;
        GuiState state;

        int framesCount;
        int currentFrame;

        TupItemTweener *currentTween;
};

#endif
