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

RasterMainWindow::RasterMainWindow(QWidget *parent): QMainWindow(parent)
{
    /*
    QAction *openAction = new QAction(tr("&Open"), this);
    openAction->setShortcuts(QKeySequence::Open);
    openAction->setStatusTip(tr("Open Image"));
    connect(openAction, SIGNAL(triggered()), this, SLOT(openProject()));

    QAction *exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAction);
    fileMenu->addAction(exitAction);
    */

    // Central widget:
    canvas = new MypaintView();
    setCentralWidget(canvas);

    // Add tools:
    QWidget* toolsWidget = new QWidget();
    QVBoxLayout* toolsLayout = new QVBoxLayout();
    toolsLayout->setSpacing(0);
    toolsLayout->setMargin(0);
    toolsLayout->setContentsMargins(0, 0, 0, 0);
    toolsLayout->setSizeConstraint(QLayout::SetFixedSize);

    // Open
    openBtn = new QPushButton("Open");
    toolsLayout->addWidget(openBtn);
    connect(openBtn, SIGNAL(pressed()), this, SLOT(openProject()));

    // Save
    saveBtn = new QPushButton("Save");
    toolsLayout->addWidget(saveBtn);
    connect(saveBtn, SIGNAL(pressed()), this, SLOT(saveProject()));

    // Clear
    clearBtn = new QPushButton("Clear");
    toolsLayout->addWidget(clearBtn);
    connect(clearBtn, SIGNAL(pressed()), canvas, SLOT(clearCanvas()));

    // Color selector
    colorBtn = new QPushButton("Click to select another brush color...");
    colorBtn->setMinimumHeight(60);
    colorBtn->setStyleSheet("color: white; background-color: black;");

    toolsLayout->addWidget(colorBtn);

    connect(colorBtn, SIGNAL(pressed()), canvas, SLOT(selectColor()));

    toolsWidget->setLayout(toolsLayout);

    QDockWidget* dockTools = new QDockWidget("Tools");
    dockTools->setWidget(toolsWidget);

    addDockWidget(Qt::RightDockWidgetArea, dockTools);

    // Add a docked widget
    QDockWidget* dockBrush = new QDockWidget("Brush Library");
    brushesSelector = new MPBrushSelector(":brushes", nullptr);
    dockBrush->setWidget(brushesSelector);
    addDockWidget(Qt::RightDockWidgetArea, dockBrush);

    connect(brushesSelector, SIGNAL(brushSelected(const QByteArray&)),
            canvas, SLOT(loadBrush(const QByteArray&)));

    tabletIsActive = false;
}

RasterMainWindow::~RasterMainWindow()
{
}

void RasterMainWindow::setTabletDevice(QTabletEvent* event)
{
    canvas->setTabletDevice(event);
}

void RasterMainWindow::openProject()
{
    // Path
    QString initPath = QDir::homePath();
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Image"), initPath);

    if (filePath.isEmpty())
        return; // false;

    canvas->loadFromFile(filePath);
}

void RasterMainWindow::saveProject()
{
    // Path
    QString initPath = QDir::homePath() + "/untitled.png";
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Image"), initPath);
    if (filePath.isEmpty())
        return; // false;

    canvas->saveToFile(filePath);
}

void RasterMainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    canvas->setSize(this->centralWidget()->rect().size());
}

void RasterMainWindow::keyPressEvent(QKeyEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "RasterMainWindow::keyPressEvent()";
    #endif

    if (event->key() == Qt::Key_F11 || event->key() == Qt::Key_Escape)
        emit closeWindow();
}
