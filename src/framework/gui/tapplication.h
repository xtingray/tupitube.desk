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

#ifndef TAPPLICATION_H
#define TAPPLICATION_H

#include "tglobal.h"
#include "themedocument.h"
// #include "thememanager.h"
#include "tvhbox.h"
#include "twizard.h"
#include "tconfig.h"
#include "tactionmanager.h"

#include <QApplication>
#include <QMap>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QObject>
#include <QLocale>

class QApplication;
class QString;
class QPalette;

typedef QMap<QString, QString> ParseArgs;

class T_GUI_EXPORT TApplication : public QApplication
{
    Q_OBJECT
    public:
        enum ColorSchema { DarkBlue };
        TApplication(int & argc, char ** argv);
        ~TApplication();
        
        void parseArgs(int &argc, char **argv);        
        void applyColors(ColorSchema cs);
        void applyPalette(const QPalette &p );        
        // void applyTheme(const QString &file);
        // void applyTheme(const ThemeDocument &kd);

        bool isArg(const QString &arg);        
        QString getParam(const QString &arg);
        void changeFont(const QFont &font); // static?

        TConfig *config(const QString &group = "General");
        bool insertGlobalAction(QAction *action, const QString& id);
        void removeGlobalAction(QAction *action);

        QAction *findGlobalAction(const QString &id);
        
    public slots:
        virtual bool firstRun();
        
    private:
        ParseArgs m_parseArgs;
        // ThemeManager m_themeManager;
        TActionManager *m_actionManager;
};

#define kApp static_cast<TApplication *>(qApp)

#endif
