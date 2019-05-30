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
        QString msg1 = "TupPackageHandler::makePackage() - projectPath -> " + projectPath;
        QString msg2 = "TupPackageHandler::makePackage() - packagePath -> " + packagePath;
        #ifdef Q_OS_WIN
            qWarning() << msg1;
            qWarning() << msg2;
        #else
            tWarning() << msg1;
            tWarning() << msg2;
        #endif
    #endif

    if (!QFile::exists(projectPath)) {        
        #ifdef TUP_DEBUG
            QString msg = "TupPackageHandler::makePackage() - Project path doesn't exist -> " + projectPath;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif

        return false;
    }

    return JlCompress::compressDir(packagePath, projectPath, true);
}

bool TupPackageHandler::importPackage(const QString &packagePath)
{
    #ifdef TUP_DEBUG
        QString msg = "TupPackageHandler::importPackage() - packagePath -> " + packagePath;
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning() << msg;
        #endif
    #endif

    QFileInfo file(packagePath);
    projectDir = file.baseName();
    gPath = CACHE_DIR + file.baseName();
    QStringList list = JlCompress::extractDir(packagePath, CACHE_DIR);
    if (list.size() == 0) {
        #ifdef TUP_DEBUG
            QString msg = "TupPackageHandler::importPackage() - Project file is empty! -> " + packagePath;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif

        return false;
    }

    return true;
}

QString TupPackageHandler::importedProjectPath() const
{
    return gPath;
}

QString TupPackageHandler::projectDirectory() const
{
    return projectDir;
}
