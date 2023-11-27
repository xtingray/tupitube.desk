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

#include "tuplibraryobject.h"
#include "tupitemfactory.h"
#include "tuppixmapitem.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QTextStream>
#include <QDir>

TupLibraryObject::TupLibraryObject(QObject *parent) : QObject(parent)
{
}

TupLibraryObject::TupLibraryObject(const QString &name, const QString &dir, TupLibraryObject::ObjectType type,
                                   QObject *parent) : QObject(parent)
{
    setSymbolName(name);
    folder = dir;
    objectType = type;
    if (type == TupLibraryObject::Audio)
        soundObject = new TupSoundObject();
}

TupLibraryObject::~TupLibraryObject()
{
}

void TupLibraryObject::setData(const QVariant &input)
{
    data = input;
}

QVariant TupLibraryObject::getData() const
{
    return data;
}

void TupLibraryObject::setDataPath(const QString &path)
{
    dataPath = path;
}

QString TupLibraryObject::getDataPath() const
{
    return dataPath;
}

void TupLibraryObject::setObjectType(TupLibraryObject::ObjectType type)
{
    objectType = type;
}

TupLibraryObject::ObjectType TupLibraryObject::getObjectType() const
{
    return objectType;
}

void TupLibraryObject::setItemType(TupLibraryObject::ItemType type)
{
    itemType = type;
}

TupLibraryObject::ItemType TupLibraryObject::getItemType() const
{
    return itemType;
}

void TupLibraryObject::enableMute(bool flag)
{
    soundObject->setMute(flag);
}

bool TupLibraryObject::isMuted()
{
    return soundObject->isMuted();
}

void TupLibraryObject::enableBackgroundTrack(bool flag)
{
    soundObject->setBackgroundTrack(flag);
}

bool TupLibraryObject::isBackgroundTrack()
{
    return soundObject->isBackgroundTrack();
}

SoundType TupLibraryObject::getSoundType()
{
    return soundObject->getSoundType();
}

void TupLibraryObject::setSoundType(SoundType type)
{
    soundObject->setSoundType(type);
}

SoundResource TupLibraryObject::getSoundResourceParams()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryObject::getSoundResourceParams()]";
    #endif

    SoundResource params;
    if (soundObject) {
        params.key = symbolName;
        params.scenes = soundObject->getAudioScenes();
        params.path = dataPath;
        params.muted = soundObject->isMuted();
        params.type = soundObject->getSoundType();
        params.isBackgroundTrack = soundObject->isBackgroundTrack();
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryObject::getSoundResourceParams()] - Fatal Error: soundObject is NULL!";
        #endif
    }

    return params;
}

void TupLibraryObject::updateSoundResourceParams(SoundResource params)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryObject::updateSoundResourceParams()] - params ->" << params.isBackgroundTrack;
    #endif

    soundObject->setMute(params.muted);
    soundObject->setSoundType(params.type);
    soundObject->setBackgroundTrack(params.isBackgroundTrack);
    soundObject->setAudioScenes(params.scenes);
}

void TupLibraryObject::setAudioScenes(QList<SoundScene> audioScenes)
{
    soundObject->setAudioScenes(audioScenes);
}

QList<SoundScene> TupLibraryObject::getAudioScenes()
{
    return soundObject->getAudioScenes();
}

QList<int> TupLibraryObject::framesToPlayAt(int sceneIndex)
{
    return soundObject->getFramesToPlayAt(sceneIndex);
}

void TupLibraryObject::updateFramesToPlay(int sceneIndex, QList<int> frames)
{
    soundObject->updateFramesToPlay(sceneIndex, frames);
}

void TupLibraryObject::setSymbolName(const QString &name)
{
    symbolName = name;
    // SQA: Check if this line is really required
    symbolName.replace(QDir::separator(), "-");
    shortId = symbolName.section('.', 0, 0);
    extension = symbolName.section('.', 1, 1).toUpper();
}

QString TupLibraryObject::getSymbolName() const
{
    return symbolName;
}

void TupLibraryObject::setFolder(const QString &dir)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryObject::setFolder()] - folder -> " << dir;
    #endif

    folder = dir;
}

