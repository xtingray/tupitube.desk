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
#include "tconfig.h"
#include "tupproject.h"
#include "tupscene.h"
#include "tuplayer.h"
#include "tuplibrary.h"
#include "tuppackagehandler.h"
#include "talgorithm.h"
#include "tbackupdialog.h"
#include "tosd.h"

#include <QDir>
#include <QMessageBox>

TupFileManager::TupFileManager() : QObject()
{
}

TupFileManager::~TupFileManager()
{
}

bool TupFileManager::save(const QString &fileName, TupProject *project)
{
    QString projectName = project->getName();

    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupFileManager::save()] - Saving file -> " << fileName;
        qDebug() << "[TupFileManager::save()] - Project name -> " << projectName;
        qDebug() << "---";
    #endif

    QFileInfo info(fileName);
    QString filename = info.baseName();
    QString currentDirName = CACHE_DIR + projectName;
    QDir projectDir(currentDirName);
    bool ok;

    // Project name has been changed by the user
    if ((filename.compare(projectName) != 0) && projectDir.exists(currentDirName)) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupFileManager::save()] - Case I: User changed project's name...";
        #endif

        QString newPath = CACHE_DIR + filename;
        project->setProjectName(filename);
        projectDir.setPath(newPath);
        project->updateLibraryPaths(newPath);
        project->setDataDir(newPath);

        if (project->soundsListSize()) // The project has at least one sound
            emit projectPathChanged();

        if (!projectDir.exists(newPath)) { // Target dir doesn't exist
            // Update the cache path with new project's name
            #ifdef TUP_DEBUG
                qDebug() << "[TupFileManager::save()] - Case IA: "
                            "Renaming old path -> " << currentDirName << " into -> " << newPath;
            #endif

            // Try to rename old folder into new one
            if (projectDir.rename(currentDirName, newPath)) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupFileManager::save()] - "
                                "Success! Directory renamed to -> " << newPath;
                #endif
            } else { // The rename action failed
                #ifdef TUP_DEBUG
                    qWarning() << "[TupFileManager::save()] - Case IA-I - Warning: Renaming action failed!";
                #endif
                // Trying to create new project's path
                if (projectDir.mkdir(newPath)) {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupFileManager::save()] - "
                                    "Directory was created successfully -> " << newPath;
                    #endif
                    // Copying the whole old folder into the new one
                    if (TAlgorithm::copyFolder(currentDirName, newPath)) {
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupFileManager::save()] - "
                                        "Success! Old path -> " << currentDirName << " copied to -> " << newPath;
                        #endif
                        // Removing old path
                        projectDir.setPath(currentDirName);
                        if (projectDir.removeRecursively()) {
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupFileManager::save()] - "
                                            "Success! Old path removed -> " << currentDirName;
                            #endif
                        } else {
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupFileManager::save()] - "
                                            "Fatal Error: Can't remove old path -> " << currentDirName;
                            #endif
                            TOsd::self()->display(TOsd::Error, tr("Can't save project! (Code %1)").arg("001"));

                            return false;
                        }
                    } else { // Copy action failed
                        #ifdef TUP_DEBUG
                            qWarning() << "[TupFileManager::save()] - "
                                          "Fatal Error: Can't copy content into new path -> " << newPath;
                        #endif
                        TOsd::self()->display(TOsd::Error, tr("Can't save project! (Code %1)").arg("002"));

                        return false;
                    }
                } else { // New path creation failed
                    #ifdef TUP_DEBUG
                        qWarning() << "[TupFileManager::save()] - "
                                      "Error: Can't create path -> " << newPath;
                    #endif
                    TOsd::self()->display(TOsd::Error, tr("Can't save project! (Code %1)").arg("003"));

                    return false;
                }
            }
        } else { // Target dir exists
            #ifdef TUP_DEBUG
                qDebug() << "[TupFileManager::save()] - Case IB: "
                            "Folder path already exists! -> " << newPath;
            #endif

            // If source dir exists
            if (projectDir.exists(currentDirName) && (newPath.compare(project->getDataDir()) != 0)) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupFileManager::save()] - "
                                "Removing existing folder -> " << currentDirName;
                #endif
                // Removing target dir
                if (projectDir.removeRecursively()) {
                    // If rename action fails, then try to create target dir
                    if (projectDir.mkdir(newPath)) {
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupFileManager::save()] - "
                                        "Directory was created successfully after deletion -> " << newPath;
                            qDebug() << "[TupFileManager::save()] - Now copying files from -> " << currentDirName;
                        #endif
                        // Copying old dir into target dir
                        if (TAlgorithm::copyFolder(currentDirName, newPath)) {
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupFileManager::save()] - "
                                            "Success! Project files copied into -> " << newPath;
                            #endif
                        } else { // Copy action failed
                            #ifdef TUP_DEBUG
                                qWarning() << "[TupFileManager::save()] - "
                                              "Fatal Error: Can't copy content into new path -> " << newPath;
                            #endif
                            TOsd::self()->display(TOsd::Error, tr("Can't save project! (Code %1)").arg("004"));

                            return false;
                        }
                    } else { // Failed while creating target dir
                        #ifdef TUP_DEBUG
                            qWarning() << "[TupFileManager::save()] - "
                                          "Error: Can't create path after removing -> " << newPath;
                        #endif
                        TOsd::self()->display(TOsd::Error, tr("Can't save project! (Code %1)").arg("005"));

                        return false;
                    }
                } else { // Failed removing target dir
                    #ifdef TUP_DEBUG
                        qWarning() << "[TupFileManager::save()] - "
                                      "Error: Can't create path after removing -> " << newPath;
                    #endif
                    TOsd::self()->display(TOsd::Error, tr("Can't save project! (Code %1)").arg("006"));

                    return false;
                }
            } else {
                #ifdef TUP_DEBUG
                    // SQA: This case is still under revision
                    qDebug() << "---";
                    qDebug() << "[TupFileManager::save()] - "
                                "User is saving the current opened project in same/other folder using same name...";
                    qDebug() << "*** fileName -> " << fileName;
                    qDebug() << "---";
                #endif
            }
        }

        // if (project->soundsListSize()) // The project has at least one sound
        if (project->hasLibrarySounds()) // The project has at least one sound
            emit soundPathsChanged();
    } else {
        // If project's path doesn't exist, create it
        if (!projectDir.exists()) {
            QString projectPath = projectDir.path();
            #ifdef TUP_DEBUG
                qDebug() << "[TupFileManager::save()] -  - Case II: Project dir doesn't exist... -> " << projectPath;
            #endif
            if (filename.compare(project->getName()) != 0) { // User renamed the source file name
                #ifdef TUP_DEBUG
                    qDebug() << "[TupFileManager::save()] - Updating project name to -> " << filename;
                #endif
                QString newPath = CACHE_DIR + filename;
                projectPath = newPath;
                projectDir.setPath(projectPath);
                project->setProjectName(filename);
            }

            if (!projectDir.exists()) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupFileManager::save()] - Creating project's directory -> " << projectPath;
                #endif
                if (projectDir.mkdir(projectPath)) {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupFileManager::save()] - Directory was created successfully -> "
                                 << projectPath;
                    #endif
                } else {
                    #ifdef TUP_DEBUG
                        qWarning() << "[TupFileManager::save()] - Error: Can't create path -> "
                                   << projectPath;
                    #endif
                    TOsd::self()->display(TOsd::Error, tr("Can't save project! (Code %1)").arg("007"));

                    return false;
                }
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupFileManager::save()] - "
                                "Warning: Project's directory already exists! -> " << projectDir.path();
                #endif
            }
        }
    }

    // Saving project components...
    {
        #ifdef TUP_DEBUG
            qDebug() << "[TupFileManager::save()] - Saving project components...";
            qDebug() << "---";
            qDebug() << "[TupFileManager::save()] - source files path -> " << projectDir.path();
        #endif

        // Save project
        QFile projectFile(projectDir.path() + "/project.tpp");

        if (projectFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupFileManager::save()] - Saving config file (TPP)";
            #endif
            QTextStream ts(&projectFile);
            QDomDocument doc;
            project->setProjectName(filename);
            doc.appendChild(project->toXml(doc));
            ts << doc.toString();
            projectFile.close();
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupFileManager::save()] - "
                              "Error: Can't create file -> " << projectDir.path() << "/project.tpp";
            #endif

            return false;
        }
    }

    // Save scenes
    {
        int totalScenes = project->getScenes().size();
        int index = 0;
        QDomDocument doc;
        QString scenePath;

        for (int i=0; i<totalScenes; i++) {
            doc.appendChild(project->getScenes().at(i)->toXml(doc));
            scenePath = projectDir.path() + "/scene" + QString::number(index) + ".tps";
            #ifdef TUP_DEBUG
                qDebug() << "[TupFileManager::save()] - Saving scene file " << i;
                qDebug() << "[TupFileManager::save()] - Scene file -> " << scenePath;
            #endif
            QFile sceneFile(scenePath);
            if (sceneFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupFileManager::save()] - Capturing stream...";
                #endif
                QTextStream st(&sceneFile);
                st << doc.toString();
                index += 1;
                doc.clear();
                sceneFile.close();
            } else {
                #ifdef TUP_DEBUG
                    qWarning() << "[TupFileManager::save()] - Error: Can't create file -> " << scenePath;
                #endif

                return false;
            }
        }
    }

    {
         // Save library
         QFile library(projectDir.path() + "/library.tpl");

         if (library.open(QIODevice::WriteOnly | QIODevice::Text)) {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupFileManager::save()] - Saving library file (TPL)";
             #endif
             QTextStream ts(&library);
             QDomDocument doc;
             doc.appendChild(project->getLibrary()->toXml(doc));

             ts << doc.toString();
             library.close();
         } else {
             #ifdef TUP_DEBUG
                 qWarning() << "[TupFileManager::save()] - "
                               "Error: Can't create file -> " << projectDir.path() << "/library.tpl";
             #endif

             return false;
         }
    }

    {
        #ifdef TUP_DEBUG
            qDebug() << "[TupFileManager::save()] - Creating TUP file...";
        #endif
        TupPackageHandler packageHandler;
        ok = packageHandler.makePackage(projectDir.path(), fileName);

        if (ok) {
            #ifdef TUP_DEBUG
                qWarning() << "[TupFileManager::save()] - Project saved at -> " << fileName;
            #endif
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupFileManager::save()] - Error: Project couldn't be saved at -> " << fileName;
            #endif

            QApplication::restoreOverrideCursor();
            QPair<int, int> dimension = TAlgorithm::screenDimension();
            int screenWidth = dimension.first;
            int screenHeight = dimension.second;

            TBackupDialog *dialog = new TBackupDialog(projectDir.path(), filename);
            dialog->show();

            int result = dialog->exec();
            if (result == QDialog::Accepted) {
                TCONFIG->beginGroup("General");
                QString path = TCONFIG->value("RecoveryDir").toString();
                TCONFIG->sync();

                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("Information"));
                msgBox.setIcon(QMessageBox::Information);
                msgBox.setText(tr("A copy of your project was successfully saved at:<br/><b>%1</b>").arg(path));
                msgBox.setInformativeText(tr("Please, contact TupiTube's developer team to recovery it. "
                                             "<b>https://www.tupitube.com</b>"));
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.show();
                msgBox.move(static_cast<int> ((screenWidth - msgBox.width()) / 2),
                            static_cast<int> ((screenHeight - msgBox.height()) / 2));
                msgBox.exec();
            } else if (result == QDialog::Rejected) {
                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("Information"));
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setText(tr("Sorry, the project <b>%1.tup</b> couldn't be recovered.<br/>"
                                  "Please, try to backup your animation files often.").arg(filename));
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.show();
                msgBox.move(static_cast<int> ((screenWidth - msgBox.width()) / 2),
                            static_cast<int> ((screenHeight - msgBox.height()) / 2));
                msgBox.exec();
            }

            return false;
        }
    }

    return ok;
}

