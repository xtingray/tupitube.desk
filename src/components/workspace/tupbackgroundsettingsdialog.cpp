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
#include "tseparator.h"

#include <QVBoxLayout>
#include <QListWidget>
#include <QDialogButtonBox>

TupBackgroundSettingsDialog::TupBackgroundSettingsDialog(QList<TupBackground::BgType> bgLayers, QList<bool> bgVisibility,
                                                         QWidget *parent): QDialog(parent)
{
    setModal(true);
    setWindowTitle(tr("Background Settings"));
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/background_settings.png")));

    QVBoxLayout *layout = new QVBoxLayout(this);
    bgList = new TupBackgroundList(this);

    for (int i=0; i<bgLayers.count(); i++) {
        QString label = "";
        switch(bgLayers.at(i)) {
            case TupBackground::VectorDynamic:
            {
                label = tr("Vector Dynamic Background");
            }
            break;
            case TupBackground::RasterDynamic:
            {
                label = tr("Raster Dyanmic Background");
            }
            break;
            case TupBackground::VectorStatic:
            {
                label = tr("Vector Static Background");
            }
            break;
            case TupBackground::RasterStatic:
            {
                label = tr("Raster Static Background");
            }
        }

        TupListItem* bgItem = new TupListItem;
        bgList->addItem(bgItem);
        TupBackgroundItem *bgWidget = new TupBackgroundItem(bgLayers.at(i), label, bgVisibility.at(i));
        bgList->setItemWidget(bgItem, bgWidget);
    }

    bgList->setDragDropMode(QAbstractItemView::InternalMove);
    bgList->setFixedHeight(170);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Apply, Qt::Horizontal, this);
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(apply()));

    layout->addWidget(bgList);
    layout->addWidget(new TSeparator());
    layout->addWidget(buttonBox);
}

TupBackgroundSettingsDialog::~TupBackgroundSettingsDialog()
{
}

void TupBackgroundSettingsDialog::apply()
{
    QList<QPair<TupBackground::BgType, bool>> valuesList;
    for (int i=0; i < bgList->count(); i++) {
         TupBackgroundItem *widget = static_cast<TupBackgroundItem *>(bgList->itemWidget(bgList->item(i)));

         QPair<TupBackground::BgType, bool> values = widget->getValues();
         idList << values.first;
         visibilityList << values.second;
    }

    emit valuesUpdated(idList, visibilityList);
    close();
}
