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

#ifndef TUPSOUNDFORM_H
#define TUPSOUNDFORM_H

#include "tglobal.h"
#include "timagebutton.h"
#include "tapplicationproperties.h"
#include "tuplibraryobject.h"

#include <QWidget>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QListWidget>

/**
 * @author Gustav Gonzalez
**/

class TUPITUBE_EXPORT TupSoundForm : public QWidget
{
    Q_OBJECT

    public:
        TupSoundForm(QWidget *parent = nullptr);
        ~TupSoundForm();

        QSize sizeHint() const;
        void setSoundParams(SoundResource params, QStringList scenesList);
        void updateFrameLimits(int sceneIndex, int framesCount);

        void loadScenesCombo(QStringList scenes);

    signals:
        void soundResourceModified(SoundResource params);

    public slots:
        void updateFramesList(int index);

    private:
        void setFramesLimit(int sceneIndex, int framesCount);

        QComboBox *scenesCombo;
        QLabel *playAtLabel;
        QListWidget *framesListWidget;
        QWidget *buttonBar;
        QSpinBox *framesBox;
        TImageButton *addFrameButton;
        TImageButton *removeFrameButton;

        SoundResource soundParams;
        int currentSceneIndex;
        QString bgTrackLabel;

        QList<int> framesMaxList;
};

#endif
