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
}

TApplicationProperties::~TApplicationProperties()
{
}

void TApplicationProperties::setHomeDir(const QString &path)
{
    homePath = path;
}

void TApplicationProperties::setBinDir(const QString &path)
{
    binPath = path;
}

void TApplicationProperties::setShareDir(const QString &path)
{
    sharePath = path;
}

void TApplicationProperties::setDataDir(const QString &path)
{
    dataPath = path;
}

void TApplicationProperties::setThemeDir(const QString &path)
{
    themePath = path;
}

void TApplicationProperties::setRasterDir(const QString &path)
{
    rasterPath = path;
}

void TApplicationProperties::setPluginDir(const QString &path)
{
    pluginPath = path;
}

void TApplicationProperties::setCacheDir(const QString &path)
{
    cachePath = path;
}

void TApplicationProperties::setRepositoryDir(const QString &path)
{
    repositoryPath = path;
}

void TApplicationProperties::setVersion(const QString &version)
{
    versionStr = version;
}

void TApplicationProperties::setRevision(const QString &revision)
{
    revisionStr = revision;
}

void TApplicationProperties::setCodeName(const QString &code)
{
    codeNameStr = code;
}

QString TApplicationProperties::homeDir() const
{
    return homePath + "/";
}

QString TApplicationProperties::binDir() const
{
    return binPath + "/";
}

QString TApplicationProperties::shareDir() const
{
    if (sharePath.isEmpty())
        return homePath + "/share";

    return sharePath + "/";
}

QString TApplicationProperties::dataDir() const
{
    if (dataPath.isEmpty()) {
        QString locale = QString(QLocale::system().name()).left(2);
        if (locale.length() < 2)
            locale = "en";
        return sharePath + "/data/xml/" + locale + "/";
    }

    return dataPath;
}

QString TApplicationProperties::themeDir() const
{
    if (themePath.isEmpty())
        return sharePath + "/themes/default/";

    return themePath;
}

QString TApplicationProperties::rasterDir() const
{
    return rasterPath;
}

QString TApplicationProperties::pluginDir() const
{
    return pluginPath + "/";
}

QString TApplicationProperties::configDir() const
{
    return QDir::homePath() + "/" + "." + qApp->applicationName() + "/";
}

QString TApplicationProperties::cacheDir() const
{
    return cachePath + "/";
}

QString TApplicationProperties::repositoryDir() const
{
    return repositoryPath + "/";
}

QString TApplicationProperties::version() const
{
    return versionStr;
}

QString TApplicationProperties::codeName() const
{
    return codeNameStr;
}

QString TApplicationProperties::revision() const
{
    return revisionStr;
}

TApplicationProperties *TApplicationProperties::instance()
{
    if (s_instance == 0)
        s_instance = new TApplicationProperties;
    return s_instance;
}
