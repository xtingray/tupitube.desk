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
#include "tuptwitter.h"
#include "tupnewproject.h"
#include "tupabout.h"
#include "tuppackagehandler.h"
#include "tuppaletteimporter.h"
#include "tuppaintareaevent.h"
#include "tuppaintareacommand.h"

// TupiTube Framework
#include "tipdialog.h"
#include "tmsgdialog.h"
#include "tosd.h"
// #include "taudioplayer.h"

#include "tupapplication.h"
#include "tuppluginmanager.h"
#include "tupprojectcommand.h"
#include "tuplocalprojectmanagerhandler.h"

// Network support
#include "tupnetprojectmanagerparams.h"
#include "tupconnectdialog.h"
#include "tuplistpackage.h"
#include "tupimportprojectpackage.h"
#include "tuplistprojectspackage.h"
#include "tupsavepackage.h"

// Qt Framework
#include <QImage>
#include <QPixmap>
#include <QResizeEvent>
#include <QMenu>
#include <QCloseEvent>
#include <QTextEdit>
#include <QFileDialog>
#include <QDomDocument>
#include <QMessageBox>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QThread>
#include <QClipboard>
#include <QDesktopServices>
#include <QFileOpenEvent>

TupMainWindow::TupMainWindow(const QString &winKey) : TabbedMainWindow(winKey), m_projectManager(nullptr), animationTab(nullptr), playerTab(nullptr),
               m_viewChat(nullptr), m_exposureSheet(nullptr), m_scenes(nullptr), isSaveDialogOpen(false), internetOn(false)
{
    #ifdef TUP_DEBUG
        qDebug() << "TupMainWindow()";
    #endif

    // Naming the main window
    appTitle = "TupiTube Desk";
    setWindowTitle(appTitle);
    setWindowIcon(QIcon(THEME_DIR + "icons/about.png"));
    setObjectName("TupMainWindow_");
    setAcceptDrops(true);
    screen = QGuiApplication::screens().at(0);

    isNetworked = false;
    exportWidget = nullptr;

    QFile file(THEME_DIR + "config/ui.qss");
    if (file.exists()) {
        file.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(file.readAll());
        if (styleSheet.length() > 0)
            setStyleSheet(styleSheet);
        file.close();
    } else {
        #ifdef TUP_DEBUG 
            qWarning() << "TupMainWindow::TupMainWindow() - theme file doesn't exist -> " + QString(THEME_DIR + "config/ui.qss");
        #endif
    }
        
    // Loading audio player plugin
    // TAudioPlayer::instance()->loadEngine("gstreamer"); // FIXME: Move this to the settings 

    // Defining the render type for the drawings
    // m_renderType = Tupi::RenderType(TCONFIG->value("RenderType").toInt());

    // Calling out the project manager
    m_projectManager = new TupProjectManager(this);

    // Calling out the events/actions manager
    m_actionManager = new TActionManager(this);

    // Setting up all the GUI...
    createGUI(); // This method is called from the tupmainwindow_gui class
    setupMenu();
    setupToolBar();

    // SQA: Web announcement comes here
    QString webMsgPath = QDir::homePath() + "/." + QCoreApplication::applicationName() + "/webmsg.html";
    QFile webMsgFile(webMsgPath);
    QString fileContent = "";
    if (webMsgFile.exists()) {
        if (webMsgFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&webMsgFile);
            while (!in.atEnd())
                fileContent += in.readLine();
        }
    }

    // Processing web msg content
    bool showWebMsg = false;
    isImageMsg = false;
    webContent = "";
    if (!fileContent.isEmpty()) {
        QDomDocument doc;
        if (doc.setContent(fileContent)) {
            QDomElement root = doc.documentElement();
            QDomNode n = root.firstChild();
            while (!n.isNull()) {
                QDomElement e = n.toElement();
                if (e.tagName() == "show") {
                    QString flag = e.text();
                    if (flag.compare("true") == 0)
                        showWebMsg = true;
                    else
                        break;
                } else if (e.tagName() == "size") {
                    QStringList numbers = e.text().split(",");
                    if (numbers.size() == 2)
                        webMsgSize = QSize(numbers.at(0).toInt(), numbers.at(1).toInt());
                } else if (e.tagName() == "text") {
                    webContent = e.text();
                } else if (e.tagName() == "image") {
                    isImageMsg = true;
                }

                n = n.nextSibling();
            }
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "TupMainWindow() - Fatal error parsing file -> " + webMsgPath;
            #endif
        }
    }

    if (showWebMsg) {
        QTimer::singleShot(0, this, SLOT(showWebMessage()));
    } else {
        TCONFIG->beginGroup("General");
        bool update = TCONFIG->value("NotifyUpdate", false).toBool();

        if (update)
            QDesktopServices::openUrl(QString(MAEFLORESTA_URL) + QString("downloads"));

        // Check if user wants to see a TupiTube tip for every time he launches the program
        bool showTips = TCONFIG->value("ShowTipOfDay", true).toBool();

        // If option is enabled, then, show a little dialog with a nice tip
        if (showTips)
            QTimer::singleShot(0, this, SLOT(showTipDialog()));
    }

    // Time to load plugins... 
    TupPluginManager::instance()->loadPlugins();

    // Defining the Animation view, as the first interface to show up   
    setCurrentPerspective(Animation);

    if (TCONFIG->firstTime()) {
        TCONFIG->beginGroup("General");
        TCONFIG->setValue("NotifyUpdate", false);
        TCONFIG->setValue("OpenLastProject", false);
        TCONFIG->setValue("ShowTipOfDay", true);
        TCONFIG->setValue("ConfirmRemoveFrame", true); 
        TCONFIG->setValue("ConfirmRemoveLayer", true); 
        TCONFIG->setValue("ConfirmRemoveScene", true); 
        TCONFIG->setValue("ConfirmRemoveObject", true); 
        TCONFIG->beginGroup("PaintArea");
        TCONFIG->setValue("GridColor", "#0000b4");
        TCONFIG->setValue("GridSeparation", 10);
    }

    TupMainWindow::requestType = None;
    lastSave = false;
}

