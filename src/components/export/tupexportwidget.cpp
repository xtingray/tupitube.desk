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

#include "tupexportwidget.h"

TupExportWidget::TupExportWidget(TupProject *work, QWidget *parent, bool isLocal) : TupExportWizard(parent)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupExportWidget()]";
        #else
            TINIT;
        #endif
    #endif

    project = work;

    if (isLocal) {
        setWindowTitle(tr("Export To Video"));
        setWindowIcon(QIcon(THEME_DIR + "icons/export_wi.png"));

        pluginPage = new TupPluginSelector();
        addPage(pluginPage);

        scenesPage = new TupSceneSelector();
        scenesPage->setScenes(work->getScenes());
        connect(this, SIGNAL(updateScenes()), scenesPage, SLOT(updateScenesList()));
        addPage(scenesPage);

        animationExport = new TupExportModule(work, TupExportModule::Animation, tr("Export To Video File"));
        connect(this, SIGNAL(exportAnimation()), animationExport, SLOT(exportIt()));
        connect(this, SIGNAL(setAnimationFileName()), animationExport, SLOT(updateNameField()));
        addPage(animationExport);

        imagesArrayExport = new TupExportModule(work, TupExportModule::ImagesArray, tr("Export To Image Sequence"));
        connect(this, SIGNAL(exportImagesArray()), imagesArrayExport, SLOT(exportIt()));
        connect(this, SIGNAL(setImagesArrayFileName()), imagesArrayExport, SLOT(updateNameField()));
        addPage(imagesArrayExport);

        animatedImageExport = new TupExportModule(work, TupExportModule::AnimatedImage, tr("Export To Animated Image"));
        connect(this, SIGNAL(exportAnimatedImage()), animatedImageExport, SLOT(exportIt()));
        connect(this, SIGNAL(setAnimatedImageFileName()), animatedImageExport, SLOT(updateNameField()));
        addPage(animatedImageExport);

        connect(pluginPage, SIGNAL(selectedPlugin(const QString &)), this, SLOT(setExporter(const QString &)));
        connect(pluginPage, SIGNAL(animationFormatSelected(int, const QString &)), animationExport, SLOT(setCurrentFormat(int, const QString &)));
        connect(pluginPage, SIGNAL(imagesArrayFormatSelected(int, const QString &)), imagesArrayExport, SLOT(setCurrentFormat(int, const QString &)));
        connect(pluginPage, SIGNAL(animatedImageFormatSelected(int, const QString &)), animatedImageExport, SLOT(setCurrentFormat(int, const QString &)));

        connect(scenesPage, SIGNAL(selectedScenes(const QList<int> &)), animationExport, SLOT(setScenesIndexes(const QList<int> &)));
        connect(scenesPage, SIGNAL(selectedScenes(const QList<int> &)), imagesArrayExport, SLOT(setScenesIndexes(const QList<int> &)));
        connect(scenesPage, SIGNAL(selectedScenes(const QList<int> &)), animatedImageExport, SLOT(setScenesIndexes(const QList<int> &)));

        loadPlugins();
        pluginPage->selectFirstItem();
    } else {
        setWindowTitle(tr("Post Animation In TupiTube"));
        setWindowIcon(QIcon(THEME_DIR + "icons/net_document.png"));

        scenesPage = new TupSceneSelector();
        scenesPage->setScenes(work->getScenes());
        connect(this, SIGNAL(updateScenes()), scenesPage, SLOT(updateScenesList()));
        addPage(scenesPage);

        videoProperties = new TupVideoProperties();
        connect(this, SIGNAL(saveVideoToServer()), videoProperties, SLOT(postIt()));
        addPage(videoProperties);

        connect(scenesPage, SIGNAL(selectedScenes(const QList<int> &)), videoProperties, SLOT(setScenesIndexes(const QList<int> &)));
    }
}

TupExportWidget::~TupExportWidget()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[~TupExportWidget()]";
        #else
            TEND;
        #endif
    #endif
}

void TupExportWidget::loadPlugins()
{
    QList<TupExportInterface *> pluginList;
    foreach (QObject *plugin, TupPluginManager::instance()->getFormats()) {
        if (plugin) {
            TupExportInterface *exporter = qobject_cast<TupExportInterface *>(plugin);
            if (exporter) {
                int index = -1;
                if (exporter->key().compare(tr("Video Formats")) == 0)
                    index = 0;
                if (exporter->key().compare(tr("Open Video Format")) == 0)
                    index = 1;
                if (exporter->key().compare(tr("Image Sequence")) == 0)
                    index = 2;
                if (exporter->key().compare(tr("Animated Image")) == 0)
                    index = 3;

                #if defined(Q_OS_LINUX) || defined(Q_OS_WIN)
                    pluginList.insert(index, exporter);
                #else
                    if (exporter->key().compare(tr("Open Video Format")) != 0)
                        pluginList.insert(index, exporter);
                #endif
            } else {
                #ifdef TUP_DEBUG
                    QString msg = "TupExportWidget::loadPlugins() - [ Fatal Error ] - Can't load export plugin";
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
            }
        }
    }

    for (int i=0; i<pluginList.size(); i++) {
         TupExportInterface *exporter = pluginList.at(i);
         pluginPage->addPlugin(exporter->key());
         plugins.insert(exporter->key(), exporter);
    }
}

void TupExportWidget::setExporter(const QString &plugin)
{
    if (plugins.contains(plugin)) {
        TupExportInterface* currentExporter = plugins[plugin];
        pluginPage->setFormats(currentExporter->availableFormats());

        if (currentExporter)
            animationExport->setCurrentExporter(currentExporter);

        imagesArrayExport->setCurrentExporter(currentExporter);
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupExportWidget::setExporter() - [ Fatal Error ] - Can't load export plugin -> " + plugin;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }
}

QString TupExportWidget::videoTitle() const
{
    return videoProperties->title();
}

QString TupExportWidget::videoTopics() const
{
    return videoProperties->topics();
}

QString TupExportWidget::videoDescription() const
{
    return videoProperties->description();
}

QList<int> TupExportWidget::videoScenes() const
{
    return videoProperties->scenesList();
}

bool TupExportWidget::isComplete()
{
    return videoProperties->isComplete();
}
