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

#include "tupstoryboardparser.h"

TupStoryboardParser::TupStoryboardParser(const QString &package)
{
    index = -1;
    hash = 0;
    storyboard = "";

    if (requestDoc.setContent(package)) {
        QDomElement root = requestDoc.documentElement();
        QDomNode n = root.firstChild();
        while (!n.isNull()) {
               QDomElement e = n.toElement();
               if (e.tagName() == "sceneIndex") {
                   index = e.text().toInt();
                   hash++;
               } else if (e.tagName() == "storyboard") {
                          QString input = "";
                          QTextStream text(&input);
                          text << n;
                          storyboard = input;
                          hash++;
               }
               n = n.nextSibling();
        }
    }
}

TupStoryboardParser::~TupStoryboardParser()
{
}

bool TupStoryboardParser::checksum()
{
    if (hash == 2)
        return true;

    return false;
}

int TupStoryboardParser::sceneIndex()
{
    return index;
}

QString TupStoryboardParser::storyboardXml() const
{
    return storyboard;
}

QDomDocument TupStoryboardParser::request() const
{
    return requestDoc;
}