TupMainWindow::~TupMainWindow()
{
    #ifdef TUP_DEBUG
        qDebug() << "~TupMainWindow()";
    #endif

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->clear(QClipboard::Clipboard);

    delete TupPluginManager::instance();
    delete TOsd::self();
 
    delete m_projectManager;
    delete penView;
}

void TupMainWindow::createNewLocalProject()
{
    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "TupMainWindow::createNewLocalProject()";
    #endif

    TupMainWindow::requestType = NewLocalProject;
    m_projectManager->setupNewProject();
    m_projectManager->setOpen(true);
 
    enableToolViews(true);
    setMenuItemsContext(true);

    setWorkSpace();
}

void TupMainWindow::createNewNetProject(const QString &title, const QStringList &users)
{
    isNetworked = true;
    projectName = title;
    setWindowTitle(appTitle + " - " + projectName + " " + tr("[ by %1 | net mode ]").arg(netUser));

    if (m_viewChat) {
        removeToolView(m_viewChat);
        delete m_viewChat;
    }

    m_viewChat = addToolView(netProjectManager->communicationWidget(), Qt::BottomDockWidgetArea, All, "Chat");
    m_viewChat->setVisible(false);

    enableToolViews(true);
    setMenuItemsContext(true);
    m_exposureSheet->updateFramesState();
    m_projectManager->setOpen(true);

    setWorkSpace(users);
}

