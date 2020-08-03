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

#include "tuppluginselector.h"


TupPluginSelector::TupPluginSelector() : TupExportWizardPage(tr("Select Plugin"))
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPluginSelector()]";
    #endif

    setTag("PLUGIN");
    QWidget *container = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(container);

    pluginList = new QListWidget;
    pluginList->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(pluginList, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(selectedPluginItem(QListWidgetItem *)));
    layout->addWidget(pluginList);

    formatList = new QListWidget;
    formatList->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(formatList, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(selectedFormatItem(QListWidgetItem *)));
    layout->addWidget(formatList);

    setWidget(container);
    reset();
}

TupPluginSelector::~TupPluginSelector()
{
}

bool TupPluginSelector::isComplete() const
{
    return pluginList->selectedItems().count() > 0 && formatList->selectedItems().count() > 0;
}

void TupPluginSelector::reset()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPluginSelector::reset()]";
    #endif

    pluginList->clearSelection();
    formatList->clearSelection();
    formatList->clear();

    videoFormats.clear();
    imageFormats.clear();
}

void TupPluginSelector::addPlugin(TupExportInterface::Plugin pluginId, const QString &pluginName)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPluginSelector::addPlugin()] - plugin -> " << pluginName;
        qDebug() << "[TupPluginSelector::addPlugin()] - plugin id-> " << pluginId;
    #endif

    new QListWidgetItem(pluginName, pluginList);
    plugins << pluginId;
}

void TupPluginSelector::selectedPluginItem(QListWidgetItem *item)
{
    Q_UNUSED(item)

    #ifdef TUP_DEBUG
        qDebug() << "[TupPluginSelector::selectedPluginItem()]";
    #endif

    int pluginIndex = pluginList->currentRow();
    if (pluginIndex != -1) {
        currentPlugin = plugins.at(pluginIndex);
        emit selectedPlugin(currentPlugin);
        emit completed();
    }
}

void TupPluginSelector::selectFirstPlugin()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPluginSelector::selecteFirstPlugin()]";
    #endif

    if (pluginList->item(0)) {
        pluginList->item(0)->setSelected(true);
        currentPlugin = plugins.at(0);
        emit selectedPlugin(plugins.at(0));
        emit completed();
    }
}

void TupPluginSelector::selectedFormatItem(QListWidgetItem *item)
{
    Q_UNUSED(item)
    #ifdef TUP_DEBUG
        qDebug() << "[TupPluginSelector::selectedFormatItem()]";
    #endif

    int formatIndex = formatList->currentRow();
    if (formatIndex != -1) {
        if (currentPlugin == TupExportInterface::VideoFormats) {
            TupExportInterface::Format format = videoFormats.at(formatIndex);
            extension = getFormatExtension(format);
            emit animationFormatSelected(format, extension);
        } else if (currentPlugin == TupExportInterface::ImageSequence) {
            TupExportInterface::Format format = imageFormats.at(formatIndex);
            extension = getFormatExtension(format);
            emit imagesArrayFormatSelected(format, extension);
        }
        emit completed();
    }
}

void TupPluginSelector::setFormats(TupExportInterface::Plugin plugin,TupExportInterface::Formats formats)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPluginSelector::setFormats()]";
    #endif

    formatList->clear();

    if (plugin == TupExportInterface::VideoFormats) {
        videoFormats.clear();

        if (formats & TupExportInterface::MP4) {
            new QListWidgetItem(tr("MP4 Video"), formatList);
            videoFormats << TupExportInterface::MP4;
        }

        #if defined(Q_OS_LINUX) || defined(Q_OS_WIN)
            /* SQA: This code is temporarily disabled
            if (formats & TupExportInterface::GIF) {
                new QListWidgetItem(tr("Animated GIF"), m_formatList);
            }
            */

            if (formats & TupExportInterface::MPEG) {
                new QListWidgetItem(tr("MPEG Video"), formatList);
                videoFormats << TupExportInterface::MPEG;
            }

            if (formats & TupExportInterface::AVI) {
                new QListWidgetItem(tr("AVI Video"), formatList);
                videoFormats << TupExportInterface::AVI;
            }
        #endif

        if (formats & TupExportInterface::MOV) {
            new QListWidgetItem(tr("QuickTime Video"), formatList);
            videoFormats << TupExportInterface::MOV;
        }
    } else if (plugin == TupExportInterface::ImageSequence) {
        imageFormats.clear();

        if (formats & TupExportInterface::PNG) {
            new QListWidgetItem(tr("PNG Image Sequence"), formatList);
            imageFormats << TupExportInterface::PNG;
        }

        if (formats & TupExportInterface::JPEG) {
            new QListWidgetItem(tr("JPEG Image Sequence"), formatList);
            imageFormats << TupExportInterface::JPEG;
        }

        if (formats & TupExportInterface::SVG) {
            new QListWidgetItem(tr("SVG Image Sequence"), formatList);
            imageFormats << TupExportInterface::SVG;
        }

        /* SQA: Pending for implementation
        if (formats & TupExportInterface::APNG) {
            new QListWidgetItem(tr("Animated PNG (APNG)"), formatList);
        }
        */
    }
}

char const* TupPluginSelector::getFormatExtension(TupExportInterface::Format formatId)
{ 
    // if (formatId == TupExportInterface::MPEG)
    //     return ".mpg";

    if (formatId == TupExportInterface::MP4)
        return ".mp4";

    if (formatId == TupExportInterface::AVI)
        return ".avi";

    if (formatId == TupExportInterface::MOV)
        return ".mov";

    // if (formatId == TupExportInterface::GIF)
    //     return ".gif";

    if (formatId == TupExportInterface::PNG)
        return ".png";

    if (formatId == TupExportInterface::SVG)
        return ".svg";

    return ".none";
}

const char* TupPluginSelector::getFileExtension()
{
    return extension;
}
