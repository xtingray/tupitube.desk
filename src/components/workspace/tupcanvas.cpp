/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez                                                     *
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

#include "tupcanvas.h"
#include "timagebutton.h"
#include "tuppendialog.h"
#include "tuponiondialog.h"

#include <QColorDialog>
#include <QIcon>
#include <QBoxLayout>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QBuffer>

TupCanvas::TupCanvas(QWidget *parent, Qt::WindowFlags flags, TupGraphicsScene *gScene,
                   const QPointF centerPoint, const QSize &screenSize, TupProject *work, qreal scaleFactor,
                   int angle, TupBrushManager *manager) : QFrame(parent, flags)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCanvas()]";
    #endif

    setWindowTitle("TupiTube Desk");
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/animation_mode.png")));

    hand = Right;
    scene = gScene;
    frameIndex = scene->currentFrameIndex();

    size = work->getDimension();
    currentColor = manager->penColor();
    brushManager = manager;
    project = work;

    screen = QGuiApplication::screens().at(0);

    graphicsView = new TupCanvasView(this, gScene, screenSize, size, work->getBgColor());
    connect(graphicsView, SIGNAL(rightClick()), this, SIGNAL(rightClick()));
    connect(graphicsView, SIGNAL(zoomIn()), this, SLOT(wakeUpZoomIn()));
    connect(graphicsView, SIGNAL(zoomOut()), this, SLOT(wakeUpZoomOut()));
    connect(graphicsView, SIGNAL(frameBackward()), this, SLOT(oneFrameBack()));
    connect(graphicsView, SIGNAL(frameForward()), this, SLOT(oneFrameForward()));

    graphicsView->centerOn(centerPoint);
    graphicsView->scale(scaleFactor, scaleFactor);
    graphicsView->rotate(angle);

    TImageButton *frameBackward = new TImageButton(QPixmap(THEME_DIR + "icons/frame_backward_big.png"), 50, this, true);
    frameBackward->setToolTip(tr("Frame Backward"));
    connect(frameBackward, SIGNAL(clicked()), this, SLOT(oneFrameBack()));

    TImageButton *frameForward = new TImageButton(QPixmap(THEME_DIR + "icons/frame_forward_big.png"), 50, this, true);
    frameForward->setToolTip(tr("Frame Forward"));
    connect(frameForward, SIGNAL(clicked()), this, SLOT(oneFrameForward()));

    TImageButton *pencil = new TImageButton(QPixmap(THEME_DIR + "icons/pencil_big.png"), 50, this, true);
    pencil->setToolTip(tr("Pencil"));
    connect(pencil, SIGNAL(clicked()), this, SLOT(wakeUpPencil()));

    TImageButton *polyline = new TImageButton(QPixmap(THEME_DIR + "icons/polyline_big.png"), 50, this, true);
    polyline->setToolTip(tr("Polyline"));
    connect(polyline, SIGNAL(clicked()), this, SLOT(wakeUpPolyline()));

    TImageButton *rectangle = new TImageButton(QPixmap(THEME_DIR + "icons/square_big.png"), 50, this, true);
    rectangle->setToolTip(tr("Rectangle"));
    connect(rectangle, SIGNAL(clicked()), this, SLOT(wakeUpRectangle()));

    TImageButton *ellipse = new TImageButton(QPixmap(THEME_DIR + "icons/ellipse_big.png"), 50, this, true);
    ellipse->setToolTip(tr("Ellipse"));
    connect(ellipse, SIGNAL(clicked()), this, SLOT(wakeUpEllipse()));

    TImageButton *selection = new TImageButton(QPixmap(THEME_DIR + "icons/selection_big.png"), 50, this, true);
    selection->setToolTip(tr("Selection"));
    connect(selection, SIGNAL(clicked()), this, SLOT(wakeUpSelection()));

    TImageButton *trash = new TImageButton(QPixmap(THEME_DIR + "icons/delete_big.png"), 50, this, true);
    trash->setToolTip(tr("Delete Selection"));
    trash->setShortcut(QKeySequence(Qt::Key_Backspace));
    connect(trash, SIGNAL(clicked()), this, SLOT(wakeUpDeleteSelection()));

    TImageButton *nodes = new TImageButton(QPixmap(THEME_DIR + "icons/nodes_big.png"), 50, this, true);
    nodes->setToolTip(tr("Nodes"));
    connect(nodes, SIGNAL(clicked()), this, SLOT(wakeUpNodes()));

    TImageButton *undo = new TImageButton(QPixmap(THEME_DIR + "icons/undo_big.png"), 50, this, true);
    undo->setToolTip(tr("Undo"));
    connect(undo, SIGNAL(clicked()), this, SLOT(undo()));

    TImageButton *redo = new TImageButton(QPixmap(THEME_DIR + "icons/redo_big.png"), 50, this, true);
    redo->setToolTip(tr("Redo"));
    connect(redo, SIGNAL(clicked()), this, SLOT(redo()));

    TImageButton *zoomIn = new TImageButton(QPixmap(THEME_DIR + "icons/zoom_in_big.png"), 50, this, true);
    zoomIn->setToolTip(tr("Zoom In"));
    connect(zoomIn, SIGNAL(clicked()), this, SLOT(wakeUpZoomIn()));

    TImageButton *zoomOut = new TImageButton(QPixmap(THEME_DIR + "icons/zoom_out_big.png"), 50, this, true);
    zoomOut->setToolTip(tr("Zoom Out"));
    connect(zoomOut, SIGNAL(clicked()), this, SLOT(wakeUpZoomOut()));

    TImageButton *images = new TImageButton(QPixmap(THEME_DIR + "icons/bitmap_big.png"), 50, this, true);
    images->setToolTip(tr("Images"));
    connect(images, SIGNAL(clicked()), this, SLOT(wakeUpLibrary()));

    TImageButton *color = new TImageButton(QPixmap(THEME_DIR + "icons/color_palette_big.png"), 50, this, true);
    color->setToolTip(tr("Color Palette"));
    connect(color, SIGNAL(clicked()), this, SLOT(colorDialog()));

    TImageButton *size = new TImageButton(QPixmap(THEME_DIR + "icons/pen_properties.png"), 50, this, true);
    size->setToolTip(tr("Pen Size"));
    connect(size, SIGNAL(clicked()), this, SLOT(penDialog()));

    TImageButton *onion = new TImageButton(QPixmap(THEME_DIR + "icons/onion_big.png"), 50, this, true);
    onion->setToolTip(tr("Onion Skin Factor"));
    connect(onion, SIGNAL(clicked()), this, SLOT(onionDialog()));

    TImageButton *close = new TImageButton(QPixmap(THEME_DIR + "icons/close_big.png"), 50, this, true);
    close->setToolTip(tr("Close Full Screen"));
    connect(close, SIGNAL(clicked()), this, SIGNAL(closeHugeCanvas()));

    QBoxLayout *controls = new QBoxLayout(QBoxLayout::TopToBottom);
    controls->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    controls->setContentsMargins(3, 10, 3, 3);
    controls->setSpacing(5);

    controls->addWidget(frameBackward);
    controls->addWidget(frameForward);
    controls->addWidget(pencil);
    controls->addWidget(polyline);
    controls->addWidget(rectangle);
    controls->addWidget(ellipse);
    controls->addWidget(selection);
    controls->addWidget(trash);
    controls->addWidget(nodes);
    controls->addWidget(undo);
    controls->addWidget(redo);
    controls->addWidget(zoomIn);
    controls->addWidget(zoomOut);
    controls->addWidget(images);
    controls->addWidget(color);
    controls->addWidget(size);
    controls->addWidget(onion);
    controls->addWidget(close);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    if (hand == Right)
        layout->addLayout(controls);

    layout->addWidget(graphicsView);

    if (hand == Left)
        layout->addLayout(controls);

    setLayout(layout);
}

