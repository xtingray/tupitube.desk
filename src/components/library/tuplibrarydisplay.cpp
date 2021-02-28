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
    previewPanel = new TupItemPreview(this);
    soundPlayer = new TupSoundPlayer(this);
    connect(soundPlayer, SIGNAL(frameUpdated(int)), this, SIGNAL(frameUpdated(int)));

    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->addWidget(previewPanel);
    layout->addWidget(soundPlayer);
    layout->setContentsMargins(0, 0, 0, 0);
 
    showDisplay();
}

TupLibraryDisplay::~TupLibraryDisplay()
{
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

void TupLibraryDisplay::render(QGraphicsItem *item)
{
    previewPanel->render(item);
}

void TupLibraryDisplay::render(const QImage &img)
{
    previewPanel->render(img);
}

void TupLibraryDisplay::showDisplay()
{
    if (!previewPanel->isVisible()) {
        previewPanel->show();
        soundPlayer->hide();
    }
}

void TupLibraryDisplay::setSoundParams(const QString &path, int frameIndex)
{
    soundPlayer->setSoundParams(path, frameIndex);
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
