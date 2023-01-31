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

#include "tupprojectscanner.h"
#include "tupproject.h"
#include "tuppackagehandler.h"

TupProjectScanner::TupProjectScanner() : QObject()
{
}

TupProjectScanner::~TupProjectScanner()
{
}

bool TupProjectScanner::read(const QString &filename, const QString &tempFolder)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProjectScanner::read()] - fileName -> " << filename;
    #endif

    TupProject *project = new TupProject;
    TupPackageHandler packageHandler;
    if (packageHandler.importPackage(filename, tempFolder)) {
        QDir projectDir(packageHandler.importedProjectPath());
        projectPath = projectDir.path();
        QFile *projectFile = new QFile(projectPath + "/project.tpp");

        if (projectFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
            project->fromXml(QString::fromLocal8Bit(projectFile->readAll()));
            projectFile->close();
            delete projectFile;
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupProjectScanner::read()] - Error while open .tpp file. Name -> "
                           << projectFile->fileName();
                qWarning() << "[TupProjectScanner::read()] - Path -> "
                           << projectDir.path();
                qWarning() << "[TupProjectScanner::read()] - Error Description -> "
                           << projectFile->errorString();
            #endif
            delete projectFile;

            return false;
        }

        projectName = project->getName();
        objectsTotal = 0;
        QFile *libraryFile = new QFile(projectDir.path() + "/library.tpl");
        if (libraryFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
            if (!scanLibrary(QString::fromLocal8Bit(libraryFile->readAll()))) {
                #ifdef TUP_DEBUG
                    qWarning() << "[TupProjectScanner::read()] - Fatal Error: Library file is invalid! -> "
                               << libraryFile->fileName();
                #endif
                libraryFile->close();
                delete libraryFile;

                return false;
            }

            libraryFile->close();
            delete libraryFile;
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupProjectScanner::read()] - Error while open .tpl file. Name -> " << libraryFile->fileName();
                qWarning() << "[TupProjectScanner::read()] - Path -> " << projectDir.path();
                qWarning() << "[TupProjectScanner::read()] - Error Description -> " << libraryFile->errorString();
            #endif
            delete libraryFile;

            return false;
        }

        QStringList scenes = projectDir.entryList(QStringList() << "*.tps", QDir::Readable | QDir::Files);
        scenesTotal = scenes.count();

        QFile *sceneFile;
        QString xml;
        if (scenesTotal > 0) {
            foreach (QString scenePath, scenes) {
                scenePath = projectDir.path() + "/" + scenePath;
                sceneFile = new QFile(scenePath);

                if (sceneFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
                    xml = QString::fromLocal8Bit(sceneFile->readAll());
                    QString sceneName = readSceneName(xml);
                    if (!sceneName.isEmpty()) {
                        sceneLabels << sceneName;
                        scenePaths << scenePath;
                        sceneContents << xml;
                        if (xml.indexOf("<symbol id=") == -1)
                            libraryFlags << false;
                        else
                            libraryFlags << true;
                    } else {
                        #ifdef TUP_DEBUG
                            qWarning() << "[TupProjectScanner::read()] - Fatal Error: Scene file seems to be invalid -> "
                                       << scenePath;
                        #endif
                        sceneFile->close();
                        delete sceneFile;

                        return false;
                    }

                    sceneFile->close();
                    delete sceneFile;
                } else {
                    #ifdef TUP_DEBUG
                        qWarning() << "[TupProjectScanner::read()] - Fatal Error: Can't open file -> " << scenePath;
                    #endif
                    delete sceneFile;

                    return false;
                }
            }
            delete project;

            return true; // Everything's OK!
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupProjectScanner::read()] - Fatal Error: No scene files found (*.tps)";
            #endif

            return false;
        }
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupProjectScanner::read()] - Fatal Error: Can't import package -> " << filename;
    #endif

    return false;
}

QString TupProjectScanner::getProjectName() const
{
    return projectName;
}

int TupProjectScanner::scenesCount()
{
    return scenesTotal;
}

QList<QString> TupProjectScanner::getSceneLabels()
{
    return sceneLabels;
}

QString TupProjectScanner::readSceneName(const QString &xml) const
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProjectScanner::readSceneName()]";
    #endif

    QDomDocument doc;
    if (!doc.setContent(xml)) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupProjectScanner::readSceneName()] - Error while processing XML file";
        #endif

        return "";
    }
    QDomElement root = doc.documentElement();

    return root.attribute("name");
}

