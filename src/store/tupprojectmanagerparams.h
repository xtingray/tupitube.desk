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

#ifndef TUPPROJECTMANAGERPARAMS_H
#define TUPPROJECTMANAGERPARAMS_H

#include "tglobal.h"

#include <QString>
#include <QSize>
#include <QColor>

class TUPITUBE_EXPORT TupProjectManagerParams
{
    public:
        TupProjectManagerParams();
        virtual ~TupProjectManagerParams();

        void setProjectName(const QString &name);
        QString getProjectManager() const;

        void setAuthor(const QString &getAuthor);
        QString getAuthor() const;

        // void setTags(const QString &getTags);
        // QString getTags() const;

        void setDescription(const QString &getDescription);
        QString getDescription() const;

        void setBgColor(const QColor color);
        QColor getBgColor();

        void setDimension(const QSize &getDimension);
        QSize getDimension() const;

        void setFPS(const int getFPS);
        int getFPS() const;

    private:
        QString projectName;
        QString author;
        // QString tags;
        QColor bgColor;
        QString description;
        QSize dimension;
        int fps;
};

#endif
