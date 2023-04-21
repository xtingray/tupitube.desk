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

#include "tupnewsdialog.h"
#include "tseparator.h"

#include <QBoxLayout>
#include <QTabWidget>
#include <QPushButton>
#include <QIcon>

TupNewsDialog::TupNewsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("TupiTube Updates"));
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/updates.png")));

    QVBoxLayout *layout = new QVBoxLayout(this);
    // layout->setMargin(15);

    releasePage = new QTextBrowser;
    releaseDocument = new QTextDocument(releasePage);
    releasePage->setDocument(releaseDocument);

    newsPage = new QTextBrowser;
    newsDocument = new QTextDocument(newsPage);
    newsPage->setDocument(newsDocument);

    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->addTab(releasePage, tr("Updates"));
    tabWidget->addTab(newsPage, tr("News"));

    QPushButton *closeButton = new QPushButton(this);
    closeButton->setIcon(QIcon(THEME_DIR + "icons/close.png"));
    closeButton->setToolTip(tr("Close"));
    closeButton->setMinimumWidth(60);

    layout->addWidget(closeButton);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
    setAttribute(Qt::WA_DeleteOnClose, true);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(closeButton, 1, Qt::AlignRight);

    layout->addWidget(tabWidget);
    layout->addWidget(new TSeparator);
    layout->addLayout(buttonLayout);
    resize(700, 405);
}

TupNewsDialog::~TupNewsDialog()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupNewsDialog()]";
    #endif
}

void TupNewsDialog::setSource(const QString &releasePath, const QString &newsPath)
{
    QStringList path;

    #ifdef Q_OS_WIN
        QString resources = SHARE_DIR + "html/";
    #else
        QString resources = SHARE_DIR + "data/html/";
    #endif

    path << resources + "css";
    path << resources + "images";
    releasePage->setSearchPaths(path);
    releasePage->setOpenExternalLinks(true);
    releasePage->setSource(QUrl::fromLocalFile(releasePath));

    newsPage->setSearchPaths(path);
    newsPage->setOpenExternalLinks(true);
    newsPage->setSource(QUrl::fromLocalFile(newsPath));
}
