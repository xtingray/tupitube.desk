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

#include "tupmainwindow.h"
#include "tupapplication.h"

#include "tcommandhistory.h"
#include "toolview.h"
#include "tviewbutton.h"
// #include "taudioplayer.h"

void TupMainWindow::createGUI()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupMainWindow::createGUI()]";
    #endif

    TAction *hideAction = new TAction(QPixmap(THEME_DIR + "icons/hide_top_panel.png"), tr("Hide top panels"),
                                      QKeySequence(tr("Alt")), this, SLOT(hideTopPanels()), m_actionManager);
    m_actionManager->insert(hideAction, "hideaction", "file");
    addSpecialButton(hideAction);

    // Adding the color palette to the left side of the interface 
    m_colorPalette = new TupColorPaletteWidget;
    connect(m_colorPalette, SIGNAL(colorSpaceChanged(TColorCell::FillType)), this, SLOT(updateFillTool(TColorCell::FillType)));

    colorView = addToolView(m_colorPalette, Qt::LeftDockWidgetArea, Animation, "Color Palette", QKeySequence(tr("Shift+P")));
    connect(colorView, SIGNAL(visibilityChanged(bool)), this, SLOT(updateColorPanelStatus(bool)));
    // colorView->setShortcut(QKeySequence(tr("Shift+P")));

    m_actionManager->insert(colorView->toggleViewAction(), "show_palette");
    addToPerspective(colorView->toggleViewAction(), Animation);

    connectWidgetToPaintArea(m_colorPalette);

    // Adding the pen parameters widget to the left side of the interface
    m_brushWidget = new TupBrushWidget;
    penView = addToolView(m_brushWidget, Qt::LeftDockWidgetArea, Animation, "Pen", QKeySequence(tr("Shift+B")));
    connect(penView, SIGNAL(visibilityChanged(bool)), this, SLOT(updatePenPanelStatus(bool)));

    m_actionManager->insert(penView->toggleViewAction(), "show_pen");
    addToPerspective(penView->toggleViewAction(), Animation);
    connectWidgetToPaintArea(m_brushWidget);

    // Adding the objects library widget to the left side of the interface
    m_libraryWidget = new TupLibraryWidget(this);
    m_libraryWidget->setLibrary(m_projectManager->getProject()->getLibrary());

    libraryView = addToolView(m_libraryWidget, Qt::LeftDockWidgetArea, Animation, "Library", QKeySequence(tr("Shift+L")));
    connect(libraryView, SIGNAL(visibilityChanged(bool)), this, SLOT(updateLibraryPanelStatus(bool)));

    m_actionManager->insert(libraryView->toggleViewAction(), "show_library");
    addToPerspective(libraryView->toggleViewAction(), Animation);

    new TAction(QPixmap(THEME_DIR + "icons/bitmap.png"), tr("Image"), QKeySequence(tr("Alt+B")), m_libraryWidget, SLOT(importImageGroup()),
        m_actionManager, "importImageGroup");

    new TAction(QPixmap(THEME_DIR + "icons/bitmap_array.png"), tr("Image Sequence"), QKeySequence(tr("Alt+Shift+B")),
        m_libraryWidget, SLOT(importImageSequence()), m_actionManager, "importImageSequence");

    new TAction(QPixmap(THEME_DIR + "icons/svg.png"), tr("SVG File"), QKeySequence(tr("Alt+S")), m_libraryWidget, SLOT(importSvgGroup()),
		m_actionManager, "importSvg");

    new TAction(QPixmap(THEME_DIR + "icons/svg_array.png"), tr("SVG Sequence"), QKeySequence(tr("Alt+Shift+S")), m_libraryWidget, 
		SLOT(importSvgSequence()), m_actionManager, "importSvgSequence");

    //new TAction(QPixmap(), tr("Audio File..."), QKeySequence(), m_libraryWidget, SLOT(importSound()),
    //            m_actionManager, "importAudioFile");

    // SQA: Temporary code
    //m_actionManager->enable("importSvg", false);
    //m_actionManager->enable("importSvgArray", false);

    connectWidgetToManager(m_libraryWidget);
    connectWidgetToLocalManager(m_libraryWidget);

    // Adding the scenes widget to the right side of the interface

    m_scenes = new TupScenesWidget;
    scenesView = addToolView(m_scenes, Qt::RightDockWidgetArea, Animation, "Scenes Manager", QKeySequence(tr("Shift+C")));
    connect(scenesView, SIGNAL(visibilityChanged(bool)), this, SLOT(updateScenesPanelStatus(bool)));
    m_actionManager->insert(scenesView->toggleViewAction(), "show_scenes");
    addToPerspective(scenesView->toggleViewAction(), Animation);

    connectWidgetToManager(m_scenes);
    connectWidgetToLocalManager(m_scenes);

    // Adding the exposure sheet to the right side of the interface
    m_exposureSheet = new TupExposureSheet(this, m_projectManager->getProject());
    connect(m_exposureSheet, SIGNAL(newPerspective(int)), this, SLOT(changePerspective(int)));

    exposureView = addToolView(m_exposureSheet, Qt::RightDockWidgetArea, Animation, "Exposure Sheet", QKeySequence(tr("Shift+E")));
    // connect(exposureView, SIGNAL(visibilityChanged(bool)), this, SLOT(checkTimeLineVisibility(bool)));

    m_actionManager->insert(exposureView->toggleViewAction(), "show_exposure");
    addToPerspective(exposureView->toggleViewAction(), Animation);

    connectWidgetToManager(m_exposureSheet);
    connectWidgetToLocalManager(m_exposureSheet);

    // Adding the time line widget to the bottom side of the interface
    m_timeLine = new TupTimeLine(m_projectManager->getProject());
    connect(m_timeLine, SIGNAL(newPerspective(int)), this, SLOT(changePerspective(int)));

    timeView = addToolView(m_timeLine, Qt::BottomDockWidgetArea, Animation, "Time Line", QKeySequence(tr("Shift+T")));
    // connect(timeView, SIGNAL(visibilityChanged(bool)), this, SLOT(checkExposureVisibility(bool)));

    m_actionManager->insert(timeView->toggleViewAction(), "show_timeline");
    addToPerspective(timeView->toggleViewAction(), Animation);

    connectWidgetToManager(m_timeLine);
    connectWidgetToLocalManager(m_timeLine);

    /*
    // SQA Code
    QListWidget *list = new QListWidget();
    list->addItems(QStringList() << "Hello! I am the bottom dock! ;)");

    timeView = addToolView(list, Qt::BottomDockWidgetArea, Animation, "Time Line", QKeySequence(tr("Shift+T")));
    // connect(timeView, SIGNAL(visibilityChanged(bool)), this, SLOT(checkExposureVisibility(bool)));

    m_actionManager->insert(timeView->toggleViewAction(), "show_timeline");
    addToPerspective(timeView->toggleViewAction(), Animation);
    */

    /* SQA: Define if scripting support should be enabled
    // Adding the script editor to the bottom side, if kinas was enabled
#ifdef ENABLE_KINAS
    KinasWidget *m_scriptEditor = new KinasWidget;
    addToolView(m_scriptEditor, Qt::BottomDockWidgetArea, Animation, "TupiTube Script", QKeySequence(tr("Shift+K"));
#endif
    */

    enableToolViews(false);
}

