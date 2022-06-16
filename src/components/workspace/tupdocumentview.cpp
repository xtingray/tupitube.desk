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
#include "talgorithm.h"
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
#include "tcolorcell.h"
#include "tosd.h"
#include "tupfilterinterface.h"
#include "tupmodessettingsdialog.h"
#include "tuppapagayoapp.h"
#include "tuplipsync.h"

#include <QDir>
#include <QApplication>
#include <QToolBar>
#include <QPixmap>
#include <QGridLayout>
#include <QCameraImageCapture>
#include <QCamera>
#include <QCameraInfo>

TupDocumentView::TupDocumentView(TupProject *work, bool netFlag, const QStringList &users, QWidget *parent):
                                 QMainWindow(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupDocumentView()]";
    #endif

    setWindowIcon(QPixmap(THEME_DIR + "icons/animation_mode.png"));
    setContextMenuPolicy(Qt::NoContextMenu);

    project = work;
    currentTool = nullptr;
    onionEnabled = true;
    fullScreenOn = false;
    rasterWindowOn = false;
    viewAngle = 0;
    isNetworked = netFlag;
    onLineUsers = users;
    dynamicFlag = false;
    staticFlag = false;
    colorSpace = TColorCell::Contour;
    contourColor = Qt::black;
    currentDock = ExposureSheet;
    zoomFactor = "100";

    cameraMode = false;
    photoCounter = 1;
    nodesScaleFactor = 1;
    screen = QGuiApplication::screens().at(0);

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
    connect(paintArea, SIGNAL(eyeDropperLaunched()), this, SLOT(launchEyeDropperTool()));

    connect(paintArea, SIGNAL(cursorPosition(const QPointF &)), verticalRuler, SLOT(movePointers(const QPointF&)));
    connect(paintArea, SIGNAL(cursorPosition(const QPointF &)), horizontalRuler, SLOT(movePointers(const QPointF&)));
    connect(paintArea, SIGNAL(changedZero(const QPointF&)), this, SLOT(changeRulerOrigin(const QPointF&)));
    connect(paintArea, SIGNAL(requestTriggered(const TupProjectRequest *)), this, SIGNAL(requestTriggered(const TupProjectRequest *)));
    connect(paintArea, SIGNAL(localRequestTriggered(const TupProjectRequest *)), this, SIGNAL(localRequestTriggered(const TupProjectRequest *)));

    /* SQA: This connections don't work on Windows
    connect(paintArea, &TupPaintArea::scaled, this, &TupDocumentView::updateZoomVars);
    connect(paintArea, &TupPaintArea::rotated, this, &TupDocumentView::updateRotationVars);
    connect(paintArea, &TupPaintArea::zoomIn, this, &TupDocumentView::applyZoomIn);
    connect(paintArea, &TupPaintArea::zoomOut, this, &TupDocumentView::applyZoomOut);
    connect(paintArea, &TupPaintArea::newPerspective, this, &TupDocumentView::newPerspective);
    connect(paintArea, &TupPaintArea::eyeDropperLaunched, this, &TupDocumentView::launchEyeDropperTool);

    connect(paintArea, &TupPaintArea::cursorPosition, verticalRuler, &TupRuler::movePointers);
    connect(paintArea, &TupPaintArea::cursorPosition, horizontalRuler, &TupRuler::movePointers);
    connect(paintArea, &TupPaintArea::changedZero, this, &TupDocumentView::changeRulerOrigin);
    connect(paintArea, &TupPaintArea::requestTriggered, this, &TupDocumentView::requestTriggered);
    connect(paintArea, &TupPaintArea::localRequestTriggered, this, &TupDocumentView::localRequestTriggered);
    */

    setupDrawActions();
    createLateralToolBar();
    createToolBar();

    status = new TupPaintAreaStatus(TupPaintAreaStatus::Vector, contourPen(), fillBrush());

    connect(status, SIGNAL(newFramePointer(int)), this, SLOT(goToFrame(int)));
    connect(status, SIGNAL(clearFrameClicked()), this, SLOT(clearFrame()));
    connect(status, SIGNAL(resetClicked()), this, SLOT(resetWorkSpaceTransformations()));
    connect(status, SIGNAL(safeAreaClicked()), this, SLOT(drawActionSafeArea()));
    connect(status, SIGNAL(gridClicked()), this, SLOT(drawGrid()));
    connect(status, SIGNAL(angleChanged(int)), this, SLOT(setRotationAngle(int)));
    connect(status, SIGNAL(zoomChanged(qreal)), this, SLOT(setZoomFactor(qreal)));
    connect(status, SIGNAL(fullClicked()), this, SLOT(showFullScreen()));

    connect(paintArea, SIGNAL(frameChanged(int)), status, SLOT(updateFrameIndex(int)));
    connect(paintArea, SIGNAL(cursorPosition(const QPointF &)), status, SLOT(showPos(const QPointF &)));

    /* SQA: This connections don't work on Windows
    connect(status, &TupPaintAreaStatus::newFramePointer, this, &TupDocumentView::goToFrame);
    connect(status, &TupPaintAreaStatus::clearFrameClicked, this, &TupDocumentView::clearFrame);
    connect(status, &TupPaintAreaStatus::resetClicked, this, &TupDocumentView::resetWorkSpaceTransformations);
    connect(status, &TupPaintAreaStatus::safeAreaClicked, this, &TupDocumentView::drawActionSafeArea);
    connect(status, &TupPaintAreaStatus::gridClicked, this, &TupDocumentView::drawGrid);
    connect(status, &TupPaintAreaStatus::angleChanged, this, &TupDocumentView::setRotationAngle);
    connect(status, &TupPaintAreaStatus::zoomChanged, this, &TupDocumentView::setZoomFactor);
    connect(status, &TupPaintAreaStatus::fullClicked, this, &TupDocumentView::showFullScreen);

    connect(paintArea, &TupPaintArea::frameChanged, status, &TupPaintAreaStatus::updateFrameIndex);
    connect(paintArea, &TupPaintArea::cursorPosition, status, &TupPaintAreaStatus::showPos);
    */

    brushManager()->initBgColor(project->getBgColor());

    connect(brushManager(), SIGNAL(penChanged(const QPen &)), this, SLOT(updatePen(const QPen &)));
    connect(brushManager(), SIGNAL(brushChanged(const QBrush &)), this, SLOT(updateBrush(const QBrush &)));
    connect(brushManager(), SIGNAL(bgColorChanged(const QColor &)), this, SLOT(updateBgColor(const QColor &)));

    /* SQA: This connections don't work on Windows
    connect(brushManager(), &TupBrushManager::penChanged, this, &TupDocumentView::updatePen);
    connect(brushManager(), &TupBrushManager::brushChanged, this, &TupDocumentView::updateBrush);
    connect(brushManager(), &TupBrushManager::bgColorChanged, this, &TupDocumentView::updateBgColor);
    */

    setStatusBar(status);

    // SQA: Find out why this timer instruction is required?
    QTimer::singleShot(500, this, SLOT(loadPlugins()));

    if (!isNetworked)
        saveTimer();

    paintArea->updateLoadingFlag(false);
}

TupDocumentView::~TupDocumentView()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupDocumentView()]";
    #endif

    if (currentTool)
        currentTool->saveConfig();

    if (paintArea) {
        delete paintArea;
        paintArea = nullptr;
    }

    if (configurationArea) {
        delete configurationArea;
        configurationArea = nullptr;
    }
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
        paintArea->goToFrame(index - 1);
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
    TAction::ActionId tool = currentTool->toolId();
    if (tool == TAction::ObjectSelection || tool == TAction::NodesEditor
        || tool == TAction::Polyline || tool == TAction::Text)
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
        zoomFactor = QString::number(zoom);
        status->setZoomPercent(zoomFactor);
    }
}

void TupDocumentView::applyZoomOut()
{
    qreal zoom = status->currentZoomFactor();
    if (zoom >= 15) {
        zoom -= 5;
        zoomFactor = QString::number(zoom);
        status->setZoomPercent(zoomFactor);
    }
}

void TupDocumentView::updateNodesScale(qreal factor)
{
    if (currentTool) {
        nodesScaleFactor *= factor;
        TAction::ActionId tool = currentTool->toolId();

        if (tool == TAction::ObjectSelection || tool == TAction::NodesEditor ||
            tool == TAction::Polyline || tool == TAction::Motion ||
            tool == TAction::Rotation || tool == TAction::Shear ||
            tool == TAction::LipSyncTool || tool == TAction::Text)
            currentTool->resizeNode(1 / nodesScaleFactor);
    }
}

void TupDocumentView::setZoomPercent(const QString &percent)
{
    nodesScaleFactor = percent.toDouble() / 100;
    status->setZoomPercent(percent);
    zoomFactor = percent;
}

