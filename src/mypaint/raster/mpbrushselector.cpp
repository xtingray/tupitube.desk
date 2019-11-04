/*
    Copyright © 2015 by The QTMyPaint Project

    This file is part of QTMyPaint, a Qt-based interface for MyPaint C++ library.

    QTMyPaint is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QTMyPaint is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QTMyPaint. If not, see <http://www.gnu.org/licenses/>.
*/
#include "mpbrushselector.h"

#include <QDir>
#include <QListWidget>

#define BRUSH_CONTENT_EXT ".myb"
#define BRUSH_PREVIEW_EXT "_prev.png"
#define BRUSH_LIST        "brushes.conf"
#define ICON_SZ           64

MPBrushSelector::MPBrushSelector(const QString &brushLibPath, QWidget *parent): QTabWidget(parent), brushesPath(brushLibPath)
{
    // First, we parse the "order.conf" file to fill m_brushLib
    QFile fileOrder(brushLibPath + QDir::separator() + BRUSH_LIST);
    if (fileOrder.open(QIODevice::ReadOnly)) {
        // Decode the order info. This code replace the auto-parse code (commented below)
        // Note about encoding : we consider this is ASCII file (UTF8 would be safer
        // but this is libMyPaint dev team decision)
        QString currentGroup; // no group for now.
        QStringList brushesGroup;

        while(!fileOrder.atEnd()) {
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
                currentGroup = line.section(':',1).trimmed(); // Get the name after the first ':' separator
                brushesGroup.clear();
                continue;
            }

            // Ok, line contains a (partial) file path. Let's check that the file exists before we include it:
            if (QFileInfo(brushLibPath + QDir::separator() + line + BRUSH_CONTENT_EXT).isReadable())
                brushesGroup << line;
        }
        // last group :
        if (!currentGroup.isEmpty() && !brushesGroup.isEmpty())
            brushLib.insert(currentGroup, brushesGroup);

        // Now we create a QListWidget (displaying icons) for each stringList
        foreach (const QString &caption, brushLib.keys()) {
            const QStringList subList = brushLib.value(caption);
            if (subList.isEmpty())
                continue; // this should not happen...

            QListWidget* brushesList = new QListWidget();
            brushesList->setUniformItemSizes(true);
            brushesList->setViewMode(QListView::IconMode);
            brushesList->setResizeMode(QListView::Adjust);
            brushesList->setMovement(QListView::Static);
            brushesList->setFlow(QListView::LeftToRight);
            brushesList->setSelectionMode(QAbstractItemView::SingleSelection);
            brushesList->setIconSize(QSize(ICON_SZ,ICON_SZ));
            connect(brushesList, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(itemClicked(QListWidgetItem*)));

            // Add this ListWidget to the TabWidget:
            addTab(brushesList, caption);
            // Populate the ListWidget with brushes (and their preview):
            for (int n = 0; n < subList.count(); n++) {
                QString name(subList[n]);
                QIcon preview(brushesPath + QDir::separator() + name + BRUSH_PREVIEW_EXT);
                QListWidgetItem* item = new QListWidgetItem(preview, QString(), brushesList, n); // no need to show the name as it is already visible in preview
                // item->setIconSize(QSize(128,128));
                item->setToolTip(QString("%1 in \"%2\".").arg(name).arg(caption));
            }
        }
        // for now, no brush is selected. Expecting some order from owner...
    }
}

void MPBrushSelector::itemClicked(QListWidgetItem *item)
{
    QListWidget* brushesList = item->listWidget();
    if (brushesList) {
        QString caption;
        // first of all, we will deselect all other items in other panels :
        for (int index = 0; index < count(); index++) {
            QListWidget *list = dynamic_cast<QListWidget*>(widget(index));
            if (list != brushesList)
                list->clearSelection();
            else
                caption = tabText(index);
        }
        // fine, let's read this one and emit the content to any receiver:
        const QStringList subList = brushLib.value(caption);
        QFile file(brushesPath + QDir::separator() + subList.at(item->type()) + BRUSH_CONTENT_EXT);
        // qDebug(f.fileName().toAscii());
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray content = file.readAll();
            content.append(static_cast<char>(0));
            emit brushSelected(content); // Read the whole file and broadcast is as a char* buffer
        }
    }
}

void MPBrushSelector::selectBrush (QString brushName)
{
    if (!isValid())
        return;

    QListWidget * brushTab = nullptr;
    QListWidgetItem * brushItem = nullptr;
    // We search for the brush requested :
    for (int page = count()-1; page >= 0 && !brushItem; page--) {
        // reverse loop so we leave it with first page
        brushTab = dynamic_cast<QListWidget*>(widget(page));
        QString caption = tabText(page);
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
        setCurrentWidget(brushTab);
        brushTab->setCurrentItem(brushItem);
        itemClicked(brushItem);
    }
}
