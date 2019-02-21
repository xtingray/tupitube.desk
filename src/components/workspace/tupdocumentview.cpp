/***************************************************************************
 *   Project TUPITUBE DESK                                                 *
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

#include "tupdocumentview.h"
#include "tupapplication.h"
#include "tconfig.h"
#include "tuprequestbuilder.h"
#include "tuppaintareaproperties.h"
#include "tuppluginmanager.h"
#include "tupprojectresponse.h"
#include "tuppaintareaevent.h"
#include "tuppaintareacommand.h"
#include "tupgraphicsscene.h"
#include "tupscene.h"
#include "tuplayer.h"
#include "polylinetool.h"
#include "tupimagedialog.h"
#include "tupstoryboarddialog.h"
#include "tupcamerainterface.h"
#include "tupreflexinterface.h"
#include "tupbasiccamerainterface.h"
#include "tupcameradialog.h"
#include "tuplibrary.h"
#include "tuppapagayoimporter.h"
#include "tuppapagayodialog.h"
#include "tcolorcell.h"
#include "tosd.h"
#include "tupfilterinterface.h"

#include <QDir>
#include <QApplication>
#include <QToolBar>
#include <QPixmap>
#include <QDesktopWidget>
#include <QGridLayout>
#include <QCameraImageCapture>
#include <QCamera>

TupDocumentView::TupDocumentView(TupProject *work, QWidget *parent, bool netFlag, const QStringList &users) :
                                 QMainWindow(parent)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupDocumentView()]";
        #else
            TINIT;
        #endif
    #endif

    setWindowIcon(QPixmap(THEME_DIR + "icons/animation_mode.png"));

    project = work;
    currentTool = NULL;
    onionEnabled = true;
    fullScreenOn = false;
    viewAngle = 0;
    isNetworked = netFlag;
    onLineUsers = users;
    dynamicFlag = false;
    staticFlag = false;

    cameraMode = false;
    photoCounter = 1;
    nodesScaleFactor = 1;

    actionManager = new TActionManager(this);

    actionGroup = new QActionGroup(this);
    actionGroup->setExclusive(true);

    QWidget *workspace = new QWidget;
    QGridLayout *layout = new QGridLayout(workspace);

    horizontalRuler = new TupRuler(Qt::Horizontal, this);
    verticalRuler = new TupRuler(Qt::Vertical, this);
    layout->addWidget(horizontalRuler, 0, 1);
    layout->addWidget(verticalRuler, 1, 0);

    paintArea = new TupPaintArea(project);
    layout->addWidget(paintArea, 1, 1);
    // paintArea->setUseOpenGL(false);

    TCONFIG->beginGroup("OnionParameters");
    opacityFactor = TCONFIG->value("OnionFactor", -1).toDouble();
    if (opacityFactor < 0)
        opacityFactor = 0.5;
    paintArea->setOnionFactor(opacityFactor);

    setCentralWidget(workspace);

    connect(paintArea, SIGNAL(scaled(qreal)), this, SLOT(updateZoomVars(qreal)));
    connect(paintArea, SIGNAL(rotated(int)), this, SLOT(updateRotationVars(int)));
    connect(paintArea, SIGNAL(zoomIn()), this, SLOT(applyZoomIn()));
    connect(paintArea, SIGNAL(zoomOut()), this, SLOT(applyZoomOut()));
    connect(paintArea, SIGNAL(newPerspective(int)), this, SIGNAL(newPerspective(int)));

    // connect(paintArea, SIGNAL(cursorPosition(const QPointF &)), this, SLOT(showPos(const QPointF &)));
    connect(paintArea, SIGNAL(cursorPosition(const QPointF &)), verticalRuler, SLOT(movePointers(const QPointF&)));
    connect(paintArea, SIGNAL(cursorPosition(const QPointF &)), horizontalRuler, SLOT(movePointers(const QPointF&)));
    connect(paintArea, SIGNAL(changedZero(const QPointF&)), this, SLOT(changeRulerOrigin(const QPointF&)));
    connect(paintArea, SIGNAL(requestTriggered(const TupProjectRequest *)), this, SIGNAL(requestTriggered(const TupProjectRequest *)));
    connect(paintArea, SIGNAL(localRequestTriggered(const TupProjectRequest *)), this, SIGNAL(localRequestTriggered(const TupProjectRequest *)));

    setupDrawActions();
    createLateralToolBar();
    createToolBar();

    status = new TupPaintAreaStatus(contourPen(), fillBrush());
    connect(status, SIGNAL(newFramePointer(int)), this, SLOT(goToFrame(int)));
    connect(status, SIGNAL(resetClicked()), this, SLOT(resetWorkSpaceTransformations()));
    connect(status, SIGNAL(safeAreaClicked()), this, SLOT(drawActionSafeArea()));
    connect(status, SIGNAL(gridClicked()), this, SLOT(drawGrid()));
    connect(status, SIGNAL(angleChanged(int)), this, SLOT(setRotationAngle(int)));
    connect(status, SIGNAL(zoomChanged(qreal)), this, SLOT(setZoomFactor(qreal)));
    connect(status, SIGNAL(antialiasChanged(bool)), this, SLOT(setAntialiasing(bool)));
    connect(status, SIGNAL(fullClicked()), this, SLOT(showFullScreen()));

    connect(paintArea, SIGNAL(frameChanged(int)), status, SLOT(updateFrameIndex(int)));
    connect(paintArea, SIGNAL(cursorPosition(const QPointF &)), status, SLOT(showPos(const QPointF &)));
    brushManager()->initBgColor(project->getBgColor());

    connect(brushManager(), SIGNAL(penChanged(const QPen &)), this, SLOT(updatePen(const QPen &)));
    connect(brushManager(), SIGNAL(brushChanged(const QBrush &)), this, SLOT(updateBrush(const QBrush &)));
    connect(brushManager(), SIGNAL(bgColorChanged(const QColor &)), this, SLOT(updateBgColor(const QColor &)));

    setStatusBar(status);

    // SQA: Find out why this timer instruction is required?
    QTimer::singleShot(500, this, SLOT(loadPlugins()));

    // SQA: Temporarily disabled  
    // if (!isNetworked)
    //     saveTimer();

    paintArea->updateLoadingFlag(false);
}

TupDocumentView::~TupDocumentView()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[~TupDocumentView()]";
        #else
            TEND;
        #endif
    #endif

    if (currentTool)
        currentTool->saveConfig();

    if (paintArea) {
        delete paintArea;
        paintArea = NULL;
    }

    if (configurationArea) {
        delete configurationArea;
        configurationArea = NULL;
    }

    // delete k;
}

void TupDocumentView::setWorkSpaceSize(int width, int height)
{
    wsDimension = QSize(width, height);
}

void TupDocumentView::setAntialiasing(bool useIt)
{
    paintArea->setAntialiasing(useIt);
}

void TupDocumentView::goToFrame(int index)
{
    int framesTotal = currentFramesTotal();
    if (index <= framesTotal) {
        paintArea->goToFrame(index - 1);
    } else {
        index = framesTotal;
        paintArea->goToFrame(index -1);
    }

    status->setFramePointer(index);
}

void TupDocumentView::drawGrid()
{
    paintArea->drawGrid(!paintArea->getGridState());
}

void TupDocumentView::drawActionSafeArea()
{
    paintArea->drawActionSafeArea(!paintArea->getSafeAreaState());
}

void TupDocumentView::updateRotationAngleFromRulers(int angle)
{
    viewAngle = angle;

    TupRuler::Transformation flag = TupRuler::None;
    if (angle != 0 && angle != 90 && angle != 180  && angle != 270)
        flag = TupRuler::Rotation;
    else
        flag = TupRuler::None;

    verticalRuler->updateCurrentTransformation(flag);
    horizontalRuler->updateCurrentTransformation(flag);
}

void TupDocumentView::setRotationAngle(int angle)
{
    updateRotationAngleFromRulers(angle);
    paintArea->setRotationAngle(angle);
}

void TupDocumentView::updateRotationVars(int angle)
{
    QString toolName = currentTool->name();
    if (toolName.compare(tr("Object Selection")) == 0 || toolName.compare(tr("Nodes Selection")) == 0 
        || toolName.compare(tr("PolyLine")) == 0)
        currentTool->clearSelection();

    updateRotationAngleFromRulers(angle);
    status->updateRotationAngle(angle);
}

void TupDocumentView::setZoomFactor(qreal factor)
{
    paintArea->setZoom(factor);
    verticalRuler->setRulerZoom(factor);
    horizontalRuler->setRulerZoom(factor);

    updateNodesScale(factor);
}

void TupDocumentView::updateZoomVars(qreal factor)
{
    status->updateZoomFactor(factor);
    verticalRuler->setRulerZoom(factor);
    horizontalRuler->setRulerZoom(factor);

    updateNodesScale(factor);
}

void TupDocumentView::applyZoomIn()
{
    qreal zoom = status->currentZoomFactor();
    if (zoom <= 495) {
        zoom += 5;
        status->setZoomPercent(QString::number(zoom));
    }
}

void TupDocumentView::applyZoomOut()
{
    qreal zoom = status->currentZoomFactor();
    if (zoom >= 15) {
        zoom -= 5;
        status->setZoomPercent(QString::number(zoom));
    }
}

void TupDocumentView::updateNodesScale(qreal factor)
{
    if (currentTool) {
        nodesScaleFactor *= factor;
        QString toolName = currentTool->name();
        if (toolName.compare(tr("Object Selection")) == 0 || toolName.compare(tr("Nodes Selection")) == 0 || 
            toolName.compare(tr("PolyLine")) == 0 || toolName.compare(tr("Position Tween")) == 0 ||
            toolName.compare(tr("Rotation Tween")) == 0)
            currentTool->resizeNodes(1 / nodesScaleFactor);
    }
}

void TupDocumentView::setZoomPercent(const QString &percent)
{
    nodesScaleFactor = percent.toDouble() / 100;
    status->setZoomPercent(percent);
}

/*
void TupDocumentView::showPos(const QPointF &point)
{
    QPoint dot = point.toPoint();
    QString message =  "X: " +  QString::number(dot.x()) + " Y: " + QString::number(dot.y());
    status->updatePosition(message);
}
*/

