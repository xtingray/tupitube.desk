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

#ifndef TMAINWINDOW_H
#define TMAINWINDOW_H

#include "tglobal.h"
#include "tbuttonbar.h"
#include "toolview.h"
#include "tviewbutton.h"
#include "tmainwindowabstractsettings.h"
#include "taction.h"

#include <QMainWindow>
#include <QHash>
#include <QMap>
#include <QKeySequence>
#include <QTimer>
#include <QMenu>
#include <QApplication>
#include <QDesktopWidget>
#include <QSettings>
#include <QHashIterator>
#include <QMenuBar>
#include <QCloseEvent>

class TButtonBar;
class ToolView;
class TMainWindowAbstractSettings;

class T_GUI_EXPORT TMainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        enum
         {
           None = 0, DefaultPerspective
         };

        TMainWindow(QWidget *parent = nullptr);
        ~TMainWindow();

        ToolView *addToolView(QWidget *widget, Qt::DockWidgetArea area, int perspective = DefaultPerspective,
                              const QString &code = QString(), QKeySequence shortcut = QKeySequence(""));

        void removeToolView(ToolView *view);

        void addToPerspective(QWidget *widget, int perspective = DefaultPerspective);
        void removeFromPerspective(QWidget *widget);
        void setCurrentPerspective(int wsp);
        int currentPerspective() const;

        void addToPerspective(QAction *action, int perspective);
        void addToPerspective(const QList<QAction *> &actions, int perspective);
        void removeFromPerspective(QAction *action);

        void enableToolViews(bool flag);

        void setAutoRestore(bool autoRestore);
        bool autoRestore() const;

        void setSettingsHandler(TMainWindowAbstractSettings *config);
        void restoreGUI();
        void saveGUI();

        QHash<Qt::ToolBarArea, TButtonBar *> buttonBars() const;
        QHash<TButtonBar *, QList<ToolView*> > toolViews() const;

        void enableSpecialBar(bool flag);
        void addSpecialButton(TAction *action);

    private:
        Qt::DockWidgetArea toDockWidgetArea(Qt::ToolBarArea area);
        Qt::ToolBarArea toToolBarArea(Qt::DockWidgetArea area);

    signals:
        void perspectiveChanged(int wps);

    protected:
        void addButtonBar(Qt::ToolBarArea area);

    protected:
        virtual void closeEvent(QCloseEvent *event);
        virtual void showEvent(QShowEvent *event);

    private:
        ToolView *m_forRelayout;

    private:
        QHash<Qt::ToolBarArea, TButtonBar *> m_buttonBars;
        QHash<TButtonBar *, QList<ToolView*> > m_toolViews;
        QHash<QWidget *, int> m_managedWidgets;
        QHash<QAction *, int> m_managedActions;
        QToolBar *specialToolBar;

        int perspective;

        TMainWindowAbstractSettings *settings;
        bool m_autoRestore;
};

#endif