void TupMainWindow::setWorkSpace(const QStringList &users)
{
    #ifdef TUP_DEBUG
        qDebug() << "TupMainWindow::setWorkSpace()";
    #endif

    TCONFIG->beginGroup("General");
    bool getNews = TCONFIG->value("GetNews", true).toBool();
    if (getNews) {
        // Downloading maefloresta Twitter status
        TupTwitter *twitter = new TupTwitter();
        twitter->start();
        connect(twitter, SIGNAL(pageReady()), this, SLOT(addTwitterPage()));
        connect(twitter, SIGNAL(newUpdate(bool)), this, SLOT(setUpdateFlag(bool)));
    }

    if (m_projectManager->isOpen()) {
        if (TupMainWindow::requestType == NewLocalProject || TupMainWindow::requestType == NewNetProject)
            TOsd::self()->display(tr("Information"), tr("Opening a new document..."));

        contextMode = TupProject::FRAMES_MODE;

        // Setting undo/redo actions
        setUndoRedoActions();

        animationTab = new TupDocumentView(m_projectManager->getProject(), isNetworked, users, this);

        TCONFIG->beginGroup("Network");
        QString server = TCONFIG->value("Server").toString();
        if (isNetworked && server.compare("tupitu.be") == 0) {
            connect(animationTab, SIGNAL(requestExportImageToServer(int, int, const QString &, const QString &, const QString &)),                         
                    netProjectManager, SLOT(sendExportImageRequest(int, int, const QString &, const QString &, const QString &)));
            connect(animationTab, SIGNAL(updateStoryboard(TupStoryboard *, int)), netProjectManager, SLOT(updateStoryboardRequest(TupStoryboard *, int)));
            connect(animationTab, SIGNAL(postStoryboard(int)), netProjectManager, SLOT(postStoryboardRequest(int))); 
        }

        QWidget *animationWidget = new QWidget();
        animationWidget->setWindowTitle(tr("Animation"));
        animationWidget->setWindowIcon(QPixmap(THEME_DIR + "icons/animation_mode.png"));
        QBoxLayout *tabLayout = new QBoxLayout(QBoxLayout::TopToBottom, animationWidget);
        tabLayout->addWidget(animationTab);
        addWidget(animationWidget);

        connectWidgetToManager(animationTab);
        connectWidgetToLocalManager(animationTab);
        connectWidgetToPaintArea(animationTab);
        connect(animationTab, SIGNAL(modeHasChanged(TupProject::Mode)), this, SLOT(restoreFramesMode(TupProject::Mode)));
        connect(animationTab, SIGNAL(colorChangedFromFullScreen(const QColor &)), this, SLOT(updatePenColor(const QColor &)));
        connect(animationTab, SIGNAL(projectSizeHasChanged(const QSize)), this, SLOT(resizeProjectDimension(const QSize))); 
        connect(animationTab, SIGNAL(newPerspective(int)), this, SLOT(changePerspective(int)));
        connect(animationTab, SIGNAL(contourColorChanged(const QColor &)), m_colorPalette, SLOT(updateContourColor(const QColor &))); 
        connect(animationTab, SIGNAL(fillColorChanged(const QColor &)), m_colorPalette, SLOT(updateFillColor(const QColor &)));
        connect(animationTab, SIGNAL(bgColorChanged(const QColor &)), m_colorPalette, SLOT(updateBgColor(const QColor &)));
        connect(animationTab, SIGNAL(colorModeChanged(TColorCell::FillType)), m_colorPalette, SLOT(checkColorButton(TColorCell::FillType)));
        connect(animationTab, SIGNAL(penWidthChanged(int)), this, SLOT(updatePenThickness(int)));
        connect(this, SIGNAL(activeDockChanged(TupDocumentView::DockType)), animationTab, SLOT(updateActiveDock(TupDocumentView::DockType)));

        animationTab->setAntialiasing(true);

        int width = animationTab->workSpaceSize().width();
        int height = animationTab->workSpaceSize().height();
        animationTab->setWorkSpaceSize(width, height);

        TupProject *project = m_projectManager->getProject();
        int pWidth = project->getDimension().width();
        int pHeight = project->getDimension().height();

        double proportion = 1;
        if (pWidth >= pHeight)
            proportion = static_cast<double>(width) / static_cast<double>(pWidth);
        else
            proportion = static_cast<double>(height) / static_cast<double>(pHeight);

        if (proportion <= 0.5) {
            animationTab->setZoomPercent("20");
        } else if (proportion > 0.5 && proportion <= 0.75) {
            animationTab->setZoomPercent("25");
        } else if (proportion > 0.75 && proportion <= 1.7) {
            animationTab->setZoomPercent("50");
        } else if (proportion > 1.7 && proportion < 2) {
            animationTab->setZoomPercent("75");
        }

        // TupCamera Widget
        cameraWidget = new TupCameraWidget(m_projectManager->getProject(), isNetworked);
        connect(cameraWidget, SIGNAL(projectAuthorUpdated(const QString &)), this, SLOT(updateProjectAuthor(const QString &)));
        connectWidgetToManager(cameraWidget);

        // Player widget must be hidden while the Player tab is not visible
        cameraWidget->setVisible(false);

        connect(m_libraryWidget, SIGNAL(soundUpdated()), cameraWidget, SLOT(updateSoundItems()));

        m_libraryWidget->setNetworking(isNetworked);

        if (isNetworked) {
            connect(cameraWidget, SIGNAL(requestForExportVideoToServer(const QString &, const QString &, const QString &, int, const QList<int>)), 
                    netProjectManager, SLOT(sendVideoRequest(const QString &, const QString &, const QString &, int, const QList<int>)));
        } else {
            connect(animationTab, SIGNAL(autoSave()), this, SLOT(callSave()));
        }

        playerTab = new TupAnimationspace(cameraWidget);
        playerTab->setWindowIcon(QIcon(THEME_DIR + "icons/play_small.png"));
        playerTab->setWindowTitle(tr("Player"));                    
        connect(playerTab, SIGNAL(newPerspective(int)), this, SLOT(changePerspective(int)));
        addWidget(playerTab);

        connect(animationTab, SIGNAL(updateFPS(int)), cameraWidget, SLOT(setStatusFPS(int)));

        connect(exposureView, SIGNAL(visibilityChanged(bool)), this, SLOT(checkTimeLineVisibility(bool)));
        connect(timeView, SIGNAL(visibilityChanged(bool)), this, SLOT(checkExposureVisibility(bool)));

        exposureView->expandDock(true);
        currentDock = TupDocumentView::ExposureSheet;

        // SQA: Code useful for future features
        // if (!isNetworked) 
        //     connect(animationTab, SIGNAL(autoSave()), this, SLOT(callSave()));

        m_projectManager->undoModified();
        m_colorPalette->init();
        m_colorPalette->setBgColor(project->getBgColor());

        TCONFIG->beginGroup("BrushParameters");
        int thickness = TCONFIG->value("Thickness", 3).toInt();
        m_brushWidget->init(thickness);
        // m_brushWidget->setThickness(thickness);

        if (TupMainWindow::requestType == OpenLocalProject || TupMainWindow::requestType == OpenNetProject)
            TOsd::self()->display(tr("Information"), tr("Project <b>%1</b> opened!").arg(m_projectManager->getProject()->getName()));

        m_exposureSheet->setScene(0);
        connect(this, SIGNAL(tabHasChanged(int)), this, SLOT(updateCurrentTab(int)));

        m_projectManager->clearUndoStack();

        if (internetOn && newsTab)
            addWidget(newsTab);
    }
}

void TupMainWindow::addTwitterPage()
{
    if (tabCount() == 2) {
        QString twitterPath = QDir::homePath() + "/." + QCoreApplication::applicationName() + "/twitter.html";
        if (QFile::exists(twitterPath)) {
            #ifdef TUP_DEBUG
                qDebug() << "TupMainWindow::addTwitterPage() - Loading page -> " + twitterPath;
            #endif

            internetOn = true;
            newsTab = new TupTwitterWidget(this);
            newsTab->setSource(twitterPath);
            connect(newsTab, SIGNAL(newPerspective(int)), this, SLOT(changePerspective(int)));
            addWidget(newsTab);

            helpAction->setEnabled(true);
        } else {
            #ifdef TUP_DEBUG
               qWarning() << "TupMainWindow::addTwitterPage() - Warning: Couldn't load page -> " + twitterPath;
            #endif
        }
    }
}

void TupMainWindow::newProject()
{
    #ifdef TUP_DEBUG
        qWarning() << "Creating new project...";
    #endif

    if (cancelChanges())
        return;

    TupNewProject *wizard = new TupNewProject(this);
    wizard->show();

    wizard->move(static_cast<int> ((screen->geometry().width() - wizard->width()) / 2),
                 static_cast<int> ((screen->geometry().height() - wizard->height()) / 2));

    wizard->focusProjectLabel();

    if (wizard->exec() != QDialog::Rejected) {
        if (wizard->useNetwork()) {
            TupMainWindow::requestType = NewNetProject;
            setupNetworkProject(wizard->parameters());
            netUser = wizard->login();
        } else {
            setupLocalProject(wizard->parameters());
            createNewLocalProject();
        }
    }

    delete wizard;
}

