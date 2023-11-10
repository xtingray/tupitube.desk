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

#ifndef TUPSOUNDOBJECT_H
#define TUPSOUNDOBJECT_H

#include "tglobal.h"
#include "tupabstractserializable.h"

class TUPITUBE_EXPORT TupSoundObject : public QObject, public TupAbstractSerializable
{
    public:
        TupSoundObject(QObject *parent = nullptr);
        ~TupSoundObject();

        void enableMute(bool flag);
        bool isMuted();

        QList<SoundScene> getAudioScenes();
        SoundScene getAudioSceneAt(int sceneIndex);
        QList<int> getAudioFrames(int sceneIndex);
        void updateFramesToPlay(int sceneIndex, QList<int> frames);

        void addSceneToPlay(SoundScene scene);
        void updateScene(int sceneIndex, SoundScene scene);
        void removeSceneToPlay(int sceneIndex);

        SoundType getSoundType();
        void setSoundType(SoundType type);

    public:
        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;

    private:
        SoundType soundType;
        bool mute;
        QList<SoundScene> audioScenes;
};

#endif
