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

#include <QDebug>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

RasterMainWindow::RasterMainWindow(TupProject *project, const QString &winKey, const QColor contourColor,
                                   QWidget *parent): TMainWindow(winKey, parent)
{
    QAction *exportAction = new QAction(tr("&Export as Image"), this);
    exportAction->setShortcuts(QKeySequence::Open);
    exportAction->setStatusTip(tr("Export as Image"));
    connect(exportAction, SIGNAL(triggered()), this, SLOT(exportProject()));

    QAction *closeAction = new QAction(tr("Exit Raster Mode"), this);
    closeAction->setShortcuts(QKeySequence::Quit);
    closeAction->setStatusTip(tr("Exit Raster Mode"));
    connect(closeAction, &QAction::triggered, this, &QWidget::close);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(exportAction);
    fileMenu->addAction(closeAction);

    // Central widget:
    rasterCanvas = new RasterCanvas(project, contourColor, this);
    connect(rasterCanvas, SIGNAL(closeWindow()), this, SIGNAL(closeWindow()));
    connect(rasterCanvas, SIGNAL(zoomIn()), this, SLOT(applyZoomIn()));
    connect(rasterCanvas, SIGNAL(zoomOut()), this, SLOT(applyZoomOut()));
    setCentralWidget(rasterCanvas);

    colorWidget = new RasterColorWidget(contourColor, project->getBgColor(), this);
    connect(colorWidget, SIGNAL(paintAreaEventTriggered(const TupPaintAreaEvent *)),
            this, SIGNAL(paintAreaEventTriggered(const TupPaintAreaEvent *)));
    connect(colorWidget, SIGNAL(paintAreaEventTriggered(const TupPaintAreaEvent *)),
            this, SLOT(processColorEvent(const TupPaintAreaEvent *)));

    colorView = addToolView(colorWidget, Qt::LeftDockWidgetArea, Raster, "Brush Color", QKeySequence(tr("Shift+C")));

    brushesWidget = new RasterBrushesWidget(RASTER_DIR + "brushes");
    connect(brushesWidget, SIGNAL(brushSelected(const QByteArray&)),
            rasterCanvas, SLOT(loadBrush(const QByteArray&)));

    brushesView = addToolView(brushesWidget, Qt::LeftDockWidgetArea, Raster, "Brushes", QKeySequence(tr("Shift+B")));
    brushesView->expandDock(true);

    status = new TupPaintAreaStatus(TupPaintAreaStatus::Raster);
    connect(status, SIGNAL(zoomChanged(qreal)), this, SLOT(setZoomFactor(qreal)));
    connect(status, SIGNAL(angleChanged(int)), this, SLOT(setRotationAngle(int)));
    connect(rasterCanvas, SIGNAL(rotated(int)), status, SLOT(updateRotationAngle(int)));
    setStatusBar(status);
    status->setZoomPercent("100");

    /*
    // Add tools:
    QWidget* toolsWidget = new QWidget();
    QVBoxLayout* toolsLayout = new QVBoxLayout();
    toolsLayout->setSpacing(0);
    toolsLayout->setMargin(0);
    toolsLayout->setContentsMargins(0, 0, 0, 0);
    toolsLayout->setSizeConstraint(QLayout::SetFixedSize);

    // Open
    openBtn = new QPushButton(tr("Open"));
    toolsLayout->addWidget(openBtn);
    connect(openBtn, SIGNAL(pressed()), this, SLOT(openProject()));

    // Save
    saveBtn = new QPushButton(tr("Save"));
    toolsLayout->addWidget(saveBtn);
    connect(saveBtn, SIGNAL(pressed()), this, SLOT(exportProject()));

    // Clear
    clearBtn = new QPushButton(tr("Clear"));
    toolsLayout->addWidget(clearBtn);
    connect(clearBtn, SIGNAL(pressed()), rasterCanvas, SLOT(clearCanvas()));

    // Color selector
    colorBtn = new QPushButton(tr("Color Palette"));
    colorBtn->setMinimumHeight(60);
    colorBtn->setStyleSheet("color: white; background-color: black;");

    toolsLayout->addWidget(colorBtn);

    connect(colorBtn, SIGNAL(pressed()), rasterCanvas, SLOT(selectColor()));

    toolsWidget->setLayout(toolsLayout);

    QDockWidget* dockTools = new QDockWidget(tr("Tools"));
    dockTools->setWidget(toolsWidget);

    addDockWidget(Qt::LeftDockWidgetArea, dockTools);

    // Add a docked widget
    QDockWidget* dockBrush = new QDockWidget(tr("Brush Library"));
    brushesSelector = new RasterBrushSelector(RASTER_DIR + "brushes", nullptr);
    dockBrush->setWidget(brushesSelector);
    addDockWidget(Qt::LeftDockWidgetArea, dockBrush);

    connect(brushesSelector, SIGNAL(brushSelected(const QByteArray&)),
            rasterCanvas, SLOT(loadBrush(const QByteArray&)));
    */

    tabletIsActive = false;
}

RasterMainWindow::~RasterMainWindow()
{
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
}

/*
void RasterMainWindow::setZoomPercent(const QString &percent)
{
    status->setZoomPercent(percent);
}
*/

void RasterMainWindow::closeEvent(QCloseEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "RasterMainWindow::closeEvent(QCloseEvent)";
    #endif

    colorView->expandDock(false);
    brushesView->expandDock(false);

    TMainWindow::closeEvent(event);
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

void RasterMainWindow::exportProject()
{
    // Path
    QString initPath = QDir::homePath() + "/untitled.png";
    QString filePath = QFileDialog::getSaveFileName(this, tr("Export Image"), initPath);
    if (filePath.isEmpty())
        return; // false;

    rasterCanvas->saveToFile(filePath);
}

void RasterMainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    rasterCanvas->setSize(this->centralWidget()->rect().size());
}

void RasterMainWindow::keyPressEvent(QKeyEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "RasterMainWindow::keyPressEvent()";
    #endif

    if (event->key() == Qt::Key_F11 || event->key() == Qt::Key_Escape)
        emit closeWindow();
}