void TupDocumentView::clearFrame()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupDocumentView::clearFrame()]";
    #endif

    int scene = paintArea->currentSceneIndex();
    int layer = paintArea->currentLayerIndex();
    int frame = paintArea->currentFrameIndex();

    TupProjectRequest event = TupRequestBuilder::createFrameRequest(scene, layer, frame, TupProjectRequest::Reset);
    emit requestTriggered(&event);
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
    new TAction(QPixmap(THEME_DIR + "icons/modes_settings.png"), tr("Modes Settings"), QKeySequence(),
                this, SLOT(showModesSettings()), actionManager, "modes_settings");

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

    new TAction(QPixmap(THEME_DIR + "icons/share.png"), tr("Post Frame On TupiTube"), QKeySequence(tr("Ctrl+@")),
                this, SLOT(postImage()), actionManager, "post_image");

    /*
    new TAction(QPixmap(THEME_DIR + "icons/onion_color.png"), tr("Onion Color"), QKeySequence(),
                          this, SLOT(activeOnionColorScheme()), actionManager, "onion_color");
    */

    /*
    TCONFIG->beginGroup("Network");
    QString server = TCONFIG->value("Server").toString();
    if (isNetworked && server.compare("tupitu.be") == 0) {
        new TAction(QPixmap(THEME_DIR + "icons/import_project.png"), tr("Export Frame To Gallery"), QKeySequence(tr("@")),
                    this, SLOT(postImage()), actionManager, "post_image");
    }
    */

    new TAction(QPixmap(THEME_DIR + "icons/storyboard.png"), tr("Storyboard Settings"), QKeySequence(tr("Ctrl+Shift+S")),
                this, SLOT(storyboardSettings()), actionManager, "storyboard");

    /*
    #ifdef Q_OS_WIN
        if (QSysInfo::windowsVersion() != QSysInfo::WV_XP) {
            new TAction(QPixmap(THEME_DIR + "icons/camera.png"), tr("Camera"), QKeySequence(tr("Ctrl+Shift+C")),
                        this, SLOT(cameraInterface()), actionManager, "camera");
        }
    #else
        new TAction(QPixmap(THEME_DIR + "icons/camera.png"), tr("Camera"), QKeySequence(tr("Ctrl+Shift+C")),
                    this, SLOT(cameraInterface()), actionManager, "camera");
    #endif
    */

    new TAction(QPixmap(THEME_DIR + "icons/camera.png"), tr("Camera"), QKeySequence(tr("Ctrl+Shift+C")),
                this, SLOT(cameraInterface()), actionManager, "camera");

    new TAction(QPixmap(THEME_DIR + "icons/papagayo.png"), tr("Papagayo Lip-sync Module"), QKeySequence(tr("Ctrl+Shift+P")),
                this, SLOT(papagayoManager()), actionManager, "papagayo");
}

void TupDocumentView::createLateralToolBar()
{
    toolbar = new QToolBar(tr("Draw tools"), this);
    toolbar->setIconSize(QSize(16, 16));
    toolbar->setMovable(false);
    addToolBar(Qt::LeftToolBarArea, toolbar);
    connect(toolbar, SIGNAL(actionTriggered(QAction*)), this, SLOT(selectToolFromMenu(QAction*)));

    // Brushes menu
    shapesMenu = new QMenu(tr("Brushes"), toolbar);
    shapesMenu->setIcon(QPixmap(THEME_DIR + "icons/square.png"));
    connect(shapesMenu, SIGNAL(triggered(QAction*)), this, SLOT(selectToolFromMenu(QAction*)));

    // Motion Tween menu
    motionMenu = new QMenu(tr("Tweening"), toolbar);
    motionMenu->setIcon(QPixmap(THEME_DIR + "icons/motion_tween.png"));
    connect(motionMenu, SIGNAL(triggered(QAction*)), this, SLOT(selectToolFromMenu(QAction*)));

    // Misc Tools menu
    miscMenu = new QMenu(tr("Misc Tools"), toolbar);
    miscMenu->setIcon(QPixmap(THEME_DIR + "icons/export_frame.png"));
    // connect(miscMenu, SIGNAL(triggered(QAction *)), this, SLOT(selectToolFromMenu(QAction*)));
}

// SQA: This method must be protected while every new project is being loaded

void TupDocumentView::loadPlugins()
{
    #ifdef TUP_DEBUG
        qWarning() << "[TupDocumentView::loadPlugins()]";
    #endif

    bool videoPluginLoaded = false;
    bool imagePluginLoaded = false;

    foreach (QObject *plugin, TupPluginManager::instance()->getFormats()) {
        if (plugin) {
            TupExportInterface *exporter = qobject_cast<TupExportInterface *>(plugin);
            if (exporter) {
                #ifdef TUP_DEBUG
                    qWarning() << "[TupDocumentView::loadPlugins()] - plugin -> " << exporter->key();
                #endif

                if (exporter->key() == TupExportInterface::VideoFormats) {
                    videoPlugin = exporter;
                    videoPluginLoaded = true;
                }

                if (exporter->key() == TupExportInterface::ImageSequence) {
                    imagePlugin = exporter;
                    imagePluginLoaded = true;
                }

                if (videoPluginLoaded && imagePluginLoaded)
                    break;
            } else {
                #ifdef TUP_DEBUG
                    qWarning() << "[TupDocumentView::loadPlugins()] - Fatal Error: plugin is NULL -> " << plugin->objectName();
                #endif
            }
        }
    }

    if (!videoPluginLoaded) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupDocumentView::loadPlugins()] - Warning: Couldn't found plugin -> "
                       << tr("Video Formats");
        #endif
    }

    if (!imagePluginLoaded) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupDocumentView::loadPlugins()] - Warning: Couldn't found plugin -> "
                       << tr("Image Sequence");
        #endif
    }

    QVector<TAction*> brushTools(3);
    QVector<TAction*> tweenTools(7);

    foreach (QObject *plugin, TupPluginManager::instance()->getTools()) {
        TupToolPlugin *tool = qobject_cast<TupToolPlugin *>(plugin);

        if (tool->toolType() != TupToolInterface::Tweener && tool->toolType() != TupToolInterface::LipSync)
            connect(tool, SIGNAL(closeHugeCanvas()), this, SLOT(closeFullScreen()));

        if (tool->toolType() != TupToolInterface::Tweener)
            connect(tool, SIGNAL(callForPlugin(int,int)), this, SLOT(loadPlugin(int,int)));

        QStringList::iterator it;
        QList<TAction::ActionId> keys = tool->keys();

        for (int i = 0; i < keys.size(); i++) {
            TAction::ActionId toolId = keys.at(i);
            TAction *action = tool->getAction(toolId);
            if (action) {
                #ifdef TUP_DEBUG
                    qWarning() << "[TupDocumentView::loadPlugins()] - Tool Loaded -> " << action->text();
                #endif

                action->setIconVisibleInMenu(true);
                connect(action, SIGNAL(triggered()), this, SLOT(selectTool()));
                action->setParent(plugin);
                action->setCheckable(true);
                actionGroup->addAction(action);

                // QString toolName = action->text();
                switch (tool->toolType()) {
                    case TupToolInterface::Brush:
                    {
                      // SQA: Experimental plugin (enable it only for testing)
                      // if (toolId == TAction::Scheme)
                      //     schemeAction = action;

                      if (toolId == TAction::Pencil)
                          pencilAction = action;

                      if (toolId == TAction::Ink)
                          inkAction = action;

                      // SQA: This code has been disabled temporarily
                      /*
                      if (toolId == TAction::Eraser) {
                          action->setDisabled(true);
                          brushTools[2] = action;
                      }
                      */

                      if (toolId == TAction::Polyline) {
                          polyLineAction = action;

                          TupToolPlugin *tool = qobject_cast<TupToolPlugin *>(action->parent());
                          connect(paintArea, SIGNAL(closePolyLine()), tool, SLOT(initEnv()));
                          connect(this, SIGNAL(closePolyLine()), tool, SLOT(initEnv()));
                      }

                      if (toolId == TAction::Line) {
                          brushTools[2] = action;

                          TupToolPlugin *tool = qobject_cast<TupToolPlugin *>(action->parent());
                          connect(paintArea, SIGNAL(closeLine()), tool, SLOT(endItem()));
                          connect(this, SIGNAL(closeLine()), tool, SLOT(endItem()));
                      }

                      if (toolId == TAction::Rectangle) {
                          brushTools[0] = action;
                          shapesMenu->setDefaultAction(action);
                      }

                      if (toolId == TAction::Ellipse)
                          brushTools[1] = action;

                      // if (toolId == TAction::Text)
                      //     textAction = action;
                    }
                    break;
                    case TupToolInterface::Tweener:
                    {
                      if (toolId == TAction::Motion) {
                          tweenTools[0] = action;
                          motionMenu->setDefaultAction(action);
                      }

                      if (toolId == TAction::Rotation)
                          tweenTools[1] = action;

                      if (toolId == TAction::Scale)
                          tweenTools[2] = action;

                      if (toolId == TAction::Shear)
                          tweenTools[3] = action;

                      if (toolId == TAction::Opacity)
                          tweenTools[4] = action;

                      if (toolId == TAction::Color)
                          tweenTools[5] = action;

                      TupToolPlugin *tool = qobject_cast<TupToolPlugin *>(action->parent());
                      connect(tool, SIGNAL(tweenRemoved()), this, SLOT(updatePaintArea()));
                    }
                    break;
                    case TupToolInterface::Selection:
                    {
                      if (toolId == TAction::ObjectSelection)
                          selectionAction = action;

                      if (toolId == TAction::NodesEditor)
                          nodesAction = action;

                      if (toolId == TAction::Text)
                          textAction = action;
                    }
                    break;
                    case TupToolInterface::Fill:
                    {
                      if (toolId == TAction::FillTool)
                          fillAction = action;
                    }
                    break;
                    case TupToolInterface::LipSync:
                    {
                      if (toolId == TAction::LipSyncTool)
                          papagayoAction = action;
                    }
                    break;
                    case TupToolInterface::Color:
                    {
                      if (toolId == TAction::EyeDropper)
                          eyedropperAction = action;
                    }
                    break;
                    default:
                    break;
                } // end switch
            } else {
              #ifdef TUP_DEBUG
                  qDebug() << "[TupDocumentView::loadPlugins()] - Fatal Error: Tool action is NULL -> " << *it;
              #endif
            }
        }
    } // end foreach

    for (int i = 0; i < brushTools.size(); ++i)
         shapesMenu->addAction(brushTools.at(i));

    for (int i = 0; i < 6; ++i)
         motionMenu->addAction(tweenTools.at(i));

    miscMenu->addAction(actionManager->find("export_image"));

    // TCONFIG->beginGroup("Network");
    // QString server = TCONFIG->value("Server").toString();
    // if (isNetworked && server.compare("tupitu.be") == 0)
    //     miscMenu->addAction(actionManager->find("post_image"));

    miscMenu->addAction(actionManager->find("post_image"));
    miscMenu->addAction(actionManager->find("storyboard"));
    // miscMenu->addAction(actionManager->find("papagayo"));

    foreach (QObject *plugin, TupPluginManager::instance()->getFilters()) {
        AFilterInterface *filterInterface = qobject_cast<AFilterInterface *>(plugin);
        QStringList::iterator it;
        QStringList keys = filterInterface->keys();

        for (it = keys.begin(); it != keys.end(); ++it) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupDocumentView::loadPlugins()] - Filter Loaded: " << *it;
            #endif

            // TAction *filter = filterInterface->actions()[*it];
            TAction *filter = filterInterface->getAction(*it);
            if (filter) {
                connect(filter, SIGNAL(triggered()), this, SLOT(applyFilter()));
                filterMenu->addAction(filter);
            }
        }
    }

    toolbar->addAction(pencilAction);
    toolbar->addAction(inkAction);
    // SQA: Enable it only for debugging goals
    // toolbar->addAction(schemeAction);
    toolbar->addAction(polyLineAction);

    toolbar->addSeparator();
    toolbar->addAction(shapesMenu->menuAction());
    toolbar->addSeparator();
    toolbar->addAction(selectionAction);
    toolbar->addAction(nodesAction);
    toolbar->addSeparator();
    toolbar->addAction(textAction);
    toolbar->addSeparator();
    toolbar->addAction(fillAction);
    toolbar->addSeparator();
    toolbar->addAction(motionMenu->menuAction());

    /*
    #ifdef Q_OS_WIN
        if (QSysInfo::windowsVersion() != QSysInfo::WV_XP) {
            toolbar->addSeparator();
            toolbar->addAction(actionManager->find("camera"));
        }
    #else
        toolbar->addSeparator();
        toolbar->addAction(actionManager->find("camera"));
    #endif
    */

    toolbar->addSeparator();
    toolbar->addAction(actionManager->find("camera"));

    toolbar->addSeparator();
    toolbar->addAction(papagayoAction);

    toolbar->addSeparator();
    toolbar->addAction(miscMenu->menuAction());

    brushTools.clear();
    tweenTools.clear();

    pencilAction->trigger();
    paintArea->setFocus();
}

