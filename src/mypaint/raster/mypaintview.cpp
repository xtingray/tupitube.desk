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

#include <QGraphicsRectItem>
#include <QBrush>
#include <QPixmap>
#include <QCursor>

#include "tapplicationproperties.h"
#include "mypaintview.h"

MypaintView::MypaintView()
{
    tableInUse = false;

    mypaint = MPHandler::handler();

    connect(mypaint, SIGNAL(newTile(MPSurface*, MPTile*)), this, SLOT(onNewTile(MPSurface*, MPTile*)));
    connect(mypaint, SIGNAL(updateTile(MPSurface*, MPTile*)), this, SLOT(onUpdateTile(MPSurface*, MPTile*)));
    connect(mypaint, SIGNAL(clearedSurface(MPSurface*)), this, SLOT(onClearedSurface(MPSurface*)));

    // Set scene
    gScene.setSceneRect(this->rect());
    setScene(&gScene);
    setAlignment((Qt::AlignLeft | Qt::AlignTop));
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QCursor cursor = QCursor(THEME_DIR + "icons/cursor/pencil.png", 0, 11);
    viewport()->setCursor(cursor);
}

MypaintView::~MypaintView()
{
}

void MypaintView::setSize(QSize size)
{
    mypaint->setSurfaceSize(size);
}

void MypaintView::setTabletDevice(QTabletEvent* event)
{
    if (event->type() == QEvent::TabletEnterProximity) {
        tableInUse = true;
    } else if(event->type() == QEvent::TabletLeaveProximity) {
        tableInUse = false;
    }

    updateCursor(event);
}

void MypaintView::onNewTile(MPSurface *surface, MPTile *tile)
{
    Q_UNUSED(surface)
    gScene.addItem(tile);
}

void MypaintView::onUpdateTile(MPSurface *surface, MPTile *tile)
{
    Q_UNUSED(surface)
    tile->update();
}

void MypaintView::onClearedSurface(MPSurface *surface)
{
    Q_UNUSED(surface)
}

void MypaintView::loadBrush(const QByteArray &content)
{
    MPHandler::handler()->loadBrush(content);
}

void MypaintView::tabletEvent(QTabletEvent *event)
{
    tableInUse = true;

    switch (event->type()) {
        case QEvent::TabletPress:
            if (event->pointerType() == QTabletEvent::Pen) {
                MPHandler::handler()->startStroke();
                event->accept();
            }
        break;
        case QEvent::TabletRelease:
            if (event->pointerType() == QTabletEvent::Pen) {
                // Finalize the stroke sequence.
                event->accept();
            }
        break;
        case QEvent::TabletMove:
            if (event->pointerType() == QTabletEvent::Pen) {
                QPointF pt(mapToScene(event->pos()));
                MPHandler::handler()->strokeTo(static_cast<float>(pt.x()), static_cast<float>(pt.y()),
                                               static_cast<float>(event->pressure()), event->xTilt(), event->yTilt());
                event->accept();
            }
        break;
        default:
        break;
    }
}

void MypaintView::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    MPHandler::handler()->startStroke();
}

void MypaintView::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    if (!tableInUse) {
        QPointF pt = mapToScene(event->pos());
        MPHandler::handler()->strokeTo(static_cast<float>(pt.x()), static_cast<float>(pt.y()));
    }
}

void MypaintView::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

void MypaintView::selectColor()
{
    QPushButton* button = dynamic_cast<QPushButton*>(sender());
    if (button) {
        QColor newColor = QColorDialog::getColor(color, window(), "Select the brush color", QColorDialog::ShowAlphaChannel);
        if (newColor.isValid()) {
            button->setStyleSheet(QString("color: %1; background-color: %2;").arg((newColor.lightnessF()>0.5)?"black":"white")
                                  .arg(newColor.name()));

            MPHandler *mypaint = MPHandler::handler();
            mypaint->setBrushColor(newColor);
        }
    }
}

void MypaintView::clearCanvas()
{
    mypaint->clearSurface();
}

void MypaintView::saveToFile(QString filePath)
{
    QImage image = mypaint->renderImage();
    image.save(filePath);
}

void MypaintView::loadFromFile(QString filePath)
{
    // Clear the surface
    mypaint->clearSurface();

    // Laod the new image
    QImage image = QImage(filePath);
    mypaint->loadImage(image);
}

void MypaintView::updateCursor(const QTabletEvent *event)
{
    QCursor cursor;
    if (event->type() != QEvent::TabletLeaveProximity) {
        if (event->pointerType() == QTabletEvent::Eraser) {
            cursor = QCursor(QPixmap(":/resources/cursor-eraser.png"), 3, 28);
        } else {
            switch (event->device()) {
            case QTabletEvent::Stylus:
                cursor = QCursor(QPixmap(":/resources/cursor-pencil.png"), 0, 0);
                break;
            case QTabletEvent::Airbrush:
                cursor = QCursor(QPixmap(":/resources/cursor-airbrush.png"), 3, 4);
                break;
            case QTabletEvent::RotationStylus: {
                QImage origImg(QLatin1String(":/resources/cursor-felt-marker.png"));
                QImage img(32, 32, QImage::Format_ARGB32);
                QColor solid = color;
                solid.setAlpha(255);
                img.fill(solid);

                QPainter painter(&img);
                QTransform transform = painter.transform();
                transform.translate(16, 16);
                transform.rotate(-event->rotation());
                painter.setTransform(transform);
                painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                painter.drawImage(-24, -24, origImg);
                painter.setCompositionMode(QPainter::CompositionMode_HardLight);
                painter.drawImage(-24, -24, origImg);
                painter.end();
                cursor = QCursor(QPixmap::fromImage(img), 16, 16);
            } break;
            default:
                break;
            }
        }
    }
    setCursor(cursor);
}