bool TupFileManager::load(const QString &fileName, TupProject *project)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupFileManager::load()] - fileName ->" << fileName;
    #endif

    TupPackageHandler packageHandler;
    if (packageHandler.importPackage(fileName)) {
        QDir projectDir(packageHandler.importedProjectPath());
        QString projectConfigPath = projectDir.path() + "/project.tpp";
        QFile pfile(projectConfigPath);

        if (pfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            int size = pfile.size();
            if (size == 0) {
                #ifdef TUP_DEBUG
                    qWarning() << "[TupFileManager::load()] - Fatal Error: Project file (TPP) has size ZERO! ->" << projectConfigPath;
                #endif
                TOsd::self()->display(TOsd::Error, tr("Can't open project config file!"));

                return false;
            }

            project->fromXml(QString::fromLocal8Bit(pfile.readAll()));
            pfile.close();
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupFileManager::load()] - Error while open .tpp file. Name ->" << pfile.fileName();
                qWarning() << "[TupFileManager::load()] - Path ->" << projectDir.path();
                qWarning() << "[TupFileManager::load()] - Error Description ->" << pfile.errorString();
            #endif

            return false;
        }

        project->setDataDir(packageHandler.importedProjectPath());
        // Loading library assets
        project->loadLibrary(projectDir.path() + "/library.tpl");

        QStringList scenes = projectDir.entryList(QStringList() << "*.tps", QDir::Readable | QDir::Files);
        QFile *file;
        QDomDocument doc;
        QString xml;
        QDomElement root;

        #ifdef TUP_DEBUG
            qDebug() << "[TupFileManager::load()] - scenes.count() ->" << scenes.count();
        #endif

        if (scenes.count() > 0) {
            int index = 0;
            foreach (QString scenePath, scenes) {
                scenePath = projectDir.path() + "/" + scenePath;
                file = new QFile(scenePath);
					 
                if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
                    int size = file->size();
                    if (size == 0) {
                        #ifdef TUP_DEBUG
                            qWarning() << "[TupFileManager::load()] - Fatal Error: Scene file (TPS) has size ZERO! ->" << scenePath;
                        #endif
                        TOsd::self()->display(TOsd::Error, tr("Can't open scene config file!"));

                        return false;
                    }

                    xml = QString::fromLocal8Bit(file->readAll());
                    if (!doc.setContent(xml)) {
                        #ifdef TUP_DEBUG
                            qWarning() << "[TupFileManager::load()] - Fatal Error: Can't open XML scene file! ->" << scenePath;
                        #endif
                        TOsd::self()->display(TOsd::Error, tr("Can't load scene config file!"));

                        return false;
                    }

                    root = doc.documentElement();
                    QString sceneName = root.attribute("name");
                    scenesLabels << sceneName;
                    project->createScene(sceneName, index, true)->fromXml(xml);
                    index += 1;
                    doc.clear();
                    file->close();
                    delete file;
                } else {
                    #ifdef TUP_DEBUG
                        qWarning() << "[TupFileManager::load()] - Error: Can't open TPS file ->" << scenePath;
                    #endif
                    TOsd::self()->display(TOsd::Error, tr("Can't read scene config file!"));

                    return false;
                }
            }
            project->setOpen(true);

            return true;
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupFileManager::load()] - Error: No scene files found (*.tps)";
            #endif

            return false;
        }
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupFileManager::load()] - Error: Can't import package ->" << fileName;
    #endif

    return false;
}