void TupDocumentView::loadPlugin(int menu, int actionID)
{
    #ifdef TUP_DEBUG
        qWarning() << "[TupDocumentView::loadPlugin()] - Menu -> "
                   << menu << " - Action -> " << actionID << " - currentDock -> " << currentDock;
    #endif

    TAction *action = nullptr;
    switch (menu) {
        case TAction::Arrows:
            {
                if (fullScreenOn) {
                    if (actionID == TAction::Left_Arrow) {
                        paintArea->goOneFrameBack();
                    } else if (actionID == TAction::Right_Arrow) {
                        paintArea->goOneFrameForward();
                    } else if (actionID == TAction::Up_Arrow) {
                        paintArea->goOneLayerBack();
                    } else if (actionID == TAction::Down_Arrow) {
                        paintArea->goOneLayerForward();
                    } else if (actionID == TAction::Right_QuickCopy) {
                        paintArea->copyFrameForward();
                    } else if (actionID == TAction::Left_Delete) {
                        paintArea->removeCurrentFrame();
                    }
                    return;
                } else {
                    if (currentDock == ExposureSheet) {
                        if (actionID == TAction::Up_Arrow) {
                            paintArea->goOneFrameBack();
                        } else if (actionID == TAction::Down_Arrow) {
                            paintArea->goOneFrameForward();
                        } else if (actionID == TAction::Down_QuickCopy) {
                            paintArea->copyFrameForward();
                        } else if (actionID == TAction::Up_Delete) {
                            paintArea->removeCurrentFrame();
                        } else if (actionID == TAction::Left_Arrow) {
                            paintArea->goOneLayerBack();
                        } else if (actionID == TAction::Right_Arrow) {
                            paintArea->goOneLayerForward();
                        }
                        return;
                    } else if (currentDock == TimeLine) {
                        if (actionID == TAction::Left_Arrow) {
                            paintArea->goOneFrameBack();
                        } else if (actionID == TAction::Right_Arrow) {
                            paintArea->goOneFrameForward();
                        } else if (actionID == TAction::Right_QuickCopy) {
                            paintArea->copyFrameForward();
                        } else if (actionID == TAction::Left_Delete) {
                            paintArea->removeCurrentFrame();
                        } else if (actionID == TAction::Up_Arrow) {
                            paintArea->goOneLayerBack();
                        } else if (actionID == TAction::Down_Arrow) {
                            paintArea->goOneLayerForward();
                        }
                        return;
                    }
                }
            }
        break;
        case TAction::ColorMenu:
            {
                if (actionID == TAction::ColorPalette) {
                    if (fullScreenOn) {
                        // QColor currentColor = brushManager()->penColor();
                        emit colorDialogRequested(brushManager()->penColor());
                    }
                    return;
                } else if (actionID == TAction::EyeDropper) {
                    launchEyeDropperTool();
                    return;
                }
            }
        break;
        case TAction::BrushesMenu:
            {
                QList<QAction*> brushActions = shapesMenu->actions();

                switch (actionID) {
                    case TAction::Pencil:
                    {
                        action = pencilAction;
                    }
                    break;
                    case TAction::Ink:
                    {
                        action = inkAction;
                    }
                    break;
                    // SQA: Enable it only for debugging
                    /*
                    case TupToolPlugin::SchemeTool:
                    {
                        action = schemeAction;
                    }
                    break;
                    */
                    case TAction::Polyline:
                    {
                        action = polyLineAction;
                    }
                    break;
                    case TAction::Rectangle:
                    {
                        action = static_cast<TAction *> (brushActions[0]);
                    }
                    break;
                    case TAction::Ellipse:
                    {
                        action = static_cast<TAction *> (brushActions[1]);
                    }
                    break;
                    case TAction::Line:
                    {
                        action = static_cast<TAction *> (brushActions[2]);
                    }
                    break;
                    case TAction::Text:
                    {
                        action = textAction;
                    }
                    break;
                    default:
                    {
                        // No Action
                    }
                }
            }
        break;
        case TAction::SelectionMenu:
            {
                switch (actionID) {
                    case TAction::Delete:
                    {
                        paintArea->deleteItems();
                    }
                    break;
                    case TAction::NodesEditor:
                    {
                        action = nodesAction;
                    }
                    break;
                    case TAction::ObjectSelection:
                    {
                        action = selectionAction;
                    }
                    break;
                    default:
                    {
                        // No Action
                    }
                }
            }
        break;
        case TAction::FillMenu:
            {
                if (actionID == TAction::FillTool)
                    action = fillAction;
                    // fillMode = TColorCell::Inner;

                /*
                if (index == TAction::ContourFill) {
                    action = fillAction;
                    fillMode = TColorCell::Contour;
                }
                */
            }
        break;
        /*
        case TAction::ZoomMenu:
            {
                action = shiftAction;
            }
        break;
        */
        default:
            {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupDocumentView::loadPlugin()] - Error: Invalid Menu Index / No plugin loaded";
                #endif
                return;
            }
    }

    if (action) {
        if (fullScreenOn) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupDocumentView::loadPlugin()] - Calling action -> " << action->text();
            #endif

            // QString toolName = tr("%1").arg(action->text());
            TAction::ActionId tool = action->actionId();
            if (tool != currentTool->toolId()) {
                action->trigger();
                fullScreen->updateCursor(action->cursor());
            }
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupDocumentView::loadPlugin()] - Error: Action pointer is NULL!";
        #endif
        return;
    }
}