void TupDocumentView::setupDrawActions()
{
    new TAction(QPixmap(THEME_DIR + "icons/copy.png"), tr("Copy"), QKeySequence(), 
                paintArea, SLOT(copyItems()), actionManager, "copy");

    new TAction(QPixmap(THEME_DIR + "icons/paste.png"), tr("Paste"), QKeySequence(), 
                paintArea, SLOT(pasteItems()), actionManager, "paste");

    new TAction(QPixmap(THEME_DIR + "icons/cut.png"), tr("Cut"), QKeySequence(),
                paintArea, SLOT(cutItems()), actionManager, "cut");

    // new TAction(QPixmap(THEME_DIR + "icons/delete.png"), tr("Delete"), QKeySequence(Qt::Key_Delete), 
    //             paintArea, SLOT(deleteItems()), actionManager, "delete");

    new TAction(QPixmap(THEME_DIR + "icons/delete.png"), tr("Delete"), QKeySequence(),
                paintArea, SLOT(deleteItems()), actionManager, "delete");
   
    /* 
    TAction *group = new TAction(QPixmap(THEME_DIR + "icons/group.png"), tr("&Group"), QKeySequence(tr("Ctrl+G")), 
                                 paintArea, SLOT(groupItems()), actionManager, "group");
    // SQA: Enabled just for initial development
    group->setDisabled(true);

    TAction *ungroup = new TAction(QPixmap(THEME_DIR + "icons/ungroup.png"), tr("&Ungroup"), 
                                    QKeySequence(tr("Ctrl+Shift+G")) , paintArea, SLOT(ungroupItems()),
                                    actionManager, "ungroup");
    // SQA: Enabled just for initial development
    ungroup->setDisabled(true);
    */

    new TAction(QPixmap(THEME_DIR + "icons/layer.png"), tr("Onion Skin"), QKeySequence(Qt::Key_U), 
                this, SLOT(enableOnionFeature()), actionManager, "onion");

    new TAction(QPixmap(THEME_DIR + "icons/onion.png"), tr("Onion Skin Factor"), QKeySequence(tr("Ctrl+Shift+S")), 
                this, SLOT(setDefaultOnionFactor()), actionManager, "onion_factor");

    new TAction(QPixmap(THEME_DIR + "icons/export_frame.png"), tr("Export Frame As Image"), QKeySequence(tr("@")),
                this, SLOT(exportImage()), actionManager, "export_image");

    /*
    new TAction(QPixmap(THEME_DIR + "icons/onion_color.png"), tr("Onion Color"), QKeySequence(),
                          this, SLOT(activeOnionColorScheme()), actionManager, "onion_color");
    */

    TCONFIG->beginGroup("Network");
    QString server = TCONFIG->value("Server").toString();

    if (isNetworked && server.compare("tupitu.be") == 0) {
        new TAction(QPixmap(THEME_DIR + "icons/import_project.png"), tr("Export Frame To Gallery"), QKeySequence(tr("@")),
                    this, SLOT(postImage()), actionManager, "post_image");
    }

    new TAction(QPixmap(THEME_DIR + "icons/storyboard.png"), tr("Storyboard Settings"), QKeySequence(tr("Ctrl+Shift+S")),
                this, SLOT(storyboardSettings()), actionManager, "storyboard");

    #ifdef Q_OS_WIN
        if (QSysInfo::windowsVersion() != QSysInfo::WV_XP) {
            new TAction(QPixmap(THEME_DIR + "icons/camera.png"), tr("Camera"), QKeySequence(tr("Ctrl+Shift+C")),
                        this, SLOT(cameraInterface()), actionManager, "camera");
        }
    #else
        new TAction(QPixmap(THEME_DIR + "icons/camera.png"), tr("Camera"), QKeySequence(tr("Ctrl+Shift+C")),
                    this, SLOT(cameraInterface()), actionManager, "camera");
    #endif

    new TAction(QPixmap(THEME_DIR + "icons/papagayo.png"), tr("Papagayo Lip-sync Files"), QKeySequence(tr("Ctrl+Shift+P")),
                this, SLOT(papagayoManager()), actionManager, "papagayo");
}

void TupDocumentView::createLateralToolBar()
{
    toolbar = new QToolBar(tr("Draw tools"), this);
    toolbar->setIconSize(QSize(16, 16));
    addToolBar(Qt::LeftToolBarArea, toolbar);
    connect(toolbar, SIGNAL(actionTriggered(QAction *)), this, SLOT(selectToolFromMenu(QAction *)));

    // Brushes menu
    shapesMenu = new QMenu(tr("Brushes"), toolbar);
    shapesMenu->setIcon(QPixmap(THEME_DIR + "icons/square.png"));
    connect(shapesMenu, SIGNAL(triggered(QAction *)), this, SLOT(selectToolFromMenu(QAction*)));

    // Motion Tween menu
    motionMenu = new QMenu(tr("Tweening"), toolbar);
    motionMenu->setIcon(QPixmap(THEME_DIR + "icons/position_tween.png"));
    connect(motionMenu, SIGNAL(triggered(QAction *)), this, SLOT(selectToolFromMenu(QAction*)));

    // Misc Tools menu
    miscMenu = new QMenu(tr("Misc Tools"), toolbar);
    miscMenu->setIcon(QPixmap(THEME_DIR + "icons/export_frame.png"));
    // connect(miscMenu, SIGNAL(triggered(QAction *)), this, SLOT(selectToolFromMenu(QAction*)));
}

// SQA: This method must be protected while every new project is being loaded

void TupDocumentView::loadPlugins()
{
    bool imagePluginLoaded = false; 
    foreach (QObject *plugin, TupPluginManager::instance()->getFormats()) {
        if (plugin) {
            TupExportInterface *exporter = qobject_cast<TupExportInterface *>(plugin);
            if (exporter) {
                #ifdef TUP_DEBUG
                    QString msg = "TupDocumentView::loadPlugins() - plugin: " + exporter->key();
                    #ifdef Q_OS_WIN
                        qWarning() << msg;
                    #else
                        tWarning() << msg;
                    #endif
                #endif

                if (exporter->key().compare(tr("Image Sequence")) == 0) {
                    imagePlugin = exporter;
                    imagePluginLoaded = true;
                    break;
                }
            }
        }
    }

    if (!imagePluginLoaded) {
        #ifdef TUP_DEBUG
            QString msg = "TupDocumentView::loadPlugins() - Warning: Couldn't found plugin -> " + tr("Image Sequence");
            #ifdef Q_OS_WIN
                qWarning() << msg;
            #else
                tWarning() << msg;
            #endif
        #endif
    }

    QVector<TAction*> brushTools(3);
    QVector<TAction*> tweenTools(7);

    foreach (QObject *plugin, TupPluginManager::instance()->getTools()) {
             TupToolPlugin *tool = qobject_cast<TupToolPlugin *>(plugin);

             if (tool->toolType() != TupToolInterface::Tweener && tool->toolType() != TupToolInterface::LipSync) {
                 connect(tool, SIGNAL(closeHugeCanvas()), this, SLOT(closeFullScreen()));
                 connect(tool, SIGNAL(callForPlugin(int, int)), this, SLOT(loadPlugin(int, int)));
             }

             QStringList::iterator it;
             QStringList keys = tool->keys();

             for (it = keys.begin(); it != keys.end(); ++it) {
                  #ifdef TUP_DEBUG
                      QString msg = "TupDocumentView::loadPlugins() - Tool Loaded: " + *it;
                      #ifdef Q_OS_WIN
                          qWarning() << msg;
                      #else
                          tWarning() << msg;
                      #endif
                  #endif

                  TAction *action = tool->actions()[*it];

                  if (action) {
                      action->setIconVisibleInMenu(true);
                      connect(action, SIGNAL(triggered()), this, SLOT(selectTool()));
                      action->setParent(plugin);
                      action->setCheckable(true);
                      actionGroup->addAction(action);

                      QString toolName = action->text();

                      switch (tool->toolType()) {
                              case TupToolInterface::Brush:
                                 {
                                   // SQA: Experimental plugin (enable it only for testing)
                                   // if (toolName.compare(tr("Scheme")) == 0)
                                   //     k->schemeAction = action;

                                   if (toolName.compare(tr("Pencil")) == 0)
                                       pencilAction = action;

                                   if (toolName.compare(tr("Ink")) == 0)
                                       inkAction = action;

                                   // SQA: This code has been disabled temporarily
                                   /*
                                   if (toolName.compare(tr("Eraser")) == 0) {
                                       action->setDisabled(true);
                                       brushTools[2] = action;
                                   }
                                   */

                                   if (toolName.compare(tr("PolyLine")) == 0) {
                                       polyLineAction = action;

                                       TupToolPlugin *tool = qobject_cast<TupToolPlugin *>(action->parent());
                                       connect(paintArea, SIGNAL(closePolyLine()), tool, SLOT(initEnv()));
                                       connect(this, SIGNAL(closePolyLine()), tool, SLOT(initEnv()));
                                   }

                                   if (toolName.compare(tr("Line")) == 0) {
                                       brushTools[2] = action;

                                       TupToolPlugin *tool = qobject_cast<TupToolPlugin *>(action->parent());
                                       connect(paintArea, SIGNAL(closeLine()), tool, SLOT(endItem()));
                                       connect(this, SIGNAL(closeLine()), tool, SLOT(endItem()));
                                   }

                                   if (toolName.compare(tr("Rectangle")) == 0) {
                                       brushTools[0] = action;

                                       shapesMenu->setDefaultAction(action);
                                   }

                                   if (toolName.compare(tr("Ellipse")) == 0)
                                       brushTools[1] = action;

                                   // if (toolName.compare(tr("Text")) == 0)
                                   //     textAction = action;
                                 }
                                 break;
                              case TupToolInterface::Tweener:
                                 {
                                   if (toolName.compare(tr("Position Tween")) == 0) {
                                       tweenTools[0] = action;
                                       motionMenu->setDefaultAction(action);
                                   }

                                   if (toolName.compare(tr("Rotation Tween")) == 0)
                                       tweenTools[1] = action;

                                   if (toolName.compare(tr("Scale Tween")) == 0)
                                       tweenTools[2] = action;

                                   if (toolName.compare(tr("Shear Tween")) == 0)
                                       tweenTools[3] = action;

                                   if (toolName.compare(tr("Opacity Tween")) == 0)
                                       tweenTools[4] = action;

                                   if (toolName.compare(tr("Coloring Tween")) == 0)
                                       tweenTools[5] = action;

                                   TupToolPlugin *tool = qobject_cast<TupToolPlugin *>(action->parent());
                                   connect(tool, SIGNAL(tweenRemoved()), this, SLOT(updatePaintArea()));

                                   /*
                                   if (toolName.compare(tr("Composed Tween")) == 0) {
                                       action->setDisabled(true);
                                       tweenTools[6] = action;
                                   }
                                   */
                                 }
                                 break;
                              case TupToolInterface::Selection:
                                 {
                                   if (toolName.compare(tr("Object Selection")) == 0)
                                       selectionAction = action;

                                   if (toolName.compare(tr("Nodes Selection")) == 0)
                                       nodesAction = action;
                                 }
                                 break;
                              case TupToolInterface::Fill:
                                 {
                                   if (toolName.compare(tr("Fill Tool")) == 0)
                                       fillAction = action;
                                 }
                                 break;
                               case TupToolInterface::LipSync:
                                 {
                                   if (toolName.compare(tr("Papagayo Lip-sync")) == 0)
                                       papagayoAction = action;
                                 }
                                 break;
                               default:
                                 break;
                      }
                  } else {
                      #ifdef TUP_DEBUG
                          QString msg = "TupDocumentView::loadPlugins() - Fatal Error: Tool action is NULL -> " + *it;
                          #ifdef Q_OS_WIN
                              qDebug() << msg;
                          #else
                              tError() << msg;
                          #endif
                      #endif
                  }
             }
    } // end foreach

    for (int i = 0; i < brushTools.size(); ++i)
         shapesMenu->addAction(brushTools.at(i));

    // SQA: The Composed Tween is under development. This line is temporaly disabled
    // for (int i = 0; i < tweenTools.size(); ++i)
    for (int i = 0; i < 6; ++i)
         motionMenu->addAction(tweenTools.at(i));

    miscMenu->addAction(actionManager->find("export_image"));

    TCONFIG->beginGroup("Network");
    QString server = TCONFIG->value("Server").toString();

    if (isNetworked && server.compare("tupitu.be") == 0)
        miscMenu->addAction(actionManager->find("post_image"));

    miscMenu->addAction(actionManager->find("storyboard"));
    miscMenu->addAction(actionManager->find("papagayo"));

    foreach (QObject *plugin, TupPluginManager::instance()->getFilters()) {
             AFilterInterface *filterInterface = qobject_cast<AFilterInterface *>(plugin);
             QStringList::iterator it;
             QStringList keys = filterInterface->keys();

             for (it = keys.begin(); it != keys.end(); ++it) {
                  #ifdef TUP_DEBUG
                      QString msg = "TupDocumentView::loadPlugins() - Filter Loaded: " + *it;
                      #ifdef Q_OS_WIN
                          qDebug() << msg;
                      #else
                          tDebug("plugins") << msg;
                      #endif
                  #endif

                  TAction *filter = filterInterface->actions()[*it];
                  if (filter) {
                      connect(filter, SIGNAL(triggered()), this, SLOT(applyFilter()));
                      filterMenu->addAction(filter);
                  }
             }
    }

    toolbar->addAction(pencilAction);
    toolbar->addAction(inkAction);
    // SQA: Enable it only for debugging goals
    // toolbar->addAction(k->schemeAction);
    toolbar->addAction(polyLineAction);

    // SQA: Temporarily disabled
    // toolbar->addAction(textAction);

    toolbar->addSeparator();
    toolbar->addAction(shapesMenu->menuAction());
    toolbar->addSeparator();
    toolbar->addAction(selectionAction);
    toolbar->addAction(nodesAction);
    toolbar->addSeparator();
    toolbar->addAction(fillAction);
    toolbar->addSeparator();
    toolbar->addAction(motionMenu->menuAction());

    #ifdef Q_OS_WIN
        if (QSysInfo::windowsVersion() != QSysInfo::WV_XP) {
            toolbar->addSeparator();
            toolbar->addAction(actionManager->find("camera"));
        }
    #else
        toolbar->addSeparator();
        toolbar->addAction(actionManager->find("camera"));
    #endif

    toolbar->addSeparator();
    toolbar->addAction(miscMenu->menuAction());

    brushTools.clear();
    tweenTools.clear();

    pencilAction->trigger();
    paintArea->setFocus();
}