void TupLibraryObject::updateFolder(const QString &projectPath, const QString &newFolder)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryObject::updateFolder()] - "
                    "current folder -> " << dataPath << " - new folder -> " << newFolder;
    #endif

    QFileInfo finfo(dataPath);
    QString filename = finfo.fileName();
    QString root = projectPath;

    switch (objectType) {
        case TupLibraryObject::Image:
        {
            root += "/images/";
        }
        break;
        case TupLibraryObject::Audio:
        {
            root += "/audio/";
        }
        break;
        case TupLibraryObject::Svg:
        {
            root += "/svg/";
        }
        break;
        case TupLibraryObject::Item:
        {
            root += "/obj/";
        }
        break;
        default:
        break;
    }

    QString newPath = root;
    QDir dir(newPath);
    if (!newFolder.isEmpty()) {
        newPath += newFolder + "/";
        if (!dir.exists(newPath)) {
            if (dir.mkpath(newPath)) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupLibraryObject::updateFolder()] - "
                                "Path created successfully -> " << newPath;
                #endif
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupLibraryObject::updateFolder()] - "
                                "Fatal Error: Couldn't create path -> " << newPath;
                #endif

                return;
            }
        }
    } else {
        if (objectType == Audio) {
            if (soundObject->getSoundType() == Lipsync) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupLibraryObject::updateFolder()] - "
                                "Fatal Error: New lipsync audio folder can't be empty!";
                #endif
            }
        }
    }
    newPath += filename;

    if (dataPath.compare(newPath) != 0) { // if old path is different
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryObject::updateFolder()] - old dataPath -> " << dataPath;
        #endif
        if (dir.rename(dataPath, newPath)) {
            folder = newFolder;
            dataPath = newPath;
            #ifdef TUP_DEBUG
                qDebug() << "[TupLibraryObject::updateFolder()] - new dataPath -> " << dataPath;
            #endif
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupLibraryObject::updateFolder()] - Fatal Error: Couldn't rename folder -> "
                         << dataPath << " to " << newPath;
            #endif
        }
    }
}

QString TupLibraryObject::getFolder() const
{
    return folder;
}

QString TupLibraryObject::getShortId() const
{
    return shortId;
}

QString TupLibraryObject::getExtension() const
{
    return extension;
}

void TupLibraryObject::fromXml(const QString &xml)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryObject::fromXml()] - xml -> " << xml;
    #endif

    QDomDocument document;
    if (!document.setContent(xml)) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryObject::fromXml()] - Fatal Error: Invalid XML structure!";
        #endif

        return;
    }
    
    QDomElement objectTag = document.documentElement();
    if (objectTag.tagName() == "object") {
        setSymbolName(objectTag.attribute("id"));
        if (symbolName.isEmpty()) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupLibraryObject::fromXml()] - Fatal Error: Symbol name is empty!";
            #endif

            return;
        }
       
        bool isOk = false; 
        int index = objectTag.attribute("type").toInt(&isOk);
        if (isOk) {
            objectType = TupLibraryObject::ObjectType(index);
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupLibraryObject::fromXml()] - Fatal Error: Invalid object type!";
            #endif

            return;
        }

        switch (objectType) {
            case TupLibraryObject::Image:
            case TupLibraryObject::Svg:
            case TupLibraryObject::Item:
             {
                 if (xml.startsWith("<text")) {
                     itemType = Text;
                     if (!objectTag.isNull()) {
                         QDomElement objectData = objectTag.firstChild().toElement();
                         QString data;
                         {
                             QTextStream ts(&data);
                             ts << objectData;
                         }

                         QByteArray array = data.toLocal8Bit();
                         if (!array.isEmpty() && !array.isNull()) {
                             loadRawData(array);
                         } else {
                             #ifdef TUP_DEBUG
                                 qDebug() << "[TupLibraryObject::fromXml()] - "
                                             "Object data is empty! -> " << symbolName;
                             #endif

                             return;
                         }
                     } else {
                         #ifdef TUP_DEBUG
                             qDebug() << "[TupLibraryObject::fromXml()] - "
                                         "Fatal Error: Object data from xml is NULL -> " << symbolName;
                         #endif

                         return;
                     }
                 } else {
                     if (xml.startsWith("<path"))
                        itemType = Path;
                     else if (xml.startsWith("<ellipse"))
                        itemType = Ellipse;
                     else if (xml.startsWith("<rect"))
                         itemType = Rect;
                     else if (xml.startsWith("<group"))
                         itemType = Group;

                     dataPath = objectTag.attribute("path");
                     int index = dataPath.lastIndexOf("/");
                     if (index > 0)
                         folder = dataPath.left(index);
                 }
             }
            break;
            case TupLibraryObject::Audio:
             {
                 /*
                 <object path="audio.mp3" id="audio.mp3" type="3">
                   <sound mute="0" soundType="2">
                     <scene index="0" frames="0,20,50" />
                     <scene index="1" frames="0,10,32" />
                     <scene index="2" frames="0,15,27" />
                   </sound>
                 </object>
                 */

                 dataPath = objectTag.attribute("path");
                 int index = dataPath.lastIndexOf("/");
                 if (index > 0)
                     folder = dataPath.left(index);

                 if (!objectTag.isNull()) {
                     soundObject = new TupSoundObject();
                     if (objectTag.hasChildNodes()) {
                         QDomElement objectData = objectTag.firstChild().toElement();
                         QString data;
                         {
                             QTextStream ts(&data);
                             ts << objectData;
                         }

                         soundObject->fromXml(data);
                     } else {
                         // Old versions of TupiTube
                         soundObject->setDefaultValues();
                     }
                 }
             }
            break;
            default:
             {
                 #ifdef TUP_DEBUG
                     qDebug() << "[TupLibraryObject::fromXml()] - "
                                 "Unknown object type -> " << objectType;
                 #endif

                 return;
             }
        }
    }
}