bool TupMainWindow::cancelChanges()
{
    if (m_projectManager->isProjectModified()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Question"));
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText(tr("The document has been modified."));
        msgBox.setInformativeText(tr("Do you want to save the project?"));

        msgBox.addButton(QString(tr("Save")), QMessageBox::AcceptRole);
        msgBox.addButton(QString(tr("Discard")), QMessageBox::NoRole);
        msgBox.addButton(QString(tr("Cancel")), QMessageBox::DestructiveRole);
        msgBox.show();

        msgBox.move(static_cast<int> ((screen->geometry().width() - msgBox.width()) / 2),
                     static_cast<int> ((screen->geometry().height() - msgBox.height()) / 2));

        int ret = msgBox.exec();
        switch (ret) {
            case QMessageBox::AcceptRole:
                 lastSave = true;
                 saveProject();
                 return false;
            case QMessageBox::DestructiveRole:
                 return true;
        }
    }

    return false;
}

void TupMainWindow::closeInterface()
{
    #ifdef TUP_DEBUG
        qDebug() << "TupMainWindow::closeInterface()";
    #endif

    if (cancelChanges())
        return;

    closeProject();
}

bool TupMainWindow::closeProject()
{
    #ifdef TUP_DEBUG
        qDebug() << "TupMainWindow::closeProject()";
    #endif

    // SQA: Verify this conditional
    if (!m_projectManager->isOpen())
        return true;

    if (!mainToolBar->isVisible())
        hideTopPanels();

    resetUI();

    return true;
}

void TupMainWindow::resetUI()
{
    #ifdef TUP_DEBUG
        qDebug() << "TupMainWindow::resetUI()";
    #endif

    disconnect(exposureView, SIGNAL(visibilityChanged(bool)), this, SLOT(checkTimeLineVisibility(bool)));
    disconnect(timeView, SIGNAL(visibilityChanged(bool)), this, SLOT(checkExposureVisibility(bool)));

    colorView->expandDock(false);
    penView->expandDock(false);
    libraryView->expandDock(false);
    scenesView->expandDock(false);
    exposureView->expandDock(false);
    timeView->expandDock(false);

    setUpdatesEnabled(false);
    setMenuItemsContext(false);
    updateOpenRecentMenu(m_recentProjectsMenu, m_recentProjects);

    if (animationTab)
        animationTab->closeInterface();

    removeAllWidgets();

    if (internetOn) {
        delete newsTab;
        newsTab = nullptr;
    }

    if (playerTab) {
        delete playerTab;
        playerTab = nullptr;
    }

    if (animationTab) {
        delete animationTab;
        animationTab = nullptr;
    }

    if (exportWidget) {
        delete exportWidget;
        exportWidget = nullptr;
    }

    m_exposureSheet->closeAllScenes();
    m_timeLine->closeAllScenes();
    m_scenes->closeAllScenes();
    m_libraryWidget->resetGUI();

    m_fileName = QString();

    enableToolViews(false);
    setUpdatesEnabled(true);
    setWindowTitle(appTitle);

    if (isNetworked) {
        m_viewChat->expandDock(false);
        // netProjectManager->closeProject();
    }

    m_projectManager->closeProject();
    m_projectManager->removeProjectPath(CACHE_DIR + projectName);

    resetMousePointer();
}

void TupMainWindow::setupNetworkProject()
{
    TupConnectDialog *netDialog = new TupConnectDialog(this);
    netDialog->show();

    netDialog->move(static_cast<int> ((screen->geometry().width() - netDialog->width()) / 2),
                    static_cast<int> ((screen->geometry().height() - netDialog->height()) / 2));

    TupNetProjectManagerParams *params = new TupNetProjectManagerParams();

    if (netDialog->exec() == QDialog::Accepted) {
        params->setServer(netDialog->server());
        params->setPort(netDialog->port());
        netUser = netDialog->login();
        params->setLogin(netUser);
        params->setPassword(netDialog->password());

        setupNetworkProject(params);
    }
}

void TupMainWindow::setupNetworkProject(TupProjectManagerParams *params)
{
    if (closeProject()) {
        netProjectManager =  new TupNetProjectManagerHandler;
        connect(netProjectManager, SIGNAL(authenticationSuccessful()), this, SLOT(requestProject()));
        connect(netProjectManager, SIGNAL(openNewArea(const QString &, const QStringList &)), 
                this, SLOT(createNewNetProject(const QString &, const QStringList &)));
        connect(netProjectManager, SIGNAL(updateUsersList(const QString &, int)), this, SLOT(updateUsersOnLine(const QString &, int)));
        connect(netProjectManager, SIGNAL(connectionHasBeenLost()), this, SLOT(unexpectedClose()));
        connect(netProjectManager, SIGNAL(savingSuccessful()), this, SLOT(netProjectSaved()));
        connect(netProjectManager, SIGNAL(postOperationDone()), this, SLOT(resetMousePointer()));

        m_projectManager->setHandler(netProjectManager, true);
        m_projectManager->setParams(params);
        author = params->getAuthor();
    }
}

void TupMainWindow::setupLocalProject(TupProjectManagerParams *params)
{
    #ifdef TUP_DEBUG
        qDebug() << "TupMainWindow::setupLocalProject()";
    #endif

    if (closeProject()) {
        isNetworked = false;
        m_projectManager->setHandler(new TupLocalProjectManagerHandler, false);
        m_projectManager->setParams(params);
        projectName = params->getProjectManager();
        author = params->getAuthor();
        setWindowTitle(appTitle +  " - " + projectName + " [ " + tr("by") + " " + author + " ]");
        kAppProp->setProjectDir(projectName);
    }
}

