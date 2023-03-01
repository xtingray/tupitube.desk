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

#include "tuppluginmanager.h"
#include "tupfilterinterface.h"
#include "tuptoolinterface.h"
#include "tupexportinterface.h"

#include <QDir>

TupPluginManager *TupPluginManager::s_instance = nullptr;

TupPluginManager::TupPluginManager(QObject *parent) : QObject(parent)
{
}

TupPluginManager::~TupPluginManager()
{
    unloadPlugins();
}

TupPluginManager *TupPluginManager::instance()
{
    if (!s_instance)
        s_instance = new TupPluginManager;
    
    return s_instance;
}

void TupPluginManager::loadPlugins()
{
    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupPluginManager::loadPlugins()] - Searching for plugins...";
    #endif

    filters.clear();
    tools.clear();
    formats.clear();
    
    QDir pluginDirectory = QDir(kAppProp->pluginDir());
    #ifdef TUP_DEBUG
        qDebug() << "[TupPluginManager::loadPlugins()] - plugin dir -> " << pluginDirectory.path();
    #endif

    foreach (QString fileName, pluginDirectory.entryList(QStringList() << "*.so" << "*.dll" << "*.dylib", QDir::Files)) {
        QPluginLoader *loader = new QPluginLoader(pluginDirectory.absoluteFilePath(fileName));
        QObject *plugin = qobject_cast<QObject*>(loader->instance());

        #ifdef TUP_DEBUG
            qDebug() << "[TupPluginManager::loadPlugins()] - Loading plugin from -> " << fileName;
            qDebug() << "[TupPluginManager::loadPlugins()] - Plugin is loaded? -> " << loader->isLoaded();
        #endif

        if (plugin) {
            AFilterInterface *filter = qobject_cast<AFilterInterface *>(plugin);

            if (filter) {
                filters << plugin;
            } else {
                TupToolInterface *tool = qobject_cast<TupToolInterface *>(plugin);
                if (tool) {
                    tools << plugin;
                } else {
                    TupExportInterface *exporter = qobject_cast<TupExportInterface *>(plugin);
                    if (exporter) {
                        formats << plugin;
                    }
                }
            }

            loaders << loader;
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupPluginManager::loadPlugins()] - Fatal Error: Something happened while loading the plugin -> " << loader->errorString();
            #endif
        }
    }

    #ifdef TUP_DEBUG
        qDebug() << "---";
    #endif
}

void TupPluginManager::unloadPlugins()
{
    #ifdef TUP_DEBUG
        qWarning() << "TupPluginManager::unloadPlugins() - Unloading plugins...";
    #endif

    foreach (QPluginLoader *loader, loaders) {
        delete loader->instance();
        delete loader;
    }
}

QObjectList TupPluginManager::getTools() const
{
    return tools;
}

QObjectList TupPluginManager::getFilters() const
{
    return filters;
}

QObjectList TupPluginManager::getFormats() const
{
    return formats;
}
