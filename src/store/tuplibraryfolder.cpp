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

#include "tuplibraryfolder.h"
#include "tuplibraryobject.h"
#include "tupprojectloader.h"
#include "tupproject.h"

TupLibraryFolder::TupLibraryFolder(const QString &key, TupProject *animation, QObject *parent) : QObject(parent)
{
    id = key;
    project = animation;
    loadingProject = false;
}

TupLibraryFolder::~TupLibraryFolder()
{
}

TupLibraryObject *TupLibraryFolder::createSymbol(TupLibraryObject::Type type, const QString &name, const QByteArray &data, 
                                                 const QString &folder, bool loaded)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupLibraryFolder::createSymbol()]";
        #else
            T_FUNCINFO;
            tWarning() << " - Creating symbol -> " << name;
            tWarning() << " - type -> " << type;
            tWarning() << " - folder -> " << folder;
            tWarning() << " - size -> " << data.size();
        #endif
    #endif

    if (data.isNull()) {
        #ifdef TUP_DEBUG
            QString msg = "TupLibraryFolder::createSymbol() - [ Fatal Error ] - Data is null!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return 0;
    }

    if (data.isEmpty()) {
        #ifdef TUP_DEBUG
            QString msg = "TupLibraryFolder::createSymbol() - [ Fatal Error ] - Data is empty!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return 0;
    }

    TupLibraryObject *object = new TupLibraryObject(name, folder, type, this);

    if (!object->loadRawData(data)) {
        #ifdef TUP_DEBUG
            QString msg = "TupLibraryFolder::createSymbol() - [ Fatal Error ] - Object have no data raw!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif

        delete object;
        return 0;
    }

    bool ret;

    if (folder.length() == 0)
        ret = addObject(object);
    else
        ret = addObject(folder, object);

    bool success = object->saveData(project->dataDir());
    if (success) {
        if (type == TupLibraryObject::Sound) {
            QPair<int, QString> soundRecord;
            soundRecord.first = object->frameToPlay();
            soundRecord.second = object->dataPath();
            soundRecords << soundRecord;
        }

        if (loaded && ret)
            TupProjectLoader::createSymbol(type, name, id, data, project);

        return object;
    }

    #ifdef TUP_DEBUG
        QString msg = "TupLibraryFolder::createSymbol() - [ Fatal Error ] - Object couldn't be saved!";
        #ifdef Q_OS_WIN
            qDebug() << msg;
        #else
            tError() << msg;
        #endif
    #endif

    return 0;
}

bool TupLibraryFolder::addObject(TupLibraryObject *object)
{
    if (!objects.contains(object->symbolName())) {
        objects.insert(object->symbolName(), object);
        return true;
    }

    return false;
}

bool TupLibraryFolder::addObject(const QString &folderName, TupLibraryObject *object)
{
    foreach (TupLibraryFolder *folder, folders) {
        if (folder->getId().compare(folderName) == 0) {
            LibraryObjects bag = folder->getObjects();
            if (!bag.contains(object->symbolName())) {
                folder->addObject(object);
                return true;
            }
        }
    }

    return false;
}

bool TupLibraryFolder::reloadObject(const QString &key)
{
    foreach (QString oid, objects.keys()) {
        if (oid.compare(key) == 0) {
            QString path = objects[key]->dataPath();
            if (QFile::exists(path))
                return objects[key]->loadData(path);
        }
    }

    #ifdef TUP_DEBUG
        QString msg = "TupLibraryFolder::reloadObject() - [ Fatal Error ] - Object ID wasn't found -> " + key;
        #ifdef Q_OS_WIN
            qDebug() << msg;
        #else
            tError() << msg;
        #endif
    #endif

    return false;
}

bool TupLibraryFolder::addFolder(TupLibraryFolder *folder)
{
    if (!folders.contains(folder->getId())) {
        folders.insert(folder->getId(), folder);
        return true;
    }

    return false;
}

