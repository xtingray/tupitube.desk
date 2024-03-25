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
#include "tuivalues.h"

#ifdef TUP_DEBUG
  #include <QDebug>
  #ifdef Q_OS_UNIX
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
    // Initializing the crash handler (bug catcher)
    TupCrashHandler::init();
#endif
#endif

    // Setting TupiTube Desk version values
    kAppProp->setVersion(TUPITUBE_VERSION);
    kAppProp->setRevision(REVISION);
    kAppProp->setCodeName(CODE_NAME);

    // Defining TupiTube Desk global variables
    #if defined(Q_OS_MAC)
    QDir appDirPath(QApplication::applicationDirPath());
    #endif
    TCONFIG->beginGroup("General");

    if (TCONFIG->value("RandomSeed", 0).toDouble() == 0.0) {
        TAlgorithm::random(); 
        TCONFIG->setValue("ClientID", TAlgorithm::randomString(20));
    }

    QString cachePath = "";
    if (TCONFIG->firstTime()) {
        #if defined(Q_OS_MAC) 
            TCONFIG->setValue("Home", appDirPath.absolutePath());
		#else
            TCONFIG->setValue("Home", QString::fromLocal8Bit(::getenv("TUPITUBE_HOME")));
        #endif

        cachePath = QDir::tempPath();
        TCONFIG->setValue("Cache", cachePath);        
    } else {
        cachePath = TCONFIG->value("Cache").toString();
        if (cachePath.isEmpty()) {
            cachePath = QDir::tempPath();
            TCONFIG->setValue("Cache", cachePath);
        }

        QDir dir(cachePath);
        if (!dir.exists()) {
            cachePath = QDir::tempPath();
            TCONFIG->setValue("Cache", cachePath);
        }
    } 

#if defined(Q_OS_MAC)
    kAppProp->setHomeDir(TCONFIG->value("Home").toString());
    kAppProp->setBinDir(appDirPath.absolutePath());
    kAppProp->setPluginDir(appDirPath.absolutePath() + "/plugins");
    kAppProp->setShareDir(appDirPath.absolutePath() + "/share");
#else
    kAppProp->setHomeDir(TCONFIG->value("Home").toString());
    QString binPath = QString::fromLocal8Bit(::getenv("TUPITUBE_BIN"));
    QString pluginPath = QString::fromLocal8Bit(::getenv("TUPITUBE_PLUGIN"));
    QString sharePath = QString::fromLocal8Bit(::getenv("TUPITUBE_SHARE"));

    kAppProp->setBinDir(binPath);
    kAppProp->setPluginDir(pluginPath);
    kAppProp->setShareDir(sharePath);
#endif

    QString locale = "";
    QList<QString> langSupport = TCONFIG->languages();
    if (TCONFIG->firstTime()) {
        locale = QString(QLocale::system().name()).left(2);
        if (locale.length() < 2) {
            locale = "en";
        } else if (locale.compare("en") != 0 && !langSupport.contains(locale)) {
            locale = "en";
        }
        TCONFIG->beginGroup("General");
        TCONFIG->setValue("Language", locale);
    } else {
        locale = TCONFIG->value("Language", "en").toString();
        if (locale.compare("en") != 0 && !langSupport.contains(locale))
            locale = "en";
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

    QString themePath = kAppProp->shareDir() + "themes/default/";

    kAppProp->setThemeDir(themePath);
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    if (screenWidth > HD_WIDTH) // Big resolutions
        kAppProp->setIconsDir(themePath + "icons/hd/");
    else
        kAppProp->setIconsDir(themePath + "icons/");

    kAppProp->setCursorsDir(themePath + "cursors/");
    kAppProp->setRasterResourcesDir(kAppProp->shareDir() + "themes/raster/");

    // Setting the repository directory (where the projects are saved)
    application.createCache(cachePath);

    QStyle *style = QStyleFactory::create("fusion");
    QApplication::setStyle(style);

    QString langFile = "";
    if (locale.compare("en") != 0) {
        #ifdef Q_OS_WIN
            langFile = kAppProp->shareDir() + "translations/tupi_" + locale + ".qm";
        #else
            langFile = kAppProp->shareDir() + "data/translations/tupi_" + locale + ".qm";
        #endif

        if (QFile::exists(langFile)) {
            // Loading localization files...
            QTranslator *translator = new QTranslator;
            translator->load(langFile);
            application.installTranslator(translator);
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[main.cpp] - Error: Can't open file ->" << langFile;
            #endif
        }
    }

    #ifdef TUP_DEBUG
        qWarning() << "---";
        qWarning() << "  [main.cpp] - CACHE path ->" << cachePath;
        qWarning() << "  [main.cpp] - TUPITUBE_BIN ->" << QString::fromLocal8Bit(::getenv("TUPITUBE_BIN"));
        qWarning() << "  [main.cpp] - TUPITUBE_PLUGIN ->" << QString::fromLocal8Bit(::getenv("TUPITUBE_PLUGIN"));
        qWarning() << "  [main.cpp] - TUPITUBE_SHARE ->" << QString::fromLocal8Bit(::getenv("TUPITUBE_SHARE"));
        qWarning() << "  [main.cpp] - themePath ->" <<  themePath;
        qWarning() << "  [main.cpp] - locale ->" << locale;
        qWarning() << "  [main.cpp] - lang file ->" << langFile;
        qWarning() << "  [main.cpp] - plugins path ->" << kAppProp->pluginDir();
        qWarning() << "---";
    #endif

    QString sourceFile = "";
    if (argc == 2) {
        sourceFile = QString(argv[1]);
    } else {
        if (argc == 1) {
            bool openLast = TCONFIG->value("OpenLastProject").toBool();
            if (openLast) {
                QString files = TCONFIG->value("Recents").toString();
                if (!files.isEmpty()) {
                    QStringList recents = files.split(';');
                    sourceFile = recents.first();
                }
            }
        }
    }

    TupMainWindow *mainWindow = new TupMainWindow("ideality", sourceFile);
    mainWindow->showMaximized();
    #if defined(Q_OS_MAC)
        application.setMainWindow(mainWindow);
    #endif

    // Looking for plugins for TupiTube Desk
    QApplication::addLibraryPath(kAppProp->pluginDir());

    // Loading visual components required for the Crash Handler
    #if defined(Q_OS_UNIX) && defined(TUP_DEBUG)
        CHANDLER->setConfig(DATA_DIR + "crashhandler.xml");
        CHANDLER->setImagePath(THEME_DIR + "icons/");
    #endif

    /*
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
                qWarning() << "[main.cpp] - Opening project -> " << project;
            #endif

            if (project.endsWith(".tup") || project.endsWith(".TUP"))
                mainWindow->openProject(project);
        }
    } */

    // It's time to play with TupiTube Desk!
    return application.exec();
}