void TupMainWindow::setupMenu()
{
    // Setting up the file menu
    setupFileActions();

    // Menu File	
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_actionManager->find("new_project"));
    m_fileMenu->addAction(m_actionManager->find("open_project"));
    m_fileMenu->addAction(m_actionManager->find("open_demo"));

    // SQA: This code has been disabled temporary
    // m_fileMenu->addAction(m_actionManager->find("opennetproject"));
    // m_fileMenu->addAction(m_actionManager->find("exportprojectserver"));

    // Adding Option Open Recent	
    m_recentProjectsMenu = new QMenu(tr("Open Recent"), this);

    TCONFIG->beginGroup("General");
    QString files = TCONFIG->value("Recents").toString();
    QStringList recents = files.split(';');
    if (files.isEmpty())
        recents.clear();
    updateOpenRecentMenu(m_recentProjectsMenu, recents);	
    m_fileMenu->addMenu(m_recentProjectsMenu);

    // Adding Options save, save as, close, export, import palettes and exit	
    m_fileMenu->addAction(m_actionManager->find("save_project"));

    m_fileMenu->addAction(m_actionManager->find("save_project_as"));
    m_fileMenu->addAction(m_actionManager->find("close_project"));

    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_actionManager->find("export"));
    m_fileMenu->addAction(m_actionManager->find("post"));

    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_actionManager->find("Exit"));

    // Setting up the Preferences menu
    setPreferencesAction();
    m_settingsMenu = menuBar()->addMenu(tr("&Edit"));

    m_settingsMenu->addAction(m_actionManager->find("preferences"));
    // Temporary out while SQA is done
    // m_actionManager->enable("preferences", false);

    // Temporary out while SQA is done
    // Setting up the insert menu
    // setupInsertActions();
    // Menu Insert
    m_insertMenu = menuBar()->addMenu(tr("&Import"));

    // Adding Options import bitmap and import audio file
    m_insertMenu->addAction(m_actionManager->find("importImageGroup"));
    m_insertMenu->addAction(m_actionManager->find("importImageSequence"));
    m_insertMenu->addAction(m_actionManager->find("importSvg"));
    m_insertMenu->addAction(m_actionManager->find("importSvgSequence"));
    // m_insertMenu->addAction(m_actionManager->find("importAudioFile"));

    m_insertMenu->addSeparator();
    m_insertMenu->addAction(m_actionManager->find("importGimpPalettes"));

    // SQA: Action disabled while Library module is fixed
    m_insertMenu->addAction(m_actionManager->find("importPapagayoLipSync"));

    /*
    // Setting up the window menu
    m_windowMenu = menuBar()->addMenu(tr("&Window"));
    // Adding Options show debug, palette, pen, library, timeline, scenes, exposure, help
    m_windowMenu->addAction(m_actionManager->find("show_palette"));
    m_windowMenu->addAction(m_actionManager->find("show_pen"));
    m_windowMenu->addAction(m_actionManager->find("show_library"));
    m_windowMenu->addAction(m_actionManager->find("show_timeline"));
    m_windowMenu->addAction(m_actionManager->find("show_scenes"));
    m_windowMenu->addAction(m_actionManager->find("show_exposure"));
    */

    // Setup perspective menu
    m_viewMenu = new QMenu(tr("Modules"), this);
    QActionGroup *group = new QActionGroup(this);
    group->setExclusive(true);

    // Adding Option Animation
    QAction *drawingPerspective = new QAction(tr("Animation"), this);
    drawingPerspective->setIcon(QPixmap(THEME_DIR + "icons/animation_mode.png")); 
    drawingPerspective->setIconVisibleInMenu(true);
    drawingPerspective->setShortcut(QKeySequence("Ctrl+1"));
    drawingPerspective->setData(Animation);
    group->addAction(drawingPerspective);

    // Adding Option Player 
    QAction *animationPerspective = new QAction(tr("Player"), this);
    animationPerspective->setIcon(QPixmap(THEME_DIR + "icons/play_small.png"));
    animationPerspective->setIconVisibleInMenu(true);
    animationPerspective->setShortcut(QKeySequence("Ctrl+2"));
    animationPerspective->setData(Player);
    group->addAction(animationPerspective);

    // Adding Option News
    TCONFIG->beginGroup("General");
    bool getNews = TCONFIG->value("GetNews", true).toBool();
    if (getNews) {
        QAction *newsPerspective = new QAction(tr("News"), this);
        newsPerspective->setIcon(QPixmap(THEME_DIR + "icons/news_mode.png"));
        newsPerspective->setIconVisibleInMenu(true);
        newsPerspective->setShortcut(QKeySequence("Ctrl+3"));
        newsPerspective->setData(News);
        group->addAction(newsPerspective);
    }

    m_viewMenu->addActions(group->actions());
    connect(group, SIGNAL(triggered(QAction *)), this, SLOT(changePerspective(QAction *)));
    menuBar()->addMenu(m_viewMenu);
	
    // Setting up the help menu
    setupHelpActions();
    m_helpMenu = new QMenu(tr("&Help"), this);
    menuBar()->addMenu(m_helpMenu);
    m_helpMenu->addAction(m_actionManager->find("help"));
    m_helpMenu->addAction(m_actionManager->find("youtube"));
    m_helpMenu->addAction(m_actionManager->find("tip_of_day"));
    m_helpMenu->addSeparator();
    m_helpMenu->addAction(m_actionManager->find("about_tupitube"));

    setMenuItemsContext(false);

    // SQA: Temporary code
    // menuBar()->setVisible(false);
}