QDomElement TupLibraryObject::toXml(QDomDocument &doc) const
{
    QDomElement object = doc.createElement("object");
    object.setAttribute("id", symbolName);
    object.setAttribute("type", objectType);
    QFileInfo finfo(dataPath);
    QString path = finfo.fileName();
    if (!folder.isEmpty()) {
        path = folder + "/" + finfo.fileName();
    } else {
        if (objectType == Audio) {
            if (soundObject->getSoundType() == Lipsync) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupLibraryObject::toXml()] - "
                                "Fatal Error: Lipsync audio folder can't be empty!";
                #endif
            }
        }
    }

    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupLibraryObject::toXml()] - id -> " << symbolName;
        qDebug() << "[TupLibraryObject::toXml()] - folder -> " << folder;
        qDebug() << "[TupLibraryObject::toXml()] - dataPath -> " << dataPath;
        qDebug() << "[TupLibraryObject::toXml()] - Saving element -> " << path;
    #endif

    switch (objectType) {
            case Image:
            case Svg:
            case Item:
            {
                if (itemType == Text) {
                    QGraphicsItem *item = qvariant_cast<QGraphicsItem *>(data);
                    if (item) {
                        if (TupAbstractSerializable *serializable = dynamic_cast<TupAbstractSerializable *>(item))
                            object.appendChild(serializable->toXml(doc));
                    }
                } else {
                    object.setAttribute("path", path);
                }
            }
            break;
            case Audio:
            {
                object.setAttribute("path", path);
                object.appendChild(soundObject->toXml(doc));

                /*
                object.setAttribute("soundType", soundType);
                object.setAttribute("mute", mute);
                object.setAttribute("playAt", playAtFrame);
                */
            }
            break;
            default:
            {
            }
    }
    
    return object;
}

bool TupLibraryObject::loadRawData(const QByteArray &data)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryObject::loadRawData()]";
    #endif

    rawData = data;

    switch (objectType) {
            case TupLibraryObject::Image:
            {
                 QPixmap pixmap;
                 bool isOk = pixmap.loadFromData(data);
                 if (!isOk) {
                     #ifdef TUP_DEBUG
                         qDebug() << "[TupLibraryObject::loadRawData()] - "
                                     "Fatal Error: Can't load image -> " << symbolName;
                     #endif

                     return false;
                 }

                 TupPixmapItem *item = new TupPixmapItem;
                 item->setPixmap(pixmap);
                 setData(QVariant::fromValue(static_cast<QGraphicsItem *>(item)));
            }
            break;
            case TupLibraryObject::Svg:
            {
                 setData(QVariant::fromValue(QString(data)));
            }
            break;
            case TupLibraryObject::Item:
            {
                 if (itemType == Text) {
                     setData(QString::fromLocal8Bit(data));
                 } else {
                     QString input = QString::fromLocal8Bit(data);
                     xmlString = input;

                     TupItemFactory factory;
                     QGraphicsItem *item = factory.create(input);
                     setData(QVariant::fromValue(item));
                 }
            }
            break;
            case TupLibraryObject::Audio:
            {
                 setData(QVariant::fromValue(data));
            }
            break;
            default:
            {
                 return false;
            }
    }
    
    return true;
}

bool TupLibraryObject::loadDataFromPath(const QString &dataDir)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryObject::loadDataFromPath()] - dataDir -> " << dataDir;
        qDebug() << "[TupLibraryObject::loadDataFromPath()] - dataPath -> " << dataPath;
    #endif

    QString path = "";

    switch (objectType) {
            case TupLibraryObject::Image:
            {
                 path = "/images/";
            }
            break;
            case TupLibraryObject::Audio:
            {
                 path = "/audio/";
            }
            break;
            case TupLibraryObject::Svg:
            {
                 path = "/svg/";
            }
            break;
            case TupLibraryObject::Item:
            {
                 path = "/obj/";
            }
            break;
            default: 
                 return false; 
    }

    dataPath = dataDir + path + dataPath;

    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryObject::loadDataFromPath()] - dataPath -> " << dataPath;
    #endif

    return loadData(dataPath);
}

