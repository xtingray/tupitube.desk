/*
    Copyright © 2015 by The QTMyPaint Project

    This file is part of QTMyPaint, a Qt-based interface for MyPaint C++ library.

    QTMyPaint is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QTMyPaint is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QTMyPaint. If not, see <http://www.gnu.org/licenses/>.
*/

#include "rastermainwindow.h"
#include "tapplicationproperties.h"

#include <QDebug>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

RasterMainWindow::RasterMainWindow(TupProject *project, const QString &winKey, TupProject::Mode context, int scene,
                                   const QColor contourColor, const QString &zoomFactor, QWidget *parent): TMainWindow(winKey, parent)
{
    spaceContext = context;
    sceneIndex = scene;
    projectSize = project->getDimension();
    tupBg = project->getBackgroundFromScene(scene);

    TCONFIG->beginGroup("Raster");
    TCONFIG->setValue("ProjectWidth", projectSize.width());
    TCONFIG->setValue("ProjectHeight", projectSize.height());
    TCONFIG->sync();

    #ifdef TUP_DEBUG
        qDebug() << "RasterMainWindow::RasterMainWindow() - projectSize: " << projectSize;
        qDebug() << "RasterMainWindow::RasterMainWindow() - zoomFactor: " << zoomFactor;
    #endif

    createTopResources();
    createCentralWidget(project, contourColor);

    colorWidget = new RasterColorWidget(contourColor, project->getBgColor(), this);
    connect(colorWidget, SIGNAL(paintAreaEventTriggered(const TupPaintAreaEvent *)),
            this, SIGNAL(paintAreaEventTriggered(const TupPaintAreaEvent *)));
    connect(colorWidget, SIGNAL(paintAreaEventTriggered(const TupPaintAreaEvent *)),
            this, SLOT(processColorEvent(const TupPaintAreaEvent *)));

    colorView = addToolView(colorWidget, Qt::LeftDockWidgetArea, Raster, "Brush Color", QKeySequence(tr("Shift+C")));

    brushesWidget = new RasterBrushesWidget(RASTER_RESOURCES_DIR + "brushes");
    connect(brushesWidget, SIGNAL(brushSelected(const QByteArray&)),
            rasterCanvas, SLOT(loadBrush(const QByteArray&)));

    brushesView = addToolView(brushesWidget, Qt::LeftDockWidgetArea, Raster, "Brushes", QKeySequence(tr("Shift+B")));
    brushesView->expandDock(true);

    status = new TupPaintAreaStatus(TupPaintAreaStatus::Raster);
    connect(status, SIGNAL(resetClicked()), this, SLOT(resetWorkSpaceTransformations()));
    connect(status, SIGNAL(safeAreaClicked()), this, SLOT(drawActionSafeArea()));
    connect(status, SIGNAL(gridClicked()), this, SLOT(drawGrid()));
    connect(status, SIGNAL(zoomChanged(qreal)), this, SLOT(setZoomFactor(qreal)));
    connect(status, SIGNAL(angleChanged(int)), this, SLOT(setRotationAngle(int)));
    connect(rasterCanvas, SIGNAL(rotated(int)), status, SLOT(updateRotationAngle(int)));
    setStatusBar(status);

    status->setZoomPercent(zoomFactor);
    tabletIsActive = false;

    brushesWidget->loadInitSettings();
}

RasterMainWindow::~RasterMainWindow()
{
}

void RasterMainWindow::createTopResources()
{
    QAction *exportAction = new QAction(tr("&Export as Image"), this);
    exportAction->setShortcuts(QKeySequence::Open);
    exportAction->setStatusTip(tr("Export as Image"));
    connect(exportAction, SIGNAL(triggered()), this, SLOT(exportImage()));

    QAction *closeAction = new QAction(tr("Exit Raster Mode"), this);
    closeAction->setShortcuts(QKeySequence::Quit);
    closeAction->setStatusTip(tr("Exit Raster Mode"));
    connect(closeAction, &QAction::triggered, this, &QWidget::close);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(exportAction);
    fileMenu->addAction(closeAction);
}

