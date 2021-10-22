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
// #include "tupitemgroup.h"
// #include "taudioplayer.h"

#include <QGraphicsScene>
#include <QPainter>

TupLibraryObject::TupLibraryObject(QObject *parent) : QObject(parent)
{
}

TupLibraryObject::TupLibraryObject(const QString &name, const QString &dir, TupLibraryObject::ObjectType type,
                                   QObject *parent) : QObject(parent)
{
    setSymbolName(name);
    folder = dir;
    objectType = type;
    objectIsSoundResource = false;
    lipsyncVoice = false;
    mute = false;
    playAt = 0;
}

TupLibraryObject::~TupLibraryObject()
{
}

/**
 * Items, pics, text, etc...
 * @param data 
 */
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
    mute = flag;
}

bool TupLibraryObject::isMuted()
{
    return mute;
}

int TupLibraryObject::frameToPlay()
{
    return playAt;
}

void TupLibraryObject::updateFrameToPlay(int frame)
{
    playAt = frame;
}

bool TupLibraryObject::isLipsyncVoice()
{
    return lipsyncVoice;
}

void TupLibraryObject::setLipsyncVoiceFlag(bool isVoice)
{
    lipsyncVoice = isVoice;
}

void TupLibraryObject::setSymbolName(const QString &name)
{
    symbolName = name;
    // SQA: Check if this line is really required
    symbolName.replace(QDir::separator(), "-");
    smallId = symbolName.section('.', 0, 0);
    extension = symbolName.section('.', 1, 1).toUpper();
}

QString TupLibraryObject::getSymbolName() const
{
    return symbolName;
}

void TupLibraryObject::setFolder(const QString &dir)
{
    folder = dir;
}

