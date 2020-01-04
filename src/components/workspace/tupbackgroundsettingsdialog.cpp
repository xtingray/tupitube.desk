/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez                                                     *
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

#include "tupbackgroundsettingsdialog.h"
#include "tupbackgroundlist.h"
#include "tupbackgrounditem.h"
#include "tseparator.h"

#include <QVBoxLayout>
#include <QListWidget>
#include <QDialogButtonBox>

TupBackgroundSettingsDialog::TupBackgroundSettingsDialog(QWidget *parent): QDialog(parent)
{
    setModal(true);
    setWindowTitle(tr("Background Settings"));
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/background_settings.png")));

    QVBoxLayout *layout = new QVBoxLayout(this);
    TupBackgroundList *list = new TupBackgroundList(this);

    TupListItem* bgItem1 = new TupListItem;
    list->addItem(bgItem1);
    TupBackgroundItem *bgWidget1 = new TupBackgroundItem(tr("Vector Dynamic Background"));
    list->setItemWidget(bgItem1, bgWidget1);

    TupListItem* bgItem2 = new TupListItem;
    list->addItem(bgItem2);
    TupBackgroundItem *bgWidget2 = new TupBackgroundItem(tr("Vector Static Background"));
    list->setItemWidget(bgItem2, bgWidget2);

    TupListItem* bgItem3 = new TupListItem;
    list->addItem(bgItem3);
    TupBackgroundItem *bgWidget3 = new TupBackgroundItem(tr("Raster Dyanmic Background"));
    list->setItemWidget(bgItem3, bgWidget3);

    TupListItem* bgItem4 = new TupListItem;
    list->addItem(bgItem4);
    TupBackgroundItem *bgWidget4 = new TupBackgroundItem(tr("Raster Static Background"));
    list->setItemWidget(bgItem4, bgWidget4);

    list->setDragDropMode(QAbstractItemView::InternalMove);
    list->setFixedHeight(170);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Apply, Qt::Horizontal, this);
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(apply()));

    layout->addWidget(list);
    layout->addWidget(new TSeparator());
    layout->addWidget(buttonBox);
}

TupBackgroundSettingsDialog::~TupBackgroundSettingsDialog()
{
}

void TupBackgroundSettingsDialog::apply()
{
}
