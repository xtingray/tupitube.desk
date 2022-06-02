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

#include "tapptheme.h"
#include "tconfig.h"
#include "tapplicationproperties.h"

QString TAppTheme::themeSettings()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TAppTheme::themeSettings()] - Loading ui.qss -> " << THEME_DIR + "config/ui.qss";
    #endif

    QString settings = "";
    TCONFIG->beginGroup("Theme");
    QString bgColor = TCONFIG->value("BgColor", "#a0a0a0").toString();

    QFile file(THEME_DIR + "config/ui.qss");
    if (file.exists()) {
        file.open(QFile::ReadOnly);
        settings = QLatin1String(file.readAll());
        if (settings.length() == 0) {
        #ifdef TUP_DEBUG 
            qWarning() << "[TAppTheme::getThemeSettings()] - Fatal Error: Theme settings input is empty!";
        #endif
        }
        file.close();
    } else {
        #ifdef TUP_DEBUG 
            qWarning() << "[TAppTheme::getThemeSettings()] - "
                          "Fatal Error: Theme file doesn't exist -> " << QString(THEME_DIR + "config/ui.qss");
        #endif
    }

    return settings.replace("BG_PARAM", bgColor);
}
