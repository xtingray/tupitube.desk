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

#include "tconfigurationdialog.h"
#include "tapplicationproperties.h"

TConfigurationDialog::TConfigurationDialog(QWidget *parent) : QDialog(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *pagesLayout = new QHBoxLayout;
    
    list = new QListWidget(this);
    list->setFlow(QListView::TopToBottom);
    list->setWrapping(false);
    list->setViewMode(QListView::IconMode);
    list->setIconSize(QSize(96, 84));
    list->setMovement(QListView::Static);
    list->setSpacing(10);
    
    connect(list, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));

    QWidget *widget = new QWidget;
    widget->setFixedWidth(130);
    QVBoxLayout *listLayout = new QVBoxLayout(widget);
    listLayout->addWidget(list);

    pageArea = new QStackedWidget;
    pagesLayout->addWidget(widget);
    pagesLayout->addWidget(pageArea, 1);

    mainLayout->addLayout(pagesLayout);
    
    QHBoxLayout *buttonBox = new QHBoxLayout; 

    QPushButton *applyButton = new QPushButton;
    applyButton->setIcon(QIcon(THEME_DIR + "icons/apply.png"));
    applyButton->setToolTip(tr("&Apply"));
    applyButton->setMinimumWidth(60);
    applyButton->setDefault(true);

    QPushButton *cancelButton = new QPushButton;
    cancelButton->setIcon(QIcon(THEME_DIR + "icons/close.png"));
    cancelButton->setToolTip(tr("&Cancel"));
    cancelButton->setMinimumWidth(60);

    buttonBox->addWidget(cancelButton);
    buttonBox->addWidget(applyButton);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->setAlignment(Qt::AlignRight);
    bottomLayout->addLayout(buttonBox);

    mainLayout->addWidget(new TSeparator());
    mainLayout->addLayout(bottomLayout);
}

TConfigurationDialog::~TConfigurationDialog()
{
    delete list;
    delete pageArea;
}

void TConfigurationDialog::addPage(QWidget *page, const QString &label, const QIcon &icon)
{
    QListWidgetItem *pageItem = new QListWidgetItem(list);
    pageItem->setIcon(icon);
    pageItem->setText(label);
    pageItem->setTextAlignment(Qt::AlignHCenter);
    pageItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    
    pageArea->addWidget(page);
}

QWidget *TConfigurationDialog::currentPage() const
{
    return pageArea->currentWidget();
}

void TConfigurationDialog::apply()
{
}

void TConfigurationDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;
    
    pageArea->setCurrentIndex(list->row(current));
}

void TConfigurationDialog::setCurrentItem(int row)
{
   list->setCurrentRow(row);
}
