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

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "tglobal.h"
#include "themedocument.h"

#include <QXmlDefaultHandler>
#include <QPalette>
#include <QApplication>

// class ThemeDocument;

/**
 * @author David Cuadrado
*/
class T_GUI_EXPORT ThemeManager : public QXmlDefaultHandler
{
    public:
        ThemeManager();
        ~ThemeManager();
        
        bool applyTheme(const QString &file);
        bool applyTheme(const ThemeDocument &dd);
        
        bool startElement(const QString& , const QString& , const QString& qname, const QXmlAttributes& atts);
        
        bool endElement(const QString& ns, const QString& localname, const QString& qname);
        
        bool error(const QXmlParseException & exception);
        bool fatalError(const QXmlParseException & exception);

        bool characters(const QString & ch);
        
        QColor getColor(const QXmlAttributes& atts);
        
    private:
        QString m_root,m_qname;
        QPalette m_palette;
};

#endif