bool TupLibraryObject::loadData(const QString &path)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryObject::loadData()] - path -> " << path;
    #endif

    switch (objectType) {
            case TupLibraryObject::Image:
            case TupLibraryObject::Svg:
            case TupLibraryObject::Item:
            case TupLibraryObject::Audio:
            {
                 QFile file(path);
                 if (file.exists()) {
                     if (file.open(QIODevice::ReadOnly)) {
                         QByteArray array = file.readAll(); 
                         #ifdef TUP_DEBUG
                             qDebug() << "[TupLibraryObject::loadData()] - Object path -> " << path;
                             qDebug() << "[TupLibraryObject::loadData()] - Object size -> " << array.size();
                         #endif
                         if (!array.isEmpty() && !array.isNull()) {
                             loadRawData(array);
                             file.close();
                         } else {
                             #ifdef TUP_DEBUG
                                 qDebug() << "[TupLibraryObject::loadData()] - "
                                             "Warning: Object file is empty -> " << path;
                             #endif
                             file.close();

                             return false;
                         }
                     } else {
                         #ifdef TUP_DEBUG
                             qDebug() << "[TupLibraryObject::loadData()] - "
                                         "Fatal Error: Can't access object file -> " << path;
                         #endif

                         return false;
                     }
                 } else {
                     #ifdef TUP_DEBUG
                         qDebug() << "[TupLibraryObject::loadData()] - "
                                     "Fatal Error: Object file doesn't exist -> " << path;
                     #endif

                     return false;
                 }
            }
            break;
            default:
                 return false;
    }

    return true;
}

bool TupLibraryObject::saveData(const QString &projectDir)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryObject::saveData()] - project dir -> " << projectDir;
    #endif

    switch (objectType) {
            case TupLibraryObject::Item:
            {
                 QString path = projectDir + "/obj/";
                 if (!folder.isEmpty())
                     path += folder + "/";
                 if (!QFile::exists(path)) {
                     QDir dir;
                     dir.mkpath(path);
                 }

                 dataPath = path + symbolName;
                 QFile file (dataPath);
                 if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                     QTextStream out(&file);
                     out << QString(rawData);
                     file.close();

                     return true;
                 } else {
                     #ifdef TUP_DEBUG
                         qDebug() << "[TupLibraryObject::saveData()] - "
                                     "Fatal Error: Lack of permission to save item file -> " << dataPath;
                     #endif
                     file.close();

                     return false;
                 }
            }

            case TupLibraryObject::Audio:
            {
                 QString path = projectDir + "/audio/";
                 if (!folder.isEmpty()) {
                     path += folder + "/";
                 } else {
                     if (soundObject->getSoundType() == Lipsync) {
                         #ifdef TUP_DEBUG
                             qDebug() << "[TupLibraryObject::saveData()] - "
                                         "Fatal Error: Lipsync audio folder can't be empty!";
                         #endif
                     }
                 }

                 if (!QFile::exists(path)) {
                     QDir dir;
                     dir.mkpath(path);
                 }
           
                 dataPath = path + symbolName;

                 QFile file(dataPath);
                 if (file.open(QIODevice::WriteOnly)) {
                     qint64 isOk = file.write(rawData);
                     file.close();

                     if (isOk != -1) {
                         #ifdef TUP_DEBUG
                             qDebug() << "[TupLibraryObject::saveData()] - "
                                         "Audio file has been saved successfully -> "
                                         << dataPath;
                         #endif

                         return true;
                     } else {
                         #ifdef TUP_DEBUG
                             qDebug() << "[TupLibraryObject::saveData()] - "
                                         "Fatal Error: Can't save audio file -> "
                                         << dataPath;
                         #endif

                         return false;
                     }
                 } else {
                     #ifdef TUP_DEBUG
                         qDebug() << "[TupLibraryObject::saveData()] - "
                                     "Fatal Error: Lack of permissions to save audio file -> "
                                  << dataPath;
                     #endif
                     file.close();

                     return false;
                 }
            }

            case TupLibraryObject::Svg:
            {
                 QString path = projectDir + "/svg/";
                 if (!folder.isEmpty())
                     path += folder + "/";
                 if (!QFile::exists(path)) {
                     QDir dir;
                     dir.mkpath(path);
                 }

                 dataPath = path + symbolName;

                 QFile file(dataPath);
                 if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                     QTextStream out(&file);
                     out << data.toString();
                     file.close();

                     return true;
                 } else {
                     #ifdef TUP_DEBUG
                         qDebug() << "[TupLibraryObject::saveData()] - "
                                     "Fatal Error: Lack of permissions to save SVG file -> "
                                  << dataPath;
                     #endif
                     file.close();

                     return false;
                 }
            }

            case TupLibraryObject::Image:
            {
                 QString path = projectDir + "/images/";
                 if (!folder.isEmpty())
                     path += folder + "/";
                 if (!QFile::exists(path)) {
                     QDir dir;
                     if (!dir.mkpath(path)) { 
                         #ifdef TUP_DEBUG
                             qDebug() << "[TupLibraryObject::saveData()] - "
                                         "Fatal Error: Can't create image path -> " << path;
                         #endif

                         return false;
                     }
                 }

                 dataPath = path + symbolName;

                 QFile file(dataPath);
                 if (file.open(QIODevice::WriteOnly)) {
                     qint64 isOk = file.write(rawData);
                     file.close();

                     if (isOk != -1) {
                         #ifdef TUP_DEBUG
                             qDebug() << "[TupLibraryObject::saveData()] - "
                                         "Image file saved successfully -> " << path << symbolName;
                         #endif

                          return true;
                     } else {
                         #ifdef TUP_DEBUG
                             qDebug() << "[TupLibraryObject::saveData()] - "
                                         "Fatal Error: Can't save image file -> " << path << symbolName;
                         #endif

                         return false;
                     }
                 } else {
                     #ifdef TUP_DEBUG
                         qDebug() << "[TupLibraryObject::saveData()] - "
                                     "Fatal Error: Insufficient permissions to save image file -> " << path << symbolName;
                     #endif
                     file.close();

                     return false;
                 }
            }

            default: 
            {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupLibraryObject::saveData()] - "
                                "Fatal Error: Type is not supported -> " << objectType;
                #endif
            }
    }

    return false;
}

