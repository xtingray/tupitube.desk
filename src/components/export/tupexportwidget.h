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

#ifndef TUPEXPORTWIDGET_H
#define TUPEXPORTWIDGET_H

#include "tglobal.h"
#include "tupproject.h"
#include "tupexportwizard.h"
#include "tuppluginmanager.h"
#include "tosd.h"

#include "tuppluginselector.h"
#include "tupsceneselector.h"
#include "tupexportmodule.h"
#include "tupvideoproperties.h"

class TUPITUBE_EXPORT TupExportWidget : public TupExportWizard
{
    Q_OBJECT

    public:
        enum ExportType { Local, Scene, Frame };
        enum OutputFormat { Animation = 0, ImagesArray, AnimatedImage };
        TupExportWidget(TupProject *project, QWidget *parent = nullptr, ExportType type = Local);
        ~TupExportWidget();

        void setProjectParams(const QString &username, const QString &secret, const QString &path);
        QString videoTitle() const;
        QString videoTopics() const;
        QString videoDescription() const;
        QList<int> videoScenes() const;
        bool isComplete();

    private slots:
        void setExporter(TupExportInterface::Plugin plugin);
        void updateWindowTitle();

    private:
        void loadPlugins();
		
    private:
        TupPluginSelector *pluginPage;
        TupSceneSelector *scenesPage;
        TupExportModule *animationExport;
        TupExportModule *imagesArrayExport;
        TupExportModule *animatedImageExport;
        TupVideoProperties *videoProperties;

        const TupProject *project;
        QHash<TupExportInterface::Plugin, TupExportInterface *> plugins;
        ExportType exportFlag;
};

#endif
