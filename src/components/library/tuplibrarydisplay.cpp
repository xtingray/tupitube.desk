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

TupLibraryDisplay::TupLibraryDisplay() : QWidget()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryDisplay()]";
    #endif

    previewPanel = new TupItemPreview(this);
    soundPlayer = new TupSoundPlayer(this);
    connect(soundPlayer, SIGNAL(frameUpdated(int)), this, SIGNAL(frameUpdated(int)));
    connect(soundPlayer, SIGNAL(muteEnabled(bool)), this, SIGNAL(muteEnabled(bool)));

    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->addWidget(previewPanel);
    layout->addWidget(soundPlayer);
    layout->setContentsMargins(0, 0, 0, 0);
 
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

    soundPlayer->resetMediaPlayer();
}

void TupLibraryDisplay::render(QGraphicsItem *item)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryDisplay::render(QGraphicsItem)]";
    #endif

    previewPanel->render(item);
}

void TupLibraryDisplay::render(const QPixmap &img)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryDisplay::render(QImage)] - img.isNull() -> " << img.isNull();
    #endif

    previewPanel->render(img);
}

void TupLibraryDisplay::showDisplay()
{
    if (!previewPanel->isVisible()) {
        previewPanel->show();
        soundPlayer->hide();
    }
}

void TupLibraryDisplay::setSoundParams(TupLibraryObject *sound)
{
    soundPlayer->setSoundParams(sound);
}

void TupLibraryDisplay::showSoundPlayer()
{
    if (!soundPlayer->isVisible()) {
        previewPanel->hide();
        soundPlayer->show();
    }
}

void TupLibraryDisplay::stopSoundPlayer()
{
    if (soundPlayer->isVisible()) {
        if (soundPlayer->isPlaying())
            soundPlayer->stopFile();
    }
}

bool TupLibraryDisplay::isSoundPanelVisible()
{
    return soundPlayer->isVisible();
}

QString TupLibraryDisplay::getSoundID() const
{
    return soundPlayer->getSoundID();
}

void TupLibraryDisplay::updateSoundInitFrame(int frame)
{
    soundPlayer->updateInitFrame(frame);
}

void TupLibraryDisplay::enableLipSyncInterface(SoundType soundType, int frame)
{
    soundPlayer->enableLipSyncInterface(soundType, frame);
}
