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

#include "rasterbrusheswidget.h"

#include <QLabel>
#include <QListWidget>
#include <QGroupBox>

#define BRUSH_CONTENT_EXT ".myb"
#define BRUSH_PREVIEW_EXT "_prev.png"
#define BRUSH_LIST        "brushes.conf"
#define ICON_SZ           64

RasterBrushesWidget::RasterBrushesWidget(const QString &brushLibPath, QWidget *parent):
                                         TupModuleWidgetBase(parent), brushesPath(brushLibPath)
{
    #ifdef TUP_DEBUG
       qDebug() << "[RasterBrushesWidget()]";
    #endif

    setWindowTitle(tr("Brush Properties"));
    setWindowIcon(QIcon(THEME_DIR + "icons/brush.png"));

    TCONFIG->beginGroup("Raster");
    groupIndex = TCONFIG->value("BrushGroup", 0).toInt();
    brushIndex = TCONFIG->value("BrushIndex", 0).toInt();

    #ifdef TUP_DEBUG
       qDebug() << "[RasterBrushesWidget()] - groupIndex -> " << groupIndex;
       qDebug() << "[RasterBrushesWidget()] - brushIndex -> " << brushIndex;
    #endif

    QGroupBox *groupBox = new QGroupBox();
    QVBoxLayout *buttonsLayout = new QVBoxLayout(groupBox);
    buttonsLayout->setMargin(0);
    buttonsLayout->setSpacing(0);

    stackedWidget = new QStackedWidget(this);

    // First, we parse the "order.conf" file to fill m_brushLib
    QFile fileOrder(brushLibPath + QDir::separator() + BRUSH_LIST);

    if (fileOrder.open(QIODevice::ReadOnly)) {
        // Decode the order info. This code replace the auto-parse code (commented below)
        // Note about encoding : we consider this is ASCII file (UTF8 would be safer
        // but this is libMyPaint dev team decision)
        QString currentGroup; // no group for now.
        QStringList brushesGroup;

        QVector<QString> sortedList(6);

        while (!fileOrder.atEnd()) {
            QString line(fileOrder.readLine().trimmed()); // Get a line without begin/end extra space chars
            if (line.isEmpty() || line.startsWith("#")) // empty line or starting with # are ignored
                continue;

            if (line.startsWith("Group:")) {
                // brushes below this line are owned by this group:
                // first, we store the last brushesGroup (if any). Note that declaring 2 groups with the same name
                // is wrong (only the last one will be visible)
                if (!currentGroup.isEmpty() && !brushesGroup.isEmpty())
                    brushLib.insert(currentGroup, brushesGroup);

                // Now, we prepare to get the brushes for this new group:
                currentGroup = line.section(':', 1).trimmed(); // Get the name after the first ':' separato
                if (currentGroup.compare("Art1") == 0) {
                    currentGroup = tr("Art Set 1");
                    sortedList[0] = currentGroup;
                } else if (currentGroup.compare("Art2") == 0) {
                    currentGroup = tr("Art Set 2");
                    sortedList[1] = currentGroup;
                } else if (currentGroup.compare("Art3") == 0) {
                    currentGroup = tr("Art Set 3");
                    sortedList[2] = currentGroup;
                } else if (currentGroup.compare("Classic") == 0) {
                    currentGroup = tr("Classic");
                    sortedList[3] = currentGroup;
                } else if (currentGroup.compare("Experimental") == 0) {
                    currentGroup = tr("Experimental");
                    sortedList[4] = currentGroup;
                } else if (currentGroup.compare("Erasers") == 0) {
                    currentGroup = tr("Erasers");
                    sortedList[5] = currentGroup;
                }

                brushesGroup.clear();
                continue;
            }

            // Ok, line contains a (partial) file path. Let's check that the file exists before we include it:
            if (QFileInfo(brushLibPath + QDir::separator() + line + BRUSH_CONTENT_EXT).isReadable())
                brushesGroup << line;
        }
        // Last group :
        if (!currentGroup.isEmpty() && !brushesGroup.isEmpty())
            brushLib.insert(currentGroup, brushesGroup);

        // Now we create a QListWidget (displaying icons) for each stringList
        for (int i = 0; i < sortedList.size(); ++i) {
            QString caption = sortedList.at(i);
            const QStringList subList = brushLib.value(caption);
            if (subList.isEmpty())
                continue; // this should not happen...

            QListWidget* brushesList = new QListWidget();
            brushesList->setWindowTitle(caption);
            brushesList->setUniformItemSizes(true);
            brushesList->setViewMode(QListView::IconMode);
            brushesList->setResizeMode(QListView::Adjust);
            brushesList->setMovement(QListView::Static);
            brushesList->setFlow(QListView::LeftToRight);
            brushesList->setSelectionMode(QAbstractItemView::SingleSelection);
            brushesList->setIconSize(QSize(ICON_SZ, ICON_SZ));
            connect(brushesList, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(itemClicked(QListWidgetItem*)));

            QString iconPath = THEME_DIR;
            if (caption.compare(tr("Art Set 1")) == 0) {
                iconPath += "icons/art1.png";
            } else if (caption.compare(tr("Art Set 2")) == 0) {
                iconPath += "icons/art2.png";
            } else if (caption.compare(tr("Art Set 3")) == 0) {
                iconPath += "icons/art3.png";
            } else if (caption.compare(tr("Classic")) == 0) {
                iconPath += "icons/classic.png";
            } else if (caption.compare(tr("Experimental")) == 0) {
                iconPath += "icons/experimental.png";
            } else if (caption.compare(tr("Erasers")) == 0) {
                iconPath += "icons/eraser.png";
            }

            RasterButton *button = new RasterButton(i, caption);
            button->setIcon(QIcon(iconPath));
            button->setCheckable(true);
            connect(button, SIGNAL(buttonClicked(int)), this, SLOT(updateBrushesPanel(int)));
            buttonsList.insert(i, button);

            // Populate the ListWidget with brushes (and their preview):
            for (int n = 0; n < subList.count(); n++) {
                QString name(subList[n]);
                QIcon preview(brushesPath + QDir::separator() + name + BRUSH_PREVIEW_EXT);
                // no need to show the name as it is already visible in preview
                QListWidgetItem* item = new QListWidgetItem(preview, QString(), brushesList, n);
                item->setToolTip(QString("%1 in \"%2\".").arg(name).arg(caption));
            }
            brushesSet.insert(i, brushesList);
        }

        for (int i=0; i<buttonsList.size(); i++) {
            buttonsLayout->addWidget(buttonsList.at(i));
            // Add this ListWidget to the QStackedWidget:
            stackedWidget->addWidget(brushesSet.at(i));
        }
    }

    addChild(groupBox);
    addChild(stackedWidget);
}