bool TupProjectScanner::scanLibrary(const QString &xml)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProjectScanner::scanLibrary()]";
    #endif

    QDomDocument document;
    if (!document.setContent(xml)) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupProjectScanner::scanLibrary()] - Fatal Error: Library XML file is invalid!";
        #endif

        return false;
    }

    library.key = "library";
    QDomElement root = document.documentElement();
    QDomNode domNode = root.firstChild();

    while (!domNode.isNull()) {
        QDomElement e = domNode.toElement();
        if (!e.isNull()) {
            /*
            if (e.tagName() == "object") {
                if (scanObject(domNode)) {
                    library.objects << object;
                } else {
                    #ifdef TUP_DEBUG
                        qWarning() << "[TupProjectScanner::scanLibrary()] - Fatal Error: Library XML file is invalid!";
                    #endif

                    return false;
                }
            } else
            */
            if (e.tagName() == "folder") {
                QDomDocument folderDocument;
                folderDocument.appendChild(folderDocument.importNode(domNode, true));
                Folder folder;
                QString folderId = e.attribute("id");
                if (scanObjects(folderId, folderDocument.toString(0))) {
                    if (folderId == "library") {
                        library.key = folderId;
                        library.objects = objects;
                    } else {
                        folder.key = folderId;
                        folder.objects = objects;
                        library.folders << folder;
                    }
                } else {
                    #ifdef TUP_DEBUG
                        qWarning() << "[TupProjectScanner::scanLibrary()] - Fatal Error: Can't read objects from folder -> "
                                   << folder.key;
                    #endif

                    return false;
                }
            }
        }
        domNode = domNode.nextSibling();
    }

    return true;
}

bool TupProjectScanner::scanObjects(const QString &folderId, const QString &xml)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProjectScanner::scanObjects()] - Folder -> " << folderId;
    #endif

    objects.clear();
    QDomDocument document;
    if (!document.setContent(xml)) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupProjectScanner::scanObjects()] - Fatal Error: Can't read XML input!";
        #endif

        return false;
    }

    QDomElement root = document.documentElement();
    QDomNode domNode = root.firstChild();
    while (!domNode.isNull()) {
        QDomElement e = domNode.toElement();
        if (!e.isNull()) {
            if (e.tagName() == "object") {
                if (scanObject(domNode)) {
                    objects << object;
                } else {
                    #ifdef TUP_DEBUG
                        qWarning() << "[TupProjectScanner::scanObjects()] - Fatal Error: Library XML file is invalid!";
                    #endif

                    return false;
                }
            }
        }
        domNode = domNode.nextSibling();
    }

    return true;
}

bool TupProjectScanner::scanObject(QDomNode node)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProjectScanner::scanObject()]";
    #endif

    QDomDocument objectDocument;
    objectDocument.appendChild(objectDocument.importNode(node, true));
    QString xml = objectDocument.toString(0);

    QDomDocument document;
    if (!document.setContent(xml)) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupProjectScanner::scanObject()] - Fatal Error: Invalid XML structure!";
        #endif

        return false;
    }

    QDomElement objectTag = document.documentElement();
    if (objectTag.tagName() == "object") {
        object.key = objectTag.attribute("id");
        if (object.key.isEmpty()) {
            #ifdef TUP_DEBUG
                qWarning() << "[TupProjectScanner::scanObject()] - Fatal Error: Symbol name is empty!";
            #endif

            return false;
        }

        bool isOk = false;
        int index = objectTag.attribute("type").toInt(&isOk);
        if (isOk) {
            object.type = TupLibraryObject::ObjectType(index);
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupProjectScanner::scanObject()] - Fatal Error: Invalid object type!";
            #endif

            return false;
        }

        object.path = objectTag.attribute("path");
    }

    objectsTotal++;

    return true;
}

bool TupProjectScanner::isLibraryEmpty()
{
    return objectsTotal == 0;
}

TupProjectScanner::Folder TupProjectScanner::getLibrary()
{
    return library;
}

QString TupProjectScanner::getProjectPath() const
{
    return projectPath;
}

QList<QString> TupProjectScanner::getScenePaths()
{
    return scenePaths;
}

QList<bool> TupProjectScanner::getSceneLibraryFlags()
{
    return libraryFlags;
}

QList<QString> TupProjectScanner::getSceneContents()
{
    return sceneContents;
}