void TupMainWindow::openProject()
{
    if (cancelChanges())
        return;

    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();

    QString package = QFileDialog::getOpenFileName(this, tr("Open TupiTube project"), path,
                      tr("TupiTube Project Package (*.tup)"));

    if (package.isEmpty() || !package.endsWith(".tup")) 
        return;

    openProject(package);
}

void TupMainWindow::openExample()
{
    if (cancelChanges())
        return;

#ifdef Q_OS_WIN
    QString example = SHARE_DIR + "html/examples/example.tup";
#else
    QString example = SHARE_DIR + "data/html/examples/example.tup";
#endif
    if (QFile::exists(example)) {
        if (m_fileName.compare(example) != 0)
            openProject(example);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "TupMainWindow::openExample() - Fatal Error: Couldn't open example file -> " + QString(example);
        #endif
        TOsd::self()->display(tr("Error"), tr("Cannot open project!"), TOsd::Error);
    }
}

void TupMainWindow::openProject(const QString &path)
{
    #ifdef TUP_DEBUG
        qWarning() << "TupMainWindow::openProject() - Opening project: " + path;
    #endif

    if (path.isEmpty() || !path.endsWith(".tup"))
        return;

    m_projectManager->setHandler(new TupLocalProjectManagerHandler, false);
    isNetworked = false;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    m_actionManager->enable("open_project", false);
    if (closeProject()) {
        setUpdatesEnabled(false);
        tabWidget()->setCurrentWidget(animationTab);

        if (m_projectManager->loadProject(path)) {
            if (QDir::isRelativePath(path))
                m_fileName = QDir::currentPath() + "/" + path;
            else
                m_fileName = path;

            TupMainWindow::requestType = OpenLocalProject;
            projectName = m_projectManager->getProject()->getName();
            // kAppProp->setRasterBgDir(projectName);
            updateRecentProjectList();
            updateOpenRecentMenu(m_recentProjectsMenu, m_recentProjects);

            enableToolViews(true);
            setMenuItemsContext(true);
            setUpdatesEnabled(true);

            m_exposureSheet->updateFramesState();

            author = m_projectManager->getProject()->getAuthor();
            if (author.length() <= 0)
                author = "Anonymous";

            setWindowTitle(appTitle + " - " + projectName + " [ " + tr("by") + " " + author + " ]");
            setWorkSpace();

            m_exposureSheet->updateLayerOpacity(0, 0);
            m_exposureSheet->initLayerVisibility();
            m_timeLine->initLayerVisibility();

            int last = path.lastIndexOf("/");
            QString dir = path.left(last);
            saveDefaultPath(dir);
        } else {
            setUpdatesEnabled(true);
            TOsd::self()->display(tr("Error"), tr("Cannot open project!"), TOsd::Error);
        }
    }

    m_actionManager->enable("open_project", true);
    QApplication::restoreOverrideCursor();
}

void TupMainWindow::updateRecentProjectList()
{
    int pos = m_recentProjects.indexOf(m_fileName);
    if (pos == -1) {
        m_recentProjects.push_front(m_fileName);
        if (m_recentProjects.count() > 5)
            m_recentProjects.removeLast();
    } else {
        m_recentProjects.push_front(m_recentProjects.takeAt(pos));
    }
}

void TupMainWindow::openProjectFromServer()
{
    TupMainWindow::requestType = OpenNetProject;
    setupNetworkProject();
}

void TupMainWindow::importProjectToServer()
{
    TupMainWindow::requestType = ImportProjectToNet;
    setupNetworkProject();
}

void TupMainWindow::preferences()
{
    TupPreferencesDialog *dialog = new TupPreferencesDialog(this);
    dialog->show();

    dialog->move(static_cast<int> ((screen->geometry().width() - dialog->width()) / 2),
                 static_cast<int> ((screen->geometry().height() - dialog->height()) / 2));

    if (dialog->exec() == QDialog::Accepted) {
        if (animationTab)
            animationTab->updateWorkspace();
    }
}

void TupMainWindow::showHelp()
{
    QDesktopServices::openUrl(QString("http://maefloresta.com/wiki"));
}

void TupMainWindow::aboutTupiTube()
{
    TupAbout *about = new TupAbout(this);
    about->show();
}

void TupMainWindow::showTipDialog()
{
    QStringList labels;
    labels << tr("Tip Of The Day") << tr("Previous") << tr("Next") << tr("Close");
    QString videos = QDir::homePath() + "/." + QCoreApplication::applicationName() + "/videos.xml";

    TipDialog *tipDialog = new TipDialog(labels, videos, DATA_DIR + "tips.xml", this);
    tipDialog->show();

    tipDialog->move(static_cast<int> ((screen->geometry().width() - tipDialog->width()) / 2),
                    static_cast<int> ((screen->geometry().height() - tipDialog->height()) / 2));
}

void TupMainWindow::openYouTubeChannel()
{
    QDesktopServices::openUrl(QString("https://www.youtube.com/tupitube"));
}