RasterBrushesWidget::~RasterBrushesWidget()
{
    #ifdef TUP_DEBUG
       qDebug() << "~RasterBrushesWidget()";
    #endif
}

void RasterBrushesWidget::loadInitSettings()
{
    // Setting latest parameters
    buttonsList.at(groupIndex)->setChecked(true);
    stackedWidget->setCurrentIndex(groupIndex);
    const QStringList subList = brushLib.value(buttonsList.at(groupIndex)->getLabel());

    /*
    qDebug() << "--> List Size: " << subList.size();
    qDebug() << "--> Group Index: " << groupIndex;
    qDebug() << "--> Brush Index: " << brushIndex;
    */

    selectBrush(subList.at(brushIndex));
}

void RasterBrushesWidget::itemClicked(QListWidgetItem *item)
{
    QListWidget* brushesList = item->listWidget();
    if (brushesList) {
        QString caption;
        // first of all, we will deselect all other items in other panels :
        for (int index = 0; index < stackedWidget->count(); index++) {
            QListWidget *list = dynamic_cast<QListWidget*>(stackedWidget->widget(index));
            if (list != brushesList)
                list->clearSelection();
            else
                caption = stackedWidget->widget(index)->windowTitle();
        }

        const QStringList subList = brushLib.value(caption);

        // fine, let's read this one and emit the content to any receiver:
        QFile file(brushesPath + QDir::separator() + subList.at(item->type()) + BRUSH_CONTENT_EXT);
        if (file.open(QIODevice::ReadOnly)) {
            TCONFIG->beginGroup("Raster");
            TCONFIG->setValue("BrushIndex", item->type());
            TCONFIG->sync();

            QByteArray content = file.readAll();
            content.append(static_cast<char>(0));
            emit brushSelected(content); // Read the whole file and broadcast is as a char* buffer
        }
    }
}

void RasterBrushesWidget::selectBrush(QString brushName)
{
    if (!isValid())
        return;

    QListWidget * brushTab = nullptr;
    QListWidgetItem * brushItem = nullptr;
    // We search for the brush requested :
    for (int page = stackedWidget->count()-1; page >= 0 && !brushItem; page--) {
        // reverse loop so we leave it with first page
        brushTab = dynamic_cast<QListWidget*>(stackedWidget->widget(page));
        QString caption = stackedWidget->widget(page)->windowTitle();
        const QStringList subList = brushLib.value(caption);
        if (!brushName.isEmpty()) {
            for (int idx = 0 ; idx < subList.count() ; idx++) {
                if (subList.at(idx) == brushName) {
                    brushItem = brushTab->item(idx);
                    break;
                }
            }
        }
    }

    // default one : we use the first tab page & the first item available:
    if (!brushItem && brushTab && brushTab->count())
        brushItem = brushTab->item(0);

    // Update GUI + load the brush (if any)
    if (brushItem){
        stackedWidget->setCurrentWidget(brushTab);
        brushTab->setCurrentItem(brushItem);
        itemClicked(brushItem);
    }
}

void RasterBrushesWidget::updateBrushesPanel(int index)
{
    if (groupIndex != index) {
        groupIndex = index;
        stackedWidget->setCurrentIndex(index);
        TCONFIG->beginGroup("Raster");
        TCONFIG->setValue("BrushGroup", index);
        TCONFIG->sync();

        foreach(RasterButton *button, buttonsList) {
            if (button->getIndex() != index) {
                button->setState(false);
                button->setChecked(false);
            }
        }
    }
}