void TupDocumentView::loadPlugin(int menu, int index)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupDocumentView::loadPlugin()]";
        #else
            T_FUNCINFO;
            tWarning() << "Menu: " << menu;
            tWarning() << "Index: " << index;
        #endif
    #endif

    TAction *action = 0;
    // TColorCell::FillType fillMode;

    switch (menu) {
        case TupToolPlugin::Arrows:
            {
                if (currentDock == ExposureSheet) {
                    if (index == TupToolPlugin::UpArrow) {
                        paintArea->goOneFrameBack();
                    } else if (index == TupToolPlugin::DownArrow) {
                        paintArea->goOneFrameForward();
                    } else if (index == TupToolPlugin::QuickCopyDown) {
                        paintArea->copyFrameForward();
                    } else if (index == TupToolPlugin::DeleteUp) {
                        paintArea->removeCurrentFrame();
                    } else if (index == TupToolPlugin::LeftArrow) {
                        paintArea->goOneLayerBack();
                    } else if (index == TupToolPlugin::RightArrow) {
                        paintArea->goOneLayerForward();
                    } 

                    return;
                }

                if (currentDock == TimeLine) {
                    if (index == TupToolPlugin::LeftArrow) {
                        paintArea->goOneFrameBack();
                    } else if (index == TupToolPlugin::RightArrow) {
                        paintArea->goOneFrameForward();
                    } else if (index == TupToolPlugin::QuickCopyRight) {
                        paintArea->copyFrameForward();
                    } else if (index == TupToolPlugin::DeleteLeft) {
                        paintArea->removeCurrentFrame();
                    } else if (index == TupToolPlugin::UpArrow) {
                        paintArea->goOneLayerBack();
                    } else if (index == TupToolPlugin::DownArrow) {
                        paintArea->goOneLayerForward();
                    }

                    return;
                }
            }
            break;
            case TupToolPlugin::ColorMenu:
                {
                    if (index == TupToolPlugin::ColorTool) {
                        if (fullScreenOn) {
                            QColor currentColor = brushManager()->penColor();
                            emit openColorDialog(currentColor);
                        }
                        return;
                    }
                }
            break;
            case TupToolPlugin::BrushesMenu:
                {
                    QList<QAction*> brushActions = shapesMenu->actions();

                    switch (index) {
                        case TupToolPlugin::PencilTool:
                        {
                            action = pencilAction;
                        }
                        break;
                        case TupToolPlugin::InkTool:
                        {
                            action = inkAction;
                        }
                        break;
                        // SQA: Enable it only for debugging
                        /*
                        case TupToolPlugin::SchemeTool:
                        {
                            action = k->schemeAction;
                        }
                        break;
                        */
                        case TupToolPlugin::PolyLineTool:
                        {
                            action = polyLineAction;
                        }
                        break;
                        case TupToolPlugin::RectangleTool:
                        {
                            action = (TAction *) brushActions[0];
                        }
                        break;
                        case TupToolPlugin::EllipseTool:
                        {
                            action = (TAction *) brushActions[1];
                        }
                        break;
                        case TupToolPlugin::LineTool:
                        {
                            action = (TAction *) brushActions[2];
                        }
                        break;
                    }
                }
            break;
            case TupToolPlugin::SelectionMenu:
                {
                    switch (index) {
                        case TupToolPlugin::Delete:
                        {
                            paintArea->deleteItems();
                        }
                        break;
                        case TupToolPlugin::NodesTool:
                        {
                            action = nodesAction;
                        }
                        break;
                        case TupToolPlugin::ObjectsTool:
                        {
                            action = selectionAction;
                        }
                        break;
                    }
                }
            break;
            case TupToolPlugin::FillMenu:
                {
                    if (index == TupToolPlugin::FillTool)
                        action = fillAction;
                        // fillMode = TColorCell::Inner;
       
                    /*
                    if (index == TupToolPlugin::ContourFill) {
                        action = fillAction;
                        fillMode = TColorCell::Contour;
                    }
                    */
                }
            break;
            /*
            case TupToolPlugin::ZoomMenu:
                {
                    action = k->shiftAction;
                }
            break;
            */
            default:
                {
                    #ifdef TUP_DEBUG
                        QString msg = "TupDocumentView::loadPlugin() - Error: Invalid Menu Index / No plugin loaded";
                        #ifdef Q_OS_WIN
                            qDebug() << msg;
                        #else
                            tError() << msg;
                        #endif
                    #endif
                    return;
                }
            break;
    }

    if (action) {
        if (fullScreenOn) {
            QString toolName = tr("%1").arg(action->text());
            if (toolName.compare(currentTool->name()) != 0) {
                action->trigger();
                fullScreen->updateCursor(action->cursor());
            }

            /*
            if (menu == TupToolPlugin::FillMenu) {
                QString icon = "internal_fill.png";
                if (fillMode == TColorCell::Contour)
                    icon = "line_fill.png";
                else
                    emit fillToolEnabled();

                currentTool->setColorMode(fillMode);
                QCursor cursor = QCursor(kAppProp->themeDir() + "cursors/" + icon, 0, 11);
                fullScreen->updateCursor(cursor);
            }
            */
        }
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupDocumentView::loadPlugin() - Error: Action pointer is NULL!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return;
    }
}

