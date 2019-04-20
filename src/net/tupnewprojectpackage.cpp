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

#include "tupnewprojectpackage.h"

TupNewProjectPackage::TupNewProjectPackage(const QString &pName, const QString &user, const QString &desc,
                                           const QString &bg, const QString &size, const QString &speed):
                                           QDomDocument()
{
    QDomElement root = createElement("project_new");
    root.setAttribute("version", "0");
    
    name = createTextNode(pName);
    author = createTextNode(user);
    description = createTextNode(desc);
    bgcolor = createTextNode(bg);
    dimension = createTextNode(size);
    fps = createTextNode(speed);
    
    root.appendChild(createElement("name")).appendChild(name);
    root.appendChild(createElement("author")).appendChild(author);
    root.appendChild(createElement("description")).appendChild(description);
    root.appendChild(createElement("bgcolor")).appendChild(bgcolor);
    root.appendChild(createElement("dimension")).appendChild(dimension);
    root.appendChild(createElement("fps")).appendChild(fps);

    appendChild(root);
}

TupNewProjectPackage::~TupNewProjectPackage()
{
}

void TupNewProjectPackage::setName(const QString &pName)
{
    name.setData(pName);
}

void TupNewProjectPackage::setAuthor(const QString &user)
{
    author.setData(user);
}

void TupNewProjectPackage::setDescription(const QString &desc)
{
    description.setData(desc);
}

void TupNewProjectPackage::setBgColor(const QString &bg)
{
    bgcolor.setData(bg);
}

void TupNewProjectPackage::setDimension(const QString &size)
{
    dimension.setData(size);
}

void TupNewProjectPackage::setFps(const QString &speed)
{
    fps.setData(speed);
}