void TupMainWindow::importPalettes()
{
    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Import Gimp Palettes"), path, tr("Gimp Palette (*.gpl *.txt *.css)"));

    if (files.count() > 0) { 
        QStringList::ConstIterator file = files.begin();
        bool isOk = true;
        while (file != files.end()) {
            TupPaletteImporter importer;
            bool ok = importer.import(*file, TupPaletteImporter::Gimp);
            if (ok) {
                QString home = getenv("HOME");
                QString path = home + "/.tupitube/palettes";
                ok = importer.saveFile(path);
                if (ok) {
                    m_colorPalette->parsePaletteFile(importer.getFilePath());
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "TupMainWindow::importPalettes() - Fatal Error: Couldn't import file -> " + QString(*file);
                    #endif
                    isOk = false;
                }
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "TupMainWindow::importPalettes() - Fatal Error: Couldn't import palette -> " + QString(*file);
                #endif
                isOk = false;
            }
            file++;
        }

        if (isOk) {
            path = files.at(0);
            int last = path.lastIndexOf("/");
            QString dir = path.left(last);
            saveDefaultPath(dir);

            TOsd::self()->display(tr("Information"), tr("Gimp palette import was successful"), TOsd::Info);
        } else {
            TOsd::self()->display(tr("Error"), tr("Gimp palette import was unsuccessful"), TOsd::Error);
        }
    }
}

void TupMainWindow::connectWidgetToManager(QWidget *widget)
{
    connect(widget, SIGNAL(requestTriggered(const TupProjectRequest *)), m_projectManager, 
            SLOT(handleProjectRequest(const TupProjectRequest *)));

    connect(m_projectManager, SIGNAL(responsed(TupProjectResponse*)), widget, 
            SLOT(handleProjectResponse(TupProjectResponse *)));

    // SQA: Pending for revision
    //connect(widget, SIGNAL(postPage(QWidget *)), this, SLOT(addPage(QWidget *)));
}

void TupMainWindow::connectWidgetToLocalManager(QWidget *widget)
{
    connect(widget, SIGNAL(localRequestTriggered(const TupProjectRequest *)),
            m_projectManager, SLOT(handleLocalRequest(const TupProjectRequest *)));
}

void TupMainWindow::disconnectWidgetToManager(QWidget *widget)
{
    disconnect(widget, SIGNAL(requestTriggered(const TupProjectRequest *)), m_projectManager,
            SLOT(handleProjectRequest(const TupProjectRequest *)));

    disconnect(m_projectManager, SIGNAL(responsed(TupProjectResponse*)), widget,
            SLOT(handleProjectResponse(TupProjectResponse *)));
}

void TupMainWindow::connectWidgetToPaintArea(QWidget *widget)
{
    connect(widget, SIGNAL(paintAreaEventTriggered(const TupPaintAreaEvent *)), 
            this, SLOT(createPaintCommand(const TupPaintAreaEvent *)));
}

void TupMainWindow::saveAs()
{
    #ifdef TUP_DEBUG
        qDebug() << "TupMainWindow::saveAs()";
    #endif

    TCONFIG->beginGroup("General");
    QString home = TCONFIG->value("DefaultPath", QDir::homePath()).toString();
    home.append("/" + projectName);
    isSaveDialogOpen = true;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Project As"), home,
                       tr("TupiTube Project Package (*.tup)"));
    if (fileName.isEmpty()) {
        isSaveDialogOpen = false;
        return;
    }

    if (!fileName.endsWith(".tup", Qt::CaseInsensitive))
        fileName += ".tup";

    isSaveDialogOpen = false;
    int indexPath = fileName.lastIndexOf("/");
    int indexFile = fileName.length() - indexPath;
    QString name = fileName.right(indexFile - 1);
    QString path = fileName.left(indexPath + 1);

    QDir directory(path);
    if (!directory.exists()) {
        TOsd::self()->display(tr("Error"), tr("Directory does not exist! Please, choose another path."), TOsd::Error);
        #ifdef TUP_DEBUG
            QString file = path.toLocal8Bit();
            qDebug() << "TupMainWindow::saveAs() - Fatal Error: Directory doesn't exist! -> " + file;
        #endif
        return;
    } else {
        QFile file(directory.filePath(name));
        if (!file.open(QIODevice::ReadWrite)) {
            file.remove();
            TOsd::self()->display(tr("Error"), tr("Insufficient permissions. Please, pick another path."), TOsd::Error);
            return;
        }
        file.remove();
    }

    int dotIndex = name.lastIndexOf(".tup");
    projectName = name.left(dotIndex);

    m_fileName = fileName;

    if (isNetworked) {
        isNetworked = false;
        m_projectManager->setHandler(new TupLocalProjectManagerHandler, false);
        setWindowTitle(appTitle + " - " + projectName + " [ " + tr("by") + " " + author + " ]");
    }

    saveProject();
}

void TupMainWindow::saveProject()
{
    #ifdef TUP_DEBUG
        qDebug() << "TupMainWindow::saveProject()";
    #endif

    if (!isNetworked) {
        if (isSaveDialogOpen)
            return;

        if (m_fileName.isEmpty()) {
            saveAs();
            return;
        }

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        m_actionManager->enable("save_project", false);
        m_actionManager->enable("save_project_as", false);

        if (m_projectManager->saveProject(m_fileName)) {  
            updateRecentProjectList();
            
            TOsd::self()->display(tr("Information"), tr("Project <b>%1</b> saved").arg(projectName));
            // projectSaved = true;
            int indexPath = m_fileName.lastIndexOf("/");
            int indexFile = m_fileName.length() - indexPath;
            QString name = m_fileName.right(indexFile - 1);
            int indexDot = name.lastIndexOf(".");
            name = name.left(indexDot);

            setWindowTitle(appTitle + " - " + name + " [ " + tr("by") +  " " +  author + " ]");

            int last = m_fileName.lastIndexOf("/");
            QString dir = m_fileName.left(last);
            saveDefaultPath(dir);
        } else {
            TOsd::self()->display(tr("Error"), tr("Cannot save the project!"), TOsd::Error);
        }
        m_actionManager->enable("save_project", true);
        m_actionManager->enable("save_project_as", true);
        if (isSaveDialogOpen)
            isSaveDialogOpen = false;
        QApplication::restoreOverrideCursor();
    } else {
        TupSavePackage package(lastSave);
        netProjectManager->sendPackage(package);

        if (!lastSave)
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        else
            lastSave = false;
    }
}

