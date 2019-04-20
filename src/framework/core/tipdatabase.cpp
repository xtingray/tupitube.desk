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

#include "tipdatabase.h"

#include <QDomDocument>
#include <QFile>
#include <QList>

TipDatabase::TipDatabase(const QString &videoPath, const QString &tipPath, QWidget *parent) : QWidget(parent)
{
    loadVideos(videoPath);
    if (!videos.isEmpty())
        currentVideoIndex = TAlgorithm::random() % videos.count();

    loadTips(tipPath);
    if (!tips.isEmpty())
        currentTipIndex = TAlgorithm::random() % tips.count();
}

TipDatabase::~TipDatabase()
{
}

QString TipDatabase::video() const
{
    if (currentVideoIndex >= 0 && currentVideoIndex < videos.count())
        return videos[currentVideoIndex];

    return "";
}

QString TipDatabase::tip() const
{
    if (currentTipIndex >= 0 && currentTipIndex < tips.count())
        return tips[currentTipIndex];

    return "";
}

void TipDatabase::nextVideo()
{
    if (videos.isEmpty())
        return;

    currentVideoIndex += 1;

    if (currentVideoIndex >= (int) videos.count())
        currentVideoIndex = 0;
}

void TipDatabase::nextTip()
{
    if (tips.isEmpty())
        return;

    currentTipIndex += 1;

    if (currentTipIndex >= (int) tips.count())
        currentTipIndex = 0;
}

void TipDatabase::previousVideo()
{
    if (videos.isEmpty())
        return;

    currentVideoIndex -= 1;

    if (currentVideoIndex < 0)
        currentVideoIndex = videos.count() - 1;
}

void TipDatabase::prevTip()
{
    if (tips.isEmpty())
        return;

    currentTipIndex -= 1;

    if (currentTipIndex < 0)
        currentTipIndex = tips.count() - 1;
}

void TipDatabase::loadVideos(const QString &videoPath)
{
    QDomDocument doc;
    QFile file(videoPath);

    if (!file.exists()) {
        #ifdef TUP_DEBUG
            QString msg = "TipDatabase::loadVideos() - Fatal Error: File doesn't exist -> " + videoPath;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tWarning() << msg;
            #endif
        #endif
	    return;	
	}
	
    if (!file.open(QIODevice::ReadOnly)) {
        #ifdef TUP_DEBUG
            QString msg = "TipDatabase::loadVideos() - Fatal Error: Insufficient permissions to read file -> " + videoPath;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tWarning() << msg;
            #endif
        #endif
        return;
    }
   
    if (!doc.setContent(&file)) {
        file.close();
        #ifdef TUP_DEBUG
            QString msg = "TipDatabase::loadVideos() - Fatal Error: Can't load XML file -> " + videoPath;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return;
    }
    file.close();
   
    QDomElement element = doc.documentElement();
    QDomNode node = element.firstChild();

    while(!node.isNull()) {
        QDomElement e1 = node.toElement();
        if (!e1.isNull()) {
            if (e1.tagName() == "video") {
                QDomNode n1 = e1.firstChild();
                QString url = "";
                QString title = "";
                while(!n1.isNull()) {
                    QDomElement e2 = n1.toElement();
                    if (!e2.isNull()) {
                        if (e2.tagName() == "url") {
                            url = e2.text();
                        }
                        if (e2.tagName() == "title") {
                            title = e2.text();
                            if (title.length() > 40)
                                title = title.left(40) + "...";
                        }
                    }
                    n1 = n1.nextSibling();
                }

                QString record;
                record = "<html>\n";
                record += "<head>\n";
                record += "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html;charset=utf-8\">\n";
                record += "<link rel=\"stylesheet\" type=\"text/css\" href=\"file:tupitube.css\" />\n";
                record += "</head>\n";
                record += "<body class=\"tupi_background5\">\n";
                record += "<p><center><b>";
                record += tr("Did you already watch this video?");
                record += "</b></center></p>\n";
                record += "<p><center>";
                record += "<a href=\"https://youtube.com/watch?v=" + url + "\">";
                record += "<img src=\"file:youtube_player.png\"/>";
                record += "</a>";
                record += "</center></p>\n";
                record += "<p><center>";
                record += "<a href=\"https://youtube.com/watch?v=" + url + "\">";
                record += title;
                record += "</a>";
                record += "</center></p>\n";
                record += "\n</body>\n";
                record += "</html>";

                videos << record;
            }
        }
        node = node.nextSibling();
    }
}

void TipDatabase::loadTips(const QString &tipPath)
{
    QDomDocument doc;
    QFile file(tipPath);
    
    if (!file.open(QIODevice::ReadOnly)) {
        #ifdef TUP_DEBUG
            QString msg = "TipDatabase::loadTips() - Fatal Error: Insufficient permissions to read file -> " + tipPath;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return;
    }
    
    if (!doc.setContent(&file)) {
        file.close();
        #ifdef TUP_DEBUG
            QString msg = "TipDatabase::loadTips() - Fatal Error: Can't load XML file -> " + tipPath;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return;
    }
    file.close();
    
    QDomElement element = doc.documentElement();
    QDomNode n = element.firstChild();

    while(!n.isNull()) {
        QDomElement e = n.toElement();

        if(!e.isNull()) {
            if (e.tagName() == "tip") {
                int index = TAlgorithm::random() % 3;
                QString record;
                record = "<html>\n";
                record += "<head>\n";
                record += "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html;charset=utf-8\">\n";
                record += "<link rel=\"stylesheet\" type=\"text/css\" href=\"file:tupitube.css\" />\n";
                record += "</head>\n";
                record += "<body class=\"tip_background0" + QString::number(index) + "\">\n";
                record += e.text();
                record += "\n</body>\n";
                record += "</html>";
                tips << record;
            }
        }
        n = n.nextSibling();

    }
}

int TipDatabase::videosCount()
{
    return videos.count();
}
