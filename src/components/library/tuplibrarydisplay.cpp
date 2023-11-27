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

#include "tuplibrarydisplay.h"

#include <QBoxLayout>
#include <QGraphicsItem>
#include <QMimeData>
#include <QDrag>

TupLibraryDisplay::TupLibraryDisplay() : QWidget()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryDisplay()]";
    #endif

    previewPanel = new TupItemPreview(this);
    soundPlayer = new TupSoundPlayer();
    connect(soundPlayer, SIGNAL(muteEnabled(bool)), this, SIGNAL(muteEnabled(bool)));
    connect(soundPlayer, SIGNAL(soundResourceModified(SoundResource)),
            this, SIGNAL(soundResourceModified(SoundResource)));

    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    layout->addWidget(previewPanel);
    layout->addWidget(soundPlayer);
    layout->setContentsMargins(0, 0, 0, 0);
 
    isVisual = false;
    showDisplay();
}

TupLibraryDisplay::~TupLibraryDisplay()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupLibraryDisplay()]";
    #endif

    delete soundPlayer;
    delete previewPanel;
}

QSize TupLibraryDisplay::sizeHint() const
{
    return QWidget::sizeHint().expandedTo(QSize(100, 100));
}

void TupLibraryDisplay::reset()
{
    soundPlayer->reset();
    previewPanel->reset();
}

void TupLibraryDisplay::resetSoundPlayer()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryDisplay::resetSoundPlayer()]";
    #endif

    soundPlayer->reset();
}

void TupLibraryDisplay::render(bool flag, QGraphicsItem *item)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryDisplay::render(bool, QGraphicsItem)]";
    #endif

    isVisual = flag;
    previewPanel->render(item);
}

void TupLibraryDisplay::render(bool flag, const QPixmap &img)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryDisplay::render(bool, QImage)] - img.isNull() -> " << img.isNull();
    #endif

    isVisual = flag;
    previewPanel->render(img);
}

void TupLibraryDisplay::showDisplay()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryDisplay::showDisplay()]";
    #endif

    if (!previewPanel->isVisible()) {
        previewPanel->show();
        soundPlayer->hide();
        soundPlayerVisible = false;
    }
}

void TupLibraryDisplay::setSoundParams(SoundResource params, QStringList scenesList, QList<int> frameLimits)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryDisplay::setSoundParams()]";
    #endif

    soundPlayer->setSoundParams(params, scenesList, frameLimits);
}

void TupLibraryDisplay::updateFrameLimit(int sceneIndex, int maxFrames)
{
    soundPlayer->updateFrameLimit(sceneIndex, maxFrames);
}

void TupLibraryDisplay::showSoundPlayer()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryDisplay::showSoundPlayer()]";
    #endif

    if (!soundPlayer->isVisible()) {
        previewPanel->hide();
        soundPlayer->show();
        soundPlayerVisible = true;
    }
}

void TupLibraryDisplay::stopSoundPlayer()
{
    if (soundPlayer) {
        if (soundPlayer->isPlaying())
            soundPlayer->stopFile();
    }
}

bool TupLibraryDisplay::isSoundPanelVisible()
{
    return soundPlayerVisible;
}

QString TupLibraryDisplay::getSoundID() const
{
    return soundPlayer->getSoundID();
}

void TupLibraryDisplay::enableLipSyncInterface(SoundType soundType, QList<SoundScene> scenes)
{
    soundPlayer->enableLipSyncInterface(soundType, scenes);
}

void TupLibraryDisplay::mousePressEvent(QMouseEvent *event)
{
    if (isVisual) {
        TupItemPreview *child = static_cast<TupItemPreview*>(childAt(event->pos()));
        if (!child) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupLibraryDisplay::mousePressEvent] - Fatal Error: Library asset is NULL!";
            #endif
            return;
        }

        QPixmap pixmap = QPixmap(THEME_DIR + "icons/bitmap.png");
        QByteArray itemData;
        QDataStream dataStream(&itemData, QIODevice::WriteOnly);
        dataStream << pixmap << QPoint(event->pos() - child->pos());

        QMimeData *mimeData = new QMimeData;
        mimeData->setData("application/x-dnditemdata", itemData);
        QUrl url("asset://");
        QList<QUrl> list;
        list << url;
        mimeData->setUrls(list);

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(pixmap);

        drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryDisplay::mousePressEvent()] - No drag action.";
        #endif
    }
}