void TupDocumentView::selectTool()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupDocumentView::selectTool()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    TAction *action = qobject_cast<TAction *>(sender());

    if (action) {
        QString toolName = tr("%1").arg(action->text());

        if (currentTool) {
            QString currentName = currentTool->name();
            if (toolName.compare(currentName) == 0)
                return;

            if (currentName.compare(tr("Pencil")) == 0)
                disconnect(currentTool, SIGNAL(penWidthChanged(int)), this, SIGNAL(penWidthChanged(int)));

            if (currentName.compare(tr("Papagayo Lip-sync")) == 0)
                disconnect(currentTool, SIGNAL(importLipSync()), this, SLOT(importPapagayoLipSync()));

            currentTool->saveConfig();
            QWidget *toolConfigurator = currentTool->configurator();
            if (toolConfigurator)
                configurationArea->close();
        }

        TupToolPlugin *tool = qobject_cast<TupToolPlugin *>(action->parent());
        currentTool = tool;
        tool->setName(toolName);
        paintArea->setCurrentTool(toolName);

        if (!action->icon().isNull())
            status->updateTool(toolName, action->icon().pixmap(15, 15));

        int minWidth = 0;

        switch (tool->toolType()) {
                case TupToolInterface::Brush: 
                     status->enableFullScreenFeature(true);
                     if (toolName.compare(tr("Pencil")) == 0 || toolName.compare(tr("PolyLine")) == 0) {
                         minWidth = 130;
                         if (toolName.compare(tr("Pencil")) == 0)
                             connect(currentTool, SIGNAL(penWidthChanged(int)), this, SIGNAL(penWidthChanged(int)));
                     } else if (toolName.compare(tr("Text"))==0) {
                                minWidth = 350;
                     } else { 
                         if (toolName.compare(tr("Rectangle"))==0 || toolName.compare(tr("Ellipse"))==0 || toolName.compare(tr("Line"))==0) { 
                             minWidth = 130;
                             shapesMenu->setDefaultAction(action);
                             shapesMenu->setActiveAction(action);

                             if (!action->icon().isNull())
                                 shapesMenu->menuAction()->setIcon(action->icon());
                         }
                     }
                     /* SQA: Enable it only for debugging
                     if (toolName.compare(tr("Scheme"))==0)
                         minWidth = 130;
                     */
                     break;
                     
                case TupToolInterface::Tweener:
                     status->enableFullScreenFeature(false);
                     minWidth = 220;
                     motionMenu->setDefaultAction(action);
                     motionMenu->setActiveAction(action);
                     if (!action->icon().isNull())
                         motionMenu->menuAction()->setIcon(action->icon());
                     break;
                case TupToolInterface::Fill:
                     {
                         emit fillToolEnabled();

                         QCursor cursor = QCursor(kAppProp->themeDir() + "cursors/internal_fill.png", 0, 11);
                         paintArea->viewport()->setCursor(cursor);
                         status->enableFullScreenFeature(true);

                         currentTool->setColorMode(TColorCell::Inner);
                         fillAction->trigger();
                     }
                     break;
                case TupToolInterface::Selection:
                     status->enableFullScreenFeature(true);
                     if (toolName.compare(tr("Object Selection"))==0) {
                         minWidth = 130;
                         connect(paintArea, SIGNAL(itemAddedOnSelection(TupGraphicsScene *)),
                                 tool, SLOT(initItems(TupGraphicsScene *)));
                     } 
                     break;
                case TupToolInterface::View:
                     status->enableFullScreenFeature(true);

                     if (toolName.compare(tr("Shift"))==0) {
                         tool->setProjectSize(project->getDimension());
                         if (fullScreenOn)
                             tool->setActiveView("FULL_SCREEN");
                         else
                             tool->setActiveView("WORKSPACE");
                     }
                     break;
                case TupToolInterface::LipSync:
                     status->enableFullScreenFeature(false);
                     minWidth = 220;
                     connect(currentTool, SIGNAL(importLipSync()), this, SLOT(importPapagayoLipSync()));

                     miscMenu->setDefaultAction(action);
                     miscMenu->setActiveAction(action);
                     if (!action->icon().isNull())
                         miscMenu->menuAction()->setIcon(action->icon());
                     break;
                default:
                     break;
        }

        QWidget *toolConfigurator = tool->configurator();

        if (toolConfigurator) {
            configurationArea = new TupConfigurationArea(this);
            configurationArea->setConfigurator(toolConfigurator, minWidth);
            addDockWidget(Qt::RightDockWidgetArea, configurationArea);
            toolConfigurator->show();
            if (!configurationArea->isVisible())
                configurationArea->show();
        } else {
            if (configurationArea->isVisible())
                configurationArea->close();
        }

        paintArea->setTool(tool);

        if (tool->toolType() != TupToolInterface::Fill)
            paintArea->viewport()->setCursor(action->cursor());

        if (toolName.compare(tr("Object Selection")) == 0 || toolName.compare(tr("Nodes Selection")) == 0 ||
            toolName.compare(tr("PolyLine")) == 0 || toolName.compare(tr("Position Tween")) == 0 ||
            toolName.compare(tr("Rotation Tween")) == 0)
            tool->updateZoomFactor(1 / nodesScaleFactor);
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupDocumentView::selectTool() - Fatal Error: Action from sender() is NULL";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }
}

void TupDocumentView::selectToolFromMenu(QAction *action)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupDocumentView::selectToolFromMenu()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    QMenu *menu = qobject_cast<QMenu *>(action->parent());
    if (menu) {
        TAction *tool = qobject_cast<TAction *>(menu->activeAction());
        if (tool) {
            if (tool->text().compare(currentTool->name()) == 0)
                return;
            else
                tool->trigger(); // this line calls selectTool()
        } else {
            tool = qobject_cast<TAction *>(menu->defaultAction());
            if (tool) {
                tool->trigger();
            } else {
                #ifdef TUP_DEBUG
                    QString msg = "TupDocumentView::selectToolFromMenu() - Default action is NULL";
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
            }
        }
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupDocumentView::selectToolFromMenu() - Warning: Action with NO parent! Aborting...";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tFatal() << msg;
            #endif
        #endif
    } 
}

bool TupDocumentView::handleProjectResponse(TupProjectResponse *response)
{
    if (TupFrameResponse *frameResponse = static_cast<TupFrameResponse *>(response)) {
        switch (frameResponse->action()) {
            case TupProjectRequest::Add:
                if (cameraMode)
                    QApplication::restoreOverrideCursor();
            break;
            default:
            break;
        }
    }

    return paintArea->handleResponse(response);
}

void TupDocumentView::applyFilter()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        // SQA: Check this code to understand how filters work 
        /*
        SQA issue: Pending to check this code  
        AFilterInterface *aFilter = qobject_cast<AFilterInterface *>(action->parent());
        QString filter = action->text();
        
        TupFrame *frame = paintArea->currentFrame();
        if (frame) {
            aFilter->filter(action->text(), frame->components());
            paintArea->redrawAll();
        }
        */
    }
}

double TupDocumentView::backgroundOpacity(TupFrame::FrameType type)
{
    double opacity = 1.0;
    int sceneIndex = paintArea->currentSceneIndex();
    TupScene *scene = project->sceneAt(sceneIndex);
    if (scene) {
        TupBackground *bg = scene->sceneBackground();
        if (bg) {
            if (type == TupFrame::StaticBg) {
                opacity = bg->staticOpacity();
            } else if (type == TupFrame::DynamicBg) {
                       opacity = bg->dynamicOpacity();
            }
        }
    }

   return opacity;
}