void TupMainWindow::openRecentProject()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        openProject(action->text());
}

// SQA: Check if this method is still used for something
void TupMainWindow::showAnimationMenu(const QPoint &point)
{
    QMenu *menu = new QMenu(tr("Animation"), playerTab);
    menu->addAction(tr("New camera"), this, SLOT(newViewCamera()));
    menu->exec(point);
    delete menu;
}

#if defined(Q_OS_MAC)
bool TupMainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
        openProject(openEvent->file());
        return false;
    }

    return QMainWindow::event(event); 
}
#endif

void TupMainWindow::closeEvent(QCloseEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "TupMainWindow::closeEvent(QCloseEvent)";
    #endif

    if (cancelChanges()) {
        event->ignore();
		return;
    } else {
        QString newsPath = QDir::homePath() + "/." + QCoreApplication::applicationName() + "/twitter.html";
        if (QFile::exists(newsPath)) {
            QFile file(newsPath);
            file.remove();
        }

        TCONFIG->beginGroup("General");
        TCONFIG->setValue("Recents", m_recentProjects);

        resetUI();
        TMainWindow::closeEvent(event);
    }
}

void TupMainWindow::createPaintCommand(const TupPaintAreaEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "TupMainWindow::createPaintCommand()";
    #endif

    if (!animationTab) {
        #ifdef TUP_DEBUG
            qDebug() << "TupMainWindow::createPaintCommand() - No animation tab... aborting!";
        #endif
        return;
    }

    TupPaintAreaCommand *command = animationTab->createPaintCommand(event);

    if (command) { 
        // SQA: Implement Undo procedure for "Color" actions 
        // SQA: Refactor pointer cast
        m_projectManager->createCommand((TupProjectCommand *) command);

        // Updating color on the Pen module interface
        if (event->getAction() == TupPaintAreaEvent::ChangePenColor)
            m_brushWidget->setPenColor(qvariant_cast<QColor>(event->getData()));

        if (event->getAction() == TupPaintAreaEvent::ChangePenThickness)
            m_brushWidget->setPenThickness(qvariant_cast<int>(event->getData()));
    } 
}

void TupMainWindow::updatePenColor(const QColor &color)
{
    #ifdef TUP_DEBUG
        qDebug() << "TupMainWindow::updatePenColor()";
    #endif

    TupPaintAreaEvent *event = new TupPaintAreaEvent(TupPaintAreaEvent::ChangePenColor, color);
    createPaintCommand(event);
}

void TupMainWindow::updatePenThickness(int thickness)
{
    #ifdef TUP_DEBUG
        qDebug() << "TupMainWindow::updatePenThickness()" << thickness;
    #endif

    TupPaintAreaEvent *event = new TupPaintAreaEvent(TupPaintAreaEvent::ChangePenThickness, thickness);
    createPaintCommand(event);
}

void TupMainWindow::addPage(QWidget *widget)
{
    addWidget(widget);
}

void TupMainWindow::updateCurrentTab(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "TupMainWindow::updateCurrentTab()";
    #endif

    if (index == 1) {  // Player mode 
        lastTab = 1;
        updatePlayer();
        cameraWidget->setVisible(true);
        cameraWidget->updateFirstFrame();
        cameraWidget->setFocus();

        TCONFIG->beginGroup("AnimationParameters");
        bool autoPlay = TCONFIG->value("AutoPlay", false).toBool();
        if (autoPlay)
            QTimer::singleShot(0, this, SLOT(doPlay()));
    } else {
        if (index == 0) { // Animation mode
            if (playerTab)
                cameraWidget->setVisible(false);

            animationTab->updatePerspective(); // Just for Papagayo UI
            if (lastTab == 1)
                cameraWidget->doStop();

            /*
            if (scenesView->isExpanded())
                scenesView->expandDock(true);
            */

            if (contextMode != TupProject::FRAMES_MODE) {
                if (exposureView->isExpanded()) {
                    exposureView->expandDock(false);
                    exposureView->enableButton(false);
                } else if (timeView->isExpanded()) {
                    timeView->expandDock(false);
                    timeView->enableButton(false);
                }
            }

            animationTab->updatePaintArea();
            lastTab = 0;
        } else {
            cameraWidget->setVisible(false);
            if (index == 3)
                lastTab = 3;
        }
    }
}

void TupMainWindow::exportProject()
{
    // QDesktopWidget desktop;

    exportWidget = new TupExportWidget(m_projectManager->getProject(), this);
    connect(exportWidget, SIGNAL(isDone()), animationTab, SLOT(updatePaintArea()));
    exportWidget->show();

    exportWidget->move(static_cast<int> ((screen->geometry().width() - exportWidget->width()) / 2),
                       static_cast<int> ((screen->geometry().height() - exportWidget->height()) / 2));

    exportWidget->exec();
}

void TupMainWindow::callSave()
{
    if (m_projectManager->isProjectModified())
        saveProject();
}

