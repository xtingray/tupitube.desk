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

#include "tupsoundobject.h"
#include "tupitemfactory.h"
#include "tuppixmapitem.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QTextStream>
#include <QDir>

TupSoundObject::TupSoundObject(QObject *parent) : QObject(parent)
{
    soundType = NoSound;
    mute = false;

    SoundScene initRecord;
    initRecord.sceneIndex = 0;
    initRecord.frames[0] = 0;
    audioScenes << initRecord;
}

TupSoundObject::~TupSoundObject()
{
}

void TupSoundObject::enableMute(bool flag)
{
    mute = flag;
}

bool TupSoundObject::isMuted()
{
    return mute;
}

void TupSoundObject::setAudioScenes(QList<SoundScene> scenes)
{
    audioScenes = scenes;
}

QList<SoundScene> TupSoundObject::getAudioScenes()
{
    return audioScenes;
}

SoundScene TupSoundObject::getAudioSceneAt(int sceneIndex)
{
    if (sceneIndex < audioScenes.size())
        return audioScenes.at(sceneIndex);

    SoundScene scene;
    return scene;
}

QList<int> TupSoundObject::getFramesToPlayAt(int sceneIndex)
{
    if (sceneIndex < audioScenes.size()) {
        SoundScene scene = audioScenes.at(sceneIndex);
        return scene.frames;
    }

    return QList<int>();
}

void TupSoundObject::updateFramesToPlay(int sceneIndex, QList<int> frames)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundObject::updateFramesToPlay()] - sceneIndex ->" << sceneIndex;
    #endif

    if (sceneIndex < audioScenes.size()) {
        SoundScene scene = audioScenes.at(sceneIndex);
        scene.frames = frames;
        audioScenes.replace(sceneIndex, scene);
    }
}

void TupSoundObject::addSceneToPlay(SoundScene scene)
{
    audioScenes << scene;
}

void TupSoundObject::updateSoundScene(int sceneIndex, SoundScene scene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundObject::updateSoundScene()] - scene -> " << sceneIndex;
    #endif

    if (sceneIndex < audioScenes.count())
        audioScenes.replace(sceneIndex, scene);
}

void TupSoundObject::removeSceneToPlay(int sceneIndex)
{
    if (sceneIndex < audioScenes.count())
        audioScenes.removeAt(sceneIndex);
}

SoundType TupSoundObject::getSoundType()
{
    return soundType;
}

void TupSoundObject::setSoundType(SoundType type)
{
    soundType = type;
}

void TupSoundObject::fromXml(const QString &xml)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundObject::fromXml()] - xml -> " << xml;
    #endif

    QDomDocument document;
    if (!document.setContent(xml)) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupSoundObject::fromXml()] - Fatal Error: Invalid XML structure!";
        #endif

        return;
    }

    QDomElement root = document.documentElement();
    if (root.tagName() == "sound") {
        soundType = SoundType(root.attribute("soundType").toInt());
        mute = root.attribute("mute", "true").toInt() ? true : false;

        QDomNode n = root.firstChild();
        while (!n.isNull()) {
            QDomElement e = n.toElement();
            if (!e.isNull()) {
                if (e.tagName() == "scene") {
                    int sceneIndex = e.attribute("index").toInt();
                    QList<QString> framesArray = e.attribute("frames").split(",");
                    QList<int> frames;
                    foreach(QString frame, framesArray)
                        frames << frame.toInt();

                    SoundScene record;
                    record.sceneIndex = sceneIndex;
                    record.frames = frames;

                    audioScenes << record;
                }
            }

            n = n.nextSibling();
        } // end while
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupSoundObject::fromXml()] - Fatal Error: XML element is NOT a sound!";
        #endif
    }
}

/*
   <sound soundType="2" mute="0">
     <scene index="0" frames="0,20,50" />
     <scene index="1" frames="0,10,32" />
     <scene index="2" frames="0,15,27" />
   </sound>
*/
QDomElement TupSoundObject::toXml(QDomDocument &doc) const
{
    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupSoundObject::toXml()] - soundType -> " << soundType;
        qDebug() << "[TupSoundObject::toXml()] - mute -> " << mute;
    #endif

    QDomElement root = doc.createElement("sound");
    root.setAttribute("soundType", soundType);
    root.setAttribute("mute", mute);

    QString framesString = "";
    for (int i = 0; i < audioScenes.count(); i++) {
         SoundScene scene = audioScenes.at(i);
         QList<int> frames = scene.frames;
         if (!frames.isEmpty()) {
             for (int j = 0; j < frames.count(); j++)
                  framesString = QString::number(frames.at(j)) + ",";
             framesString.chop(1);
         }

         QDomElement element = doc.createElement("scene");
         element.setAttribute("index", scene.sceneIndex);
         element.setAttribute("frames", framesString);
         root.appendChild(element);
    }

    return root;
}