void TupDocumentView::selectTool()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupDocumentView::selectTool()]";
    #endif

    TAction *action = qobject_cast<TAction *>(sender());

    if (action) {
        QString toolName = tr("%1").arg(action->text());
        TAction::ActionId toolId = action->actionId();

        if (currentTool) {
            if (toolId == currentTool->toolId())
                return;

            if (currentTool->toolId() == TAction::Pencil)
                disconnect(currentTool, SIGNAL(penWidthChanged(int)), this, SIGNAL(penWidthChanged(int)));

            if (currentTool->toolId() == TAction::LipSyncTool) {
                disconnect(currentTool, SIGNAL(lipsyncCreatorRequested()), this, SLOT(openLipSyncCreator()));
                disconnect(currentTool, SIGNAL(lipsyncEditionRequested(QString)), this, SLOT(openLipSyncCreator(QString)));
            }

            if (currentTool->toolId() == TAction::EyeDropper) {
                disconnect(currentTool, SIGNAL(colorPicked(TColorCell::FillType, const QColor &)),
                                        this, SIGNAL(colorChanged(TColorCell::FillType, const QColor &)));
                disconnect(paintArea, SIGNAL(cursorPosition(const QPointF &)), this, SLOT(refreshEyeDropperPanel()));
            }

            currentTool->saveConfig();
            QWidget *toolConfigurator = currentTool->configurator();
            if (toolConfigurator)
                configurationArea->close();
        }

        TupToolPlugin *tool = qobject_cast<TupToolPlugin *>(action->parent());
        if (tool) {
            tool->setCurrentToolName(toolName);
            tool->setToolId(toolId);
            currentTool = tool;

            paintArea->setCurrentTool(toolId);

            if (!action->icon().isNull())
                status->updateTool(toolName, action->icon().pixmap(15, 15));

            int minWidth = 0;

            switch (tool->toolType()) {
                case TupToolInterface::Brush:
                {
                    status->enableFullScreenFeature(true);
                    if (toolId == TAction::Pencil || toolId == TAction::Polyline) {
                        minWidth = 130;
                        if (toolId == TAction::Pencil)
                            connect(currentTool, SIGNAL(penWidthChanged(int)), this, SIGNAL(penWidthChanged(int)));
                    /*
                    } else if (toolId == TAction::Text) {
                        minWidth = 130;
                    */
                    } else {
                        if (toolId == TAction::Rectangle || toolId == TAction::Ellipse || toolId == TAction::Line) {
                            minWidth = 130;
                            shapesMenu->setDefaultAction(action);
                            shapesMenu->setActiveAction(action);

                            if (!action->icon().isNull())
                                shapesMenu->menuAction()->setIcon(action->icon());
                        }
                    }
                    /*
                        SQA: Enable it only for debugging
                        if (toolId == TAction::Scheme)
                            minWidth = 130;
                    */
                }
                break;
                case TupToolInterface::Tweener:
                {
                    status->enableFullScreenFeature(false);
                    minWidth = 230;
                    motionMenu->setDefaultAction(action);
                    motionMenu->setActiveAction(action);
                    if (!action->icon().isNull())
                        motionMenu->menuAction()->setIcon(action->icon());
                }
                break;
                case TupToolInterface::Fill:
                {
                    QString cursorIcon = "line_fill.png";
                    if (colorSpace == TColorCell::Background) {
                        TCONFIG->beginGroup("ColorPalette");
                        TCONFIG->setValue("CurrentColorMode", TColorCell::Contour);

                        emit colorModeChanged(TColorCell::Contour);
                    } else {
                        if (colorSpace == TColorCell::Inner)
                            cursorIcon = "internal_fill.png";
                    }
                    QCursor cursor = QCursor(kAppProp->themeDir() + "cursors/" + cursorIcon, 0, 11);
                    paintArea->viewport()->setCursor(cursor);
                    status->enableFullScreenFeature(true);

                    fillAction->trigger();
                }
                break;
                case TupToolInterface::Selection:
                {
                    status->enableFullScreenFeature(true);
                    if (toolId == TAction::ObjectSelection) {
                        tool->setProjectSize(project->getDimension());
                        minWidth = 130;
                        connect(paintArea, SIGNAL(itemAddedOnSelection(TupGraphicsScene*)),
                                tool, SLOT(initItems(TupGraphicsScene*)));
                    }
                }
                break;
                /*
                case TupToolInterface::View:
                {
                    status->enableFullScreenFeature(true);
                    if (toolId == TAction::Shift) {
                        tool->setProjectSize(project->getDimension());
                        if (fullScreenOn)
                            tool->setActiveView("FULL_SCREEN");
                        else
                            tool->setActiveView("WORKSPACE");
                    }
                }
                break;
                */
                case TupToolInterface::LipSync:
                {
                    status->enableFullScreenFeature(false);
                    minWidth = 220;
                    connect(currentTool, SIGNAL(lipsyncCreatorRequested()), this, SLOT(openLipSyncCreator()));
                    connect(currentTool, SIGNAL(lipsyncEditionRequested(QString)), this, SLOT(openLipSyncCreator(QString)));

                    /*
                    miscMenu->setDefaultAction(action);
                    miscMenu->setActiveAction(action);
                    if (!action->icon().isNull())
                        miscMenu->menuAction()->setIcon(action->icon());
                    */
                }
            }

            QWidget *toolConfigurator = tool->configurator();
            if (toolConfigurator) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupDocumentView::selectTool()] - Showing plugin settings panel";
                #endif
                configurationArea = new TupConfigurationArea(this);
                configurationArea->setConfigurator(toolConfigurator, minWidth);
                addDockWidget(Qt::RightDockWidgetArea, configurationArea);
                toolConfigurator->show();
                if (!configurationArea->isVisible())
                    configurationArea->show();
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupDocumentView::selectTool()] - No settings panel";
                #endif
                if (configurationArea->isVisible())
                    configurationArea->close();
            }

            paintArea->setTool(tool);

            if (tool->toolType() != TupToolInterface::Fill)
                paintArea->viewport()->setCursor(action->cursor());

            if (toolId == TAction::ObjectSelection || toolId == TAction::NodesEditor || toolId == TAction::Polyline
                || toolId == TAction::Motion || toolId == TAction::Rotation || toolId == TAction::Shear
                || toolId == TAction::LipSyncTool || toolId == TAction::Text)
                tool->updateZoomFactor(1 / nodesScaleFactor);
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupDocumentView::selectTool()] - Fatal Error: Action from sender() is NULL";
        #endif
    }
}

void TupDocumentView::selectToolFromMenu(QAction *action)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupDocumentView::selectToolFromMenu()]";
    #endif

    QMenu *menu = qobject_cast<QMenu *>(action->parent());
    if (menu) {
        TAction *tool = qobject_cast<TAction *>(menu->activeAction());

        if (tool) {
            if (tool->actionId() == currentTool->toolId())
                return;
            else
                tool->trigger(); // this line calls selectTool()
        } else {
            tool = qobject_cast<TAction *>(menu->defaultAction());
            if (tool) {
                tool->trigger();
            } else {
                #ifdef TUP_DEBUG
                    qDebug() <<  "[TupDocumentView::selectToolFromMenu()] - Default action is NULL";
                #endif
            }
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupDocumentView::selectToolFromMenu()] - Warning: Action with NO parent! Aborting...";
        #endif
    }
}

bool TupDocumentView::handleProjectResponse(TupProjectResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupDocumentView::handleProjectResponse()]";
    #endif

    if (TupFrameResponse *frameResponse = static_cast<TupFrameResponse *>(response)) {
        switch (frameResponse->getAction()) {
            case TupProjectRequest::Add:
            {
                if (cameraMode)
                    QApplication::restoreOverrideCursor();
            }
            break;
            case TupProjectRequest::AddRasterItem:
            {
                if (rasterWindow) {
                    if (response->getMode() == TupProjectResponse::Undo)
                        rasterWindow->undoRasterItem();

                    if (response->getMode() == TupProjectResponse::Redo)
                        rasterWindow->redoRasterItem();
                }
            }
            break;
            case TupProjectRequest::ClearRasterCanvas:
            {
                if (rasterWindow) {
                    if (response->getMode() == TupProjectResponse::Undo)
                        rasterWindow->undoClearRasterAction();

                    if (response->getMode() == TupProjectResponse::Redo)
                        rasterWindow->redoClearRasterAction();
                }
            }
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
            if (type == TupFrame::VectorStaticBg) {
                opacity = bg->vectorStaticOpacity();
            } else if (type == TupFrame::VectorDynamicBg) {
                opacity = bg->vectorDynamicOpacity();
            }
        }
    }

   return opacity;
}