void TupMainWindow::setMenuItemsContext(bool flag)
{
/*
#ifdef TUP_DEBUG
    qDebug() << "[TupMainWindow::setMenuItemsContext()] - flag -> " << flag;
#endif
*/
    m_actionManager->enable("save_project", flag);
    m_actionManager->enable("save_project_as", flag);
    m_actionManager->enable("close_project", flag);
    m_actionManager->enable("hideaction", flag);
    m_actionManager->enable("export", flag);
    m_actionManager->enable("post", flag);
    m_actionManager->enable("importImageGroup", flag);

    /*
    m_actionManager->enable("show_palette", flag);
    m_actionManager->enable("show_pen", flag);
    m_actionManager->enable("show_library", flag);
    m_actionManager->enable("show_timeline", flag);
    m_actionManager->enable("show_scenes", flag);
    m_actionManager->enable("show_exposure", flag);
    */

    m_insertMenu->setEnabled(flag);
    // m_windowMenu->setEnabled(flag);
    m_viewMenu->setEnabled(flag);
}

void TupMainWindow::setupFileActions()
{
    TAction *newProject = new TAction(QPixmap(THEME_DIR + "icons/new.png"), tr("New Project"), QKeySequence(tr("Ctrl+N")),
				      this, SLOT(newProject()), m_actionManager);
    newProject->setStatusTip(tr("Open new project"));
    m_actionManager->insert(newProject, "new_project", "file");

    TAction *openFile = new TAction(QPixmap(THEME_DIR + "icons/open.png"), tr("Open Project"), QKeySequence(tr("Ctrl+O")), 
				    this, SLOT(openProject()), m_actionManager);
    m_actionManager->insert(openFile, "open_project", "file");
    openFile->setStatusTip(tr("Load existent project"));

    // SQA: This code has been disabled temporary
    /*
    TAction *openNetFile = new TAction(QPixmap(THEME_DIR + "icons/net_document.png"), tr("Open Project From Server..."), 
				       tr(""), this, SLOT(openProjectFromServer()), m_actionManager);
    m_actionManager->insert(openNetFile, "opennetproject", "file");

    TAction *importNetFile = new TAction(QPixmap(THEME_DIR + "icons/import_project.png"), tr("Export Project To Server..."), tr(""), this, 
					 SLOT(importProjectToServer()), m_actionManager);
    m_actionManager->insert(importNetFile, "exportprojectserver", "file");
    */

    TAction *openDemo = new TAction(QPixmap(THEME_DIR + "icons/open.png"), tr("Open Example"), tr(""),
                    this, SLOT(openExample()), m_actionManager);
    m_actionManager->insert(openDemo, "open_demo", "file");
    openDemo->setStatusTip(tr("Open example project"));

    TAction *save = new TAction(QPixmap(THEME_DIR + "icons/save.png"), tr( "Save Project" ),
				QKeySequence(tr("Ctrl+S")), this, SLOT(saveProject()), m_actionManager);
    m_actionManager->insert(save, "save_project", "file");
    save->setStatusTip(tr("Save current project in current location"));

    TAction *saveAs = new TAction(QPixmap(THEME_DIR + "icons/save_as.png"), tr("Save Project As..."),
				  QKeySequence(tr("Ctrl+Shift+S")), this, SLOT(saveAs()), m_actionManager);
    saveAs->setStatusTip(tr("Open dialog box to save current project in any location"));
    m_actionManager->insert(saveAs, "save_project_as", "file");

    TAction *close = new TAction(QPixmap(THEME_DIR + "icons/close.png"), tr("Cl&ose Project"), QKeySequence(tr("Ctrl+W")),
                 this, SLOT(closeInterface()), m_actionManager);
    close->setStatusTip(tr("Close active project"));
    m_actionManager->insert(close, "close_project", "file");

    // Import Palette action
    TAction *importPalette = new TAction(QPixmap(THEME_DIR + "icons/import.png"), tr("&Import GIMP Palettes"),
					 QKeySequence(tr("Shift+G")), this, SLOT(importPalettes()), m_actionManager);
    importPalette->setStatusTip(tr("Import palettes"));
    m_actionManager->insert(importPalette, "importGimpPalettes", "file");

    TAction *importPapagayo = new TAction(QPixmap(THEME_DIR + "icons/papagayo.png"), tr("&Import Papagayo Lip-sync"),
                                         QKeySequence(tr("Alt+P")), this, SLOT(importPapagayoLipSync()), m_actionManager);
    importPapagayo->setStatusTip(tr("Import Papagayo lip-sync"));
    m_actionManager->insert(importPapagayo, "importPapagayoLipSync", "file");

    // Export Project action
    TAction *exportProject = new TAction(QPixmap(THEME_DIR + "icons/export.png"), tr("&Export Project"),
                                         QKeySequence(tr("Ctrl+R")), this, SLOT(exportProject()), m_actionManager);
    exportProject->setStatusTip(tr("Export project to several video formats"));
    m_actionManager->insert(exportProject, "export", "file");

    // Export Project action
    TAction *postProject = new TAction(QPixmap(THEME_DIR + "icons/share.png"), tr("&Post Project"),
                                       QKeySequence(tr("Ctrl+P")), this, SLOT(postProject()), m_actionManager);
    postProject->setStatusTip(tr("Post project on TupiTube's network"));
    m_actionManager->insert(postProject, "post", "file");

    // Visit TupiTube's Network action
    TAction *openNetwork = new TAction(QPixmap(THEME_DIR + "icons/social_network.png"), tr("Open TupiTube's Network"),
                                       QKeySequence(), this, SLOT(openTupiTubeNetwork()), m_actionManager);
    openNetwork->setStatusTip(tr("Open TupiTube's Network"));
    m_actionManager->insert(openNetwork, "open_network", "file");

    // Exit action
    TAction *exit = new TAction(QPixmap(THEME_DIR + "icons/exit.png"), tr("Quit"), QKeySequence(tr("Ctrl+Q")),
                                qApp, SLOT(closeAllWindows()), m_actionManager);

    exit->setStatusTip(tr("Close application"));
    m_actionManager->insert(exit, "exit", "file");

    // when the last window is closed, the application should quit
    connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));
}

