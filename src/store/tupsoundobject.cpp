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
    backgroundTrack = false;
}

TupSoundObject::~TupSoundObject()
{
}

void TupSoundObject::setDefaultValues()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundObject::setDefaultValues()]";
    #endif

    SoundScene scene;
    scene.sceneIndex = 0;
    scene.frames << 1;
    audioScenes << scene;
}

void TupSoundObject::setMute(bool flag)
{
    mute = flag;
}

bool TupSoundObject::isMuted()
{
    return mute;
}

void TupSoundObject::setBackgroundTrack(bool flag)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundObject::setBackgroundTrack()] - flag ->" << flag;
    #endif

    backgroundTrack = flag;
}

bool TupSoundObject::isBackgroundTrack()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundObject::setBackgroundTrack()] - backgroundTrack ->" << backgroundTrack;
    #endif

    return backgroundTrack;
}

void TupSoundObject::setDuration(const QString &time)
{
    duration = time;
}

QString TupSoundObject::getDuration() const
{
    return duration;
}

void TupSoundObject::setAudioScenes(QList<SoundScene> scenes)
{
    audioScenes = scenes;
}

QList<SoundScene> TupSoundObject::getAudioScenes()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundObject::getAudioScenes()] - audioScenes.count() ->" << audioScenes.count();
    #endif

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
        qDebug() << "[TupSoundObject::updateFramesToPlay()] - frames ->" << frames;
    #endif

    for(int i=0; i<audioScenes.size(); i++) {
        SoundScene scene = audioScenes.at(i);
        if (scene.sceneIndex == sceneIndex) {
            scene.frames = frames;
            audioScenes.replace(i, scene);

            return;
        }
    }

    SoundScene scene;
    scene.sceneIndex = sceneIndex;
    scene.frames = frames;
    audioScenes << scene;
}

void TupSoundObject::addSceneToPlay(SoundScene scene)
{
    audioScenes << scene;
}

void TupSoundObject::updateSoundScene(int sceneIndex, SoundScene scene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundObject::updateSoundScene()] - scene ->" << sceneIndex;
    #endif

    if (sceneIndex < audioScenes.count())
        audioScenes.replace(sceneIndex, scene);
}

void TupSoundObject::swapSoundScenes(int sceneIndex, int newSceneIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundObject::swapSoundScenes()] - sceneIndex ->" << sceneIndex;
        qDebug() << "[TupSoundObject::swapSoundScenes()] - newSceneIndex ->" << newSceneIndex;
    #endif

    for(int i=0; i<audioScenes.size(); i++) {
        SoundScene scene = audioScenes.at(i);
        if (scene.sceneIndex == sceneIndex) {
            scene.sceneIndex = newSceneIndex;
            audioScenes.replace(i, scene);
        } else if (scene.sceneIndex == newSceneIndex) {
            scene.sceneIndex = sceneIndex;
            audioScenes.replace(i, scene);
        }
    }
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
        qDebug() << "[TupSoundObject::fromXml()] - xml ->" << xml;
    #endif

    if (xml.isEmpty()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupSoundObject::fromXml()] - Fatal Error: XML string is empty!";
        #endif

        return;
    }

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
        backgroundTrack = root.attribute("backgroundTrack", "false").toInt() ? true : false;
        duration = root.attribute("duration");

        QDomNode n = root.firstChild();
        while (!n.isNull()) {
            QDomElement e = n.toElement();
            if (!e.isNull()) {
                if (e.tagName() == "scene") {
                    int sceneIndex = e.attribute("index", 0).toInt();
                    SoundScene record;
                    record.sceneIndex = sceneIndex;
                    QString framesInput = e.attribute("frames");
                    if (!framesInput.isEmpty()) {
                        QList<QString> framesArray = framesInput.split(",");
                        if (!framesArray.isEmpty()) {
                            QList<int> frames;
                            foreach(QString frame, framesArray) {
                                if (!frame.isEmpty())
                                    frames << frame.toInt();
                            }
                            record.frames = QList<int>(frames);
                        }
                    }

                    audioScenes << record;
                }
            }

            n = n.nextSibling();
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupSoundObject::fromXml()] - Fatal Error: XML element is NOT a sound!";
        #endif
    }
}

/*
   <sound soundType="2" mute="0" backgroundTrack="0">
     <scene index="0" frames="0,20,50" />
     <scene index="1" frames="0,10,32" />
     <scene index="2" frames="0,15,27" />
   </sound>
*/
QDomElement TupSoundObject::toXml(QDomDocument &doc) const
{
    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupSoundObject::toXml()] - soundType ->" << soundType;
        qDebug() << "[TupSoundObject::toXml()] - mute ->" << mute;
        qDebug() << "[TupSoundObject::toXml()] - backgroundTrack ->" << backgroundTrack;
    #endif

    QDomElement root = doc.createElement("sound");
    root.setAttribute("soundType", soundType);
    root.setAttribute("mute", mute);
    root.setAttribute("backgroundTrack", backgroundTrack);
    root.setAttribute("duration", duration);

    QString framesString = "";
    for (int i = 0; i < audioScenes.count(); i++) {
         SoundScene scene = audioScenes.at(i);
         QList<int> frames = scene.frames;
         if (!frames.isEmpty()) {
             for (int j = 0; j < frames.count(); j++)
                  framesString = QString::number(frames.at(j)) + ",";
             framesString.chop(1);

             QDomElement element = doc.createElement("scene");
             element.setAttribute("index", scene.sceneIndex);
             element.setAttribute("frames", framesString);
             root.appendChild(element);
         }
    }

    return root;
}