void RasterMainWindow::createCentralWidget(TupProject * project, const QColor contourColor)
{
    // Central widget
    rasterCanvas = new RasterCanvas(project, contourColor, this);
    connect(rasterCanvas, SIGNAL(closeWindow()), this, SLOT(saveCanvas()));
    connect(rasterCanvas, SIGNAL(zoomIn()), this, SLOT(applyZoomIn()));
    connect(rasterCanvas, SIGNAL(zoomOut()), this, SLOT(applyZoomOut()));

    topBar = new QToolBar(tr("Raster actions"), this);
    topBar->setIconSize(QSize(16, 16));

    QAction *clearCanvas = new QAction(QIcon(THEME_DIR + "icons/new.png"), tr("Clear Canvas"), this);
    clearCanvas->setIconVisibleInMenu(true);
    clearCanvas->setShortcut(Qt::Key_Backspace);
    connect(clearCanvas, SIGNAL(triggered()), rasterCanvas, SLOT(clearCanvas()));

    QAction *undo = new QAction(QIcon(THEME_DIR + "icons/undo.png"), tr("Undo"), this);
    undo->setIconVisibleInMenu(true);
    // undo->setShortcut(QKeySequence(tr("Ctrl+Z")));
    connect(undo, SIGNAL(triggered()), rasterCanvas, SLOT(undo()));

    QAction *redo = new QAction(QIcon(THEME_DIR + "icons/redo.png"), tr("Redo"), this);
    redo->setIconVisibleInMenu(true);
    // redo->setShortcut(QKeySequence(tr("Ctrl+Y")));
    connect(redo, SIGNAL(triggered()), rasterCanvas, SLOT(redo()));

    QWidget *cEmpty0 = new QWidget();
    cEmpty0->setFixedWidth(5);

    topBar->addAction(clearCanvas);
    topBar->addWidget(cEmpty0);
    topBar->addAction(undo);
    topBar->addAction(redo);

    QString imgPath = RASTER_BG_DIR + QString::number(sceneIndex) + "/bg/";
    if (spaceContext == TupProject::RASTER_STATIC_BG_MODE) {
        // Raster Static Settings
        setWindowTitle(tr("Raster Static Background"));
        setWindowIcon(QIcon(THEME_DIR + "icons/raster_mode.png"));
        imgPath += "static_bg.png";
    } else {
        // Raster Dynamic Settings
        setWindowTitle(tr("Raster Dynamic Background"));
        setWindowIcon(QIcon(THEME_DIR + "icons/dynamic_raster_mode.png"));
        imgPath += "dynamic_bg.png";

        QLabel *dirLabel = new QLabel();
        QPixmap dirPix(THEME_DIR + "icons/mov_orientation.png");
        dirLabel->setToolTip(tr("Movement Orientation"));
        dirLabel->setPixmap(dirPix);

        QComboBox *dirCombo = new QComboBox;
        dirCombo->setToolTip(tr("Movement Orientation"));
        dirCombo->addItem(QIcon(THEME_DIR + "icons/mov_right.png"), "   " + tr("Right"));
        dirCombo->addItem(QIcon(THEME_DIR + "icons/mov_left.png"), "   " + tr("Left"));
        dirCombo->addItem(QIcon(THEME_DIR + "icons/mov_up.png"), "   " + tr("Up"));
        dirCombo->addItem(QIcon(THEME_DIR + "icons/mov_down.png"), "   " + tr("Down"));
        dirCombo->setCurrentIndex(tupBg->rasterDynamicDirection());
        connect(dirCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setBackgroundDirection(int)));

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

        QLabel *shiftLabel = new QLabel();
        QPixmap shiftPix(THEME_DIR + "icons/shift_length.png");
        shiftLabel->setToolTip(tr("Shift Length"));
        shiftLabel->setPixmap(shiftPix);

        QSpinBox *shiftSpin = new QSpinBox(this);
        shiftSpin->setSingleStep(1);
        shiftSpin->setRange(1, 1000);
        shiftSpin->setValue(tupBg->rasterDynamicShift());
        shiftSpin->setToolTip(tr("Shift Length"));
        connect(shiftSpin, SIGNAL(valueChanged(int)), this, SLOT(updateBackgroundShiftProperty(int)));

        topBar->addWidget(dEmpty0);
        topBar->addSeparator();
        topBar->addWidget(dEmpty1);
        topBar->addWidget(dirLabel);
        topBar->addWidget(dEmpty2);
        topBar->addWidget(dirCombo);
        topBar->addWidget(dEmpty3);
        topBar->addSeparator();
        topBar->addWidget(dEmpty4);
        topBar->addWidget(shiftLabel);
        topBar->addWidget(dEmpty5);
        topBar->addWidget(shiftSpin);
    }

    if (QFile::exists(imgPath)) {
        rasterCanvas->loadFromFile(imgPath);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "RasterMainWindow::createCentralWidget() - Warning: Image doesn't exist -> " + imgPath;
        #endif
    }

    QWidget *centralWidget = new QWidget();
    QVBoxLayout *centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->addWidget(topBar);
    centralLayout->addWidget(rasterCanvas);

    setCentralWidget(centralWidget);
}

