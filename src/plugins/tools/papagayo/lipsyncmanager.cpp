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

#include "lipsyncmanager.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidgetItem>

LipSyncManager::LipSyncManager(QWidget *parent): QWidget(parent)
{
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    QBoxLayout *listLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    listLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    lipSyncList = new QListWidget;
    lipSyncList->setContextMenuPolicy(Qt::CustomContextMenu);
    lipSyncList->setViewMode(QListView::ListMode);
    lipSyncList->setFlow(QListView::TopToBottom);
    lipSyncList->setMovement(QListView::Static);
    lipSyncList->setFixedHeight(68);

    listLayout->addWidget(lipSyncList);

    openButton = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/plus_sign.png"), 22);
    openButton->setToolTip(tr("Open LipSync Creator"));
    connect(openButton, SIGNAL(clicked()), this, SIGNAL(openLipSyncCreator()));

    importButton = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/import_lipsync.png"), 22);
    importButton->setToolTip(tr("Import LipSync"));
    connect(importButton, SIGNAL(clicked()), this, SIGNAL(importLipSync()));

    editButton = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/edit_sign.png"), 22);
    editButton->setToolTip(tr("Edit LipSync"));
    connect(editButton, SIGNAL(clicked()), this, SLOT(editLipSync()));

    delButton = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/minus_sign.png"), 22);
    delButton->setToolTip(tr("Remove LipSync"));
    connect(delButton, SIGNAL(clicked()), this, SLOT(removeLipSync()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setAlignment(Qt::AlignHCenter);
    buttonLayout->setMargin(0);
    buttonLayout->addWidget(openButton);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(importButton);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(editButton);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(delButton);

    layout->addLayout(listLayout);
    layout->addLayout(buttonLayout);
}

LipSyncManager::~LipSyncManager()
{
}

void LipSyncManager::loadLipSyncList(QList<QString> list)
{
    lipSyncList->clear();

    int total = list.size();
    for (int i=0; i < total; i++) {
        QListWidgetItem *item = new QListWidgetItem(lipSyncList);
        item->setText(list.at(i));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }

    lipSyncList->setCurrentRow(0);
}

bool LipSyncManager::itemExists(const QString &name)
{
    for (int i=0; i < lipSyncList->count(); i++) {
         QListWidgetItem *item = lipSyncList->item(i);
         if (name.compare(item->text()) == 0)
             return true;
    }

     return false;
}

void LipSyncManager::editLipSync()
{
    if (lipSyncList->count() > 0) {
        QListWidgetItem *item = lipSyncList->currentItem();
        if (item)
            emit editCurrentLipSync(item->text());
    }
}

void LipSyncManager::removeLipSync()
{
    #ifdef TUP_DEBUG
        qDebug() << "[LipSyncManager::removeLipSync()]";
    #endif

    if (lipSyncList->count() > 0) {
        QListWidgetItem *item = lipSyncList->currentItem();
        if (item) {
            lipSyncList->takeItem(lipSyncList->row(item));
            target = item->text();
            emit removeCurrentLipSync(target);
        }
    }
}

void LipSyncManager::resetUI()
{
    if (lipSyncList->count() > 0)
        lipSyncList->clear();
}

QString LipSyncManager::currentLipSyncName() const
{
    QListWidgetItem *item = lipSyncList->currentItem();
    return item->text();
}

int LipSyncManager::listSize()
{
    return lipSyncList->count();
}

void LipSyncManager::addNewRecord(const QString &name)
{
    QFont f = font();
    f.setPointSize(8);

    QListWidgetItem *item = new QListWidgetItem(lipSyncList);
    item->setText(name);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    lipSyncList->setCurrentItem(item);
}
