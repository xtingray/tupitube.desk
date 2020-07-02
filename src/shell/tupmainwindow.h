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

#ifndef TUPMAINWINDOW_H
#define TUPMAINWINDOW_H

#include "tactionmanager.h"
#include "tupdocumentview.h"
#include "tupanimationspace.h"
#include "tuppreferencesdialog.h"

// Modules
#include "tupexposuresheet.h"
#include "tupbrushwidget.h"
#include "tupcolorpalettewidget.h"
#include "tupsceneswidget.h"
#include "tuplibrarywidget.h"
#include "tuptimeline.h"
#include "tupcamerawidget.h"
#include "tuptwitterwidget.h"
#include "tupexportwidget.h"

#include "tabbedmainwindow.h"
#include "tupstatusbar.h"
#include "tosd.h"
#include "toolview.h"

// Projects management 
#include "tupprojectmanager.h"
#include "tupnetprojectmanagerhandler.h"

#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QUndoStack>
#include <QKeySequence>
#include <QTextBrowser>
#include <QToolBar>
#include <QScreen>
#include <QDragEnterEvent>
#include <QDropEvent>

class TupProjectManagerParams;
class TupNetProjectManagerParams;
class TupProjectResponse;

class TupMainWindow : public TabbedMainWindow
{
    Q_OBJECT

    public:

        enum Perspective {
             Animation = 0x01,
             Player = 0x02,
             News = 0x04,
             All = Animation | Player | News
        };

        enum RequestType {
             None = 0,
             NewLocalProject,
             OpenLocalProject,
             NewNetProject,
             OpenNetProject,
             ImportProjectToNet
        };

        TupMainWindow(const QString &winKey);
        ~TupMainWindow();

    private:
        void createGUI();
        void setupFileActions();
        void setPreferencesAction();
        void setupToolBar();
        void setupMenu();

        void setupHelpActions();
        void setMenuItemsContext(bool flag);

        void connectWidgetToManager(QWidget *widget);
        void disconnectWidgetToManager(QWidget *widget);
        void connectWidgetToPaintArea(QWidget *widget);
        void connectWidgetToLocalManager(QWidget *widget);

        void setupNetworkProject();
        void setupNetworkProject(TupProjectManagerParams *params);
        void setupLocalProject(TupProjectManagerParams *params);
        void setUndoRedoActions();
        void resetUI();
        void updateRecentProjectList();
        void saveDefaultPath(const QString &dir);
        bool cancelChanges();

    protected:
        #if defined(Q_OS_MAC)
            bool event(QEvent *event);
        #endif
        void closeEvent(QCloseEvent *event);
        void updateOpenRecentMenu(QMenu *menu, QStringList recents);
        void dragEnterEvent(QDragEnterEvent *event);
        void dropEvent(QDropEvent *event);

    public slots:
        void openProject(const QString &path);
        void openExample();
        void updatePenColor(const QColor &color);
        void updatePenThickness(int thickness);
        void exportProject();
        void postProject();

    private slots:
        void addTwitterPage();
        void setWorkSpace(const QStringList &users = QStringList());
        void createNewLocalProject();
        void newProject();
        void closeInterface();
        bool closeProject();
        void unexpectedClose();
        void openProject();
        void openProjectFromServer();
        void importProjectToServer();
        void saveAs();
        void saveProject();

        void showAnimationMenu(const QPoint &p);

        void changePerspective(QAction *a);
        void changePerspective(int index);

        void addPage(QWidget *widget);
        void updateCurrentTab(int index);

        void requestProject();
        void createNewNetProject(const QString &title, const QStringList &users);
        void netProjectSaved();
        void updatePlayer();
        void updatePlayer(bool removeAction);

        void resizeProjectDimension(const QSize size);
        void updateProjectAuthor(const QString &artist);

    private slots:
        void preferences();
        void showHelp();
        void aboutTupiTube();
        void openYouTubeChannel();
        void showTipDialog();
        void importPalettes();
        void openRecentProject();
        void createPaintCommand(const TupPaintAreaEvent *event);
        void callSave();
        void restoreFramesMode(TupProject::Mode contextMode);
        void resetMousePointer();
        void updateUsersOnLine(const QString &login, int state);
        void importPapagayoLipSync();
        void hideTopPanels();
        void showWebMessage();
        void setUpdateFlag(bool flag);
        void checkTimeLineVisibility(bool visible);
        void checkExposureVisibility(bool visible);
        void updateFillTool(TColorCell::FillType type);
        void openTupiTubeNetwork();
        void updateColorPanelStatus(bool flag);
        void updatePenPanelStatus(bool flag);
        void updateLibraryPanelStatus(bool flag);
        void updateScenesPanelStatus(bool flag);
        void doPlay();
        void requestSaveAction();

    private:
        QString appTitle;
        TupProjectManager *m_projectManager;
        QString m_fileName;
        bool lastSave;

    private:
        QScreen *screen;
        TupDocumentView *animationTab;
        TupAnimationspace *playerTab;
        TupTwitterWidget *newsTab;
        TupStatusBar *m_statusBar;
        TActionManager *m_actionManager;
        QMenu *m_fileMenu;
        QMenu *m_settingsMenu;
        QMenu *m_viewMenu;
        QMenu *m_insertMenu;
        QMenu *m_toolsMenu; 
        QMenu *m_windowMenu;
        QMenu *m_helpMenu;

        QStringList m_recentProjects;
        QMenu *m_recentProjectsMenu;

    // Network variables
    private:
        TupNetProjectManagerHandler *netProjectManager;
        bool isNetworked;
        ToolView *m_viewChat;

    // Components
    private:
        QToolBar *mainToolBar;
        QToolBar *alternativeToolBar;
        TupExposureSheet *m_exposureSheet;
        TupScenesWidget *m_scenes;
        TupTimeLine *m_timeLine;

        TupLibraryWidget *m_libraryWidget;
        TupColorPaletteWidget *m_colorPalette;
        TupBrushWidget *m_brushWidget;
        ToolView *exposureView;
        ToolView *colorView;
        ToolView *penView;
        ToolView *libraryView;
        ToolView *scenesView;  
        ToolView *helpView;
        ToolView *timeView;
        ToolView *exportView;
        TupExportWidget *exportWidget;

        TupCameraWidget *cameraWidget;
        bool isSaveDialogOpen; 
        bool internetOn;
        int lastTab;
        TupProject::Mode contextMode;
        TupMainWindow::RequestType requestType; 
        QString projectName;
        QString author;
        QString netUser;
        QString webContent;
        bool isImageMsg;
        QSize webMsgSize;
        TAction *helpAction;
        TupDocumentView::DockType currentDock;

    signals:
        void responsed(TupProjectResponse *);
        void updateAnimationModule(TupProject *, int, int, int);
        void activeDockChanged(TupDocumentView::DockType);
};

#endif
