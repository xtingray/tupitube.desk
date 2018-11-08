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

#include "tupinfodialog.h"

#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>

TupInfoDialog::TupInfoDialog(const QString &tags, const QString &author, 
                             const QString &desc, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Project Information"));
    setModal(true);

    QGridLayout *gridLayout = new QGridLayout;

    QLabel *tagsLabel = new QLabel(tr("Tags"));
    gridLayout->addWidget(tagsLabel, 0, 0);

    projectTags = new QLineEdit();
    projectTags->setText(tags);
    gridLayout->addWidget(projectTags, 0, 1);

    QLabel *authorLabel = new QLabel(tr("Author"));
    gridLayout->addWidget(authorLabel, 1, 0);

    authorName = new QLineEdit();
    authorName->setText(author);
    gridLayout->addWidget(authorName, 1, 1);

    QLabel *descLabel = new QLabel(tr("Description"));
    descText = new QPlainTextEdit;
    descText->setPlainText(desc);

    QVBoxLayout *descLayout = new QVBoxLayout;
    descLayout->addWidget(descLabel);
    descLayout->addWidget(descText);

    QPushButton *okButton = new QPushButton(tr("Update"), this);
    connect(okButton, SIGNAL(pressed()), this, SLOT(updateInfo()));
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), this);
    connect(cancelButton, SIGNAL(pressed()), this, SLOT(reject()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(2);
    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addWidget(okButton);
    
    QVBoxLayout *layout = new QVBoxLayout;    
    layout->addLayout(gridLayout);
    layout->addLayout(descLayout);
    layout->addLayout(buttonsLayout);

    setLayout(layout);
}

TupInfoDialog::~TupInfoDialog()
{
}

void TupInfoDialog::focusProjectLabel() 
{
    projectTags->setFocus();
    projectTags->selectAll();
}

void TupInfoDialog::updateInfo()
{
    emit dataSent(projectTags->text(),
                  authorName->text(),
                  descText->toPlainText());
    accept();
}