void TupDocumentView::createToolBar()
{
    barGrid = new QToolBar(tr("Paint area actions"), this);
    barGrid->setIconSize(QSize(16, 16));

    staticPropertiesBar = new QToolBar(tr("Static Background Properties"), this);
    dynamicPropertiesBar = new QToolBar(tr("Dynamic Background Properties"), this);

    addToolBar(barGrid);

    spaceModeCombo = new QComboBox();
    spaceModeCombo->addItem(QIcon(THEME_DIR + "icons/frames_mode.png"), tr("Frames Mode"));
    spaceModeCombo->addItem(QIcon(THEME_DIR + "icons/static_background_mode.png"), tr("Static BG Mode"));
    spaceModeCombo->addItem(QIcon(THEME_DIR + "icons/dynamic_background_mode.png"), tr("Dynamic BG Mode"));

    connect(spaceModeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setSpaceContext()));
    setSpaceContext();
    barGrid->addWidget(spaceModeCombo);

    barGrid->addAction(kApp->findGlobalAction("undo"));
    barGrid->addAction(kApp->findGlobalAction("redo"));

    barGrid->addAction(actionManager->find("copy"));
    barGrid->addAction(actionManager->find("paste"));
    barGrid->addAction(actionManager->find("cut"));
    barGrid->addAction(actionManager->find("delete"));

    barGrid->addSeparator();
    QWidget *emptyA = new QWidget();
    emptyA->setFixedWidth(5);
    barGrid->addWidget(emptyA);

    TCONFIG->beginGroup("OnionParameters");
    int preview = TCONFIG->value("PreviousFrames", -1).toInt();
    int next = TCONFIG->value("NextFrames", -1).toInt();

    prevOnionSkinSpin = new QSpinBox(this);
    prevOnionSkinSpin->setToolTip(tr("Previous Frames"));
    connect(prevOnionSkinSpin, SIGNAL(valueChanged(int)), this, SLOT(setPreviousOnionSkin(int)));

    if (preview > 0)
        prevOnionSkinSpin->setValue(preview);
    else
        prevOnionSkinSpin->setValue(1);

    barGrid->addWidget(prevOnionSkinSpin);
    barGrid->addAction(actionManager->find("onion"));

    nextOnionSkinSpin = new QSpinBox(this);
    nextOnionSkinSpin->setToolTip(tr("Next Frames"));
    connect(nextOnionSkinSpin, SIGNAL(valueChanged (int)), this, SLOT(setNextOnionSkin(int)));

    if (next > 0)
        nextOnionSkinSpin->setValue(next);
    else
        nextOnionSkinSpin->setValue(1);

    barGrid->addWidget(nextOnionSkinSpin);
    barGrid->addAction(actionManager->find("onion_factor"));

    onionFactorSpin = new QDoubleSpinBox(this);
    onionFactorSpin->setRange(0.01, 0.99);
    onionFactorSpin->setSingleStep(0.01);
    onionFactorSpin->setValue(opacityFactor);
    onionFactorSpin->setToolTip(tr("Onion Skin Factor"));
    connect(onionFactorSpin, SIGNAL(valueChanged(double)), this, SLOT(setOnionFactor(double)));

    barGrid->addWidget(onionFactorSpin);

    addToolBarBreak();

    QWidget *empty0 = new QWidget();
    empty0->setFixedWidth(5);
    QWidget *empty1 = new QWidget();
    empty1->setFixedWidth(5);

    QLabel *staticOpacityLabel = new QLabel();
    QPixmap staticPix(THEME_DIR + "icons/bg_opacity.png");
    staticOpacityLabel->setToolTip(tr("Static BG Opacity"));
    staticOpacityLabel->setPixmap(staticPix);

    QDoubleSpinBox *staticOpacityBox = new QDoubleSpinBox(this);
    staticOpacityBox->setRange(0.1, 1.0);
    staticOpacityBox->setSingleStep(0.1);
    staticOpacityBox->setValue(backgroundOpacity(TupFrame::StaticBg));
    staticOpacityBox->setToolTip(tr("Static BG Opacity"));
    connect(staticOpacityBox, SIGNAL(valueChanged(double)), this, SLOT(updateStaticOpacity(double)));

    staticPropertiesBar->addWidget(empty0);
    staticPropertiesBar->addWidget(staticOpacityLabel);
    staticPropertiesBar->addWidget(empty1);
    staticPropertiesBar->addWidget(staticOpacityBox);

    staticPropertiesBar->setVisible(false);

    QLabel *dirLabel = new QLabel();
    QPixmap dirPix(THEME_DIR + "icons/mov_orientation.png");
    dirLabel->setToolTip(tr("Movement Orientation"));
    dirLabel->setPixmap(dirPix);

    dirCombo = new QComboBox;
    dirCombo->setToolTip(tr("Movement Orientation"));
    dirCombo->addItem(QIcon(THEME_DIR + "icons/mov_right.png"), "   " + tr("Right"));
    dirCombo->addItem(QIcon(THEME_DIR + "icons/mov_left.png"), "   " + tr("Left"));
    dirCombo->addItem(QIcon(THEME_DIR + "icons/mov_up.png"), "   " + tr("Up"));
    dirCombo->addItem(QIcon(THEME_DIR + "icons/mov_down.png"), "   " + tr("Down"));
    connect(dirCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setBackgroundDirection(int)));

    QWidget *empty2 = new QWidget();
    empty2->setFixedWidth(5);
    QWidget *empty3 = new QWidget();
    empty3->setFixedWidth(5);
    QWidget *empty4 = new QWidget();
    empty4->setFixedWidth(5);
    QWidget *empty5 = new QWidget();
    empty5->setFixedWidth(5);
    QWidget *empty6 = new QWidget();
    empty6->setFixedWidth(5);
    QWidget *empty7 = new QWidget();
    empty7->setFixedWidth(5);
    QWidget *empty8 = new QWidget();
    empty8->setFixedWidth(5);

    QLabel *shiftLabel = new QLabel();
    QPixmap shiftPix(THEME_DIR + "icons/shift_length.png");
    shiftLabel->setToolTip(tr("Shift Length"));
    shiftLabel->setPixmap(shiftPix);

    shiftSpin = new QSpinBox(this);
    shiftSpin->setSingleStep(1);
    shiftSpin->setRange(1, 1000);
    shiftSpin->setToolTip(tr("Shift Length"));
    connect(shiftSpin, SIGNAL(valueChanged(int)), this, SLOT(updateBackgroundShiftProperty(int)));

    QLabel *dynamicOpacityLabel = new QLabel();
    QPixmap dynamicPix(THEME_DIR + "icons/bg_opacity.png");
    dynamicOpacityLabel->setToolTip(tr("Dynamic BG Opacity"));
    dynamicOpacityLabel->setPixmap(dynamicPix);

    QDoubleSpinBox *dynamicOpacityBox = new QDoubleSpinBox(this);
    dynamicOpacityBox->setRange(0.1, 1.0);
    dynamicOpacityBox->setSingleStep(0.1);
    dynamicOpacityBox->setValue(backgroundOpacity(TupFrame::DynamicBg));
    dynamicOpacityBox->setToolTip(tr("Dynamic BG Opacity"));
    connect(dynamicOpacityBox, SIGNAL(valueChanged(double)), this, SLOT(updateDynamicOpacity(double)));

    dynamicPropertiesBar->addWidget(dirLabel);
    dynamicPropertiesBar->addWidget(empty2);
    dynamicPropertiesBar->addWidget(dirCombo);
    dynamicPropertiesBar->addWidget(empty3);
    dynamicPropertiesBar->addSeparator();
    dynamicPropertiesBar->addWidget(empty4);
    dynamicPropertiesBar->addWidget(shiftLabel);
    dynamicPropertiesBar->addWidget(empty5);
    dynamicPropertiesBar->addWidget(shiftSpin);
    dynamicPropertiesBar->addWidget(empty6);
    dynamicPropertiesBar->addSeparator();
    dynamicPropertiesBar->addWidget(empty7);
    dynamicPropertiesBar->addWidget(dynamicOpacityLabel);
    dynamicPropertiesBar->addWidget(empty8);
    dynamicPropertiesBar->addWidget(dynamicOpacityBox);

    dynamicPropertiesBar->setVisible(false);

    addToolBar(staticPropertiesBar);
    addToolBar(dynamicPropertiesBar);
}

void TupDocumentView::closeArea()
{
    if (currentTool)
        currentTool->aboutToChangeTool();

    if (configurationArea->isVisible())
        configurationArea->close();

    paintArea->setScene(0);
    close();
}

void TupDocumentView::undo()
{
    puts("Adding undo support");
}

void TupDocumentView::redo()
{
    puts("Adding redo support");
}

void TupDocumentView::setCursor(const QCursor &cursor)
{
    Q_UNUSED(cursor);
 /*
    paintArea->setCursor(c);
 */
}

void TupDocumentView::setPreviousOnionSkin(int level)
{
    TCONFIG->beginGroup("OnionParameters");
    TCONFIG->setValue("PreviousFrames", level);

    paintArea->setPreviousFramesOnionSkinCount(level);
}

void TupDocumentView::setNextOnionSkin(int level)
{
    TCONFIG->beginGroup("OnionParameters");
    TCONFIG->setValue("NextFrames", level);

    paintArea->setNextFramesOnionSkinCount(level);
}

void TupDocumentView::changeRulerOrigin(const QPointF &zero)
{
    verticalRuler->setOrigin(zero.y());
    horizontalRuler->setOrigin(zero.x());
}

QSize TupDocumentView::sizeHint() const
{
    QSize size(parentWidget()->size());
    return size.expandedTo(QApplication::globalStrut());
}

QSize TupDocumentView::workSpaceSize() const
{
    return paintArea->size();
}

TupBrushManager *TupDocumentView::brushManager() const
{
    return paintArea->brushManager();
}

QPen TupDocumentView::contourPen() const
{
    TupBrushManager *manager = paintArea->brushManager();
    return manager->pen();
}

QBrush TupDocumentView::fillBrush() const
{
    TupBrushManager *manager = paintArea->brushManager();
    return manager->brush();
}

TupPaintAreaCommand *TupDocumentView::createPaintCommand(const TupPaintAreaEvent *event)
{
    TupPaintAreaCommand *command = new TupPaintAreaCommand(paintArea, event);
    return command;
}

void TupDocumentView::updatePaintArea()
{
#ifdef TUP_DEBUG
    #ifdef Q_OS_WIN
        qDebug() << "[TupDocumentView::updatePaintArea()]";
    #else
        T_FUNCINFO;
    #endif
#endif

    paintArea->updatePaintArea();
}

/*
void TupDocumentView::callAutoSave()
{
    emit autoSave();
}

void TupDocumentView::saveTimer()
{
    TCONFIG->beginGroup("General");
    autoSaveTime = TCONFIG->value("AutoSave", 10).toInt();

    timer = new QTimer(this);

    if (autoSaveTime != 0) {
        if (autoSaveTime < 0 || autoSaveTime > 60)
            autoSaveTime = 5;

        int saveTime = autoSaveTime*60000;
        connect(timer, SIGNAL(timeout()), this, SLOT(callAutoSave()));
        timer->start(saveTime);
    }
}
*/

void TupDocumentView::setSpaceContext()
{
    TupProject::Mode mode = TupProject::Mode(spaceModeCombo->currentIndex());
    if (mode == TupProject::FRAMES_EDITION) {
        if (dynamicFlag) {
            dynamicFlag = false;
            renderDynamicBackground();
        }
        project->updateSpaceContext(TupProject::FRAMES_EDITION);
        staticPropertiesBar->setVisible(false);
        dynamicPropertiesBar->setVisible(false);
        motionMenu->setEnabled(true);
    } else if (mode == TupProject::STATIC_BACKGROUND_EDITION) {
        if (dynamicFlag) {
            dynamicFlag = false;
            renderDynamicBackground();
        }
        project->updateSpaceContext(TupProject::STATIC_BACKGROUND_EDITION);
        staticPropertiesBar->setVisible(true);
        dynamicPropertiesBar->setVisible(false);
        motionMenu->setEnabled(false);
    } else if (mode == TupProject::DYNAMIC_BACKGROUND_EDITION) {
        dynamicFlag = true;
        project->updateSpaceContext(TupProject::DYNAMIC_BACKGROUND_EDITION);

        int sceneIndex = paintArea->currentSceneIndex();
        TupScene *scene = project->sceneAt(sceneIndex);
        if (scene) {
            TupBackground *bg = scene->sceneBackground();
            if (bg) {
                int direction = bg->dyanmicDirection();
                dirCombo->setCurrentIndex(direction);
                int shift = bg->dyanmicShift();
                shiftSpin->setValue(shift);
            }
        }
        staticPropertiesBar->setVisible(false);
        dynamicPropertiesBar->setVisible(true);
        motionMenu->setEnabled(false);
    }

    paintArea->updateSpaceContext();
    paintArea->updatePaintArea();

   if (currentTool) {
       currentTool->init(paintArea->graphicsScene());
       if (((currentTool->toolType() == TupToolInterface::Tweener) || (currentTool->toolType() == TupToolInterface::LipSync))
           && (mode != TupProject::FRAMES_EDITION)) {
           pencilAction->trigger();
       }
   }

   emit modeHasChanged(mode);
}