void TupDocumentView::createToolBar()
{
    barGrid = new QToolBar(tr("Paint area actions"), this);
    barGrid->setIconSize(QSize(16, 16));

    staticPropertiesBar = new QToolBar(tr("Vector Static BG Properties"), this);
    dynamicPropertiesBar = new QToolBar(tr("Vector Dynamic BG Properties"), this);

    addToolBar(barGrid);

    spaceModeCombo = new QComboBox();
    spaceModeCombo->addItem(QIcon(THEME_DIR + "icons/frames_mode.png"), tr("Frames Mode"));
    spaceModeCombo->addItem(QIcon(THEME_DIR + "icons/static_background_mode.png"), tr("Vector Static BG Mode"));
    spaceModeCombo->addItem(QIcon(THEME_DIR + "icons/dynamic_background_mode.png"), tr("Vector Dynamic BG Mode"));
    spaceModeCombo->addItem(QIcon(THEME_DIR + "icons/raster_mode.png"), tr("Raster Static BG Mode"));
    spaceModeCombo->addItem(QIcon(THEME_DIR + "icons/dynamic_raster_mode.png"), tr("Raster Dynamic BG Mode"));
    spaceModeCombo->addItem(QIcon(THEME_DIR + "icons/fg_mode.png"), tr("Vector Foreground Mode"));

    connect(spaceModeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setSpaceContext()));
    setSpaceContext();

    barGrid->addWidget(spaceModeCombo);
    barGrid->addAction(actionManager->find("modes_settings"));

    barGrid->addSeparator();

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
    connect(nextOnionSkinSpin, SIGNAL(valueChanged(int)), this, SLOT(setNextOnionSkin(int)));

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

    QWidget *sEmpty0 = new QWidget();
    sEmpty0->setFixedWidth(5);
    QWidget *sEmpty1 = new QWidget();
    sEmpty1->setFixedWidth(5);
    QWidget *sEmpty2 = new QWidget();
    sEmpty2->setFixedWidth(5);

    QLabel *staticOpacityLabel = new QLabel();
    staticOpacityLabel->setToolTip(tr("Static BG Opacity"));
    staticOpacityLabel->setPixmap(QPixmap(THEME_DIR + "icons/bg_opacity.png"));

    QDoubleSpinBox *staticOpacityBox = new QDoubleSpinBox(this);
    staticOpacityBox->setRange(0.1, 1.0);
    staticOpacityBox->setSingleStep(0.1);
    staticOpacityBox->setValue(backgroundOpacity(TupFrame::VectorStaticBg));
    staticOpacityBox->setToolTip(tr("Static BG Opacity"));
    connect(staticOpacityBox, SIGNAL(valueChanged(double)), this, SLOT(updateStaticOpacity(double)));

    staticPropertiesBar->addWidget(sEmpty0);
    staticPropertiesBar->addWidget(staticOpacityLabel);
    staticPropertiesBar->addWidget(sEmpty1);
    staticPropertiesBar->addWidget(staticOpacityBox);
    staticPropertiesBar->addWidget(sEmpty2);
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
    connect(dirCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setVectorBackgroundDirection(int)));

    QWidget *dEmpty0 = new QWidget();
    dEmpty0->setFixedWidth(5);
    QWidget *dEmpty1 = new QWidget();
    dEmpty1->setFixedWidth(5);
    QWidget *dEmpty2 = new QWidget();
    dEmpty2->setFixedWidth(5);
    QWidget *dEmpty3 = new QWidget();
    dEmpty3->setFixedWidth(5);
    QWidget *dEmpty4 = new QWidget();
    dEmpty4->setFixedWidth(5);
    QWidget *dEmpty5 = new QWidget();
    dEmpty5->setFixedWidth(5);
    QWidget *dEmpty6 = new QWidget();
    dEmpty6->setFixedWidth(5);
    QWidget *dEmpty7 = new QWidget();
    dEmpty7->setFixedWidth(5);

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
    dynamicOpacityBox->setValue(backgroundOpacity(TupFrame::VectorDynamicBg));
    dynamicOpacityBox->setToolTip(tr("Dynamic BG Opacity"));
    connect(dynamicOpacityBox, SIGNAL(valueChanged(double)), this, SLOT(updateDynamicOpacity(double)));

    dynamicPropertiesBar->addWidget(dirLabel);
    dynamicPropertiesBar->addWidget(dEmpty0);
    dynamicPropertiesBar->addWidget(dirCombo);
    dynamicPropertiesBar->addWidget(dEmpty1);
    dynamicPropertiesBar->addSeparator();
    dynamicPropertiesBar->addWidget(dEmpty2);
    dynamicPropertiesBar->addWidget(shiftLabel);
    dynamicPropertiesBar->addWidget(dEmpty3);
    dynamicPropertiesBar->addWidget(shiftSpin);
    dynamicPropertiesBar->addWidget(dEmpty4);
    dynamicPropertiesBar->addSeparator();
    dynamicPropertiesBar->addWidget(dEmpty5);
    dynamicPropertiesBar->addWidget(dynamicOpacityLabel);
    dynamicPropertiesBar->addWidget(dEmpty6);
    dynamicPropertiesBar->addWidget(dynamicOpacityBox);
    dynamicPropertiesBar->addWidget(dEmpty7);

    dynamicPropertiesBar->setVisible(false);

    addToolBar(staticPropertiesBar);
    addToolBar(dynamicPropertiesBar);
}

void TupDocumentView::showModesSettings()
{
    QList<TupBackground::BgType> bgLayers = project->getBackgroundFromScene(paintArea->currentSceneIndex())->layerIndexes();
    QList<bool> bgVisibility = project->getBackgroundFromScene(paintArea->currentSceneIndex())->layersVisibility();

    TupModesSettingsDialog *dialog = new TupModesSettingsDialog(bgLayers, bgVisibility, this);
    connect(dialog, SIGNAL(valuesUpdated(QList<TupBackground::BgType>, QList<bool>)),
            this, SLOT(updateBgSettings(QList<TupBackground::BgType>, QList<bool>)));
    dialog->show();
}

void TupDocumentView::updateBgSettings(QList<TupBackground::BgType> indexes, QList<bool> visibilityList)
{
    TupBackground *bg = project->getBackgroundFromScene(paintArea->currentSceneIndex());
    bg->updateLayerIndexes(indexes);
    bg->updateLayersVisibility(visibilityList);
    paintArea->updatePaintArea();

    emit projectHasChanged();
}

void TupDocumentView::openRasterMode()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    rasterWindow = new RasterMainWindow(project, "raster", spaceContext(), currentSceneIndex(),
                                        contourColor, zoomFactor, this);

    connect(rasterWindow, SIGNAL(closeWindow(const QString &)), this, SLOT(closeRasterWindow(const QString &)));
    connect(rasterWindow, SIGNAL(paintAreaEventTriggered(const TupPaintAreaEvent *)),
            this, SIGNAL(paintAreaEventTriggered(const TupPaintAreaEvent *)));
    connect(rasterWindow, SIGNAL(rasterStrokeMade()), this, SLOT(requestRasterStroke()));
    connect(rasterWindow, SIGNAL(canvasCleared()), this, SLOT(requestClearRasterCanvas()));
    connect(rasterWindow, SIGNAL(libraryCall(const QString &)), this, SLOT(importImageToLibrary(const QString &)));

    /* SQA: These connections don't work on Windows
    connect(rasterWindow, &RasterMainWindow::closeWindow, this, &TupDocumentView::closeRasterWindow);
    connect(rasterWindow, &RasterMainWindow::paintAreaEventTriggered, this, &TupDocumentView::paintAreaEventTriggered);
    connect(rasterWindow, &RasterMainWindow::rasterStrokeMade, this, &TupDocumentView::requestRasterStroke);
    connect(rasterWindow, &RasterMainWindow::canvasCleared, this, &TupDocumentView::requestClearRasterCanvas);
    connect(rasterWindow, &RasterMainWindow::libraryCall, this, &TupDocumentView::importImageToLibrary);
    */

    rasterWindowOn = true;
    rasterWindow->showFullScreen();

    QApplication::restoreOverrideCursor();
}

void TupDocumentView::importImageToLibrary(const QString &imgPath)
{
    QFile imageFile(imgPath);
    if (imageFile.open(QIODevice::ReadOnly)) {
        QByteArray data = imageFile.readAll();
        imageFile.close();

        QString extension = "png";
        QString key = QString("rasterbg0." + extension);

        TupLibrary *library = project->getLibrary();
        int i = 0;
        while(true) {
            if (!library->exists(key))
                break;
            i++;
            key = QString("rasterbg" + QString::number(i) + "." + extension).toLower();
        }

        if (imageFile.remove()) {
            int sceneIndex = paintArea->graphicsScene()->currentSceneIndex();
            int layerIndex = paintArea->graphicsScene()->currentLayerIndex();
            int frameIndex = paintArea->graphicsScene()->currentFrameIndex();

            TupProjectRequest request;
            if (!library->folderExists(tr("Raster Objects"))) {
                request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, tr("Raster Objects"),
                                                                                    TupLibraryObject::Folder, project->spaceContext(), data, QString(),
                                                                                    0, 0, 0);
                emit requestTriggered(&request);
            }

            request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, key,
                                                              TupLibraryObject::Image, project->spaceContext(), data, tr("Raster Objects"),
                                                              sceneIndex, layerIndex, frameIndex);
            emit requestTriggered(&request);

            TOsd::self()->display(TOsd::Info, tr("Image imported successfully"), 2000);
        }
    }
}

void TupDocumentView::closeRasterWindow(const QString &imgPath)
{
    if (rasterWindowOn) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        disconnect(rasterWindow, SIGNAL(closeWindow(const QString &)), this, SLOT(closeRasterWindow(const QString &)));
        disconnect(rasterWindow, SIGNAL(paintAreaEventTriggered(const TupPaintAreaEvent *)),
                   this, SIGNAL(paintAreaEventTriggered(const TupPaintAreaEvent *)));
        disconnect(rasterWindow, SIGNAL(rasterStrokeMade()), this, SLOT(requestRasterStroke()));
        disconnect(rasterWindow, SIGNAL(canvasCleared()), this, SLOT(requestClearRasterCanvas()));

        project->updateRasterBackground(spaceContext(), currentSceneIndex(), imgPath);
        paintArea->updatePaintArea();

        spaceModeCombo->setCurrentIndex(0);
        rasterWindow->close();
        rasterWindowOn = false;
        rasterWindow = nullptr;
        delete rasterWindow;

        QApplication::restoreOverrideCursor();
    }
}