void TupLibraryObject::updateFolder(const QString &projectPath, const QString &name)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryObject::updateFolder()] - current folder -> " << dataPath << " - new folder -> " << name;
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
        case TupLibraryObject::Sound:
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
    if (!name.isEmpty()) {
        newPath += name + "/";
        if (!dir.exists(newPath)) {
            if (!dir.mkpath(newPath)) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupLibraryObject::updateFolder()] - Fatal Error: Couldn't create path -> " << newPath;
                #endif
                return;
            }
            #ifdef TUP_DEBUG
                qDebug() << "[TupLibraryObject::updateFolder()] - Path created successfully -> " << newPath;
            #endif
        }
    }
    newPath += filename;

    if (dataPath.compare(newPath) != 0) { // if old path is different
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryObject::updateFolder()] - old dataPath -> " << dataPath;
        #endif
        if (dir.rename(dataPath, newPath)) {
            folder = name;
            dataPath = newPath;
            #ifdef TUP_DEBUG
                qDebug() << "[TupLibraryObject::updateFolder()] - new dataPath -> " << dataPath;
            #endif
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupLibraryObject::updateFolder()] - Fatal Error: Couldn't move object -> "
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
    return smallId;
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
                     QDomElement objectData = objectTag.firstChild().toElement();
                     if (!objectTag.isNull()) {
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
                                 qDebug() << "[TupLibraryObject::fromXml()] - Object data is empty! -> " << symbolName;
                             #endif
                             return;
                         }
                     } else {
                         #ifdef TUP_DEBUG
                             qDebug() << "[TupLibraryObject::fromXml()] - Fatal Error: Object data from xml is NULL -> " << symbolName;
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
            case TupLibraryObject::Sound:
             {
                 // <object id="audio.mp3" path="audio.mp3" type="3" soundEffect="1" playAt="30" lipsyncVoice="0" />
                 objectIsSoundResource = objectTag.attribute("soundEffect").toInt() ? true : false;
                 lipsyncVoice = objectTag.attribute("lipsyncVoice").toInt() ? true : false;
                 mute = objectTag.attribute("mute", "true").toInt() ? true : false;
                 playAt = objectTag.attribute("playAt", "1").toInt();
                 dataPath = objectTag.attribute("path");
             }
            break;
            default:
             {
                 #ifdef TUP_DEBUG
                     qDebug() << "[TupLibraryObject::fromXml()] - Unknown object type -> " << objectType;
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
    if (!folder.isEmpty())
        path = folder + "/" + finfo.fileName();

    #ifdef TUP_DEBUG
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
            case Sound:
            {
                object.setAttribute("soundEffect", objectIsSoundResource);
                object.setAttribute("lipsyncVoice", lipsyncVoice);
                object.setAttribute("mute", mute);
                object.setAttribute("playAt", playAt);
                object.setAttribute("path", path);
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
                         qDebug() << "[TupLibraryObject::loadRawData()] - Fatal Error: Can't load image -> " << symbolName;
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
            /*
            case TupLibraryObject::Text:
            {
                 setData(QString::fromLocal8Bit(data));
            }
            break;
            */
            case TupLibraryObject::Sound:
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
    #endif

    QString path = "";

    switch (objectType) {
            case TupLibraryObject::Image:
            {
                 path = "/images/";
            }
            break;
            case TupLibraryObject::Sound:
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
            case TupLibraryObject::Sound:
            {
                 QFile file(path);
                 if (file.exists()) {
                     if (file.open(QIODevice::ReadOnly)) {
                         QByteArray array = file.readAll(); 
                         #ifdef TUP_DEBUG
                             qDebug() << "[TupLibraryObject::loadData()] - Object path: " << path;
                             qDebug() << "[TupLibraryObject::loadData()] - Object size: " << array.size();
                         #endif
                         if (!array.isEmpty() && !array.isNull()) {
                             loadRawData(array);
                         } else {
                             #ifdef TUP_DEBUG
                                 qDebug() << "[TupLibraryObject::loadData()] - Warning: Image file is empty -> " << path;
                             #endif
                             return false;
                         }
                     } else {
                         #ifdef TUP_DEBUG
                             qDebug() << "[TupLibraryObject::loadData()] - Fatal Error: Can't access image file -> " << path;
                         #endif
                         return false;
                     }
                 } else {
                     #ifdef TUP_DEBUG
                         qDebug() << "[TupLibraryObject::loadData()] - Fatal Error: Image file doesn't exist -> " << path;
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

bool TupLibraryObject::saveData(const QString &dataDir)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryObject::saveData()]";
    #endif

    switch (objectType) {
            case TupLibraryObject::Item:
            {
                 QString path = dataDir + "/obj/";
                 if (folder.length() > 0)
                     path += folder + "/";
                 if (!QFile::exists(path)) {
                     QDir dir;
                     dir.mkpath(path);
                 }

                 dataPath = path + symbolName;
                 QFile file(dataPath);
                 if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                     QTextStream out(&file);
                     out << QString(rawData);
                     file.close();
                     return true;
                 } else {
                     #ifdef TUP_DEBUG
                         qDebug() << "[TupLibraryObject::saveData()] - Fatal Error: Lack of permission to save file -> " << dataPath;
                     #endif
                     return false;
                 }
            }

            case TupLibraryObject::Sound:
            {
                 QString path = dataDir + "/audio/";
                 if (folder.length() > 0)
                     path += folder + "/";
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
                             qDebug() << "[TupLibraryObject::saveData()] - Sound file has been saved successfully -> " << dataPath;
                         #endif
                         return true;
                     } else {
                         #ifdef TUP_DEBUG
                             qDebug() << "[TupLibraryObject::saveData()] - Fatal Error: Can't save file -> " << dataPath;
                         #endif
                         return false;
                     }
                 } else {
                     #ifdef TUP_DEBUG
                         qDebug() << "[TupLibraryObject::saveData()] - Fatal Error: Lack of permission to save file -> " << dataPath;
                     #endif
                     return false;
                 }
            }

            case TupLibraryObject::Svg:
            {
                 QString path = dataDir + "/svg/";
                 if (folder.length() > 0)
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
                         qDebug() << "[TupLibraryObject::saveData()] - Fatal Error: Lack of permission to save file -> " << dataPath;
                     #endif
                     return false;
                 }
            }

            case TupLibraryObject::Image:
            {
                 QString path = dataDir + "/images/";
                 if (folder.length() > 0)
                     path += folder + "/";
                 if (!QFile::exists(path)) {
                     QDir dir;
                     if (!dir.mkpath(path)) { 
                         #ifdef TUP_DEBUG
                             qDebug() << "[TupLibraryObject::saveData()] - Fatal error: Can't create path -> " << path;
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
                             qDebug() << "[TupLibraryObject::saveData()] - Image file saved successfully -> " << path << symbolName;
                         #endif
                         return true;
                     } else {
                         #ifdef TUP_DEBUG
                             qDebug() << "[TupLibraryObject::saveData()] - Fatal Error: Can't save file -> " << path << symbolName;
                         #endif
                         return false;
                     }
                 } else {
                     #ifdef TUP_DEBUG
                         qDebug() << "[TupLibraryObject::saveData()] - Fatal Error: Insufficient permissions to save file -> " << path << symbolName;
                     #endif
                     return false;
                 }
            }

            default: 
            {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupLibraryObject::saveData()] - Fatal Error: Type is not supported -> " << objectType;
                #endif
            }
    }

    return false;
}

void TupLibraryObject::setSoundResourceFlag(bool flag)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryObject::setSoundEffectFlag(bool)] - flag -> " << flag;
    #endif

    objectIsSoundResource = flag;
}

bool TupLibraryObject::isSoundResource()
{
    return objectIsSoundResource;
}

/*
bool TupLibraryObject::isNativeGroup()
{
    return isGroup;
}
*/

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
                qDebug() << "[TupLibraryObject::renderImage()] - Fatal Error: QGraphicsItem from XML failed!";
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

    if (isSoundResource()) {
        copy->setSoundResourceFlag(isSoundResource());
        copy->setLipsyncVoiceFlag(isLipsyncVoice());
        copy->updateFrameToPlay(frameToPlay());
        copy->enableMute(isMuted());
    }

    return copy;
}