TupProject::Mode TupDocumentView::spaceContext()
{
    return TupProject::Mode(spaceModeCombo->currentIndex());
}

TupProject *TupDocumentView::currentProject()
{
    return project;
}

int TupDocumentView::currentFramesTotal()
{
    int sceneIndex = paintArea->graphicsScene()->currentSceneIndex();
    int layerIndex = paintArea->graphicsScene()->currentLayerIndex();

    TupScene *scene = project->sceneAt(sceneIndex);
    if (scene) {
        TupLayer *layer = scene->layerAt(layerIndex);
        if (layer)
            return layer->framesCount();
    }

    return -1;
}

int TupDocumentView::currentSceneIndex()
{
    if (paintArea)
        return paintArea->graphicsScene()->currentSceneIndex();
   
    return -1; 
}

void TupDocumentView::updateBgColor(const QColor color)
{
   if (!isNetworked) {
       project->setBgColor(color);
       paintArea->setBgColor(color);
       emit bgColorChanged(color);
   } else {
       TupProjectRequest event = TupRequestBuilder::createSceneRequest(currentSceneIndex(), TupProjectRequest::BgColor, color.name());
       emit requestTriggered(&event);
   }
}

void TupDocumentView::enableOnionFeature()
{
    if (!onionEnabled) {
        if (prevOnionValue == 0)
            prevOnionSkinSpin->setValue(1);
        else
            prevOnionSkinSpin->setValue(prevOnionValue);

        if (nextOnionValue == 0)
            nextOnionSkinSpin->setValue(1);
        else
            nextOnionSkinSpin->setValue(nextOnionValue);

        onionEnabled = true;
    } else {
        prevOnionValue = prevOnionSkinSpin->value();
        nextOnionValue = nextOnionSkinSpin->value();
        prevOnionSkinSpin->setValue(0);
        nextOnionSkinSpin->setValue(0);
        onionEnabled = false;
    }

    paintArea->updatePaintArea();
}

void TupDocumentView::setDefaultOnionFactor()
{
    onionFactorSpin->setValue(0.5);
    setOnionFactor(0.5);
}

void TupDocumentView::setOnionFactor(double opacity)
{
    TCONFIG->beginGroup("OnionParameters");
    TCONFIG->setValue("OnionFactor", QString::number(opacity, 'f', 2));

    paintArea->setOnionFactor(opacity);
}

void TupDocumentView::showFullScreen()
{
    if (fullScreenOn || currentTool->toolType() == TupToolInterface::Tweener
        || currentTool->toolType() == TupToolInterface::LipSync)
        return;

    fullScreenOn = true;

    QDesktopWidget desktop;
    int screenW = desktop.screenGeometry().width();
    int screenH = desktop.screenGeometry().height();

    cacheScaleFactor = nodesScaleFactor;
    qreal scaleFactor = 1;

    QSize projectSize = project->getDimension();
    if (projectSize.width() < projectSize.height())
        scaleFactor = (double) (screenW - 50) / (double) projectSize.width();
    else
        scaleFactor = (double) (screenH - 50) / (double) projectSize.height();

    fullScreen = new TupCanvas(this, Qt::Window|Qt::FramelessWindowHint, paintArea->graphicsScene(),
                                  paintArea->getCenterPoint(), QSize(screenW, screenH), project, scaleFactor,
                                  viewAngle, brushManager());

    fullScreen->updateCursor(currentTool->cursor());

    QString toolName = currentTool->name();
    if (toolName.compare(tr("Shift")) == 0)
        currentTool->setActiveView("FULL_SCREEN");

    nodesScaleFactor = 1;
    updateNodesScale(scaleFactor);

    connect(this, SIGNAL(openColorDialog(const QColor &)), fullScreen, SLOT(colorDialog(const QColor &)));
    connect(fullScreen, SIGNAL(colorChangedFromFullScreen(const QColor &)), this, SIGNAL(colorChangedFromFullScreen(const QColor &)));
    connect(fullScreen, SIGNAL(penWidthChangedFromFullScreen(int)), this, SIGNAL(penWidthChanged(int)));
    connect(fullScreen, SIGNAL(onionOpacityChangedFromFullScreen(double)), this, SLOT(updateOnionOpacity(double)));
    connect(fullScreen, SIGNAL(zoomFactorChangedFromFullScreen(qreal)), this, SLOT(updateNodesScale(qreal)));
    connect(fullScreen, SIGNAL(callAction(int, int)), this, SLOT(loadPlugin(int, int)));
    connect(fullScreen, SIGNAL(requestTriggered(const TupProjectRequest *)), this, SIGNAL(requestTriggered(const TupProjectRequest *)));
    connect(fullScreen, SIGNAL(localRequestTriggered(const TupProjectRequest *)), this, SIGNAL(localRequestTriggered(const TupProjectRequest *)));
    connect(fullScreen, SIGNAL(rightClick()), this, SLOT(fullScreenRightClick()));
    connect(fullScreen, SIGNAL(rightClick()), this, SLOT(fullScreenRightClick()));
    connect(fullScreen, SIGNAL(goToFrame(int, int, int)), this, SLOT(selectFrame(int, int, int)));
    connect(fullScreen, SIGNAL(closeHugeCanvas()), this, SLOT(closeFullScreen()));

    if (toolName.compare(tr("Object Selection")) == 0)
        fullScreen->enableRubberBand();

    fullScreen->showFullScreen();
}

/*
void TupDocumentView::updatePenThickness(int size) 
{
    QPen pen = brushManager()->pen();
    pen.setWidth(size);
    emit updatePenFromFullScreen(pen);
}
*/

void TupDocumentView::updateOnionOpacity(double opacity)
{
    paintArea->setOnionFactor(opacity);
    onionFactorSpin->setValue(opacity);
}

void TupDocumentView::closeFullScreen()
{
    if (fullScreenOn) {
        disconnect(this, SIGNAL(openColorDialog(const QColor &)), fullScreen, SLOT(colorDialog(const QColor &)));
        disconnect(fullScreen, SIGNAL(colorChangedFromFullScreen(const QColor &)), this, SIGNAL(colorChangedFromFullScreen(const QColor &)));
        disconnect(fullScreen, SIGNAL(penWidthChangedFromFullScreen(int)), this, SIGNAL(penWidthChanged(int)));
        disconnect(fullScreen, SIGNAL(onionOpacityChangedFromFullScreen(double)), this, SLOT(updateOnionOpacity(double)));
        disconnect(fullScreen, SIGNAL(zoomFactorChangedFromFullScreen(qreal)), this, SLOT(updateNodesScale(qreal)));
        disconnect(fullScreen, SIGNAL(callAction(int, int)), this, SLOT(loadPlugin(int, int)));
        disconnect(fullScreen, SIGNAL(requestTriggered(const TupProjectRequest *)), this, SIGNAL(requestTriggered(const TupProjectRequest *)));
        disconnect(fullScreen, SIGNAL(localRequestTriggered(const TupProjectRequest *)), this, SIGNAL(localRequestTriggered(const TupProjectRequest *)));
        disconnect(fullScreen, SIGNAL(rightClick()), this, SLOT(fullScreenRightClick()));
        disconnect(fullScreen, SIGNAL(rightClick()), this, SLOT(fullScreenRightClick()));
        disconnect(fullScreen, SIGNAL(goToFrame(int, int, int)), this, SLOT(selectFrame(int, int, int)));
        disconnect(fullScreen, SIGNAL(closeHugeCanvas()), this, SLOT(closeFullScreen()));

        fullScreen->close();
        fullScreenOn = false;
        currentTool->init(paintArea->graphicsScene());

        fullScreen = 0;

        QString toolName = currentTool->name();
        if (toolName.compare(tr("Shift")) == 0) 
            currentTool->setActiveView("WORKSPACE");

        nodesScaleFactor = cacheScaleFactor;
        updateNodesScale(1);
    }
}

void TupDocumentView::selectFrame(int frame, int layer, int scene)
{
    paintArea->goToFrame(frame, layer, scene);
}

void TupDocumentView::selectScene(int scene)
{
    paintArea->goToScene(scene);
}

void TupDocumentView::exportImage()
{
    int sceneIndex = paintArea->graphicsScene()->currentSceneIndex();
    int frameIndex = paintArea->graphicsScene()->currentFrameIndex();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Frame As"), QDir::homePath(),
                                                        tr("Images") + " (*.png *.jpg *.svg)");
    if (!fileName.isNull()) {
        bool isOk = imagePlugin->exportFrame(frameIndex, project->getBgColor(), fileName, project->sceneAt(sceneIndex),
                                                project->getDimension(), project->getLibrary());
        updatePaintArea();
        if (isOk)
            TOsd::self()->display(tr("Information"), tr("Frame has been exported successfully"));
        else
            TOsd::self()->display(tr("Error"), tr("Can't export frame as image"), TOsd::Error);
    }
}

void TupDocumentView::postImage()
{
    int sceneIndex = paintArea->graphicsScene()->currentSceneIndex();
    int frameIndex = paintArea->graphicsScene()->currentFrameIndex();

    TupImageDialog *dialog = new TupImageDialog(this);
    dialog->show();
    QDesktopWidget desktop;
    dialog->move((int) (desktop.screenGeometry().width() - dialog->width())/2 ,
                 (int) (desktop.screenGeometry().height() - dialog->height())/2);

    if (dialog->exec() != QDialog::Rejected) {
        QString title = dialog->imageTitle();
        QString topics = dialog->imageTopics();
        QString description = dialog->imageDescription();
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        emit requestExportImageToServer(frameIndex, sceneIndex, title, topics, description);
    }
}

void TupDocumentView::storyboardSettings()
{
    QDesktopWidget desktop;
    int sceneIndex = paintArea->graphicsScene()->currentSceneIndex();

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    TupStoryBoardDialog *storySettings = new TupStoryBoardDialog(isNetworked, imagePlugin, project->getBgColor(), project->getDimension(),
                                                                 project->sceneAt(sceneIndex), currentSceneIndex(), project->getLibrary(), this);
    connect(storySettings, SIGNAL(updateStoryboard(TupStoryboard *, int)), this, SLOT(sendStoryboard(TupStoryboard *, int)));

    if (isNetworked)
        connect(storySettings, SIGNAL(postStoryboard(int)), this, SIGNAL(postStoryboard(int)));

    QApplication::restoreOverrideCursor();

    storySettings->show();
    storySettings->move((int) (desktop.screenGeometry().width() - storySettings->width())/2 ,
                        (int) (desktop.screenGeometry().height() - storySettings->height())/2);
}

