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
        projectVersion = project->getVersion();
        bgColor = project->getCurrentBgColor();
        dimension = project->getDimension();
        objectsTotal = 0;
        QFile *libraryFile = new QFile(projectDir.path() + "/library.tpl");
        if (libraryFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
            QDomDocument document;
            if (!document.setContent(QString::fromLocal8Bit(libraryFile->readAll()))) {
                #ifdef TUP_DEBUG
                    qWarning() << "[TupProjectScanner::scanLibrary()] - Fatal Error: Library XML file is invalid!";
                #endif

                return false;
            }
            QDomElement root = document.documentElement();
            QDomNode domNode = root.firstChild();
            library = scanLibrary(domNode);
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

float TupProjectScanner::getProjectVersion()
{
    return projectVersion;
}

QColor TupProjectScanner::getProjectBgColor() const
{
    return bgColor;
}

QSize TupProjectScanner::getProjectDimension() const
{
    return dimension;
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

TupProjectScanner::Folder TupProjectScanner::scanLibrary(QDomNode domNode)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProjectScanner::scanLibrary()]";
    #endif

    Folder library;
    if (!domNode.isNull()) {
        QDomElement rootElement = domNode.toElement();
        if (!rootElement.isNull()) {
            qDebug() << "rootElement.tagName() -> " << rootElement.tagName();
            qDebug() << "rootElement.attribute() -> " << rootElement.attribute("id");
            library.key = rootElement.attribute("id");

            QDomNode node = domNode.firstChild();
            while(!node.isNull()) {
                QDomElement e = node.toElement();
                qDebug() << "e.tagName() -> " << e.tagName();
                qDebug() << "e.attribute() -> " << e.attribute("id");
                if (e.tagName() == "folder") {
                    qDebug() << "Processing folder...";
                    Folder innerFolder = scanLibrary(node);
                    library.folders << innerFolder;
                } else if (e.tagName() == "object") {
                    qDebug() << "Processing object...";
                    library.objects << scanObject(e);
                }
                node = node.nextSibling();
            }
        }
    }

    return library;
}

TupProjectScanner::LibraryObject TupProjectScanner::scanObject(QDomNode node)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProjectScanner::scanObject()]";
    #endif

    TupProjectScanner::LibraryObject object;
    QDomDocument objectDocument;
    objectDocument.appendChild(objectDocument.importNode(node, true));
    QString xml = objectDocument.toString(0);

    QDomDocument document;
    if (!document.setContent(xml)) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupProjectScanner::scanObject()] - Fatal Error: Invalid XML structure!";
        #endif

        return object;
    }

    QDomElement objectTag = document.documentElement();
    if (objectTag.tagName() == "object") {
        object.key = objectTag.attribute("id");
        if (object.key.isEmpty()) {
            #ifdef TUP_DEBUG
                qWarning() << "[TupProjectScanner::scanObject()] - Fatal Error: Symbol name is empty!";
            #endif

            return object;
        }

        bool isOk = false;
        int index = objectTag.attribute("type").toInt(&isOk);
        if (isOk) {
            object.type = TupLibraryObject::ObjectType(index);
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupProjectScanner::scanObject()] - Fatal Error: Invalid object type!";
            #endif

            return object;
        }

        object.path = objectTag.attribute("path");
    }

    objectsTotal++;

    return object;
}

bool TupProjectScanner::isLibraryEmpty()
{
    qDebug() << "[TupProjectScanner::isLibraryEmpty()] - objectsTotal -> " << objectsTotal;

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

void TupProjectScanner::updateLibraryKey(int index, const QString &oldKey, const QString &key)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProjectScanner::updateLibraryKey()] - index -> " << index;
        qDebug() << "[TupProjectScanner::updateLibraryKey()] - oldKey -> " << oldKey;
        qDebug() << "[TupProjectScanner::updateLibraryKey()] - key -> " << key;
    #endif

    sceneContents[index].replace(oldKey, key);
}
