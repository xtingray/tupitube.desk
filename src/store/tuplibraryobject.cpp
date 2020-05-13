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

TupLibraryObject::TupLibraryObject(QObject *parent) : QObject(parent)
{
}

TupLibraryObject::TupLibraryObject(const QString &name, const QString &dir, TupLibraryObject::Type type,
                                   QObject *parent) : QObject(parent)
{
    setSymbolName(name);
    folder = dir;
    objectType = type;
    objectHasSoundEffect = false;
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

void TupLibraryObject::setType(TupLibraryObject::Type type)
{
    objectType = type;
}

TupLibraryObject::Type TupLibraryObject::getType() const
{
    return objectType;
}

int TupLibraryObject::frameToPlay()
{
    return playAt;
}

void TupLibraryObject::updateFrameToPlay(int frame)
{
    playAt = frame;
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

void TupLibraryObject::updateFolder(const QString &name)
{
    QFileInfo finfo(dataPath);
    QString filename = finfo.fileName();
    QDir dir = finfo.dir();

    QString newPath = dir.path() + "/";
    if (!name.isEmpty()) {
        newPath += name + "/";
        if (!dir.exists(newPath)) { 
            if (!dir.mkpath(newPath)) {
                #ifdef TUP_DEBUG
                    qDebug() << "TupLibraryObject::updateFolder() - Fatal Error: Couldn't create path -> " + newPath;
                #endif
                return;
            }
        }
    }
    newPath += filename;

    if (dataPath.compare(newPath) != 0) {
        if (dir.rename(dataPath, newPath)) {
            folder = name;
            dataPath = newPath;
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "TupLibraryObject::updateFolder() - Fatal Error: Couldn't move object -> " + dataPath + " to " + newPath;
            #endif
        }
    }
}

QString TupLibraryObject::getFolder() const
{
    return folder;
}

QString TupLibraryObject::getSmallId() const
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
        qDebug() << "TupLibraryObject::fromXml() - xml -> " << xml;
    #endif

    QDomDocument document;
    if (!document.setContent(xml)) {
        #ifdef TUP_DEBUG
            qDebug() << "TupLibraryObject::fromXml() - Fatal Error: Invalid XML structure!";
        #endif
        return;
    }
    
    QDomElement objectTag = document.documentElement();
    if (objectTag.tagName() == "object") {
        setSymbolName(objectTag.attribute("id"));
        if (symbolName.isEmpty()) {
            #ifdef TUP_DEBUG
                qDebug() << "TupLibraryObject::fromXml - Fatal Error: Symbol name is empty!";
            #endif
            return;
        }
       
        bool isOk = false; 
        int index = objectTag.attribute("type").toInt(&isOk);
        if (isOk) {
            objectType = TupLibraryObject::Type(index);
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "TupLibraryObject::fromXml - Fatal Error: Invalid object type!";
            #endif
            return;
        }

        switch (objectType) {
                case TupLibraryObject::Text:
                     {
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
                                     qDebug() << "TupLibraryObject::fromXml() - Object data is empty! -> " + symbolName;
                                 #endif
                                 return;
                             }
                         } else {
                             #ifdef TUP_DEBUG
                                 qDebug() << "TupLibraryObject::fromXml() - Fatal Error: Object data from xml is NULL -> " + symbolName;
                             #endif
                             return;
                         }
                     }
                break;
                case TupLibraryObject::Image:
                case TupLibraryObject::Svg:
                case TupLibraryObject::Item:
                     {
                         dataPath = objectTag.attribute("path");
             int index = dataPath.lastIndexOf("/");
			 if (index > 0)
                 folder = dataPath.left(index);
                     }
                break;
                case TupLibraryObject::Sound:
                     {
                         objectHasSoundEffect = objectTag.attribute("soundEffect").toInt() ? true : false;
                         playAt = objectTag.attribute("playAt").toInt();
                         dataPath = objectTag.attribute("path");
                     }
                break;
                default:
                     {
                         #ifdef TUP_DEBUG
                             qDebug() << "TupLibraryObject::fromXml() - Unknown object type: " + QString::number(objectType);
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
        qDebug() << "TupLibraryObject::toXml() - Saving element -> " + path;
    #endif
    
    switch (objectType) {
            case Text:
            {
                QGraphicsItem *item = qvariant_cast<QGraphicsItem *>(data);
                if (item) {
                    if (TupAbstractSerializable *serializable = dynamic_cast<TupAbstractSerializable *>(item))
                        object.appendChild(serializable->toXml(doc));
                }
            }
            break;
            case Image:
            case Svg:
            case Item:
            {
                object.setAttribute("path", path);
            }
            break;
            case Sound:
            {
                object.setAttribute("soundEffect", objectHasSoundEffect);
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
    rawData = data;

    switch (objectType) {
            case TupLibraryObject::Image:
            {
                 QPixmap pixmap;
                 bool isOk = pixmap.loadFromData(data);
                 if (!isOk) {
                     #ifdef TUP_DEBUG
                         qDebug() << "TupLibraryObject::loadRawData() - [ Fatal Error ] - Can't load image -> " + symbolName;
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
                 TupItemFactory factory;
                 QGraphicsItem *item = factory.create(QString::fromLocal8Bit(data));
                 setData(QVariant::fromValue(item));
            }
            break;
            case TupLibraryObject::Text:
            {
                 setData(QString::fromLocal8Bit(data));
            }
            break;
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
        qDebug() << "TupLibraryObject::loadDataFromPath() - dataDir -> " << dataDir;
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

    // return true;
}

bool TupLibraryObject::loadData(const QString &path)
{
    #ifdef TUP_DEBUG
        qDebug() << "TupLibraryObject::loadData() - path -> " << path;
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
                             qDebug() << "TupLibraryObject::loadData() - Object path: " + path;
                             qDebug() << "TupLibraryObject::loadData() - Object size: " + QString::number(array.size());
                         #endif
                         if (!array.isEmpty() && !array.isNull()) {
                             loadRawData(array);
                         } else {
                             #ifdef TUP_DEBUG
                                 qDebug() << "TupLibraryObject::loadData() - Warning: Image file is empty -> " + path;
                             #endif
                             return false;
                         }
                     } else {
                         #ifdef TUP_DEBUG
                             qDebug() << "TupLibraryObject::loadData() - Fatal Error: Can't access image file -> " + path; 
                         #endif
                         return false;
                     }
                 } else {
                     #ifdef TUP_DEBUG
                         qDebug() << "TupLibraryObject::loadData() - Fatal Error: Image file doesn't exist -> " + path;
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
                 if (!QFile::exists(path)) {
                     QDir dir;
                     dir.mkpath(path);
                 }

                 QFile file(path + symbolName);
                 if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                     QTextStream out(&file);
                     out << QString(rawData);
                     dataPath = path + symbolName;
                     return true;
                 } else {
                     #ifdef TUP_DEBUG
                         qDebug() << "TupLibraryObject::saveData() - [ Fatal Error ] - Lack of permission to save file -> " + dataPath;
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
                             qDebug() << "TupLibraryObject::saveData() - Sound file has been saved successfully -> " + dataPath;
                         #endif
                         return true;
                     } else {
                         #ifdef TUP_DEBUG
                             qDebug() << "TupLibraryObject::saveData() - [ Fatal Error ] - Can't save file -> " + dataPath;
                         #endif
                         return false;
                     }
                 } else {
                     #ifdef TUP_DEBUG
                         qDebug() << "TupLibraryObject::saveData() - [ Fatal Error ] - Lack of permission to save file -> " + dataPath;
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

                 QFile file(path + symbolName);
                 if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                     QTextStream out(&file);
                     out << data.toString();
                     dataPath = path + symbolName;
                     return true;
                 } else {
                     #ifdef TUP_DEBUG
                         qDebug() << "TupLibraryObject::saveData() - [ Fatal Error ] - Lack of permission to save file -> " + dataPath;
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
                             qDebug() << "TupLibraryObject::saveData() - Fatal error: Can't create path -> " + path;
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
                             qDebug() << "TupLibraryObject::saveData() - Image file saved successfully -> " + path + symbolName;
                         #endif
                         return true;
                     } else {
                         #ifdef TUP_DEBUG
                             qDebug() << "TupLibraryObject::saveData() - [ Fatal Error ] - Can't save file -> " + path + symbolName;
                         #endif
                         return false;
                     }
                 } else {
                     #ifdef TUP_DEBUG
                         qDebug() << "TupLibraryObject::saveData() - [ Fatal Error ] - Insufficient permissions to save file -> " + path + symbolName;
                     #endif
                     return false;
                 }
            }

            default: 
            {
                #ifdef TUP_DEBUG
                    qDebug() << "TupLibraryObject::saveData() - Fatal Error: Type is not supported -> " + QString::number(objectType);
                #endif
            }
    }

    return false;
}

void TupLibraryObject::setSoundEffectFlag(bool flag)
{
    #ifdef TUP_DEBUG
        qDebug() << "TupLibraryObject::setSoundEffectFlag(bool) - flag -> " << flag;
    #endif

    objectHasSoundEffect = flag;
}

bool TupLibraryObject::isSoundEffect()
{
    return objectHasSoundEffect;
}
