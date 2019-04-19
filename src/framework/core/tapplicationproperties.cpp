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

#include "tapplicationproperties.h"

TApplicationProperties *TApplicationProperties::s_instance = nullptr;

TApplicationProperties::TApplicationProperties()
{
    // empty
}

TApplicationProperties::~TApplicationProperties()
{
    // empty
}

void TApplicationProperties::setHomeDir(const QString &path)
{
    appHomeDir = path;
}

void TApplicationProperties::setBinDir(const QString &path)
{
    appBinDir = path;
}

void TApplicationProperties::setShareDir(const QString &path)
{
    appShareDir = path;
}

void TApplicationProperties::setDataDir(const QString &path)
{
    appDataDir = path;
}

void TApplicationProperties::setThemeDir(const QString &path)
{
    appThemeDir = path;
}

void TApplicationProperties::setPluginDir(const QString &path)
{
    appPluginDir = path;
}

void TApplicationProperties::setCacheDir(const QString &path)
{
    appCacheDir = path;
}

void TApplicationProperties::setRepositoryDir(const QString &path)
{
    appRepositoryDir = path;
}

void TApplicationProperties::setVersion(const QString &version)
{
    appVersion = version;
}

void TApplicationProperties::setCodeName(const QString &code)
{
    appCodeName = code;
}

void TApplicationProperties::setRevision(const QString &revision)
{
    appRevision = revision;
}

QString TApplicationProperties::homeDir() const
{
    return appHomeDir + "/";
}

QString TApplicationProperties::binDir() const
{
    return appBinDir + "/";
}

QString TApplicationProperties::shareDir() const
{
    if (appShareDir.isEmpty())
        return appHomeDir + "/share";

    return appShareDir + "/";
}

QString TApplicationProperties::dataDir() const
{
    if (appDataDir.isEmpty()) {
        QString locale = QString(QLocale::system().name()).left(2);
        if (locale.length() < 2)
            locale = "en";
        return appShareDir + "/data/xml/" + locale + "/";
    }

    return appDataDir;
}

QString TApplicationProperties::themeDir() const
{
    if (appThemeDir.isEmpty())
        return appShareDir + "/themes/default/";

    return appThemeDir;
}

QString TApplicationProperties::pluginDir() const
{
    return appPluginDir + "/";
}

QString TApplicationProperties::configDir() const
{
    return QDir::homePath() + "/" + "." + qApp->applicationName() + "/";
}

QString TApplicationProperties::cacheDir() const
{
    return appCacheDir + "/";
}

QString TApplicationProperties::repositoryDir() const
{
    return appRepositoryDir + "/";
}

QString TApplicationProperties::version() const
{
    return appVersion;
}

QString TApplicationProperties::codeName() const
{
    return appCodeName;
}

QString TApplicationProperties::revision() const
{
    return appRevision;
}

TApplicationProperties *TApplicationProperties::instance()
{
    if (s_instance == 0)
        s_instance = new TApplicationProperties;
    return s_instance;
}