void TupMainWindow::openTupiTubeNetwork()
{
    QDesktopServices::openUrl(QString("https://www.tupitube.com/?desk"));
}

void TupMainWindow::setPreferencesAction()
{
    TAction *preferences = new TAction(QPixmap(THEME_DIR + "icons/properties.png"), tr("Pr&eferences..."), 
                                        QKeySequence(tr("Ctrl+P")), this, SLOT(preferences()),
                                        m_actionManager, "preferences");
    preferences->setStatusTip(tr("Opens the preferences dialog box"));
}

void TupMainWindow::setupHelpActions()
{
    helpAction = new TAction(QPixmap(THEME_DIR + "icons/help_mode.png"), tr("Online Help Content"), QKeySequence(tr("F1")),
                 this, SLOT(showHelp()), m_actionManager, "help");
    helpAction->setEnabled(false);

    new TAction(QPixmap(THEME_DIR + "icons/youtube.png"), tr("YouTube Tutorials"), QKeySequence(tr("Y")),
                this, SLOT(openYouTubeChannel()), m_actionManager, "youtube");
    new TAction(QPixmap(THEME_DIR + "icons/tip.png"), tr("Tip Of The Day"), QKeySequence(tr("Ctrl+T")),
                this, SLOT(showTipDialog()), m_actionManager, "tip_of_day");
    new TAction(QPixmap(THEME_DIR + "icons/about.png"), tr("About TupiTube Desk"), QKeySequence(tr("Ctrl+K")), 
                this, SLOT(aboutTupiTube()), m_actionManager, "about_tupitube");
}

