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

#include "tupfilemanager.h"
#include "tupproject.h"
#include "tupscene.h"
#include "tuplibrary.h"
#include "tuppackagehandler.h"
#include "talgorithm.h"

#include <QDir>

TupFileManager::TupFileManager() : QObject()
{
}

TupFileManager::~TupFileManager()
{
}

bool TupFileManager::save(const QString &fileName, TupProject *project)
{
    #ifdef TUP_DEBUG
        QString msg = "TupFileManager::save() - Saving file -> " + fileName;
            #ifdef Q_OS_WIN
                qWarning() << msg;
            #else
                tWarning() << msg;
        #endif
    #endif

    QFileInfo info(fileName);
    QString name = info.baseName();	
    QString oldDirName = CACHE_DIR + project->getName();
    QDir projectDir(oldDirName);
    // Project name has been changed by the user
    if (name.compare(project->getName()) != 0) {
        project->setProjectName(name);
        projectDir.setPath(CACHE_DIR + name);    
        project->getLibrary()->updatePaths(CACHE_DIR + name);
        if (!projectDir.exists()) {
            // Update the cache path with new project's name
            if (projectDir.rename(oldDirName, projectDir.path())) {
                #ifdef TUP_DEBUG
                    QString msg = "TupFileManager::save() - Directory renamed to -> " + projectDir.path(); 
                    #ifdef Q_OS_WIN
                        qWarning() << msg;
                    #else
                        tWarning() << msg;
                    #endif
                #endif
            } else {
                // If rename action fails, then try to create new project's path
                if (!projectDir.mkdir(projectDir.path())) {
                    #ifdef TUP_DEBUG
                        QString msg = "TupFileManager::save() - Error: Can't create path -> " + projectDir.path();
                        #ifdef Q_OS_WIN
                            qDebug() << msg;
                        #else
                            tError() << msg;
                        #endif
                    #endif
                    return false;
                } else {
                    #ifdef TUP_DEBUG
                        QString msg = "TupFileManager::save() - Directory was created successfully -> " + projectDir.path();
                        #ifdef Q_OS_WIN
                            qWarning() << msg;
                        #else
                            tWarning() << msg;
                        #endif
                    #endif
                }
            }
        }
    } else {
        // If project's path doesn't exist, create it
        if (!projectDir.exists()) {
            if (!projectDir.mkdir(projectDir.path())) {
                #ifdef TUP_DEBUG
                    QString msg = "TupFileManager::save() - Error: Can't create path -> " + projectDir.path();
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
                return false;
            } else {
                #ifdef TUP_DEBUG
                    QString msg = "TupFileManager::save() - Directory was created successfully -> " + projectDir.path();
                    #ifdef Q_OS_WIN
                        qWarning() << msg;
                    #else
                        tWarning() << msg;
                    #endif
                #endif
            }
        }
    }

    {
     #ifdef TUP_DEBUG
         QString msg = "TupFileManager::save() - source files path -> " + projectDir.path();
         qDebug() << msg;
     #endif

     // Save project
     QFile projectFile(projectDir.path() + "/project.tpp");

     if (projectFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
         QTextStream ts(&projectFile);
         QDomDocument doc;
         project->setProjectName(name);
         doc.appendChild(project->toXml(doc));
         ts << doc.toString();
         projectFile.close();
     } else {
         #ifdef TUP_DEBUG
             QString msg = "TupFileManager::save() - Error: Can't create file -> " + projectDir.path() + "/project.tpp";
             #ifdef Q_OS_WIN
                 qDebug() << msg;
             #else
                 tError() << msg;
             #endif
         #endif
         return false;
     }
    }

    // Save scenes
    {
     int index = 0;
     int totalScenes = project->getScenes().size();
     for (int i = 0; i < totalScenes; i++) {
          TupScene *scene = project->getScenes().at(i);
          QDomDocument doc;
          doc.appendChild(scene->toXml(doc));
          QString scenePath = projectDir.path() + "/scene" + QString::number(index) + ".tps";
          QFile sceneFile(scenePath);

          if (sceneFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
              QTextStream st(&sceneFile);
              st << doc.toString();
              index += 1;
              sceneFile.close();
          } else {
              #ifdef TUP_DEBUG
                  QString msg = "TupFileManager::save() - Error: Can't create file -> " + scenePath;
                  #ifdef Q_OS_WIN
                      qDebug() << msg;
                  #else
                      tError() << msg;
                  #endif
              #endif
              return false;
          }
     }
    }

    {
     // Save library
     QFile lbr(projectDir.path() + "/library.tpl");

     if (lbr.open(QIODevice::WriteOnly | QIODevice::Text)) {
         QTextStream ts(&lbr);

         QDomDocument doc;
         doc.appendChild(project->getLibrary()->toXml(doc));

         ts << doc.toString();
         lbr.close();
     } else {
         #ifdef TUP_DEBUG
             QString msg = "TupFileManager::save() - Error: Can't create file -> " + projectDir.path() + "/library.tpl";
             #ifdef Q_OS_WIN
                 qDebug() << msg;
             #else
                 tError() << msg;
             #endif
         #endif
         return false;
     }
    }

    TupPackageHandler packageHandler;
    bool ok = packageHandler.makePackage(projectDir.path(), fileName);

    if (ok) {
        #ifdef TUP_DEBUG
            QString msg = "TupFileManager::save() - Project saved at -> " + fileName;
            #ifdef Q_OS_WIN
                qWarning() << msg;
            #else
                tWarning() << msg;
            #endif
        #endif
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupFileManager::save() - Error: Project couldn't be saved at -> " + fileName;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return false;
    }

    return ok;
}

