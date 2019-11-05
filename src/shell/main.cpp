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
#include "tupmainwindow.h"
#include "tapplicationproperties.h"
#include "talgorithm.h"

#ifdef TUP_DEBUG
#ifdef Q_OS_WIN
  #include <QDebug>
#else
  #include "tdebug.h"
  #include "tupcrashhandler.h"
#endif
#endif

#ifdef ENABLE_TUPISTYLE
#include "twaitstyle.h"
#endif

#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QMessageBox>
#include <QDir>
#include <QLocale>
#include <QTranslator>
#include <QDesktopWidget>
#include <QThread>
#include <QStyleFactory>

int main(int argc, char ** argv)
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    TupApplication application(argc, argv);

#ifdef Q_OS_UNIX
#ifdef TUP_DEBUG
    TDebug::setOutputChannel();
    // Initializing the crash handler (bug catcher)
    TupCrashHandler::init();
#endif
#endif

    // Setting TupiTube Desk version values
    kAppProp->setVersion(VERSION);
    kAppProp->setRevision(REVISION);
    kAppProp->setCodeName(CODE_NAME);

    // Defining TupiTube Desk global variables
    QDir appDirPath(QApplication::applicationDirPath());
    TCONFIG->beginGroup("General");

    if (TCONFIG->value("RandomSeed", 0).toDouble() == 0.0) {
        TAlgorithm::random(); 
        TCONFIG->setValue("ClientID", TAlgorithm::randomString(20));
    }

    if (TCONFIG->firstTime()) {
        #if defined(Q_OS_MAC) 
            TCONFIG->setValue("Home", appDirPath.absolutePath());
		#else
            TCONFIG->setValue("Home", QString::fromLocal8Bit(::getenv("TUPITUBE_HOME")));
        #endif
        
        #ifdef Q_OS_WIN
            if (QSysInfo::windowsVersion() == QSysInfo::WV_XP) {
                QDir dir("C:\temp");
                if (!dir.exists()) {
                    if (!dir.mkdir("C:\temp")) {
                        #ifdef TUP_DEBUG
                            qDebug() << "main.cpp - Fatal error: WinXP issue!";
                        #endif
                        return 0;
                    }
                }
                TCONFIG->setValue("Cache", QDir::tempPath());
            } else {
                TCONFIG->setValue("Cache", QDir::tempPath());
            }
        #else
            TCONFIG->setValue("Cache", QDir::tempPath());    
        #endif
    } else {
        QString cache = TCONFIG->value("Cache").toString();
        if (cache.isEmpty())
            TCONFIG->setValue("Cache", QDir::tempPath());
        QDir dir(cache);
        if (!dir.exists())
            TCONFIG->setValue("Cache", QDir::tempPath());
    } 

#if defined(Q_OS_MAC)
    kAppProp->setHomeDir(TCONFIG->value("Home").toString());
    kAppProp->setBinDir(appDirPath.absolutePath());
    kAppProp->setPluginDir(appDirPath.absolutePath() + "/plugins");
    kAppProp->setShareDir(appDirPath.absolutePath() + "/share");
#else
    kAppProp->setHomeDir(TCONFIG->value("Home").toString());
    kAppProp->setBinDir(QString::fromLocal8Bit(::getenv("TUPITUBE_BIN")));
    kAppProp->setPluginDir(QString::fromLocal8Bit(::getenv("TUPITUBE_PLUGIN")));
    kAppProp->setShareDir(QString::fromLocal8Bit(::getenv("TUPITUBE_SHARE")));
#endif

    QString locale = "";
    QList<QString> langSupport;
    langSupport << "es" << "fr" << "pt" << "zh";
    if (TCONFIG->firstTime()) {
        locale = QString(QLocale::system().name()).left(2);
        if (locale.length() < 2) {
            locale = "en";
        } else {
            if (locale.compare("en") != 0 && !langSupport.contains(locale))
                locale = "en";
        }
        TCONFIG->beginGroup("General");
        TCONFIG->setValue("Language", locale);
    } else {
        locale = TCONFIG->value("Language", "en").toString();
    }

#ifdef Q_OS_WIN
    QString xmlDir = kAppProp->shareDir() + "xml/";
#else
    QString xmlDir = kAppProp->shareDir() + "data/xml/";
#endif
    QDir dir(xmlDir + locale + "/");
    if (!dir.exists())
        kAppProp->setDataDir(xmlDir + "en/");
    else
        kAppProp->setDataDir(xmlDir + locale + "/");

    QString themeName = TCONFIG->value("Theme").toString();
    if (themeName.length() == 0) {
        themeName = "Dark";
        TCONFIG->setValue("Theme", themeName);
    }

    if (themeName.compare("Light") == 0)
        kAppProp->setThemeDir(kAppProp->shareDir() + "themes/default/");
    else
        kAppProp->setThemeDir(kAppProp->shareDir() + "themes/dark/");

    // Setting the repository directory (where the projects are saved)
    application.createCache(TCONFIG->value("Cache").toString());

#ifdef TUP_DEBUG
    qWarning() << "main.cpp - CACHE path: " + TCONFIG->value("Cache").toString();
#endif

    QStyle *style = QStyleFactory::create("fusion");
    QApplication::setStyle(style);

    if (locale.compare("en") != 0) {
        #ifdef Q_OS_WIN
            QString langFile = kAppProp->shareDir() + "translations/tupi_" + locale + ".qm";
        #else
            QString langFile = kAppProp->shareDir() + "data/translations/tupi_" + locale + ".qm";
        #endif

        #ifdef TUP_DEBUG
            qWarning() << "main.cpp - Loading lang file -> " + langFile;
        #endif

        if (QFile::exists(langFile)) {
            // Loading localization files...
            QTranslator *translator = new QTranslator;
            translator->load(langFile);
            application.installTranslator(translator);
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "main.cpp - Error: Can't open file -> " + langFile;
            #endif
        }
    }

    TupMainWindow *mainWindow = new TupMainWindow("ideality");
    mainWindow->showMaximized();
    #if defined(Q_OS_MAC)
        application.setMainWindow(mainWindow);
    #endif

    // Looking for plugins for TupiTube Desk
    #ifdef TUP_DEBUG
        qWarning() << "main.cpp - Loading plugins from: " + kAppProp->pluginDir();
    #endif
    QApplication::addLibraryPath(kAppProp->pluginDir());

    // Loading visual components required for the Crash Handler
    #if defined(Q_OS_UNIX) && defined(TUP_DEBUG)
        CHANDLER->setConfig(DATA_DIR + "crashhandler.xml");
        CHANDLER->setImagePath(THEME_DIR + "icons/");
    #endif

    if (argc == 1) {
        bool openLast = TCONFIG->value("OpenLastProject").toBool();
        if (openLast) {
            QString files = TCONFIG->value("Recents").toString();
            QStringList recents = files.split(';');
            if (!files.isEmpty())
                mainWindow->openProject(recents.first());
        }
    } else {
        // If there is a second argument, it means to open a project from the command line
        if (argc == 2) {
            QString project = QString(argv[1]);

            #ifdef TUP_DEBUG
                qWarning() << "main.cpp - Opening project -> " + project;
            #endif

            if (project.endsWith(".tup") || project.endsWith(".TUP"))
                mainWindow->openProject(project);
        }
    }

    // It's time to play with TupiTube Desk!
    return application.exec();
}
