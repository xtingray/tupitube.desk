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

#include "tupprojectimporterdialog.h"
#include "tseparator.h"
#include "tapptheme.h"

#include <algorithm>
#include <QLabel>
#include <QCheckBox>

TupProjectImporterDialog::TupProjectImporterDialog(const QString &projectName, QStringList scenes, bool libraryFlag,
                                   QWidget *parent) : QDialog(parent)
{
    setModal(true);
    setWindowTitle(projectName + " - " + tr("Project Assets"));
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/open.png")));

    setStyleSheet(TAppTheme::themeSettings());

    includeLibrary = libraryFlag;
    layout = new QVBoxLayout(this);
    setUI(scenes);
}

TupProjectImporterDialog::~TupProjectImporterDialog()
{
}

void TupProjectImporterDialog::setUI(QStringList scenes)
{
    QLabel *instructions = new QLabel(tr("Please, pick the elements you want to import:"));
    layout->addWidget(instructions);

    scenesListWidget = new QListWidget;
    for(int i=0; i < scenes.size(); i++) {
        QListWidgetItem *newItem = new QListWidgetItem;
        newItem->setFlags(newItem->flags()|Qt::ItemIsUserCheckable);
        newItem->setCheckState(Qt::Unchecked);
        newItem->setText(scenes.at(i));
        scenesListWidget->insertItem(i, newItem);
    }
    layout->addWidget(scenesListWidget);

    connect(scenesListWidget, SIGNAL(itemChanged(QListWidgetItem*)),
                     this, SLOT(addScene(QListWidgetItem*)));

    if (includeLibrary) {
        QCheckBox *libraryCheck = new QCheckBox(tr("Library"));
        connect(libraryCheck, SIGNAL(stateChanged(int)), this, SLOT(setLibraryFlag(int)));
        layout->addWidget(libraryCheck);

        TSeparator *separator = new TSeparator(Qt::Horizontal);
        layout->addWidget(separator);
    }

    QPushButton *closeButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/close.png")), "");
    closeButton->setToolTip(tr("Cancel"));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    okButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/apply.png")), "");
    okButton->setToolTip(tr("Import Assets"));
    okButton->setVisible(false);

    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));

    QWidget *buttonsWidget = new QWidget;
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonsWidget);
    buttonLayout->addWidget(closeButton);
    buttonLayout->addWidget(okButton);

    layout->addWidget(buttonsWidget, 1, Qt::AlignRight);
    layout->addStretch(1);
}

QList<int> TupProjectImporterDialog::scenes()
{
    std::sort(selectedScenes.begin(), selectedScenes.end());

    return selectedScenes;
}

bool TupProjectImporterDialog::isLibraryIncluded()
{
    return includeLibrary;
}

void TupProjectImporterDialog::addScene(QListWidgetItem *item)
{
    QString text = item->text();
    int index = scenesListWidget->row(item);
    if (item->checkState() == Qt::Checked) {
        selectedScenes << index;
        if (!okButton->isVisible())
            okButton->setVisible(true);
    } else {
        selectedScenes.removeOne(index);
        if (okButton->isVisible() && !includeLibrary)
            okButton->setVisible(false);
    }
}

void TupProjectImporterDialog::setLibraryFlag(int state)
{
    if (state == Qt::Checked) {
        includeLibrary = true;
        if (!okButton->isVisible())
            okButton->setVisible(true);
    } else {
        includeLibrary = false;
        if (okButton->isVisible() && selectedScenes.isEmpty())
            okButton->setVisible(false);
    }
}
