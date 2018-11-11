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

#include "tupimagedialog.h"

#include <QVBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QLocale>

TupImageDialog::TupImageDialog(QWidget *parent) : QDialog(parent)
{
    setModal(true);
    setWindowTitle(tr("Image Properties"));
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/animation_mode.png")));

    QLocale utf(QLocale::AnyLanguage, QLocale::AnyCountry);

    QLabel *titleLabel = new QLabel(tr("Title"));
    titleEdit = new QLineEdit(tr("My Picture"));
    titleEdit->setLocale(utf);

    connect(titleEdit, SIGNAL(textChanged(const QString &)), this, SLOT(resetTitleColor(const QString &)));
    titleLabel->setBuddy(titleEdit);

    QLabel *topicLabel = new QLabel(tr("Topics"));
    topicEdit = new QLineEdit(tr("#topic1 #topic2 #topic3"));
    topicEdit->setLocale(utf);
    connect(topicEdit, SIGNAL(textChanged(const QString &)), this, SLOT(resetTopicColor(const QString &)));
    topicLabel->setBuddy(topicEdit);

    QLabel *descLabel = new QLabel(tr("Description"));

    descText = new QTextEdit;
    descText->setLocale(utf);
    descText->setAcceptRichText(false);
    descText->setFixedHeight(80);
    descText->setText(tr("Just a little taste of my style :)"));

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(titleLabel);
    topLayout->addWidget(titleEdit);

    QHBoxLayout *middleLayout = new QHBoxLayout;
    middleLayout->addWidget(topicLabel);
    middleLayout->addWidget(topicEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);

    QPushButton *cancel = new QPushButton(tr("Cancel"));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    buttonLayout->addWidget(cancel);

    QPushButton *ok = new QPushButton(tr("Post Image"));
    connect(ok, SIGNAL(clicked()), this, SLOT(checkData()));
    buttonLayout->addWidget(ok);
    ok->setDefault(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addLayout(topLayout);
    layout->addLayout(middleLayout);
    layout->addWidget(descLabel);
    layout->addWidget(descText);
    layout->addLayout(buttonLayout);

    setLayout(layout);
}

TupImageDialog::~TupImageDialog()
{
}

void TupImageDialog::checkData()
{
    if (titleEdit->text().length() == 0) {
        titleEdit->setText(tr("Set a title for the picture here!"));
        titleEdit->selectAll();
        return;
    }

    if (topicEdit->text().length() == 0) {
        topicEdit->setText(tr("Set a title for the picture here!"));
        topicEdit->selectAll();
        return;
    }

    QDialog::accept();
}

void TupImageDialog::resetTitleColor(const QString &)
{
    QPalette palette = titleEdit->palette();
    if (titleEdit->text().length() > 0 && titleEdit->text().compare(tr("Set a title for the picture here!")) != 0)
        palette.setBrush(QPalette::Base, Qt::white);
    else 
        palette.setBrush(QPalette::Base, QColor(255, 140, 138));

    titleEdit->setPalette(palette);
}

void TupImageDialog::resetTopicColor(const QString &)
{
    QPalette palette = topicEdit->palette();
    if (topicEdit->text().length() > 0 && topicEdit->text().compare(tr("Set some topic tags for the picture here!")) != 0)
        palette.setBrush(QPalette::Base, Qt::white);
    else
        palette.setBrush(QPalette::Base, QColor(255, 140, 138));

    topicEdit->setPalette(palette);
}

QString TupImageDialog::imageTitle() const
{
     return QString::fromUtf8(titleEdit->text().toUtf8());
}

QString TupImageDialog::imageTopics() const
{
     return QString::fromUtf8(topicEdit->text().toUtf8());
}

QString TupImageDialog::imageDescription() const
{
     return QString::fromUtf8(descText->toPlainText().toUtf8());
}
