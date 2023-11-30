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

#ifndef TUPLIBRARYDISPLAY_H
#define TUPLIBRARYDISPLAY_H

#include "tglobal.h"
#include "tupitempreview.h"
#include "tupsoundplayer.h"

#include <QWidget>
#include <QMouseEvent>

/**
 * @author Gustav Gonzalez
**/

class TUPITUBE_EXPORT TupLibraryDisplay : public QWidget
{
    Q_OBJECT

    public:
        TupLibraryDisplay();
        ~TupLibraryDisplay();

        QSize sizeHint() const;
        void reset();
        void resetSoundPlayer();
        void updateCurrentSoundPath(const QString &url);

        void render(bool isVisual, QGraphicsItem *item);
        void render(bool isVisual, const QPixmap &img);

        void showDisplay();
        void showSoundPlayer();
        bool isSoundPanelVisible();
        void setSoundParams(SoundResource params, QStringList scenesList, QList<int> frameLimits);
        void updateFrameLimit(int sceneIndex, int maxframes);

        void stopSoundPlayer();     
        QString getSoundID() const;

        void enableLipSyncInterface(SoundType soundType, QList<SoundScene> scenes);

    signals:
        void muteEnabled(bool mute);
        void soundResourceModified(SoundResource params);

    protected:
        void mousePressEvent(QMouseEvent *event);

    private:
        TupItemPreview *previewPanel;
        TupSoundPlayer *soundPlayer;
        bool isVisual;
        bool soundPlayerVisible;
};

#endif