void TupMainWindow::restoreFramesMode(TupProject::Mode mode)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupMainWindow::restoreFramesMode()]" << mode << " - currentDock: " << currentDock;
    #endif

    contextMode = mode;
    if (contextMode == TupProject::FRAMES_MODE) {
        if (currentDock == TupDocumentView::ExposureSheet) {
            exposureView->expandDock(true);
        } else if (currentDock == TupDocumentView::TimeLine) {
            timeView->expandDock(true);
        }
        exposureView->enableButton(true);
        timeView->enableButton(true);
        scenesView->enableButton(true);
    } else {
        if (exposureView->isExpanded()) {
            currentDock = TupDocumentView::ExposureSheet;
            exposureView->expandDock(false);
        } else if (timeView->isExpanded()) {
            currentDock = TupDocumentView::TimeLine;
            timeView->expandDock(false);
        }

        if (scenesView->isExpanded()) {
            scenesView->expandDock(false);
        }
        exposureView->enableButton(false);
        timeView->enableButton(false);
        scenesView->enableButton(false);
    }
}

void TupMainWindow::requestProject()
{
    if (TupMainWindow::requestType == NewNetProject) {
        m_projectManager->setupNewProject();
    } else if (TupMainWindow::requestType == OpenNetProject) {
        TupListProjectsPackage package;
        netProjectManager->sendPackage(package);
    } else if (TupMainWindow::requestType == ImportProjectToNet) {
        const char *home = getenv("HOME");
        QString file = QFileDialog::getOpenFileName(this, tr("Import project package"),
                                                   home, tr("TupiTube Project Package (*.tup)"));
        if (file.length() > 0) {
            QFile project(file);
            if (project.exists()) {
                if (project.size() > 0) {
                    TupImportProjectPackage package(file);
                    netProjectManager->sendPackage(package);
                 } else {
                    TOsd::self()->display(tr("Error"), tr("Can't import project. File is empty!"), TOsd::Error);
                    netProjectManager->closeProject();
                 }
            } else {
                 TOsd::self()->display(tr("Error"), tr("Can't save the project. File doesn't exist!"), TOsd::Error);
                 netProjectManager->closeProject();
            }
        } else {
            netProjectManager->closeProject();
        }
    }
}

void TupMainWindow::unexpectedClose()
{
    if (m_projectManager->isOpen())
        resetUI();

    // QDesktopWidget desktop;
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("Fatal Error"));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(tr("The connection to the server has been lost."));
    msgBox.setInformativeText(tr("Please, try to connect again in a while"));
    msgBox.addButton(QString(tr("Close")), QMessageBox::DestructiveRole);

    msgBox.show();
    msgBox.move(static_cast<int> ((screen->geometry().width() - msgBox.width()) / 2),
                 static_cast<int> ((screen->geometry().height() - msgBox.height()) / 2));

    msgBox.exec();
}

void TupMainWindow::netProjectSaved()
{
    m_projectManager->undoModified();
    QApplication::restoreOverrideCursor();
}

void TupMainWindow::updatePlayer(bool removeAction)
{
    #ifdef TUP_DEBUG
        qDebug() << "TupMainWindow::updatePlayer()";
    #endif

    if (!removeAction)
        updatePlayer();
}

void TupMainWindow::updatePlayer()
{
    if (animationTab) {
        int sceneIndex = animationTab->currentSceneIndex();
        cameraWidget->updateScenes(sceneIndex);
    }
}

void TupMainWindow::resetMousePointer()
{
    QApplication::restoreOverrideCursor();
}

void TupMainWindow::updateUsersOnLine(const QString &login, int state)
{
    animationTab->updateUsersOnLine(login, state);
}

void TupMainWindow::resizeProjectDimension(const QSize dimension)
{
    m_projectManager->updateProjectDimension(dimension);
    disconnectWidgetToManager(cameraWidget);
    delete cameraWidget; 
    cameraWidget = new TupCameraWidget(m_projectManager->getProject(), isNetworked);
    connect(cameraWidget, SIGNAL(projectAuthorUpdated(const QString &)), this, SLOT(updateProjectAuthor(const QString &)));
    connectWidgetToManager(cameraWidget);

    playerTab->setCameraWidget(cameraWidget);
}

void TupMainWindow::saveDefaultPath(const QString &dir)
{
    TCONFIG->beginGroup("General");
    TCONFIG->setValue("DefaultPath", dir);
    TCONFIG->sync();
}

void TupMainWindow::showWebMessage()
{
    TMsgDialog *msgDialog = new TMsgDialog(webContent, webMsgSize, isImageMsg, this);
    msgDialog->show();

    msgDialog->move(static_cast<int> ((screen->geometry().width() - msgDialog->width()) / 2),
                    static_cast<int> ((screen->geometry().height() - msgDialog->height()) / 2));
}

void TupMainWindow::setUpdateFlag(bool update)
{
    #ifdef TUP_DEBUG
        qDebug() << "TupMainWindow::setUpdateFlag() - update -> " << update;
    #endif

    TCONFIG->beginGroup("General");
    TCONFIG->setValue("NotifyUpdate", update);
    TCONFIG->sync();
}

void TupMainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();
}

void TupMainWindow::dropEvent(QDropEvent *e)
{
    QList<QUrl> list = e->mimeData()->urls();
    QString project = list.at(0).toLocalFile();

    openProject(project);
}

void TupMainWindow::updateProjectAuthor(const QString &artist)
{
    author = artist;
    setWindowTitle(appTitle +  " - " + projectName + " [ " + tr("by") + " " + author + " ]");
}