void TupMainWindow::setupToolBar()
{
    mainToolBar = new QToolBar(tr("Actions Bar"), this);
    mainToolBar->setIconSize(QSize(22, 22));
    mainToolBar->setMovable(false);

    mainToolBar->addAction(m_actionManager->find("new_project"));
    mainToolBar->addAction(m_actionManager->find("open_project"));

    // SQA: This code has been disabled temporary
    // mainToolBar->addAction(m_actionManager->find("opennetproject"));

    mainToolBar->addAction(m_actionManager->find("save_project"));
    mainToolBar->addAction(m_actionManager->find("save_project_as"));
    mainToolBar->addAction(m_actionManager->find("close_project"));
    mainToolBar->addAction(m_actionManager->find("open_network"));

    addToolBar(Qt::TopToolBarArea, mainToolBar);
}

void TupMainWindow::updateOpenRecentMenu(QMenu *menu, QStringList recents)
{
    menu->clear();
    m_recentProjects.clear();

    if (recents.count() == 0) {
        m_recentProjectsMenu->setEnabled(false);
        return;
    } else {
        if (recents.count() > 5) {
            QStringList list; 
            list << recents.mid(0, 4);
            recents = list;
        }
    }

    int i = 0;
    QAction *action[5];
    foreach (QString recent, recents) {
        m_recentProjects << recent;
        action[i] = new QAction(QPixmap(THEME_DIR + "icons/recent_files.png"), recent, this); 
        action[i]->setIconVisibleInMenu(true);
        menu->addAction(action[i]);
        connect(action[i], SIGNAL(triggered()), this, SLOT(openRecentProject()));
        i++;
    }

    if (!m_recentProjectsMenu->isEnabled())
        m_recentProjectsMenu->setEnabled(true);
}