bool TupLibraryFolder::removeObject(const QString &key, bool absolute)
{
    foreach (QString oid, objects.keys()) {
        if (oid.compare(key) == 0) {
            QString path = objects[key]->dataPath();
            if (absolute) {
                QFileInfo finfo(path);
                if (finfo.isFile())
                    QFile::remove(path);
            }
            return objects.remove(key);
        }
    }

    foreach (TupLibraryFolder *folder, folders) {
        TupLibraryObject *object = folder->getObject(key);
        if (object)
            return folder->removeObject(key, absolute);
    }

    #ifdef TUP_DEBUG
        QString msg = "TupLibraryFolder::removeObject() - [ Fatal Error ] - Object ID wasn't found -> " + key;
        #ifdef Q_OS_WIN
            qDebug() << msg;
        #else
            tError() << msg;
        #endif
    #endif

    return false;
}

bool TupLibraryFolder::removeFolder(const QString &key)
{
    if (folders.contains(key)) {
        TupLibraryFolder *folder = getFolder(key);
        if (folder) { 
            LibraryObjects objects = folder->getObjects();
            foreach (QString oid, objects.keys()) {
                if (folder->removeObject(oid, true)) {
                    TupLibraryObject::Type extension = static_cast<TupLibraryObject::Type>(objects[oid]->type());
                    if (extension != TupLibraryObject::Item) {
                        if (!project->removeSymbolFromFrame(oid, extension))
                            return false;
                    }
                }
            }

            bool result = folders.remove(key);
            return result;
        }
    }

    #ifdef TUP_DEBUG
        QString msg = "TupLibraryFolder::removeFolder() - [ Fatal Error ] - Folder wasn't found -> " + key;
        #ifdef Q_OS_WIN
            qDebug() << msg;
        #else
            tError() << msg;
        #endif
    #endif

    return false;
}

bool TupLibraryFolder::moveObject(const QString &key, const QString &target)
{
    TupLibraryObject *object = getObject(key);
    if (object) {
        if (removeObject(key, false)) {
            foreach (TupLibraryFolder *folder, folders) {
                if (folder->getId().compare(target) == 0) {
                    object->updateFolder(target);
                    folder->addObject(object);
                    return true;
                }
            }
        }
    }

    return false;
}

bool TupLibraryFolder::moveObjectToRoot(const QString &key)
{
    TupLibraryObject *object = getObject(key);
    if (object) {
        if (removeObject(key, false)) {
            addObject(object);
            return true;
        }
    }

    return false;
}

void TupLibraryFolder::setId(const QString &key)
{
    id = key;
}

QString TupLibraryFolder::getId() const
{
    return id;
}

bool TupLibraryFolder::exists(const QString &key)
{
    foreach (QString oid, objects.keys()) {
        if (oid.compare(key) == 0)
            return true;
    }

    foreach (TupLibraryFolder *folder, folders) {
        if (folder->exists(key))
            return true;
    }

    #ifdef TUP_DEBUG
        QString msg = "TupLibraryFolder::exists() - [ Error ] - Object doesn't exist -> " + key;
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning() << msg;
        #endif
    #endif

    return false;
}

TupLibraryObject *TupLibraryFolder::getObject(const QString &key) const
{
    foreach (QString oid, objects.keys()) {
        if (oid.compare(key) == 0)
            return objects[oid];
    }

    foreach (TupLibraryFolder *folder, folders) {
        TupLibraryObject *object = folder->getObject(key);
        if (object)
            return object;
    }
    
    #ifdef TUP_DEBUG
        QString msg = "TupLibraryFolder::getObject() - [ Fatal Error ] - Can't get object with id -> " + key;
        #ifdef Q_OS_WIN
            qDebug() << msg;
        #else
            tError() << msg;
        #endif
    #endif
    
    return 0;
}

