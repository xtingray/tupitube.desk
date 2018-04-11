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

#include "tupapplication.h"
#include "tapplicationproperties.h"

TupApplication::TupApplication(int &argc, char **argv) : TApplication(argc, argv)
{
    setApplicationName("tupitube");
}

TupApplication::~TupApplication()
{
    #ifdef TUP_DEBUG
        QString msg = "[Destroying ~TupApplication]";
        #ifdef Q_OS_WIN
           qDebug() << msg;
        #else
           tDebug() << msg;
        #endif
    #endif
}

void TupApplication::createCache(const QString &cacheDir)
{
    QDir cache(cacheDir);
    if (!cache.exists()) {
        #ifdef TUP_DEBUG
           QString msg = "Initializing repository: " + cacheDir;
           #ifdef Q_OS_WIN
               qWarning() << msg;
           #else
               tWarning() << msg;
           #endif
        #endif

        if (!cache.mkdir(cacheDir)) {
            #ifdef TUP_DEBUG
                QString msg = "TupApplication::createCache() - Fatal Error: Can't create project repository";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif
        }
    }

    kAppProp->setCacheDir(cacheDir);
}

#if defined(Q_OS_MAC)
void TupApplication::setMainWindow(QMainWindow *mw) 
{
    mainWindow = mw;
}

bool TupApplication::event(QEvent *event)
{
    if (event->type() == QEvent::FileOpen) {
        QApplication::sendEvent(mainWindow, event);
        return false;
    }
    
    return QApplication::event(event);
}
#endif
