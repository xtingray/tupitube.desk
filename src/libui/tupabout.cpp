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

#include "tupabout.h"
#include "tanimwidget.h"
#include "tapplicationproperties.h"

#include <QTextBrowser>
#include <QLabel>
#include <QPixmap>
#include <QFile>
#include <QDomDocument>
#include <QTextStream>
#include <QScrollArea>
#include <QDir>

TupAbout::TupAbout(QWidget *parent) : TabDialog(Close, parent)
{
    // SQA: Check if these instructions are doing something for real
    setWindowIcon(QIcon(THEME_DIR + "icons/about.png"));
    setWindowTitle(tr("About TupiTube Desk"));
    setFixedSize(525, 458);

    QStringList path;
#ifdef Q_OS_WIN
    QString resources = SHARE_DIR + "html/";
#else
    QString resources = SHARE_DIR + "data/html/";
#endif
    path << resources + "css";
    path << resources + "images";
	
    QString lang = "en";
    if (QString(QLocale::system().name()).length() > 1)
        lang = QString(QLocale::system().name()).left(2);

    Qt::WindowFlags flags = nullptr;
    flags = Qt::Dialog;
    flags |= Qt::CustomizeWindowHint;
    setWindowFlags(flags);

    // Credits Tab

    QDomDocument doc;
    QString creditsFile = DATA_DIR + "credits.xml";
    QFile file(creditsFile);
    QString creditsText;

    if (!file.open(QIODevice::ReadOnly)) {
        #ifdef TUP_DEBUG
            qDebug() << "TupAbout::TupAbout() - Fatal Error: Can't open \"credits.xml\" file";
        #endif
        return;
    }

    if (!doc.setContent(&file)) {
        #ifdef TUP_DEBUG
            qDebug() << "TupAbout::TupAbout() - Fatal Error: File \"credits.xml\" is corrupt!";
        #endif
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = doc.documentElement();
    QDomNode n = docElem.firstChild();

    while (!n.isNull()) {
           QDomElement e = n.toElement();
           if (!e.isNull()) {
               if (e.tagName() == "credits")
                   creditsText = e.text();
           }
           n = n.nextSibling();
    }

    TAnimWidget *credits = new TAnimWidget(QPixmap(THEME_DIR + "/images/credits.png"), creditsText);
    addTab(credits, tr("About"));

    QPalette palette = credits->palette();
    palette.setColor(QPalette::Foreground, QColor(50, 50, 50, 255));
    credits->setPalette(palette);

    /*
    // Acknowledgment Tab 
    QString sponsorFile = QString() + "html/pages/thanks.html";
#ifdef Q_OS_WIN
    QString sponsorPath = SHARE_DIR + sponsorFile;
#else
    QString sponsorPath = SHARE_DIR + "data/" + sponsorFile;
#endif
    QTextBrowser *sponsorsText = new QTextBrowser;
    sponsorsText->setSearchPaths(path);
    sponsorsText->setOpenExternalLinks(true);
    sponsorsText->setSource(QUrl::fromLocalFile(sponsorPath));
    sponsorsText->moveCursor(QTextCursor::Start);

    addTab(sponsorsText, tr("Thanks"));
    */

    // TupiTube Description Tab 

    /*
    QString tupiFile = QString() + "html/pages/tupi_short.html";
#ifdef Q_OS_WIN
    QString tupiPath = SHARE_DIR + tupiFile;
#else
    QString tupiPath = SHARE_DIR + "data/" + tupiFile;
#endif
    QTextBrowser *tupiText = new QTextBrowser;
    tupiText->setSearchPaths(path);
    tupiText->setOpenExternalLinks(true);
    tupiText->setSource(QUrl::fromLocalFile(tupiPath));
    tupiText->moveCursor(QTextCursor::Start);

    addTab(tupiText, tr("About"));
    */

    // License Terms Tab
    /*
    QString licenseFile = QString() + "html/pages/philosophy.html"; 
#ifdef Q_OS_WIN
    QString licensePath = SHARE_DIR + licenseFile;
#else
    QString licensePath = SHARE_DIR + "data/" + licenseFile;
#endif
    QTextBrowser *licenseText = new QTextBrowser;
    licenseText->setSearchPaths(path);
    licenseText->setOpenExternalLinks(true);
    licenseText->setSource(QUrl::fromLocalFile(licensePath));
    licenseText->moveCursor(QTextCursor::Start);
    addTab(licenseText, tr("License Agreement"));
    setButtonText(Cancel, tr("Close"));
    */
}

TupAbout::~TupAbout()
{
}
