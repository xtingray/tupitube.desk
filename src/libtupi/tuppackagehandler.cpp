/***************************************************************************
 *   Project TUPITUBE DESK                                                 *
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

#include "tuppackagehandler.h"
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"
#include "quazip/JlCompress.h"

TupPackageHandler::TupPackageHandler()
{
}

TupPackageHandler::~TupPackageHandler()
{
}

bool TupPackageHandler::makePackage(const QString &projectPath, const QString &packagePath)
{
    #ifdef TUP_DEBUG
        qWarning() << "[TupPackageHandler::makePackage()] - projectPath -> " << projectPath;
        qWarning() << "[TupPackageHandler::makePackage()] - packagePath -> " << packagePath;
    #endif

    if (!QFile::exists(projectPath)) {        
        #ifdef TUP_DEBUG
            qWarning() << "[TupPackageHandler::makePackage()] - "
                          "Project path doesn't exist -> " << projectPath;
        #endif

        return false;
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupPackageHandler::makePackage()] - Calling JlCompress library...";
    #endif
    return JlCompress::compressDir(packagePath, projectPath, true);
}

bool TupPackageHandler::importPackage(const QString &packagePath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPackageHandler::importPackage()] - packagePath -> " << packagePath;
        qDebug() << "[TupPackageHandler::importPackage()] - CACHE_DIR -> " << CACHE_DIR;
        QFile file(packagePath);
        qDebug() << "[TupPackageHandler::importPackage()] - source file size -> "
                 << (QString::number(file.size()) + " bytes");
    #endif

    QFileInfo fileInfo(packagePath);
    projectDir = fileInfo.baseName();
    QuaZip zipChecker(packagePath);
    if (zipChecker.open(QuaZip::mdUnzip)) {
        zipChecker.goToFirstFile();
        QString firstFile = zipChecker.getCurrentFileName();
        int index = firstFile.indexOf("/");
        QString dirName = CACHE_DIR + firstFile.left(index);
        QDir dir(dirName);
        if (dir.exists(dirName)) {
            if (dir.removeRecursively()) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupPackageHandler::importPackage()] - "
                                "Warning: Project directory already exists. Removing successfully! -> " << dirName;
                #endif
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupPackageHandler::importPackage()] - "
                                "Fatal Error: Project directory can't be removed -> " << dirName;
                #endif
            }
        }
    }

    QStringList list = JlCompress::extractDir(packagePath, CACHE_DIR);
    if (list.size() == 0) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPackageHandler::importPackage()] - "
                        "Fatal Error: Project source file has NO elements! -> " << packagePath;
        #endif

        return false;
    }

    QString path = list.at(0);
    int index = path.indexOf("/", CACHE_DIR.length());
    gPath = path.left(index);

    return true;
}

QString TupPackageHandler::importedProjectPath() const
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPackageHandler::importedProjectPath()] - project path -> " << gPath;
    #endif

    return gPath;
}

QString TupPackageHandler::projectDirectory() const
{
    return projectDir;
}
