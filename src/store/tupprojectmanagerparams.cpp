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

#include "tupprojectmanagerparams.h"

TupProjectManagerParams::TupProjectManagerParams()
{
}

TupProjectManagerParams::~TupProjectManagerParams()
{
}

void TupProjectManagerParams::setProjectName(const QString &name)
{
    projectName = name;
}

QString TupProjectManagerParams::getProjectManager() const
{
    return projectName;
}

void TupProjectManagerParams::setAuthor(const QString &auth)
{
    author = auth;
}

QString TupProjectManagerParams::getAuthor() const
{
    return author;
}

void TupProjectManagerParams::setTags(const QString &topics)
{
    tags = topics;
}

QString TupProjectManagerParams::getTags() const
{
    return tags;
}

void TupProjectManagerParams::setBgColor(const QColor color)
{
    bgColor = color;
}

QColor TupProjectManagerParams::getBgColor()
{
    return bgColor;
}

void TupProjectManagerParams::setDescription(const QString &desc)
{
    description = desc;
}

QString TupProjectManagerParams::getDescription() const
{
    return description;
}

void TupProjectManagerParams::setDimension(const QSize &size)
{
    dimension = size;
}

QSize TupProjectManagerParams::getDimension() const
{
    return dimension;
}

void TupProjectManagerParams::setFPS(const int speed)
{
    fps = speed;
}

int TupProjectManagerParams::getFPS() const
{
    return fps;
}
