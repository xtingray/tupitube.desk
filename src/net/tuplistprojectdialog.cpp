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

#include "tuplistprojectdialog.h"

TupListProjectDialog::TupListProjectDialog(int projects, int collabs, const QString &serverName) : QDialog()
{
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/open.png")));
    setWindowTitle(tr("Projects List from Server") + " - [ " + serverName  + " ]");
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);

    if (projects > 0) {
        works = tree(true);
        connect(works, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(updateWorkTree()));
        connect(works, SIGNAL(itemSelectionChanged()), this, SLOT(updateWorkTree()));
        connect(works, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(execAccept(QTreeWidgetItem *, int)));
    }

    if (collabs > 0) {
        contributions = tree(false);
        connect(contributions, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(updateContribTree()));
        connect(contributions, SIGNAL(itemSelectionChanged()), this, SLOT(updateContribTree()));
        connect(contributions, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(execAccept(QTreeWidgetItem *, int)));
    }

    QHBoxLayout *search = new QHBoxLayout;
    TreeWidgetSearchLine *searchLine = 0;
    QToolButton *button = new QToolButton;
    button->setIcon(QIcon(THEME_DIR + "icons/zoom.png"));

    QLabel *worksLabel = new QLabel(tr("My works:"));
    QLabel *contribLabel = new QLabel(tr("My contributions:"));

    if (projects > 0 && collabs > 0) {
        QList<QTreeWidget *> trees;
        trees << works << contributions;
        searchLine = new TreeWidgetSearchLine(this, trees);
        search->addWidget(searchLine);
        search->addWidget(button);

        layout->addLayout(search);
        layout->addWidget(worksLabel);
        layout->addWidget(works);
        layout->addWidget(contribLabel);
        layout->addWidget(contributions);
    } else if (projects > 0) {
               searchLine = new TreeWidgetSearchLine(this, works);
               search->addWidget(searchLine);
               search->addWidget(button);

               layout->addLayout(search);
               layout->addWidget(worksLabel);
               layout->addWidget(works);
    } else if (collabs > 0) {
               searchLine = new TreeWidgetSearchLine(this, contributions);
               search->addWidget(searchLine);
               search->addWidget(button);

               layout->addLayout(search);
               layout->addWidget(contribLabel);
               layout->addWidget(contributions);
    }

    connect(button, SIGNAL(clicked()), searchLine, SLOT(clear()));

    //----
    QHBoxLayout *buttons = new QHBoxLayout;
    QPushButton *accept = new QPushButton(tr("OK"));
    accept->setDefault(true);
    QPushButton *cancel = new QPushButton("Cancel");
    connect(accept, SIGNAL(clicked ()), this, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

    buttons->addWidget(cancel);
    buttons->addWidget(accept);
    layout->addLayout(buttons);

    setMinimumWidth(615); 
    index = 0;
}

TupListProjectDialog::~TupListProjectDialog()
{
}

QTreeWidget *TupListProjectDialog::tree(bool myWorks)
{
    QTreeWidget *tree = new QTreeWidget;
    tree->setFixedHeight(120);
    if (myWorks)
        tree->setHeaderLabels(QStringList() << tr("Name") << tr("Description") << tr("Date"));
    else
        tree->setHeaderLabels(QStringList() << tr("Name") << tr("Author") << tr("Description") << tr("Date"));

    tree->header()->show();

    if (myWorks) {
        tree->setColumnWidth(0, 200);
        tree->setColumnWidth(1, 250);
        tree->setColumnWidth(2, 55);
    } else {
        tree->setColumnWidth(0, 150);
        tree->setColumnWidth(1, 100);
        tree->setColumnWidth(2, 200);
        tree->setColumnWidth(3, 55);
    }

    return tree;
}

void TupListProjectDialog::addWork(const QString &project, const QString &name, const QString &description, const QString &date)
{
    workList.append(project);

    QTreeWidgetItem *item = new QTreeWidgetItem(works);
    item->setText(0, name);
    item->setText(1, description);
    item->setText(2, date);

    if (index == 0) {
        isMine = true;
        works->setCurrentItem(item);
        filename = project;
    }

    index++;
}

void TupListProjectDialog::addContribution(const QString &filename, const QString &name, const QString &author, const QString &description, const QString &date)
{
    contribList.append(filename);
    authors.append(author);

    QTreeWidgetItem *item = new QTreeWidgetItem(contributions);
    item->setText(0, name);
    item->setText(1, author);
    item->setText(2, description);
    item->setText(3, date);
}

QString TupListProjectDialog::projectID() const
{
    return filename;
}

QString TupListProjectDialog::owner() const
{
    return user;
}

void TupListProjectDialog::execAccept(QTreeWidgetItem *item, int index)
{
    Q_UNUSED(item);

    if (index >= 0)
        accept();
}

void TupListProjectDialog::updateWorkTree()
{
    if (works->hasFocus()) {
        if (contribList.size() > 0)
            contributions->clearSelection();
        int index = works->currentIndex().row();
        filename = workList.at(index);
        isMine = true;
    }
}

void TupListProjectDialog::updateContribTree()
{
    if (contributions->hasFocus()) {
        if (workList.size() > 0)
            works->clearSelection();
        int index = contributions->currentIndex().row();
        isMine = false;
        filename = contribList.at(index);
        user = authors.at(index);
    }
}

bool TupListProjectDialog::workIsMine()
{
    return isMine;
}