bool TupFileManager::load(const QString &fileName, TupProject *project)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupFileManager::load()] - fileName: " + fileName;
        #else
            T_FUNCINFO << "filename: " << fileName;
        #endif
    #endif

    TupPackageHandler packageHandler;
    if (packageHandler.importPackage(fileName)) {
        QDir projectDir(packageHandler.importedProjectPath());
        QFile pfile(projectDir.path() + "/project.tpp");

        if (pfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            project->fromXml(QString::fromLocal8Bit(pfile.readAll()));
            pfile.close();
        } else {
            #ifdef TUP_DEBUG
                QString msg1 = "TupFileManager::load() - Error while open .tpp file. Name: " + pfile.fileName();
                QString msg2 = "TupFileManager::load() - Path: " + projectDir.path();
                QString msg3 = "TupFileManager::load() - Error Description: " + pfile.errorString(); 
                #ifdef Q_OS_WIN
                    qDebug() << msg1;
                    qDebug() << msg2;
                    qDebug() << msg3;
                #else
                    tError() << msg1;
                    tError() << msg2;
                    tError() << msg3;
                #endif
            #endif

            return false;
        }

        project->setDataDir(packageHandler.importedProjectPath());
        project->loadLibrary(projectDir.path() + "/library.tpl");

        QStringList scenes = projectDir.entryList(QStringList() << "*.tps", QDir::Readable | QDir::Files);
        if (scenes.count() > 0) {
            int index = 0;
            foreach (QString scenePath, scenes) {
                     scenePath = projectDir.path() + "/" + scenePath;
                     QFile file(scenePath);
					 
                     if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                         QString xml = QString::fromLocal8Bit(file.readAll());
                         QDomDocument document;
                         if (! document.setContent(xml))
                             return false;
                         QDomElement root = document.documentElement();
                         TupScene *scene = project->createScene(root.attribute("name"), index, true);
                         scene->fromXml(xml);
                         index += 1;
                         file.close();
                     } else {
                         #ifdef TUP_DEBUG
                             QString msg = "TupFileManager::load() - Error: Can't open file -> " + scenePath;
                             #ifdef Q_OS_WIN
                                 qDebug() << msg;
                             #else
                                 tError() << msg;
                             #endif
                         #endif
                         return false;
                     }
            }

            project->setOpen(true);

            return true;

        } else {
            #ifdef TUP_DEBUG
                QString msg = "TupFileManager::load() - Error: No scene files found (*.tps)";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif
            return false;
        }
    }

    #ifdef TUP_DEBUG
        QString msg = "TupFileManager::load() - Error: Can't import package -> " + fileName;
        #ifdef Q_OS_WIN
            qDebug() << msg;
        #else
            tError() << msg;
        #endif
    #endif
    return false;
}