void TupDocumentView::sendStoryboard(TupStoryboard *storyboard, int sceneIndex)
{
    if (isNetworked) {
        #ifdef TUP_DEBUG
            QString msg = "TupDocumentView::sendStoryboard() - Sending storyboard...";
            #ifdef Q_OS_WIN
                qWarning() << msg;
            #else
                tWarning() << msg;
            #endif
        #endif
        emit updateStoryboard(storyboard, sceneIndex);
    } else {
        project->sceneAt(sceneIndex)->setStoryboard(storyboard);
    }
}

void TupDocumentView::updateUsersOnLine(const QString &login, int state)
{
    if (state == 1) {
        onLineUsers << login;
    } else {
        int index = onLineUsers.indexOf(login);
        onLineUsers.removeAt(index);
    }

    // if (fullScreenOn)
    //     fullScreen->updateOnLineUsers(onLineUsers);
}

// SQA: This method must support multi-user notifications (pending)
void TupDocumentView::updateStaticOpacity(double opacity)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupDocumentView::updateStaticOpacity()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    int sceneIndex = paintArea->currentSceneIndex();
    TupScene *scene = project->sceneAt(sceneIndex);
    if (scene) {
        TupBackground *bg = scene->sceneBackground();
        if (bg) {
            bg->setStaticOpacity(opacity);
            TupProject::Mode mode = TupProject::Mode(spaceModeCombo->currentIndex());
            if (mode == TupProject::FRAMES_EDITION || mode == TupProject::STATIC_BACKGROUND_EDITION)
                paintArea->updatePaintArea();
        }
    }
}

// SQA: This method must support multi-user notifications (pending)
void TupDocumentView::updateDynamicOpacity(double opacity)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupDocumentView::updateDynamicOpacity()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

   int sceneIndex = paintArea->currentSceneIndex();
   TupScene *scene = project->sceneAt(sceneIndex);
   if (scene) {
       TupBackground *bg = scene->sceneBackground();
       if (bg) {
           bg->setDynamicOpacity(opacity);
           paintArea->updatePaintArea();
       }
   }
}

// SQA: This method must support multi-user notifications (pending)
void TupDocumentView::setBackgroundDirection(int direction)
{
   int sceneIndex = paintArea->currentSceneIndex();
   TupScene *scene = project->sceneAt(sceneIndex);
   if (scene) {
       TupBackground *bg = scene->sceneBackground();
       if (bg)
           bg->setDynamicDirection(direction);
   }
}

// SQA: This method must support multi-user notifications (pending)
void TupDocumentView::updateBackgroundShiftProperty(int shift)
{
   int sceneIndex = paintArea->currentSceneIndex();
   TupScene *scene = project->sceneAt(sceneIndex);
   if (scene) {
       TupBackground *bg = scene->sceneBackground();
       if (bg)
           bg->setDynamicShift(shift);
   }
}

void TupDocumentView::renderDynamicBackground()
{
   int sceneIndex = paintArea->currentSceneIndex();
   TupScene *scene = project->sceneAt(sceneIndex);

   if (scene) {
       TupBackground *bg = scene->sceneBackground();
       if (bg)
           bg->renderDynamicView();
   }
}

void TupDocumentView::fullScreenRightClick()
{
   if (currentTool->name().compare(tr("PolyLine")) == 0)
       emit closePolyLine();

   if (currentTool->name().compare(tr("Line")) == 0)
       emit closeLine();
}

void TupDocumentView::cameraInterface()
{
    if (cameraMode) {
        TOsd::self()->display(tr("Warning"), tr("Please, close current camera dialog first!"), TOsd::Error);
        return;
    }

    int camerasTotal = QCamera::availableDevices().count();
    if (camerasTotal > 0) {
        QList<QByteArray> cameraDevices;
        QComboBox *devicesCombo = new QComboBox;
        foreach(const QByteArray &deviceName, QCamera::availableDevices()) {
            QCamera *device = new QCamera(deviceName);
            QString description = device->deviceDescription(deviceName);
            bool found = false;
            for (int i=0; i<devicesCombo->count(); i++) {
                QString item = devicesCombo->itemText(i);
                if (item.compare(description) == 0) {
                    found = true;
                         break;
                }
            }
            if (!found) {
                devicesCombo->addItem(description);
                cameraDevices << deviceName;
            }
        }

        /* SQA: This lines should be enabled in some point at the future
        QByteArray cameraDevice = cameraDevices[0];
        QCamera *camera = new QCamera(cameraDevice);
        camera->load();
        tError() << "TupDocumentView::cameraInterface() - Camera status: " << camera->status();
        QCameraImageCapture *imageCapture = new QCameraImageCapture(camera);
        QList<QSize> resolutions = imageCapture->supportedResolutions();
        */

        QList<QSize> resolutions;    
        resolutions << QSize(1920, 1080);
        resolutions << QSize(1280, 1024);
        resolutions << QSize(1280, 960);
        resolutions << QSize(1224, 768);
        resolutions << QSize(800, 600);
        resolutions << QSize(640, 480);

        /* SQA: Check if this resolutions have any sense
        resolutions << QSize(352, 288);
        resolutions << QSize(320, 240);
        resolutions << QSize(176, 144);
        resolutions << QSize(160, 120);
        */

        QDesktopWidget desktop;
        QSize projectSize = project->getDimension();

        TupCameraDialog *cameraDialog = new TupCameraDialog(devicesCombo, projectSize, resolutions);
        cameraDialog->show();
        cameraDialog->move(static_cast<int> (desktop.screenGeometry().width() - cameraDialog->width())/2,
                           static_cast<int> (desktop.screenGeometry().height() - cameraDialog->height())/2);

        if (cameraDialog->exec() == QDialog::Accepted) {
            cameraMode = true;
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

            cameraSize = cameraDialog->cameraResolution();
            QString resolution = QString::number(cameraSize.width()) + "x" + QString::number(cameraSize.height());

            if (cameraDialog->changeProjectSize()) {
                if (cameraSize != projectSize)
                    resizeProjectDimension(cameraSize);
            } 

            if (cameraDialog->isWebcam()) {
                if (cameraDialog->useBasicCamera()) {
                    TupBasicCameraInterface *dialog = new TupBasicCameraInterface(resolution, cameraDevices, devicesCombo, cameraDialog->cameraIndex(), 
                                                                                  cameraSize, photoCounter);

                    connect(dialog, SIGNAL(pictureHasBeenSelected(int, const QString)), this, SLOT(insertPictureInFrame(int, const QString)));
                    connect(dialog, SIGNAL(closed()), this, SLOT(updateCameraMode())); 
                    dialog->show();
                    dialog->move(static_cast<int> (desktop.screenGeometry().width() - dialog->width())/2 ,
                                 static_cast<int> (desktop.screenGeometry().height() - dialog->height())/2);
                } else {
                    TupCameraInterface *dialog = new TupCameraInterface(resolution, cameraDevices, devicesCombo, cameraDialog->cameraIndex(),
                                                                        cameraSize, photoCounter);

                    connect(dialog, SIGNAL(pictureHasBeenSelected(int, const QString)), this, SLOT(insertPictureInFrame(int, const QString)));
                    connect(dialog, SIGNAL(closed()), this, SLOT(updateCameraMode())); 
                    dialog->show();
                    dialog->move(static_cast<int> (desktop.screenGeometry().width() - dialog->width())/2 ,
                                 static_cast<int> (desktop.screenGeometry().height() - dialog->height())/2);
                }
            } else { // UI for reflex cameras
                int index = cameraDialog->cameraIndex();
                TupReflexInterface *dialog = new TupReflexInterface(devicesCombo->itemText(index), resolution, cameraDevices.at(index),
                                                                    cameraSize, photoCounter);

                connect(dialog, SIGNAL(pictureHasBeenSelected(int, const QString)), this, SLOT(insertPictureInFrame(int, const QString)));
                connect(dialog, SIGNAL(closed()), this, SLOT(updateCameraMode())); 
                dialog->show();
                dialog->move(static_cast<int> (desktop.screenGeometry().width() - dialog->width())/2 ,
                             static_cast<int> (desktop.screenGeometry().height() - dialog->height())/2);
            }

            QApplication::restoreOverrideCursor();
        }
    } else {
        // No devices connected!
        TOsd::self()->display(tr("Error"), tr("No cameras detected"), TOsd::Error);
    }
}

void TupDocumentView::resizeProjectDimension(const QSize dimension)
{
#ifdef TUP_DEBUG
    #ifdef Q_OS_WIN
        qDebug() << "[TupMainWindow::resizeProjectDimension(QSize]";
    #else
        T_FUNCINFO << dimension;
    #endif
#endif

    paintArea->updateDimension(dimension);

    int width = wsDimension.width();
    int height = wsDimension.height();
    int pWidth = dimension.width();
    int pHeight = dimension.height();

    double proportion = 1;

    if (pWidth > pHeight)
        proportion = static_cast<double> (width) / static_cast<double> (pWidth);
    else
        proportion = static_cast<double> (height) / static_cast<double> (pHeight);

    if (proportion <= 0.5) {
        setZoomPercent("20");
    } else if (proportion > 0.5 && proportion <= 0.75) {
               setZoomPercent("25");
    } else if (proportion > 0.75 && proportion <= 1.5) {
               setZoomPercent("50");
    } else if (proportion > 1.5 && proportion < 2) {
               setZoomPercent("75");
    }

    emit projectSizeHasChanged(dimension);
    paintArea->updatePaintArea();
}