TupLibraryFolder *TupLibraryFolder::getFolder(const QString &key) const
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupLibraryFolder::getFolder()]";
        #else
            T_FUNCINFO << "folder id -> " << key;
        #endif
    #endif

    foreach (TupLibraryFolder *folder, folders) {
        if (folder->getId().compare(key) == 0)
            return folder;
    }

    #ifdef TUP_DEBUG
        QString msg = "TupLibraryFolder::getFolder() - [ Fatal Error ] - Can't find folder with id -> " + key;
        #ifdef Q_OS_WIN
           qDebug() << msg;
        #else
           tError() << msg;
        #endif
    #endif
   
    return 0;
}

bool TupLibraryFolder::folderExists(const QString &key) const
{
    foreach (TupLibraryFolder *folder, folders) {
        if (folder->getId().compare(key) == 0)
            return true;
    }
  
    #ifdef TUP_DEBUG
        QString msg = "TupLibraryFolder::getFolder() - [ Fatal Error ] - Can't find folder with id -> " + key;
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning() << msg;
        #endif
    #endif
  
    return false;
}

bool TupLibraryFolder::renameObject(const QString &folder, const QString &oldId, const QString &newId)
{
    TupLibraryObject *object = getObject(oldId);

    if (object) {
        removeObject(oldId, false);
        object->setSymbolName(newId);

        if (folder.length() > 0)
            return addObject(folder, object);
        else
            return addObject(object);   
    } 

    #ifdef TUP_DEBUG
        QString msg = "TupLibraryFolder::renameObject() - [ Fatal Error ] - Object not found -> " + oldId;
        #ifdef Q_OS_WIN
            qDebug() << msg;
        #else
            tError() << msg;
        #endif
    #endif


    return false;
}

bool TupLibraryFolder::renameFolder(const QString &oldId, const QString &newId)
{
    TupLibraryFolder *folder = getFolder(oldId);

    if (folder) {
        folders[oldId]->setId(newId);
        return true;
    } 

    #ifdef TUP_DEBUG
        QString msg = "TupLibraryFolder::renameFolder() - [ Fatal Error ] - Folder not found -> " + oldId;
        #ifdef Q_OS_WIN
            qDebug() << msg;
        #else
            tError() << msg;
        #endif
    #endif

    return false;
}

int TupLibraryFolder::objectsCount() const
{
    return objects.count();
}

int TupLibraryFolder::foldersCount() const
{
    return folders.count();
}

TupProject *TupLibraryFolder::getProject() const
{
    return project;
}

Folders TupLibraryFolder::getFolders() const
{
    return folders;
}

LibraryObjects TupLibraryFolder::getObjects() const
{
    return objects;
}

void TupLibraryFolder::fromXml(const QString &xml)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupLibraryFolder::fromXml()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    loadingProject = true;

    QDomDocument document;
    if (! document.setContent(xml))
        return;
    
    QDomElement root = document.documentElement();
    QDomNode domNode = root.firstChild();
    
    while (!domNode.isNull()) {
        QDomElement e = domNode.toElement();
        
        if (!e.isNull()) {
            if (e.tagName() == "object") {
                loadItem(getId(), domNode);
            } else if (e.tagName() == "folder") {
                QDomDocument folderDocument;
                folderDocument.appendChild(folderDocument.importNode(domNode, true));

                TupLibraryFolder *folder = new TupLibraryFolder(e.attribute("id"), project, this);
                addFolder(folder);

                TupProjectLoader::createSymbol(TupLibraryObject::Folder, e.attribute("id"), QString(), 
                                               "FOLDER",  project);

                // Loading the objects inside this folder
                loadObjects(e.attribute("id"), folderDocument.toString(0));
           }
        }

        domNode = domNode.nextSibling();
    }

    loadingProject = false;
}

QDomElement TupLibraryFolder::toXml(QDomDocument &doc) const
{
    QDomElement folder = doc.createElement("folder");
    folder.setAttribute("id", id);

    foreach (TupLibraryFolder *folderObject, folders)
        folder.appendChild(folderObject->toXml(doc));

    foreach (TupLibraryObject *object, objects.values())
        folder.appendChild(object->toXml(doc));

    return folder;
}