TupCanvas::~TupCanvas()
{
}

void TupCanvas::updateCursor(const QCursor &cursor)
{
    graphicsView->viewport()->setCursor(cursor);
}

void TupCanvas::closeEvent(QCloseEvent *event)
{
    delete graphicsView; 
    event->accept();
}

void TupCanvas::colorDialog(const QColor &current)
{
    QColor color = QColorDialog::getColor(current, this);
    if (color.isValid()) { 
        currentColor = color;
        emit colorChangedFromFullScreen(color);
    }
}

void TupCanvas::colorDialog()
{
    QColor color = QColorDialog::getColor(currentColor, this);
    if (color.isValid())
        emit colorChangedFromFullScreen(color);
}

void TupCanvas::penDialog()
{
    // QDesktopWidget desktop;
    TupPenDialog *dialog = new TupPenDialog(brushManager, this);
    connect(dialog, SIGNAL(updatePen(int)), this, SIGNAL(penWidthChangedFromFullScreen(int)));

    QApplication::restoreOverrideCursor();

    dialog->show();
    dialog->move(static_cast<int> ((screen->geometry().width() - dialog->width()) / 2),
                 static_cast<int> ((screen->geometry().height() - dialog->height()) / 2));
}

void TupCanvas::onionDialog()
{
    TupOnionDialog *dialog = new TupOnionDialog(brushManager->penColor(), scene->getOpacity(), this);
    connect(dialog, SIGNAL(updateOpacity(double)), this, SLOT(setOnionOpacity(double)));

    QApplication::restoreOverrideCursor();

    dialog->show();
    dialog->move(static_cast<int> ((screen->geometry().width() - dialog->width()) / 2),
                 static_cast<int> ((screen->geometry().height() - dialog->height()) / 2));
}

void TupCanvas::setOnionOpacity(double opacity)
{
    scene->setOnionFactor(opacity);
    emit onionOpacityChangedFromFullScreen(opacity); 
}