void RasterMainWindow::closeEvent(QCloseEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "RasterMainWindow::closeEvent(QCloseEvent)";
    #endif

    saveCanvas();

    colorView->expandDock(false);
    brushesView->expandDock(false);

    rasterCanvas->resetMem();

    // Cleaning memory
    colorView = nullptr;
    delete colorView;

    brushesView = nullptr;
    delete brushesView;

    colorWidget = nullptr;
    delete colorWidget;

    brushesWidget = nullptr;
    delete brushesWidget;

    rasterCanvas = nullptr;
    delete rasterCanvas;

    status = nullptr;
    delete status;
    //

    TMainWindow::closeEvent(event);
}

void RasterMainWindow::resetWorkSpaceTransformations()
{
    rasterCanvas->resetWorkSpaceCenter(projectSize);
    status->setRotationAngle("0");
    status->setZoomPercent("100");
}

void RasterMainWindow::drawGrid()
{
    rasterCanvas->drawGrid(!rasterCanvas->getGridState());
}

void RasterMainWindow::drawActionSafeArea()
{
    rasterCanvas->drawActionSafeArea(!rasterCanvas->getSafeAreaState());
}

void RasterMainWindow::setZoomFactor(qreal factor)
{
    rasterCanvas->setZoom(factor);
}

void RasterMainWindow::applyZoomIn()
{
    qreal zoom = status->currentZoomFactor();
    if (zoom <= 495) {
        zoom += 5;
        status->setZoomPercent(QString::number(zoom));
    }
}

void RasterMainWindow::applyZoomOut()
{
    qreal zoom = status->currentZoomFactor();
    if (zoom >= 15) {
        zoom -= 5;
        status->setZoomPercent(QString::number(zoom));
    }
}

void RasterMainWindow::setRotationAngle(int angle)
{
    rasterCanvas->setRotationAngle(angle);
}

void RasterMainWindow::processColorEvent(const TupPaintAreaEvent *event)
{
    QColor color = qvariant_cast<QColor>(event->getData());
    if (event->getAction() == TupPaintAreaEvent::ChangePenColor) {
        rasterCanvas->updateBrushColor(color);
    } else if (event->getAction() == TupPaintAreaEvent::ChangeBgColor) {
        rasterCanvas->setBgColor(color);
    }
}

void RasterMainWindow::setTabletDevice(QTabletEvent* event)
{
    rasterCanvas->setTabletDevice(event);
}

void RasterMainWindow::openProject()
{
    // Path
    QString initPath = QDir::homePath();
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Image"), initPath);

    if (filePath.isEmpty())
        return; // false;

    rasterCanvas->loadFromFile(filePath);
}

void RasterMainWindow::exportImage()
{
    // Path
    QString initPath = QDir::homePath() + "/dynamic_bg.png";
    if (spaceContext == TupProject::RASTER_STATIC_BG_MODE)
        initPath = QDir::homePath() + "/static_bg.png";

    QString filePath = QFileDialog::getSaveFileName(this, tr("Export Image"), initPath);
    if (filePath.isEmpty()) {
        #ifdef TUP_DEBUG
            qDebug() << "RasterMainWindow::exportImage() - File path is empty: " << filePath;
        #endif
        return;
    }

    rasterCanvas->saveToFile(filePath);
}

void RasterMainWindow::saveCanvas()
{
    QString imgPath = RASTER_BG_DIR + QString::number(sceneIndex) + "/bg/";
    QString file = imgPath + "dynamic_bg.png";
    if (spaceContext == TupProject::RASTER_STATIC_BG_MODE)
        file = imgPath + "static_bg.png";

    if (!rasterCanvas->canvasIsEmpty()) {
        if (QDir().mkpath(imgPath)) {
            rasterCanvas->saveToFile(file);

            #ifdef TUP_DEBUG
                qWarning() << "RasterMainWindow::saveCanvas() - Creating PNG image!";
            #endif

            emit closeWindow(file);
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "RasterMainWindow::saveCanvas() - Error while creating raster background path!";
                qDebug() << "Image Path: " << imgPath;
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "RasterMainWindow::saveCanvas() - Nothing new to save!";
        #endif
        if (QFile::exists(file)) {
            QFile bg(file);
            bg.remove();
        }
        emit closeWindow("");
    }
}

void RasterMainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    // rasterCanvas->setSize(this->centralWidget()->rect().size());
}

void RasterMainWindow::keyPressEvent(QKeyEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "RasterMainWindow::keyPressEvent()";
    #endif

    switch(event->key()) {
        case Qt::Key_F11:
        case Qt::Key_Escape:
        case Qt::Key_Return:
            saveCanvas();
        break;
    }
}

void RasterMainWindow::setBackgroundDirection(int direction)
{
    tupBg->setRasterDynamicDirection(direction);
}

void RasterMainWindow::updateBackgroundShiftProperty(int shift)
{
    tupBg->setRasterDynamicShift(shift);
}

/*
void RasterMainWindow::undo()
{
    rasterCanvas->undo();
}

void RasterMainWindow::redo()
{
    rasterCanvas->redo();
}
*/
