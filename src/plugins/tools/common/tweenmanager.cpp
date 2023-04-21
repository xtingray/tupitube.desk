/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *                                                                         *
 *   2019:                                                                 *
 *    Alejandro Carrasco                                                   *
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

#include "tweenmanager.h"

TweenManager::TweenManager(QWidget *parent) : QWidget(parent)
{
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    input = new QLineEdit;
    addButton = new TImageButton(QPixmap(kAppProp->themeDir() + "/icons/plus_sign.png"), 22);
    addButton->setToolTip(tr("Create a new Tween"));
    connect(input, SIGNAL(returnPressed()), this, SLOT(addTween()));
    connect(addButton, SIGNAL(clicked()), this, SLOT(addTween()));

    QHBoxLayout *lineLayout = new QHBoxLayout;
    lineLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    // lineLayout->setMargin(0);
    lineLayout->setSpacing(0);
    lineLayout->addWidget(input);
    lineLayout->addWidget(addButton);

    layout->addLayout(lineLayout);

    QBoxLayout *listLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    listLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    tweensList = new QListWidget;
    tweensList->setContextMenuPolicy(Qt::CustomContextMenu);
    tweensList->setViewMode(QListView::ListMode);
    tweensList->setFlow(QListView::TopToBottom);
    tweensList->setMovement(QListView::Static);
    tweensList->setFixedHeight(68);
    connect(tweensList, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showMenu(const QPoint &)));
    connect(tweensList, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(updateTweenData(QListWidgetItem *)));
    connect(tweensList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(editTween(QListWidgetItem *)));

    listLayout->addWidget(tweensList);

    layout->addLayout(listLayout);
}

TweenManager::~TweenManager()
{
    delete input;
    delete tweensList;
    delete addButton;
}

void TweenManager::loadTweenList(QList<QString> tweenList)
{
    tweensList->clear();

    for (int i=0; i < tweenList.size(); i++) {
        QListWidgetItem *tweenerItem = new QListWidgetItem(tweensList);
        tweenerItem->setText(tweenList.at(i));
        tweenerItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }

    tweensList->setCurrentRow(0);
}

bool TweenManager::itemExists(const QString &name)
{
     for (int i=0; i < tweensList->count(); i++) {
          QListWidgetItem *item = tweensList->item(i);
          if (name.compare(item->text()) == 0)
              return true;
     }

     return false;
}

void TweenManager::addTween()
{
    QString name = input->text();

    if (name.length() > 0) {
        if (!itemExists(name)) {
            QListWidgetItem *tweenerItem = new QListWidgetItem(tweensList);
            tweenerItem->setText(name);
            tweenerItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            input->clear();
            tweensList->setCurrentItem(tweenerItem);

            emit addNewTween(name);
        } else {
            TOsd::self()->display(TOsd::Error, tr("Tween name already exists!"));
        }
    } else {
        int i = 0;
        while (true) {
               QString num = QString::number(i); 
               if (i < 10)
                   num = "0" + QString::number(i);

               QString name = "tween" + num; 
               QList<QListWidgetItem *> items = tweensList->findItems(name, Qt::MatchExactly);
               if (items.count() == 0) {
                   input->setText(name);
                   break;
               }
               i++;
        }
    }
}

void TweenManager::editTween()
{
    QListWidgetItem *item = tweensList->currentItem();
    emit editCurrentTween(item->text());
}

void TweenManager::editTween(QListWidgetItem *item)
{
    emit editCurrentTween(item->text());
}

void TweenManager::removeTween()
{
    removeItemFromList();

    emit removeCurrentTween(target);
}

void TweenManager::removeItemFromList()
{
    QListWidgetItem *item = tweensList->currentItem();
    tweensList->takeItem(tweensList->row(item));
    target = item->text();
}

void TweenManager::showMenu(const QPoint &point)
{
    if (tweensList->count() > 0) {
        QAction *edit = new QAction(tr("Edit"), this);
        connect(edit, SIGNAL(triggered()), this, SLOT(editTween()));
        QAction *remove = new QAction(tr("Remove"), this);
        connect(remove, SIGNAL(triggered()), this, SLOT(removeTween()));

        QMenu *menu = new QMenu(tr("Options"));
        menu->addAction(edit);
        menu->addAction(remove);

        QPoint globalPos = tweensList->mapToGlobal(point);
        menu->exec(globalPos);
    }
}

void TweenManager::updateTweenData(QListWidgetItem *item)
{
    emit getTweenData(item->text());
}

void TweenManager::resetUI()
{
    input->clear();

    if (tweensList->count() > 0)
        tweensList->clear();
}

QString TweenManager::currentTweenName() const
{
    QListWidgetItem *item = tweensList->currentItem();
    return item->text();
}

int TweenManager::listSize()
{
    return tweensList->count();
}

void TweenManager::updateTweenName(const QString &name)
{
    QListWidgetItem *item = tweensList->currentItem();
    item->setText(name);
    target = name;
}