void TupDocumentView::closeInterface()
{
    if (currentTool)
        currentTool->aboutToChangeTool();

    if (configurationArea->isVisible())
        configurationArea->close();

    paintArea->setScene(nullptr);
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
    Q_UNUSED(cursor)
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
        qDebug() << "[TupDocumentView::updatePaintArea()]";
    #endif

    paintArea->updatePaintArea();
}

void TupDocumentView::callAutoSave()
{
    emit autoSave();
}

void TupDocumentView::saveTimer()
{
    TCONFIG->beginGroup("General");
    bool autoSave = TCONFIG->value("AutoSave", true).toBool();

    if (autoSave) {
        QString min = TCONFIG->value("AutoSaveTime", "5").toString();
        int saveTime = min.toInt() * 60000;
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(callAutoSave()));
        timer->start(saveTime);
    }
}

void TupDocumentView::setSpaceContext()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupDocumentView::setSpaceContext()]";
    #endif

    TupProject::Mode mode = TupProject::Mode(spaceModeCombo->currentIndex());

    switch (mode) {
        case TupProject::FRAMES_MODE:
        {
            if (dynamicFlag) {
                dynamicFlag = false;
                renderVectorDynamicFrame();
            }

            project->updateSpaceContext(TupProject::FRAMES_MODE);
            staticPropertiesBar->setVisible(false);
            dynamicPropertiesBar->setVisible(false);
            motionMenu->setEnabled(true);
        }
        break;
        case TupProject::VECTOR_STATIC_BG_MODE:
        {
            project->updateSpaceContext(TupProject::VECTOR_STATIC_BG_MODE);
            staticPropertiesBar->setVisible(true);
            dynamicPropertiesBar->setVisible(false);
            motionMenu->setEnabled(false);
        }
        break;
        case TupProject::VECTOR_DYNAMIC_BG_MODE:
        {
            dynamicFlag = true;
            project->updateSpaceContext(TupProject::VECTOR_DYNAMIC_BG_MODE);

            int sceneIndex = paintArea->currentSceneIndex();
            TupScene *scene = project->sceneAt(sceneIndex);
            if (scene) {
                TupBackground *bg = scene->sceneBackground();
                if (bg) {
                    dirCombo->setCurrentIndex(bg->vectorDynamicDirection());
                    shiftSpin->setValue(bg->vectorDynamicShift());
                }
            }
            staticPropertiesBar->setVisible(false);
            dynamicPropertiesBar->setVisible(true);
            motionMenu->setEnabled(false);
        }
        break;
        case TupProject::RASTER_STATIC_BG_MODE:
        case TupProject::RASTER_DYNAMIC_BG_MODE:
        {
            openRasterMode();
            return;
        }
        break;
        case TupProject::VECTOR_FG_MODE:
        {
            project->updateSpaceContext(TupProject::VECTOR_FG_MODE);
            staticPropertiesBar->setVisible(true);
            dynamicPropertiesBar->setVisible(false);
            motionMenu->setEnabled(false);
        }
        break;
        default:
        break;
    }

    paintArea->updateSpaceContext();
    paintArea->updatePaintArea();

    if (currentTool) {
        if (((currentTool->toolType() == TupToolInterface::Tweener)
            || (currentTool->toolType() == TupToolInterface::LipSync))
            && (mode != TupProject::FRAMES_MODE)) {
            pencilAction->trigger();
        } else {
            currentTool->init(paintArea->graphicsScene());
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
       TupProjectRequest event = TupRequestBuilder::createSceneRequest(currentSceneIndex(), TupProjectRequest::BgColor,
                                                                       color.name());
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

    int screenW = screen->geometry().width();
    int screenH = screen->geometry().height();

    cacheScaleFactor = nodesScaleFactor;
    qreal scaleFactor = 1;

    QSize projectSize = project->getDimension();
    if (projectSize.width() < projectSize.height())
        scaleFactor = static_cast<double>(screenW - 50) / static_cast<double>(projectSize.width());
    else
        scaleFactor = static_cast<double>(screenH - 50) / static_cast<double>(projectSize.height());

    fullScreen = new TupCanvas(this, Qt::Window|Qt::FramelessWindowHint, paintArea->graphicsScene(),
                                  paintArea->getCenterPoint(), QSize(screenW, screenH), project, scaleFactor,
                                  viewAngle, brushManager());

    fullScreen->updateCursor(currentTool->toolCursor());

    nodesScaleFactor = 1;
    updateNodesScale(scaleFactor);

    connect(this, SIGNAL(colorDialogRequested(const QColor &)), fullScreen, SLOT(colorDialog(const QColor &)));
    connect(fullScreen, SIGNAL(colorChanged(TColorCell::FillType, const QColor &)),
            this, SIGNAL(colorChanged(TColorCell::FillType, const QColor &)));
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

    /* SQA: These connections don't work on Windows
    connect(this, &TupDocumentView::colorDialogRequested, fullScreen, &TupCanvas::openColorDialog);
    connect(fullScreen, &TupCanvas::colorChanged, this, &TupDocumentView::colorChanged);
    connect(fullScreen, &TupCanvas::penWidthChangedFromFullScreen, this, &TupDocumentView::penWidthChanged);
    connect(fullScreen, &TupCanvas::onionOpacityChangedFromFullScreen, this, &TupDocumentView::updateOnionOpacity);
    connect(fullScreen, &TupCanvas::zoomFactorChangedFromFullScreen, this, &TupDocumentView::updateNodesScale);
    connect(fullScreen, &TupCanvas::callAction, this, &TupDocumentView::loadPlugin);
    connect(fullScreen, &TupCanvas::requestTriggered, this, &TupDocumentView::requestTriggered);
    connect(fullScreen, &TupCanvas::localRequestTriggered, this, &TupDocumentView::localRequestTriggered);
    connect(fullScreen, &TupCanvas::rightClick, this, &TupDocumentView::fullScreenRightClick);
    connect(fullScreen, &TupCanvas::goToFrame, this, &TupDocumentView::selectFrame);
    connect(fullScreen, &TupCanvas::closeHugeCanvas, this, &TupDocumentView::closeFullScreen);
    */

    if (currentTool->toolId() == TAction::ObjectSelection)
        fullScreen->enableRubberBand();

    fullScreen->showMaximized();
}

void TupDocumentView::updateOnionOpacity(double opacity)
{
    paintArea->setOnionFactor(opacity);
    onionFactorSpin->setValue(opacity);
}

void TupDocumentView::closeFullScreen()
{
    if (fullScreenOn) {
        disconnect(this, SIGNAL(colorDialogRequested(const QColor &)), fullScreen, SLOT(openColorDialog(const QColor &)));
        disconnect(fullScreen, SIGNAL(colorChanged(TColorCell::FillType, const QColor &)),
                   this, SIGNAL(colorChanged(TColorCell::FillType, const QColor &)));
        disconnect(fullScreen, SIGNAL(penWidthChangedFromFullScreen(int)), this, SIGNAL(penWidthChanged(int)));
        disconnect(fullScreen, SIGNAL(onionOpacityChangedFromFullScreen(double)), this, SLOT(updateOnionOpacity(double)));
        disconnect(fullScreen, SIGNAL(zoomFactorChangedFromFullScreen(qreal)), this, SLOT(updateNodesScale(qreal)));
        disconnect(fullScreen, SIGNAL(callAction(int, int)), this, SLOT(loadPlugin(int, int)));
        disconnect(fullScreen, SIGNAL(requestTriggered(const TupProjectRequest *)), this, SIGNAL(requestTriggered(const TupProjectRequest *)));
        disconnect(fullScreen, SIGNAL(localRequestTriggered(const TupProjectRequest *)), this, SIGNAL(localRequestTriggered(const TupProjectRequest *)));
        disconnect(fullScreen, SIGNAL(rightClick()), this, SLOT(fullScreenRightClick()));
        disconnect(fullScreen, SIGNAL(goToFrame(int, int, int)), this, SLOT(selectFrame(int, int, int)));
        disconnect(fullScreen, SIGNAL(closeHugeCanvas()), this, SLOT(closeFullScreen()));

        fullScreen->close();
        fullScreenOn = false;
        currentTool->init(paintArea->graphicsScene());

        fullScreen = nullptr;

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

void TupDocumentView::updateToolsMenu(TAction::ActionId id, const QString &actionId)
{
    if (configurationArea->isVisible())
        configurationArea->close();

    currentTool->setToolId(id);
    QAction *action = actionManager->find(actionId);
    miscMenu->setDefaultAction(action);
    miscMenu->setActiveAction(action);
    if (!action->icon().isNull())
        miscMenu->menuAction()->setIcon(action->icon());
}

void TupDocumentView::exportImage()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupDocumentView::exportImage()]";
    #endif

    updateToolsMenu(TAction::ExportImage, "export_image");

    int sceneIndex = paintArea->currentSceneIndex();
    int frameIndex = paintArea->currentFrameIndex();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Frame As"), QDir::homePath(),
                                                    tr("Images") + " (*.png *.jpg *.svg)");
    if (!fileName.isNull()) {
        bool isOk = imagePlugin->exportFrame(frameIndex, project->getBgColor(), fileName, project->sceneAt(sceneIndex),
                                             project->getDimension(), project->getLibrary());
        updatePaintArea();
        if (isOk)
            TOsd::self()->display(TOsd::Info, tr("Frame has been exported successfully"));
        else
            TOsd::self()->display(TOsd::Error, tr("Can't export frame as image"));
    }
}