void TupMainWindow::changePerspective(QAction *action)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupMainWindow::changePerspective(QAction)]";
    #endif

    int perspective = action->data().toInt();

    // Animation or Player perspective
    if (perspective == Animation || perspective == Player) {
        setCurrentTab(perspective - 1);
    } else if (perspective == News) { // News perspective
        setCurrentTab(2);
    } 

    action->setChecked(true);
}

void TupMainWindow::changePerspective(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupMainWindow::changePerspective(int)] - Updating QMainWindow interface...";
    #endif

    if (index == 4) { // Player
        m_libraryWidget->stopSoundPlayer();
        setCurrentTab(1);
    } else {
        setCurrentTab(index);
    }
}

void TupMainWindow::doPlay()
{
    cameraWidget->doPlay();
}

void TupMainWindow::setUndoRedoActions()
{
    QAction *undo = new QAction(QIcon(THEME_DIR + "icons/undo.png"), tr("Undo"), this);
    undo->setIconVisibleInMenu(true);
    undo->setShortcut(QKeySequence(tr("Ctrl+Z")));
    connect(undo, SIGNAL(triggered()), m_projectManager, SLOT(undo()));

    QAction *redo = new QAction(QIcon(THEME_DIR + "icons/redo.png"), tr("Redo"), this);
    redo->setIconVisibleInMenu(true);
    redo->setShortcut(QKeySequence(tr("Ctrl+Y")));
    connect(redo, SIGNAL(triggered()), m_projectManager, SLOT(redo()));

    kApp->insertGlobalAction(undo, "undo");
    kApp->insertGlobalAction(redo, "redo");
}

