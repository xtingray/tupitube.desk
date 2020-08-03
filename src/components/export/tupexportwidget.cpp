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
        qDebug() << "---";
        qDebug() << "[TupExportWidget()]";
    #endif

    project = work;
    exportFlag = isLocal;

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
        connect(animationExport, SIGNAL(exportHasStarted()), this, SLOT(updateWindowTitle()));
        addPage(animationExport);

        imagesArrayExport = new TupExportModule(work, TupExportModule::ImagesArray, tr("Export To Image Sequence"));
        connect(this, SIGNAL(exportImagesArray()), imagesArrayExport, SLOT(exportIt()));
        connect(this, SIGNAL(setImagesArrayFileName()), imagesArrayExport, SLOT(updateNameField()));
        connect(imagesArrayExport, SIGNAL(exportHasStarted()), this, SLOT(updateWindowTitle()));
        addPage(imagesArrayExport);

        animatedImageExport = new TupExportModule(work, TupExportModule::AnimatedImage, tr("Export To Animated Image"));
        connect(this, SIGNAL(exportAnimatedImage()), animatedImageExport, SLOT(exportIt()));
        connect(this, SIGNAL(setAnimatedImageFileName()), animatedImageExport, SLOT(updateNameField()));
        addPage(animatedImageExport);

        // connect(pluginPage, SIGNAL(selectedPlugin(const QString &)), this, SLOT(setExporter(const QString &)));
        connect(pluginPage, SIGNAL(selectedPlugin(TupExportInterface::Plugin)),
                this, SLOT(setExporter(TupExportInterface::Plugin)));

        connect(pluginPage, SIGNAL(animationFormatSelected(TupExportInterface::Format, const QString &)),
                animationExport, SLOT(setCurrentFormat(TupExportInterface::Format, const QString &)));
        connect(pluginPage, SIGNAL(imagesArrayFormatSelected(TupExportInterface::Format, const QString &)),
                imagesArrayExport, SLOT(setCurrentFormat(TupExportInterface::Format, const QString &)));

        // SQA: Pending for development
        // connect(pluginPage, SIGNAL(animatedImageFormatSelected(int, const QString &)),
        //         animatedImageExport, SLOT(setCurrentFormat(int, const QString &)));

        connect(scenesPage, SIGNAL(selectedScenes(const QList<int> &)),
                animationExport, SLOT(setScenesIndexes(const QList<int> &)));
        connect(scenesPage, SIGNAL(selectedScenes(const QList<int> &)),
                imagesArrayExport, SLOT(setScenesIndexes(const QList<int> &)));
        connect(scenesPage, SIGNAL(selectedScenes(const QList<int> &)),
                animatedImageExport, SLOT(setScenesIndexes(const QList<int> &)));

        loadPlugins();
        pluginPage->selectFirstPlugin();
    } else {
        setWindowTitle(tr("Post Animation"));
        setWindowIcon(QIcon(THEME_DIR + "icons/social_network.png"));

        scenesPage = new TupSceneSelector();
        scenesPage->setScenes(work->getScenes());
        connect(this, SIGNAL(updateScenes()), scenesPage, SLOT(updateScenesList()));
        addPage(scenesPage);

        videoProperties = new TupVideoProperties();
        connect(this, SIGNAL(postProcedureCalled()), videoProperties, SLOT(postIt()));
        connect(videoProperties, SIGNAL(postHasStarted()), this, SLOT(updateWindowTitle()));
        addPage(videoProperties);

        connect(scenesPage, SIGNAL(selectedScenes(const QList<int> &)),
                videoProperties, SLOT(setScenesIndexes(const QList<int> &)));
    }
}

TupExportWidget::~TupExportWidget()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupExportWidget()]";
    #endif
}

void TupExportWidget::loadPlugins()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExportWidget::loadPlugins()]";
    #endif

    QList<TupExportInterface *> pluginList;
    foreach (QObject *plugin, TupPluginManager::instance()->getFormats()) {
        if (plugin) {
            TupExportInterface *exporter = qobject_cast<TupExportInterface *> (plugin);
            if (exporter) {
                /*
                int index = -1;
                if (exporter->key() == TupExportInterface::VideoFormats)
                    index = 0;
                if (exporter->key() == TupExportInterface::OpenVideoFormat)
                    index = 1;
                if (exporter->key() == TupExportInterface::ImageSequence)
                    index = 2;
                if (exporter->key() == TupExportInterface::AnimatedImage)
                    index = 3;
                */

                pluginList.insert(exporter->key(), exporter);

                /*
                #if defined(Q_OS_LINUX) || defined(Q_OS_WIN)
                    pluginList.insert(exporter->key(), exporter);
                #else // Q_OS_MAC
                    if (exporter->key() != TupExportInterface::OpenVideoFormat)
                        pluginList.insert(exporter->key(), exporter);
                #endif
                */
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "TupExportWidget::loadPlugins() - [ Fatal Error ] - Can't load export plugin";
                #endif
            }
        }
    }

    for (int i = 0; i < pluginList.size(); i++) {
         TupExportInterface *exporter = pluginList.at(i);
         pluginPage->addPlugin(exporter->key(), exporter->formatName());
         plugins.insert(exporter->key(), exporter);
    }
}

void TupExportWidget::setExporter(TupExportInterface::Plugin plugin)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExportWidget::setExporter()] - plugin -> " << plugin;
    #endif

    if (plugins.contains(plugin)) {
        TupExportInterface *currentExporter = plugins[plugin];
        pluginPage->setFormats(plugin, currentExporter->availableFormats());

        if (currentExporter) {
            animationExport->setCurrentExporter(currentExporter);
            imagesArrayExport->setCurrentExporter(currentExporter);
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "TupExportWidget::setExporter() - [ Fatal Error ] - Can't load export plugin -> " << plugin;
        #endif
    }
}

void TupExportWidget::setProjectParams(const QString &username, const QString &secret, const QString &path)
{
    videoProperties->setProjectParams(username, secret, path);
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

void TupExportWidget::updateWindowTitle()
{
    QString label = tr("Posting...");
    if (exportFlag)
        label = tr("Exporting...");

    setWindowTitle(label);
    enableButtonSet(false);
}
