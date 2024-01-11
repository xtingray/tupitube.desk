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

#ifndef TUPPLUGINSELECTOR_H
#define TUPPLUGINSELECTOR_H

#include "tglobal.h"
#include "tupexportinterface.h"
#include "tupexportwizard.h"

#include <QListWidget>
#include <QListWidgetItem>

class TUPITUBE_EXPORT TupPluginSelector : public TupExportWizardPage
{
    Q_OBJECT

    public:
        TupPluginSelector();
        ~TupPluginSelector();

        bool isComplete() const;
        const char *extension;

        void resetUI();
        void addPlugin(TupExportInterface::Plugin pluginId, const QString &pluginName);
        void setFormats(TupExportInterface::Plugin plugin, TupExportInterface::Formats formats);

    public slots:
        void selectedPluginItem(QListWidgetItem *);
        void selectFirstPlugin();
        void selectedFormatItem(QListWidgetItem *);
        const char* getFileExtension();

    signals:
        void pluginSelected(TupExportInterface::Plugin key);
        void animationFormatSelected(TupExportInterface::Format format, const QString &extension);

        // SQA: Pending for implementation
        // void animatedImageFormatSelected(TupExportInterface::Format format, const QString &extension);

        void imagesArrayFormatSelected(TupExportInterface::Format format, const QString &extension);

    private:
        const char* getFormatExtension(TupExportInterface::Format formatId);

        QListWidget *pluginList;
        QListWidget *formatList;

        QList<TupExportInterface::Plugin> plugins;
        QList<TupExportInterface::Format> videoFormats;
        QList<TupExportInterface::Format> imageFormats;
        TupExportInterface::Plugin currentPlugin;
};

#endif