QString TupLibraryObject::toString() const
{
    return xmlString;
}

QPixmap TupLibraryObject::renderImage(const QString &xml, int width)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryObject::renderImage(QString, int)] - width -> " << width;
        qDebug() << "[TupLibraryObject::renderImage(QString, int)] - XML -> ";
        qDebug() << xml;
    #endif

    if (!xml.isEmpty()) {
        TupItemFactory factory;
        QGraphicsItem *item = factory.create(xml);
        if (item) {
            QGraphicsScene *scene = new QGraphicsScene;
            scene->addItem(item);

            width = (width*60) / 100;
            int height = (item->boundingRect().height() * width) / item->boundingRect().width();
            QPixmap pixmap(width, height);
            pixmap.fill(Qt::transparent);

            QPainter painter(&pixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            scene->render(&painter);

            return pixmap;
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupLibraryObject::renderImage()] - "
                            "Fatal Error: QGraphicsItem from XML failed!";
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryObject::renderImage()] - Fatal Error: XML is empty!";
        #endif
    }

    return QPixmap();
}

QPixmap TupLibraryObject::generateImage(QGraphicsItem *item, int width)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryObject::generateImage(QGraphicsItem *, int)] - width -> " << width;
    #endif

    if (item) {
        QDomDocument doc;
        doc.appendChild(dynamic_cast<TupAbstractSerializable *>(item)->toXml(doc));

        return renderImage(doc.toString(), width);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryObject::generateImage()] - Fatal Error: item is NULL!";
        #endif
    }

    return QPixmap();
}

QPixmap TupLibraryObject::generateImage(const QString &xml, int width)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryObject::generateImage(QString, int)] - width -> " << width;
        qDebug() << "[TupLibraryObject::generateImage()] - XML -> ";
        qDebug() << xml;
    #endif

    if (!xml.isEmpty()) {
        QDomDocument doc;
        if (doc.setContent(xml)) {
            return renderImage(xml, width);
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupLibraryObject::generateImage()] - Fatal Error: XML is corrupt!";
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryObject::generateImage()] - Fatal Error: XML is empty!";
        #endif
    }

    return QPixmap();
}

TupLibraryObject * TupLibraryObject::clone()
{
    TupLibraryObject *copy = new TupLibraryObject(getSymbolName(), getFolder(), getObjectType());
    copy->setDataPath(getDataPath());
    copy->setData(getData());

    SoundType soundType = soundObject->getSoundType();
    if (soundType == Effect) {
        copy->setSoundType(soundType);
        copy->enableMute(isMuted());
        copy->setAudioScenes(getAudioScenes());
    }

    return copy;
}