void TupMainWindow::importPapagayoLipSync()
{
    animationTab->importPapagayoLipSync();
}

void TupMainWindow::hideTopPanels()
{
    if (m_projectManager->isOpen()) {
        if (mainToolBar->isVisible()) {
            m_actionManager->find("hideaction")->setIcon(QIcon(QPixmap(THEME_DIR + "icons/show_top_panel.png")));
            menuBar()->setVisible(false);
            mainToolBar->setVisible(false);
        } else {
            m_actionManager->find("hideaction")->setIcon(QIcon(QPixmap(THEME_DIR + "icons/hide_top_panel.png")));
            menuBar()->setVisible(true);
            mainToolBar->setVisible(true);
        }
    }
}

void TupMainWindow::checkTimeLineVisibility(bool visible)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupMainWindow::checkTimeLineVisibility(bool)] - visible: " << visible;
    #endif

    if (visible) {
        if (timeView->isExpanded()) {
            timeView->blockSignals(true);
            timeView->expandDock(false);
            timeView->blockSignals(false);
        }

        if (scenesView->isExpanded())
            scenesView->expandDock(false);

        currentDock = TupDocumentView::ExposureSheet;
        emit activeDockChanged(TupDocumentView::ExposureSheet);
    } else {
        if (!timeView->isExpanded())
            emit activeDockChanged(TupDocumentView::None);
    }
}

void TupMainWindow::checkExposureVisibility(bool visible)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupMainWindow::checkExposureVisibility(bool)] - visible: " << visible;
    #endif

    if (visible) {
        if (exposureView->isExpanded()) {
            exposureView->blockSignals(true);
            exposureView->expandDock(false);
            exposureView->blockSignals(false);
        }
        currentDock = TupDocumentView::TimeLine;
        emit activeDockChanged(TupDocumentView::TimeLine);
    } else {
        if (!exposureView->isExpanded())
            emit activeDockChanged(TupDocumentView::None);
    }
}

void TupMainWindow::updateFillTool(TColorCell::FillType type)
{
    // Q_UNUSED(type);

    if (animationTab)
        animationTab->setFillTool(type);
}

void TupMainWindow::updateColorPanelStatus(bool flag)
{
    if (flag) {
        if (penView->isExpanded())
            penView->expandDock(false);
        if (libraryView->isExpanded())
            libraryView->expandDock(false);
    }
}

void TupMainWindow::updatePenPanelStatus(bool flag)
{
    if (flag) {
        if (colorView->isExpanded())
            colorView->expandDock(false);
        if (libraryView->isExpanded())
            libraryView->expandDock(false);
    }
}

void TupMainWindow::updateLibraryPanelStatus(bool flag)
{
    if (flag) {
        if (colorView->isExpanded())
            colorView->expandDock(false);

        if (penView->isExpanded())
            penView->expandDock(false);
    }
}

void TupMainWindow::updateScenesPanelStatus(bool flag)
{
    if (flag) {
        if (exposureView->isExpanded())
            exposureView->expandDock(false);
    }
}