void TupCanvas::oneFrameBack()
{
    if (frameIndex > 0) {
        frameIndex--;
        emit callAction(TAction::Arrows, TAction::Left_Arrow);
    }
}

void TupCanvas::oneFrameForward()
{
    frameIndex++;
    emit callAction(TAction::Arrows, TAction::Right_Arrow);
}

void TupCanvas::wakeUpPencil()
{
    emit callAction(TAction::BrushesMenu, TAction::Pencil);
}

void TupCanvas::wakeUpPolyline()
{
    emit callAction(TAction::BrushesMenu, TAction::Polyline);
}

void TupCanvas::wakeUpRectangle()
{
    emit callAction(TAction::BrushesMenu, TAction::Rectangle);
}

void TupCanvas::wakeUpEllipse()
{
    emit callAction(TAction::BrushesMenu, TAction::Ellipse);
}

void TupCanvas::wakeUpLibrary()
{
    QString graphicPath = QFileDialog::getOpenFileName (this, tr("Import a SVG file..."), QDir::homePath(),
                                                    tr("Vector") + " (*.svg *.png *.jpg *.jpeg *.gif)");
    if (graphicPath.isEmpty())
        return;

    QFile f(graphicPath);
    QFileInfo fileInfo(f);

    if (graphicPath.toLower().endsWith(".svg")) {
        QString tag = fileInfo.fileName();

        if (f.open(QIODevice::ReadOnly)) {
            QByteArray data = f.readAll();
            f.close();
            TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, tag,
                                        TupLibraryObject::Svg, TupProject::FRAMES_MODE, data, QString(),
                                        scene->currentSceneIndex(), scene->currentLayerIndex(), scene->currentFrameIndex());
            emit requestTriggered(&request);
        }
    } else {
        QString symName = fileInfo.fileName();

        if (f.open(QIODevice::ReadOnly)) {
            QByteArray data = f.readAll();
            f.close();

            QPixmap *pixmap = new QPixmap(graphicPath);
            int picWidth = pixmap->width();
            int picHeight = pixmap->height();
            int projectWidth = size.width();
            int projectHeight = size.height();

            if (picWidth > projectWidth || picHeight > projectHeight) {
                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("Information"));
                msgBox.setIcon(QMessageBox::Question);
                msgBox.setText(tr("Image is bigger than workspace."));
                msgBox.setInformativeText(tr("Do you want to resize it?"));
                msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
                msgBox.setDefaultButton(QMessageBox::Ok);
                msgBox.show();
                msgBox.move(static_cast<int> ((screen->geometry().width() - msgBox.width()) / 2),
                            static_cast<int> ((screen->geometry().height() - msgBox.height()) / 2));

                int answer = msgBox.exec();

                if (answer == QMessageBox::Yes) {
                    pixmap = new QPixmap();
                    QString extension = fileInfo.suffix().toUpper();
                    QByteArray ba = extension.toLatin1();
                    const char* ext = ba.data();
                    if (pixmap->loadFromData(data, ext)) {
                        QPixmap newpix;
                        if (picWidth > projectWidth)
                            newpix = QPixmap(pixmap->scaledToWidth(projectWidth, Qt::SmoothTransformation));
                        else
                            newpix = QPixmap(pixmap->scaledToHeight(projectHeight, Qt::SmoothTransformation));
                        QBuffer buffer(&data);
                        buffer.open(QIODevice::WriteOnly);
                        newpix.save(&buffer, ext);
                    }
                }
           }

           QString tag = symName;

           TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, tag,
                                                                               TupLibraryObject::Image, TupProject::FRAMES_MODE, data, QString(),
                                                                               scene->currentSceneIndex(), scene->currentLayerIndex(), scene->currentFrameIndex());
           emit requestTriggered(&request);

           data.clear();
        }
    }
}


void TupCanvas::wakeUpSelection()
{
    emit callAction(TAction::SelectionMenu, TAction::ObjectSelection);
}

void TupCanvas::wakeUpNodes()
{
    emit callAction(TAction::SelectionMenu, TAction::NodesEditor);
}

void TupCanvas::wakeUpDeleteSelection()
{
    emit callAction(TAction::SelectionMenu, TAction::Delete);
}

void TupCanvas::wakeUpZoomIn()
{
    graphicsView->scale(1.3, 1.3);

    emit zoomFactorChangedFromFullScreen(1.3);
}

void TupCanvas::wakeUpZoomOut()
{
    graphicsView->scale(0.7, 0.7);

    emit zoomFactorChangedFromFullScreen(0.7);
}

void TupCanvas::undo()
{
    QAction *undo = kApp->findGlobalAction("undo");
    if (undo) 
        undo->trigger();
}

void TupCanvas::redo()
{
    QAction *redo = kApp->findGlobalAction("redo");
    if (redo) 
        redo->trigger();
}

void TupCanvas::enableRubberBand()
{
    graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
}
