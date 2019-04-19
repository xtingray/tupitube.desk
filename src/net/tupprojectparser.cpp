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

#include "tupprojectparser.h"

TupProjectParser::TupProjectParser(): TupXmlParserBase()
{
    // empty
}

TupProjectParser::~TupProjectParser()
{
    // empty
}

bool TupProjectParser::startTag(const QString &tag, const QXmlAttributes &atts)
{
    Q_UNUSED(atts);

    if (root() == "server_project") {
        if (tag == "users")
            setReadText(true);
        if (tag == "data")
            setReadText(true);

        return true;
    }

    return false;
}

bool TupProjectParser::endTag(const QString &tag)
{
    Q_UNUSED(tag);
    return true;
}

void TupProjectParser::text(const QString &text)
{
    if (currentTag() == "users")
        users = text.split(",");

    if (currentTag() == "data")
        dataBox = QByteArray::fromBase64(text.toLocal8Bit());
}

QByteArray TupProjectParser::data()
{
    return dataBox;
}

QStringList TupProjectParser::partners() const
{
    return users;
}