void TupDocumentView::insertPictureInFrame(int id, const QString path)
{
    // SQA: This is a hack - remember to check the QImageEncoderSettings issue 
    QImage pixmap(path); 
    if (pixmap.size() != cameraSize) {
        int height = pixmap.height();
        int width = (cameraSize.width() * height) / cameraSize.height();
        int posX = (pixmap.width() - width)/2;
        int posY = 0;
        if (width > pixmap.width()) {
            width = pixmap.width();
            height = (cameraSize.height() * width) / cameraSize.width();
            posX = 0;
            posY = (pixmap.height() - height)/2;
        }
        QImage mask = pixmap.copy(posX, posY, width, height);
        QImage resized = mask.scaledToWidth(cameraSize.width(), Qt::SmoothTransformation);
        resized.save(path, "JPG", 100);
    } 

    QFile imgFile(path);
    QFileInfo fileInfo(imgFile);
    QString key = fileInfo.fileName().toLower();

    if (imgFile.open(QIODevice::ReadOnly)) {
        if (id > 1) {
            int layerIndex = paintArea->currentLayerIndex();
            int frameIndex = paintArea->currentFrameIndex() + 1;

            TupProjectRequest request = TupRequestBuilder::createFrameRequest(paintArea->currentSceneIndex(), layerIndex,
                                                                              frameIndex, TupProjectRequest::Add, tr("Frame"));
            emit requestTriggered(&request);

            QString selection = QString::number(layerIndex) + "," + QString::number(layerIndex) + ","
                                + QString::number(frameIndex) + "," + QString::number(frameIndex);

            request = TupRequestBuilder::createFrameRequest(paintArea->currentSceneIndex(),
                                                            layerIndex, frameIndex,
                                                            TupProjectRequest::Select, selection);
            emit requestTriggered(&request);
        } 

        QByteArray data = imgFile.readAll();
        imgFile.close();

        TupLibrary *library = project->getLibrary();
        while(library->exists(key)) {
              id++;
              QString prev = "pic";
              if (id < 10)
                  prev += "00";
              if (id >= 10 && id < 100)
                  prev += "0";
              key = prev + QString::number(id) + ".jpg";
        }

        TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, key,
                                                                            TupLibraryObject::Image, project->spaceContext(), data, QString(),
                                                                            paintArea->currentSceneIndex(), paintArea->currentLayerIndex(),
                                                                            paintArea->currentFrameIndex());
        emit requestTriggered(&request);

        photoCounter = id + 1;
    }
}

void TupDocumentView::importPapagayoLipSync()
{
    TupPapagayoDialog *dialog = new TupPapagayoDialog();
    dialog->show();

    if (dialog->exec() != QDialog::Rejected) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        QString file = dialog->getPGOFile();
        QFileInfo info(file);
        QString folder = info.fileName().toLower();

        int sceneIndex = paintArea->currentSceneIndex();
        TupScene *scene = project->sceneAt(sceneIndex);
        if (scene->lipSyncExists(folder)) {
            TOsd::self()->display(tr("Error"), tr("Papagayo project already exists!\nPlease, rename the project's file"), TOsd::Error);
            #ifdef TUP_DEBUG
                   QString msg = "TupDocumentView::importPapagayoLipSync() - Fatal Error: Papagayo file is invalid!";
                   #ifdef Q_OS_WIN
                       qDebug() << msg;
                   #else
                       tError() << msg;
                   #endif
            #endif
            QApplication::restoreOverrideCursor();
            return;
        }

        QString imagesDir = dialog->getImagesFile();
        QFile projectFile(file);
        if (projectFile.exists()) {
            if (projectFile.size() > 0) {
                QDir dir(imagesDir);
                QStringList imagesList = dir.entryList(QStringList() << "*.png" << "*.jpg" << "*.jpeg" << "*.gif" << "*.svg");
                if (imagesList.count() > 0) {
                    QString extension = ".svg";
                    QString firstImage = imagesList.at(0);
                    if (!firstImage.endsWith(".svg")) {
                        int dot = firstImage.lastIndexOf(".");
                        extension = firstImage.mid(dot);
                    }

                    int currentIndex = paintArea->currentFrameIndex();
                    TupPapagayoImporter *parser = new TupPapagayoImporter(file, project->getDimension(), extension, currentIndex);
                    if (parser->fileIsValid()) {
                        int layerIndex = paintArea->currentLayerIndex();
                        QString mouthPath = imagesDir;
                        QDir mouthDir = QDir(mouthPath);

                        // Creating Papagayo folder in the library
                        TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, folder, TupLibraryObject::Folder);
                        emit requestTriggered(&request);

                        // Adding mouth images in the library
                        foreach (QString fileName, imagesList) {
                            QString key = fileName.toLower();
                            QFile f(mouthPath + "/" + fileName);
                            if (f.open(QIODevice::ReadOnly)) {
                                QByteArray data = f.readAll();
                                f.close();
                                request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, key, TupLibraryObject::Image, project->spaceContext(), data, folder,
                                                                                  sceneIndex, layerIndex, currentIndex);
                                emit requestTriggered(&request);
                            }
                        }

                        // Adding lip-sync sound file
                        QString soundFile = dialog->getSoundFile();
                        QFile f(soundFile);
                        QFileInfo info(soundFile);
                        QString soundKey = info.fileName().toLower();

                        if (f.open(QIODevice::ReadOnly)) {
                            QByteArray data = f.readAll();
                            f.close();
                            request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, soundKey, TupLibraryObject::Sound, project->spaceContext(), data, folder,
                                                                              sceneIndex, layerIndex, currentIndex);
                            emit requestTriggered(&request);
                        }

                        // Adding Papagayo project
                        parser->setSoundFile(soundKey);
                        QString xml = parser->file2Text();

                        request = TupRequestBuilder::createLayerRequest(sceneIndex, layerIndex, TupProjectRequest::AddLipSync, xml);
                        emit requestTriggered(&request);

                        // Adding frames if they are required
                        TupScene *scene = project->sceneAt(sceneIndex);
                        if (scene) {
                            int sceneFrames = scene->framesCount();
                            int lipSyncFrames = currentIndex + parser->getFrameCount();

                            if (lipSyncFrames > sceneFrames) {
                                int layersCount = scene->layersCount();
                                for (int i = sceneFrames; i < lipSyncFrames; i++) {
                                     for (int j = 0; j < layersCount; j++) {
                                          request = TupRequestBuilder::createFrameRequest(sceneIndex, j, i, TupProjectRequest::Add, tr("Frame"));
                                          emit requestTriggered(&request);
                                     }
                                }

                                QString selection = QString::number(layerIndex) + "," + QString::number(layerIndex) + ","
                                                    + QString::number(currentIndex) + "," + QString::number(currentIndex);

                                request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, currentIndex, TupProjectRequest::Select, selection);
                                emit requestTriggered(&request);
                            }
                        }

                        if (currentTool->name().compare(tr("Papagayo Lip-sync")) != 0)
                            papagayoAction->trigger();

                        emit updateFPS(parser->getFps()); 

                        TOsd::self()->display(tr("Information"), tr("Papagayo file has been imported successfully"));
                    } else {
                        TOsd::self()->display(tr("Error"), tr("Papagayo file is invalid!"), TOsd::Error);
                        #ifdef TUP_DEBUG
                            QString msg = "TupDocumentView::importPapagayoLipSync() - Fatal Error: Papagayo file is invalid!";
                            #ifdef Q_OS_WIN
                                qDebug() << msg;
                            #else
                                tError() << msg;
                            #endif
                        #endif
                    }
                } else {
                    TOsd::self()->display(tr("Error"), tr("Images directory is empty!"), TOsd::Error);
                    #ifdef TUP_DEBUG
                        QString msg = "TupDocumentView::importPapagayoLipSync() - Fatal Error: Images directory is empty!";
                        #ifdef Q_OS_WIN
                            qDebug() << msg;
                        #else
                            tError() << msg;
                        #endif
                    #endif
                }
            } else {
                TOsd::self()->display(tr("Error"), tr("Papagayo project is invalid!"), TOsd::Error);
                #ifdef TUP_DEBUG
                    QString msg = "TupDocumentView::importPapagayoLipSync() - Fatal Error: Papagayo file is invalid!";
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
            }
        } else {
            TOsd::self()->display(tr("Error"), tr("Papagayo project is invalid!"), TOsd::Error);
            #ifdef TUP_DEBUG
                QString msg = "TupDocumentView::importPapagayoLipSync() - Fatal Error: Papagayo file doesn't exist!";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif
        }
        QApplication::restoreOverrideCursor();
    }
}

void TupDocumentView::papagayoManager()
{
    if (currentTool->name().compare(tr("Papagayo Lip-sync")) != 0) {
        TupProject::Mode mode = TupProject::Mode(spaceModeCombo->currentIndex());
        if (mode != TupProject::FRAMES_EDITION)
            spaceModeCombo->setCurrentIndex(TupProject::FRAMES_EDITION);
        papagayoAction->trigger();
    }
}

void TupDocumentView::updatePerspective()
{
    if (currentTool) {
        QString toolName = currentTool->name();
        if (!toolName.isNull()) {
            if (toolName.compare(tr("Papagayo Lip-sync")) == 0)
                currentTool->updateWorkSpaceContext();
        }
    }
}

void TupDocumentView::resetWorkSpaceTransformations()
{
    paintArea->resetWorkSpaceCenter(project->getDimension());
    status->setRotationAngle("0");
    status->setZoomPercent("100");
}

QColor TupDocumentView::projectBGColor() const
{
    return project->getBgColor();
}

void TupDocumentView::updateWorkspace()
{
    paintArea->updateGridParameters();
    paintArea->viewport()->update();
}

void TupDocumentView::updatePen(const QPen &pen)
{
    status->setPen(pen);
    emit contourColorChanged(pen.color());
}

void TupDocumentView::updateBrush(const QBrush &brush)
{
    status->setBrush(brush);
    emit fillColorChanged(brush.color());
}

void TupDocumentView::updateActiveDock(TupDocumentView::DockType currentDock)
{
    currentDock = DockType(currentDock);
}

void TupDocumentView::updateCameraMode()
{
    cameraMode = false;
}

void TupDocumentView::setFillTool(TColorCell::FillType type)
{
    if (currentTool) {
        if (currentTool->toolType() == TupToolInterface::Fill) {
            currentTool->setColorMode(type);
            QString icon = "internal_fill.png";
            if (type == TColorCell::Contour)
                icon = "line_fill.png";

            QCursor cursor = QCursor(kAppProp->themeDir() + "cursors/" + icon, 0, 11);
            paintArea->viewport()->setCursor(cursor);
        }
    }
}