void TupDocumentView::postImage()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupDocumentView::postImage()]";
    #endif

    updateToolsMenu(TAction::PostImage, "post_image");

    int sceneIndex = paintArea->graphicsScene()->currentSceneIndex();
    int frameIndex = paintArea->graphicsScene()->currentFrameIndex();
    QString fileName = CACHE_DIR + TAlgorithm::randomString(8) + ".png";

    bool isOk = imagePlugin->exportFrame(frameIndex, project->getBgColor(), fileName, project->sceneAt(sceneIndex),
                                         project->getDimension(), project->getLibrary());
    updatePaintArea();
    if (isOk)
        emit imagePostRequested(fileName);
}

void TupDocumentView::storyboardSettings()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupDocumentView::storyboardSettings()]";
    #endif

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    TupStoryBoardDialog *storySettings = new TupStoryBoardDialog(isNetworked, imagePlugin, videoPlugin, project,
                                                                 currentSceneIndex(), this);

    connect(storySettings, SIGNAL(updateStoryboard(TupStoryboard *, int)), this, SLOT(sendStoryboard(TupStoryboard *, int)));
    connect(storySettings, SIGNAL(accepted()), paintArea, SLOT(updatePaintArea()));
    connect(storySettings, SIGNAL(rejected()), paintArea, SLOT(updatePaintArea()));
    connect(storySettings, SIGNAL(projectHasChanged()), this, SIGNAL(projectHasChanged()));

    /* SQA: These connections don't work on Windows
    connect(storySettings, &TupStoryBoardDialog::updateStoryboard, this, &TupDocumentView::sendStoryboard);
    connect(storySettings, &TupStoryBoardDialog::accepted, paintArea, &TupPaintArea::updatePaintArea);
    connect(storySettings, &TupStoryBoardDialog::rejected, paintArea, &TupPaintArea::updatePaintArea);
    connect(storySettings, &TupStoryBoardDialog::projectHasChanged, this, &TupDocumentView::projectHasChanged);
    */

    if (isNetworked)
        connect(storySettings, SIGNAL(postStoryboard(int)), this, SIGNAL(postStoryboard(int)));

    QApplication::restoreOverrideCursor();

    storySettings->show();
    storySettings->move(static_cast<int>((screen->geometry().width() - storySettings->width())/2),
                        static_cast<int>((screen->geometry().height() - storySettings->height())/2));
}

void TupDocumentView::sendStoryboard(TupStoryboard *storyboard, int sceneIndex)
{
    if (isNetworked) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupDocumentView::sendStoryboard()] - Sending storyboard...";
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
        qDebug() << "[TupDocumentView::updateStaticOpacity()]";
    #endif

    int sceneIndex = paintArea->currentSceneIndex();
    TupScene *scene = project->sceneAt(sceneIndex);
    if (scene) {
        TupBackground *bg = scene->sceneBackground();
        if (bg) {
            bg->setVectorStaticOpacity(opacity);
            TupProject::Mode mode = TupProject::Mode(spaceModeCombo->currentIndex());
            if (mode == TupProject::FRAMES_MODE || mode == TupProject::VECTOR_STATIC_BG_MODE)
                paintArea->updatePaintArea();
        }
    }
}

// SQA: This method must support multi-user notifications (pending)
void TupDocumentView::updateDynamicOpacity(double opacity)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupDocumentView::updateDynamicOpacity()]";
    #endif

   int sceneIndex = paintArea->currentSceneIndex();
   TupScene *scene = project->sceneAt(sceneIndex);
   if (scene) {
       TupBackground *bg = scene->sceneBackground();
       if (bg) {
           bg->setVectorDynamicOpacity(opacity);
           paintArea->updatePaintArea();
       }
   }
}

// SQA: This method must support multi-user notifications (pending)
void TupDocumentView::setVectorBackgroundDirection(int direction)
{
   int sceneIndex = paintArea->currentSceneIndex();
   TupScene *scene = project->sceneAt(sceneIndex);
   if (scene) {
       TupBackground *bg = scene->sceneBackground();
       if (bg)
           bg->setVectorDynamicDirection(direction);
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
           bg->setVectorDynamicShift(shift);
   }
}

void TupDocumentView::renderVectorDynamicFrame()
{
    #ifdef TUP_DEBUG
       qDebug() << "[TupDocumentView::renderVectorDynamicFrame()] - Tracing...";
    #endif

   int sceneIndex = paintArea->currentSceneIndex();
   TupScene *scene = project->sceneAt(sceneIndex);

   if (scene) {
       TupBackground *bg = scene->sceneBackground();
       if (bg)
           bg->renderVectorDynamicView();
   }
}

void TupDocumentView::fullScreenRightClick()
{ 
    if (currentTool->toolId() == TAction::Polyline)
        emit closePolyLine();

    if (currentTool->toolId() == TAction::Line)
        emit closeLine();
}

void TupDocumentView::cameraInterface()
{
    if (cameraMode) {
        TOsd::self()->display(TOsd::Warning, tr("Please, close current camera dialog first!"));
        return;
    }

    int camerasTotal = QCameraInfo::availableCameras().count();
    if (camerasTotal > 0) {
        // QList<QByteArray> cameraDevices;
        QList<QCameraInfo> cameraDevices;
        QComboBox *devicesCombo = new QComboBox;
        // foreach(const QByteArray &deviceName, QCamera::availableDevices()) {
        foreach(QCameraInfo deviceName, QCameraInfo::availableCameras()) { 
            // QCamera *device = new QCamera(deviceName);
            // QString description = device->deviceDescription(deviceName);
            QString description = deviceName.description();
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
        QCameraInfo cameraDevice = cameraDevices[0];
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

        // QDesktopWidget desktop;
        QSize projectSize = project->getDimension();

        TupCameraDialog *cameraDialog = new TupCameraDialog(devicesCombo, projectSize, resolutions);
        cameraDialog->show();
        cameraDialog->move(static_cast<int> (screen->geometry().width() - cameraDialog->width())/2,
                           static_cast<int> (screen->geometry().height() - cameraDialog->height())/2);

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

                    /* SQA: These connections don't work on Windows
                    connect(dialog, &TupBasicCameraInterface::pictureHasBeenSelected, this, &TupDocumentView::insertPictureInFrame);
                    connect(dialog, &TupBasicCameraInterface::closed, this, &TupDocumentView::updateCameraMode);
                    */

                    dialog->show();
                    dialog->move(static_cast<int> (screen->geometry().width() - dialog->width()) / 2,
                                 static_cast<int> (screen->geometry().height() - dialog->height()) / 2);
                } else {
                    TupCameraInterface *dialog = new TupCameraInterface(resolution, cameraDevices, devicesCombo, cameraDialog->cameraIndex(),
                                                                        cameraSize, photoCounter);

                    connect(dialog, SIGNAL(pictureHasBeenSelected(int, const QString)), this, SLOT(insertPictureInFrame(int, const QString)));
                    connect(dialog, SIGNAL(closed()), this, SLOT(updateCameraMode()));

                    /* SQA: These connections don't work on Windows
                    connect(dialog, &TupCameraInterface::pictureHasBeenSelected, this, &TupDocumentView::insertPictureInFrame);
                    connect(dialog, &TupCameraInterface::closed, this, &TupDocumentView::updateCameraMode);
                    */

                    dialog->show();
                    dialog->move(static_cast<int> (screen->geometry().width() - dialog->width()) / 2,
                                 static_cast<int> (screen->geometry().height() - dialog->height()) / 2);
                }
            } else { // UI for reflex cameras
                int index = cameraDialog->cameraIndex();
                TupReflexInterface *dialog = new TupReflexInterface(devicesCombo->itemText(index), resolution, cameraDevices.at(index),
                                                                    cameraSize, photoCounter);

                connect(dialog, SIGNAL(pictureHasBeenSelected(int, const QString)), this, SLOT(insertPictureInFrame(int, const QString)));
                connect(dialog, SIGNAL(closed()), this, SLOT(updateCameraMode()));

                /* SQA: These connections don't work on Windows
                connect(dialog, &TupReflexInterface::pictureHasBeenSelected, this, &TupDocumentView::insertPictureInFrame);
                connect(dialog, &TupReflexInterface::closed, this, &TupDocumentView::updateCameraMode);
                */

                dialog->show();
                dialog->move(static_cast<int> (screen->geometry().width() - dialog->width()) / 2,
                             static_cast<int> (screen->geometry().height() - dialog->height()) / 2);
            }

            QApplication::restoreOverrideCursor();
        }
    } else {
        // No devices connected!
        TOsd::self()->display(TOsd::Error, tr("No cameras detected"));
    }
}

void TupDocumentView::resizeProjectDimension(const QSize dimension)
{
    #ifdef TUP_DEBUG
       qDebug() << "[TupDocumentView::resizeProjectDimension(QSize)]";
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
        while (library->exists(key)) {
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

void TupDocumentView::openLipSyncCreator()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupDocumentView::openLipSyncCreator()]";
    #endif

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    if (currentTool->toolId() != TAction::LipSyncTool)
        papagayoAction->trigger();

    TupPapagayoApp *papagayoApp = new TupPapagayoApp(Insert, project, "", getContextIndexes(), this);
    connect(papagayoApp, SIGNAL(requestTriggered(const TupProjectRequest *)),
            this, SIGNAL(requestTriggered(const TupProjectRequest *)));

    /* SQA: This connection doesn't work on Windows
    connect(papagayoApp, &TupPapagayoApp::requestTriggered, this, &TupDocumentView::requestTriggered);
    */

    papagayoApp->show();
    papagayoApp->move(static_cast<int>((screen->geometry().width() - papagayoApp->width())/2),
                      static_cast<int>((screen->geometry().height() - papagayoApp->height())/2));
    QApplication::restoreOverrideCursor();
}