bool TupFileManager::createImageProject(const QString &projectCode, const QString &imgPath, TupProject *project)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupFileManager::createImageProject()] - projectCode -> " << projectCode;
        qDebug() << "[TupFileManager::createImageProject()] - imgPath -> " << imgPath;
    #endif

    QString filename = CACHE_DIR + projectCode + ".tup";

    TupProject *newProject = new TupProject();
    newProject->setProjectName(projectCode);
    newProject->setAuthor(project->getAuthor());
    newProject->setCurrentBgColor(project->getCurrentBgColor());
    newProject->setDescription(project->getDescription());
    newProject->setDimension(project->getDimension());
    newProject->setFPS(project->getFPS(), 0);
    newProject->setDataDir(CACHE_DIR + projectCode);

    TupLibrary *library = new TupLibrary("library", newProject);
    newProject->setLibrary(library);

    TupScene * newScene = newProject->createScene(tr("Scene %1").arg(QString::number(1)), 0);
    TupLayer *newLayer = newScene->createLayer(tr("Layer %1").arg(QString::number(1)), 0);
    newLayer->createFrame(tr("Frame %1").arg(QString::number(1)), 0);
    TupFrame *frame = newLayer->frameAt(0);

    QFile file(imgPath);
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            if (library->createSymbol(TupLibraryObject::Image, "image.png", data, "") == nullptr) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupProject::createImageProject()] - Fatal Error: Image object can't be created. Data is NULL!";
                #endif

                return false;
            }

            TupLibraryObject *object = library->getObject("image.png");
            if (object) {
                TupGraphicLibraryItem *libraryItem = new TupGraphicLibraryItem(object);
                int imageW = static_cast<int>(libraryItem->boundingRect().width());
                int imageH = static_cast<int> (libraryItem->boundingRect().height());

                qreal xPos = 0;
                qreal yPos = 0;
                QSize dimension = newProject->getDimension();
                if (dimension.width() > imageW)
                    xPos = (dimension.width() - imageW) / 2;
                if (dimension.height() > imageH)
                    yPos = (dimension.height() - imageH) / 2;

                libraryItem->moveBy(xPos, yPos);

                int zLevel = frame->getTopZLevel();
                libraryItem->setZValue(zLevel);
                frame->addItem("image.png", libraryItem);
            }
        }
    }

    return save(filename, newProject);
}

QList<QString> TupFileManager::scenesList()
{
    return scenesLabels;
}
