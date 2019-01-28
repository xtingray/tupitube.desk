/***************************************************************************
 *   Project TUPITUBE DESK                                                 *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2019:                                                                 *
 *    Alejandro Carrasco Rodríguez                                         *
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

#include "titemselector.h"

#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

TItemSelector::TItemSelector(QWidget *parent) : QWidget(parent)
{
    available = new QListWidget;

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(available);

    QVBoxLayout *controlBox = new QVBoxLayout;
    controlBox->setSpacing(0);
    controlBox->setMargin(2);
    controlBox->addStretch();

    QToolButton *nextButton = new QToolButton;
    nextButton->setIcon(QPixmap(THEME_DIR + "icons/select_scene.png"));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(addCurrent()));

    controlBox->addWidget(nextButton);
    controlBox->setSpacing(5);

    QToolButton *previousButton = new QToolButton;
    previousButton->setIcon(QPixmap(THEME_DIR + "icons/unselect_scene.png"));
    connect(previousButton, SIGNAL(clicked()), this, SLOT(removeCurrent()));

    controlBox->addWidget(previousButton);
    controlBox->addStretch();

    layout->addLayout(controlBox);

    selected = new QListWidget;
    layout->addWidget(selected);

    setLayout(layout);
}

TItemSelector::~TItemSelector()
{

}

void TItemSelector::selectFirstItem() {
     if (available->item(0)) {
         available->setCurrentRow(0);
         emit changed();
     }
}

void TItemSelector::addCurrent()
{
    int row = available->currentRow();
    if (row >= 0) {
        QListWidgetItem *item = available->currentItem();
        addSelectedItem(item->text());

        emit changed();
    }
}

void TItemSelector::removeCurrent()
{
    int row = selected->currentRow();
    if (row >= 0) {
        // QListWidgetItem *item = selected->takeItem(row);
        // available->addItem(item);

        selected->takeItem(row);
        emit changed();
    }
}

void TItemSelector::upCurrent()
{
    int row = selected->currentRow();
    if (row >= 0) {
        QListWidgetItem *item = selected->takeItem(row);
        selected->insertItem(row-1, item);
        selected->setCurrentItem(item);

        emit changed();
    }
}

void TItemSelector::downCurrent()
{
    int row = selected->currentRow();
    if (row >= 0) {
        QListWidgetItem *item = selected->takeItem(row);
        selected->insertItem(row+1,item);
        selected->setCurrentItem(item);

        emit changed();
    }
}

void TItemSelector::setItems(const QStringList &items)
{
    available->clear();
    addItems(items);
}

int TItemSelector::addItem(const QString &itemLabel)
{
    QListWidgetItem *item = new QListWidgetItem(itemLabel, available);
    int index = available->count() - 1;
    item->setData(4321, index);

    return index;
}

void TItemSelector::addSelectedItem(const QString &itemLabel)
{
    QListWidgetItem *item = new QListWidgetItem(itemLabel, selected);
    QStringList params = itemLabel.split(":");
    int index = params.at(0).toInt() - 1;
    item->setData(4321, index);
}

void TItemSelector::addItems(const QStringList &items)
{
    foreach (QString item, items)
             addItem(item);
}

QStringList TItemSelector::selectedItems() const
{
    QStringList items;
    for (int row = 0; row < selected->count(); row++) {
         QListWidgetItem *item = selected->item(row);
         items << item->text();
    }

    return items;
}

QList<int> TItemSelector::selectedIndexes() const
{
    QList<int> indexes;
    for (int row = 0; row < selected->count(); row++) {
         QListWidgetItem *item = selected->item(row);
         indexes << item->data(4321).toInt();
    }

    return indexes;
}

void TItemSelector::clear()
{
    available->clear();
    reset();
}

void TItemSelector::reset()
{
    selected->clear();
    emit changed();
}