void TupDocumentView::openLipSyncCreator(const QString &lipsyncName)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupDocumentView::openLipSyncCreator(QString)] - lipsyncName -> " << lipsyncName;
    #endif

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    int sceneIndex = paintArea->currentSceneIndex();
    TupScene *scene = project->sceneAt(sceneIndex);
    if (scene) {
        TupLipSync *lipsync = scene->getLipSync(lipsyncName);
        if (lipsync) {
            QList<int> indexes;
            indexes << sceneIndex;
            indexes << scene->getLipSyncLayerIndex(lipsyncName);
            indexes << lipsync->getInitFrame();

            TupPapagayoApp *papagayoApp = new TupPapagayoApp(Update, project, lipsync, indexes, this);
            connect(papagayoApp, SIGNAL(requestTriggered(const TupProjectRequest *)),
                    this, SIGNAL(requestTriggered(const TupProjectRequest *)));

            /* SQA: This connection doesn't work on Windows
            connect(papagayoApp, &TupPapagayoApp::requestTriggered, this, &TupDocumentView::requestTriggered);
            */

            papagayoApp->show();
            papagayoApp->move(static_cast<int>((screen->geometry().width() - papagayoApp->width())/2),
                              static_cast<int>((screen->geometry().height() - papagayoApp->height())/2));
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupDocumentView::openLipSyncCreator(QString)] - "
                            "Fatal Error: Can't find lipsync -> " << lipsyncName;
            #endif
        }
    }

    QApplication::restoreOverrideCursor();
}

void TupDocumentView::papagayoManager()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupDocumentView::papagayoManager()] - Launching Papagayo plugin!";
    #endif

    if (currentTool->toolId() != TAction::LipSyncTool) {
        QAction *action = actionManager->find("export_image");
        miscMenu->setDefaultAction(action);
        miscMenu->setActiveAction(action);
        if (!action->icon().isNull())
            miscMenu->menuAction()->setIcon(action->icon());

        TupProject::Mode mode = TupProject::Mode(spaceModeCombo->currentIndex());
        if (mode != TupProject::FRAMES_MODE)
            spaceModeCombo->setCurrentIndex(TupProject::FRAMES_MODE);

        papagayoAction->trigger();
    }
}

void TupDocumentView::updatePerspective()
{
    if (currentTool) {
        if (currentTool->toolId() == TAction::LipSyncTool)
            currentTool->updateWorkSpaceContext();
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
    paintArea->updateRotParameters();
    paintArea->updateSafeParameters();

    paintArea->viewport()->update();
}

void TupDocumentView::updatePen(const QPen &pen)
{
    status->setPen(pen);
    contourColor = pen.color();
    if (currentTool) {
        if (currentTool->toolId() == TAction::Text)
            currentTool->updateTextColor(pen.color());
    }

    emit contourColorChanged(pen.color());
}

void TupDocumentView::updateBrush(const QBrush &brush)
{
    status->setBrush(brush);
    if (currentTool) {
        if (currentTool->toolId() == TAction::Text)
            currentTool->updateTextColor(brush.color());
    }

    emit fillColorChanged(brush.color());
}

void TupDocumentView::updateActiveDock(TupDocumentView::DockType dock)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupDocumentView::updateActiveDock()] - currentDock: " << dock;
    #endif

    currentDock = dock;
}

void TupDocumentView::updateCameraMode()
{
    cameraMode = false;
}

void TupDocumentView::setFillTool(TColorCell::FillType type)
{
    if (currentTool) {
        colorSpace = type;
        if (currentTool->toolId() == TAction::EyeDropper)
            currentTool->updateColorType(type);

        if (colorSpace == TColorCell::Background) {
            // Call Pencil plugin here
            if (currentTool->toolType() == TupToolInterface::Fill)
                pencilAction->trigger();
        } else {
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
}

void TupDocumentView::requestRasterStroke()
{
    TupProjectRequest request = TupRequestBuilder::createItemRequest(-1, -1, -1, 0, QPoint(), spaceContext(),
                                                                     TupLibraryObject::Item,
                                                                     TupProjectRequest::AddRasterItem, "");
    emit requestTriggered(&request);
}

void TupDocumentView::requestClearRasterCanvas()
{
    TupProjectRequest request = TupRequestBuilder::createItemRequest(-1, -1, -1, 0, QPoint(), spaceContext(),
                                                                     TupLibraryObject::Item,
                                                                     TupProjectRequest::ClearRasterCanvas, "");
    emit requestTriggered(&request);
}

void TupDocumentView::launchEyeDropperTool()
{
    #ifdef TUP_DEBUG
       qDebug() << "[TupDocumentView::launchEyeDropperTool()]";
    #endif

    enableEyeDropperTool(colorSpace);
}

void TupDocumentView::enableEyeDropperTool(TColorCell::FillType fillType)
{
    #ifdef TUP_DEBUG
       qDebug() << "[TupDocumentView::enableEyeDropperTool()]";
    #endif

    shapesMenu->setActiveAction(nullptr);
    motionMenu->setActiveAction(nullptr);
    miscMenu->setActiveAction(nullptr);

    if (eyedropperAction) {
        eyedropperAction->trigger();

        QString toolName = tr("%1").arg(eyedropperAction->text());
        TAction::ActionId toolId = eyedropperAction->actionId();

        if (currentTool) {
            if (currentTool->toolId() == TAction::Pencil)
                disconnect(currentTool, SIGNAL(penWidthChanged(int)), this, SIGNAL(penWidthChanged(int)));

            if (currentTool->toolId() == TAction::LipSyncTool) {
                disconnect(currentTool, SIGNAL(lipsyncCreatorRequested()), this, SLOT(openLipSyncCreator()));
                disconnect(currentTool, SIGNAL(lipsyncEditionRequested(QString)), this, SLOT(openLipSyncCreator(QString)));
            }

            currentTool->saveConfig();
            QWidget *toolConfigurator = currentTool->configurator();
            if (toolConfigurator)
                configurationArea->close();
        }

        TupToolPlugin *tool = qobject_cast<TupToolPlugin *>(eyedropperAction->parent());
        tool->setCurrentToolName(toolName);
        tool->setToolId(toolId);

        currentTool = tool;
        currentTool->updateColorType(fillType);

        paintArea->setCurrentTool(toolId);

        if (!eyedropperAction->icon().isNull())
            status->updateTool(toolName, eyedropperAction->icon().pixmap(15, 15));

        QWidget *toolConfigurator = tool->configurator();
        if (toolConfigurator) {
            configurationArea = new TupConfigurationArea(this);
            configurationArea->setConfigurator(toolConfigurator, 80);
            addDockWidget(Qt::RightDockWidgetArea, configurationArea);
            toolConfigurator->show();
            if (!configurationArea->isVisible())
                configurationArea->show();
        }

        paintArea->setTool(tool);
        connect(currentTool, SIGNAL(colorPicked(TColorCell::FillType,QColor)),
                                    this, SIGNAL(colorChanged(TColorCell::FillType,QColor)));
        connect(paintArea, &TupPaintArea::cursorPosition, this, &TupDocumentView::refreshEyeDropperPanel);
    } else {
        #ifdef TUP_DEBUG
           qDebug() << "[TupDocumentView::enableEyeDropperTool()] - Fatal Error: Eyedropper action is NULL!";
        #endif
    }
}

void TupDocumentView::refreshEyeDropperPanel()
{
    #ifdef TUP_DEBUG
       qDebug() << "[TupDocumentView::refreshEyeDropperPanel()]";
    #endif

    currentTool->refreshEyeDropperPanel();
}

QList<int> TupDocumentView::getContextIndexes()
{
    QList<int> indexes;
    indexes << paintArea->currentSceneIndex();
    indexes << paintArea->currentLayerIndex();
    indexes << paintArea->currentFrameIndex();

    return indexes;
}

void TupDocumentView::launchLipsyncModule(bool recorded, const QString &soundFile)
{
    #ifdef TUP_DEBUG
       qDebug() << "[TupDocumentView::launchLipsyncModule()] - soundFile -> " << soundFile;
    #endif

    papagayoManager(); // Launch plugin

    PapagayoAppMode mode = Insert;
    if (recorded)
        mode = VoiceRecorded;

    if (QFile::exists(soundFile)) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        TupPapagayoApp *papagayoApp = new TupPapagayoApp(mode, project, soundFile, getContextIndexes(), this);
        connect(papagayoApp, SIGNAL(requestTriggered(const TupProjectRequest *)),
                this, SIGNAL(requestTriggered(const TupProjectRequest *)));

        /* SQA: This connection doesn't work on Windows
        connect(papagayoApp, &TupPapagayoApp::requestTriggered, this, &TupDocumentView::requestTriggered);
        */

        papagayoApp->show();
        papagayoApp->move(static_cast<int>((screen->geometry().width() - papagayoApp->width())/2),
                          static_cast<int>((screen->geometry().height() - papagayoApp->height())/2));

        QApplication::restoreOverrideCursor();
    }
}
