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
#include "tseparator.h"
#include "toptionaldialog.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QScreen>

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
    openButton->setToolTip(tr("Open Lip-Sync Creator"));
    connect(openButton, SIGNAL(clicked()), this, SIGNAL(lipsyncCreatorRequested()));

    editPgoButton = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/edit_sign.png"), 22);
    editPgoButton->setToolTip(tr("Edit Lip-Sync"));
    connect(editPgoButton, SIGNAL(clicked()), this, SLOT(editLipSync()));

    editMouthButton = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/mouth.png"), 22);
    editMouthButton->setToolTip(tr("Edit Mouth"));
    connect(editMouthButton, SIGNAL(clicked()), this, SLOT(editMouth()));

    deleteButton = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/minus_sign.png"), 22);
    deleteButton->setToolTip(tr("Remove Lip-Sync"));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(removeLipSync()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setAlignment(Qt::AlignHCenter);
    // buttonLayout->setMargin(0);
    buttonLayout->addWidget(openButton);
    buttonLayout->addSpacing(5);
    buttonLayout->addWidget(new TSeparator(Qt::Vertical));
    buttonLayout->addSpacing(5);
    buttonLayout->addWidget(editPgoButton);
    buttonLayout->addSpacing(5);
    buttonLayout->addWidget(new TSeparator(Qt::Vertical));
    buttonLayout->addSpacing(5);
    buttonLayout->addWidget(editMouthButton);
    buttonLayout->addSpacing(5);
    buttonLayout->addWidget(new TSeparator(Qt::Vertical));
    buttonLayout->addSpacing(5);
    buttonLayout->addWidget(deleteButton);

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
            emit lipsyncEditionRequested(item->text());
    }
}

void LipSyncManager::editMouth()
{
    if (lipSyncList->count() > 0) {
        QListWidgetItem *item = lipSyncList->currentItem();
        if (item)
            emit mouthEditionRequested(item->text());
    }
}

void LipSyncManager::removeLipSync()
{
    #ifdef TUP_DEBUG
        qDebug() << "[LipSyncManager::removeLipSync()]";
    #endif

    if (lipSyncList->count() > 0) {
        TOptionalDialog dialog(tr("Are you sure you want to delete this lip-sync record?"),
                               tr("Confirmation Required"), false, false, this);
        dialog.setModal(true);
        QScreen *screen = QGuiApplication::screens().at(0);
        dialog.move(static_cast<int> ((screen->geometry().width() - dialog.sizeHint().width()) / 2),
                    static_cast<int> ((screen->geometry().height() - dialog.sizeHint().height()) / 2));
        dialog.exec();

        TOptionalDialog::Result result = dialog.getResult();
        if (result == TOptionalDialog::Accepted) {
            QListWidgetItem *item = lipSyncList->currentItem();
            if (item) {
                lipSyncList->takeItem(lipSyncList->row(item));
                target = item->text();
                emit currentLipSyncRemoved(target);
            }
        }
    }
}

void LipSyncManager::removeRecordFromList(const QString &name)
{
    #ifdef TUP_DEBUG
        qDebug() << "[LipSyncManager::removeRecordFromList()] - name -> " << name;
    #endif

    QList<QListWidgetItem *> items = lipSyncList->findItems(name, Qt::MatchExactly);
    if (items.size() == 1) {
        QListWidgetItem *item = items.first();
        if (item)
            lipSyncList->takeItem(lipSyncList->row(item));
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[LipSyncManager::removeRecordFromList()] - Warning: Can't find item -> " << name;
        #endif
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