void TupLibraryFolder::loadObjects(const QString &folder, const QString &xml)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupLibraryFolder::loadObjects()]";
        #else
            T_FUNCINFO;
            tWarning() << "Folder: " << folder;
        #endif
    #endif

    QDomDocument document;

    if (!document.setContent(xml))
        return;

    QDomElement root = document.documentElement();
    QDomNode domNode = root.firstChild();

    while (!domNode.isNull()) {
        QDomElement e = domNode.toElement();
    
        if (!e.isNull()) {
            if (e.tagName() == "object")
                loadItem(folder, domNode);
        }
        domNode = domNode.nextSibling();
    }
}

void TupLibraryFolder::loadItem(const QString &folder, QDomNode xml)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupLibraryFolder::loadItem()]";
        #else
            T_FUNCINFO;
            tWarning() << "Folder: " << folder;
        #endif
    #endif

    QDomDocument objectDocument;
    objectDocument.appendChild(objectDocument.importNode(xml, true));

    TupLibraryObject *object = new TupLibraryObject(this);
    object->fromXml(objectDocument.toString(0));

    switch (object->type()) {
        case TupLibraryObject::Image:
        case TupLibraryObject::Svg:
        case TupLibraryObject::Item:
        {
            object->loadDataFromPath(project->dataDir());
        }
        break;
        case TupLibraryObject::Sound:
        {
            object->loadDataFromPath(project->dataDir());
            if (object->isSoundEffect()) {
                QPair<int, QString> soundRecord;
                soundRecord.first = object->frameToPlay();
                soundRecord.second = object->dataPath();
                soundRecords << soundRecord;
            }
        }
        break;
        default:
        break;
    }

    if (folder.compare("library") == 0)
        addObject(object);
    else
        addObject(folder, object);

    QDomElement objectData = objectDocument.documentElement().firstChild().toElement();
    QString data;

    if (!objectData.isNull()) {
        QTextStream ts(&data);
        ts << objectData;
    }

    TupProjectLoader::createSymbol(TupLibraryObject::Type(object->type()),
                                   object->symbolName(), folder, data.toLocal8Bit(), project);
}

void TupLibraryFolder::reset()
{
    objects.clear();
    folders.clear();
}

void TupLibraryFolder::updatePaths(const QString &newPath)
{
    foreach (QString oid, objects.keys()) {
         QString oldPath = objects[oid]->dataPath();
         QFileInfo logicalPath(oldPath);
         QString filename = logicalPath.fileName();
         QString path = "";

         if (objects[oid]->type() == TupLibraryObject::Image)
             path = newPath + "/images/" + filename; 

         if (objects[oid]->type() == TupLibraryObject::Svg)
             path = newPath + "/svg/" + filename;

         if (objects[oid]->type() == TupLibraryObject::Sound)
             path = newPath + "/audio/" + filename;

         if (objects[oid]->type() == TupLibraryObject::Item)
             path = newPath + "/obj/" + filename;

         objects[oid]->setDataPath(path);

    }

    foreach (TupLibraryFolder *folder, folders)
        folder->updatePaths(newPath);    
}

bool TupLibraryFolder::isLoadingProject()
{
    return loadingProject;
}

QList<QPair<int, QString> > TupLibraryFolder::soundEffectList()
{
    return soundRecords;
}

void TupLibraryFolder::updateEffectSoundList(const QString &soundPath, int frame)
{
    int size = soundRecords.count();
    for(int i=0; i<size; i++) {
        QPair<int, QString> soundRecord = soundRecords.at(i);
        if (soundPath.compare(soundRecord.second) == 0) {
            soundRecord.first = frame;
            soundRecords.replace(i, soundRecord);
            return;
        }
    }
}
